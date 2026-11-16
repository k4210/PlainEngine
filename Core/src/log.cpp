#include <log.h>

#if LOG_ENABLED

#include <windows.h>
#include <dbghelp.h>

#pragma comment(lib, "Dbghelp.lib")

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <format>
#include <mutex>
#include <string_view>
#include <utility>

namespace PlainEngine::Core::detail
{
    namespace
    {
        void WriteOutput(HANDLE handle, bool isConsole, const char* output, std::size_t length)
        {
            std::size_t offset = 0;
            while (offset < length)
            {
                const DWORD remaining = static_cast<DWORD>(length - offset);
                DWORD written = 0;
                const BOOL succeeded = isConsole
                    ? WriteConsoleA(handle, output + offset, remaining, &written, nullptr)
                    : WriteFile(handle, output + offset, remaining, &written, nullptr);
                if (!succeeded || written == 0)
                {
                    OutputDebugStringA(output);
                    return;
                }

                offset += written;
            }
        }

        void WriteToConsole(const char* message, std::size_t length, WORD color, bool forceFlush)
        {
            static std::mutex logMutex;
            const std::lock_guard<std::mutex> lock{logMutex};
            const HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            CONSOLE_SCREEN_BUFFER_INFO consoleInfo{};
            // Redirected stdout still receives logs through WriteFile.
            const bool hasConsole = outputHandle != nullptr &&
                outputHandle != INVALID_HANDLE_VALUE &&
                GetConsoleScreenBufferInfo(outputHandle, &consoleInfo);
            const bool colorSet = hasConsole &&
                SetConsoleTextAttribute(outputHandle, color) != 0;

            WriteOutput(outputHandle, hasConsole, message, length);

            if (colorSet)
            {
                SetConsoleTextAttribute(outputHandle, consoleInfo.wAttributes);
            }

            if (forceFlush && !hasConsole &&
                outputHandle != nullptr &&
                outputHandle != INVALID_HANDLE_VALUE)
            {
                FlushFileBuffers(outputHandle);
            }
        }
    }

    std::size_t AppendStackTrace(std::array<char, kLogBufferSize>& output, std::size_t outputLength)
    {
        std::array<void*, 32> frames{};
        const USHORT frameCount = CaptureStackBackTrace(
            2, static_cast<DWORD>(frames.size()), frames.data(), nullptr);
        const HANDLE process = GetCurrentProcess();
        static const bool symbolsInitialized = []()
        {
            SymSetOptions(SymGetOptions() | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
            return SymInitialize(GetCurrentProcess(), nullptr, TRUE) != FALSE;
        }();

        const auto appendText = [&output, &outputLength](std::string_view text)
        {
            constexpr std::string_view kTruncationMarker{kLogTruncationMarker};
            const std::size_t available = output.size() - outputLength - 1;
            if (text.size() > available)
            {
                if (available >= kTruncationMarker.size())
                {
                    const std::size_t copiedLength = available - kTruncationMarker.size();
                    for (std::size_t index = 0; index < copiedLength; ++index)
                    {
                        output[outputLength++] = text[index];
                    }
                }
                else
                {
                    outputLength = output.size() - kTruncationMarker.size() - 1;
                }

                for (const char character : kTruncationMarker)
                {
                    output[outputLength++] = character;
                }
                return false;
            }

            for (const char character : text)
            {
                output[outputLength++] = character;
            }
            return true;
        };

        for (std::size_t index = 0; index < frameCount; ++index)
        {
            constexpr std::size_t kMaxSymbolNameLength = 256;
            alignas(SYMBOL_INFO) std::array<char,
                sizeof(SYMBOL_INFO) + kMaxSymbolNameLength> symbolBuffer{};
            auto* symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer.data());
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = static_cast<ULONG>(kMaxSymbolNameLength);

            DWORD64 displacement = 0;
            const DWORD64 address = reinterpret_cast<DWORD64>(frames[index]);
            const bool hasSymbol = symbolsInitialized &&
                SymFromAddr(process, address, &displacement, symbol) != FALSE;

            std::array<char, 320> line{};
            const auto result = hasSymbol
                ? std::format_to_n(
                    line.data(), line.size() - 1, "  #{} {}+0x{:X}\n",
                    index, symbol->Name, displacement)
                : std::format_to_n(
                    line.data(), line.size() - 1, "  #{} 0x{:X}\n",
                    index, address);
            const std::size_t formattedLength = static_cast<std::size_t>(result.size);
            const std::size_t length = formattedLength < line.size() - 1
                ? formattedLength
                : line.size() - 1;
            if (!appendText(std::string_view{line.data(), length}))
            {
                break;
            }
        }
        return outputLength;
    }

    std::size_t AppendLogPrefix(
        StringId channel,
        LogPriority priority,
        std::array<char, kLogBufferSize>& buffer)
    {
        const char* priorityName = "Log";
        switch (priority)
        {
        case LogPriority::log:
            break;
        case LogPriority::info:
            priorityName = "Info";
            break;
        case LogPriority::warning:
            priorityName = "Warning";
            break;
        case LogPriority::error:
            priorityName = "Error";
            break;
        case LogPriority::breakExecution:
            priorityName = "Break";
            break;
        }

        std::size_t length = 0;
        const auto appendText = [&buffer, &length](
            std::string_view text, char before, char after)
        {
            const std::size_t decorationSize =
                (before != '\0') + (after != '\0');
            assert(length + text.size() + decorationSize < buffer.size());

            if (before != '\0')
            {
                buffer[length++] = before;
            }
            for (const char character : text)
            {
                buffer[length++] = character;
            }
            if (after != '\0')
            {
                buffer[length++] = after;
            }
        };

        appendText(priorityName, '[', ']');
        if (channel)
        {
            appendText(channel.view(), '[', ']');
        }
        buffer[length++] = ' ';
        return length;
    }

    void WriteLogMessage(
        LogPriority priority,
        const char* message,
        std::size_t length)
    {
        const WORD color = [priority]
        {
            WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            switch (priority)
            {
            case LogPriority::log:
                break;
            case LogPriority::info:
                break;
            case LogPriority::warning:
                color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                break;
            case LogPriority::error:
            case LogPriority::breakExecution:
                color = FOREGROUND_RED | FOREGROUND_INTENSITY;
                break;
            }
            return color;
        }();

        WriteToConsole(message, length, color, priority == LogPriority::breakExecution);
    }
}

#endif
