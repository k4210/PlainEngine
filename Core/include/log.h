#pragma once

#ifndef LOG_ENABLED
#define LOG_ENABLED 1
#endif

#if LOG_ENABLED

#include <array>
#include <cstddef>
#include <format>
#include <intrin.h>
#include <utility>

#include <stringId.h>

namespace PlainEngine::Core::detail
{
    inline constexpr std::size_t kLogBufferSize = 2048;
    inline constexpr char kLogTruncationMarker[] = "...";

    enum class LogPriority
    {
        log,
        info,
        warning,
        error,
        breakExecution
    };

    std::size_t AppendLogPrefix(
        StringId channel,
        LogPriority priority,
        std::array<char, kLogBufferSize>& buffer);

    std::size_t AppendStackTrace(std::array<char, kLogBufferSize>& buffer, std::size_t length);

    void WriteLogMessage(LogPriority priority, const char* message, std::size_t length);

    template <typename... Args>
    void FormatLogMessage(
        StringId channel,
        LogPriority priority,
        std::format_string<Args...> format,
        Args&&... args)
    {
        std::array<char, kLogBufferSize> buffer{};
        std::size_t length = AppendLogPrefix(channel, priority, buffer);
        const std::size_t kCapacity = buffer.size() - length - 2;
        const auto result = std::format_to_n(
            buffer.data() + length, kCapacity, format,
            std::forward<Args>(args)...);
        const std::size_t formattedLength = static_cast<std::size_t>(result.size);
        std::size_t messageLength = formattedLength < kCapacity ? formattedLength : kCapacity;

        if (formattedLength > kCapacity)
        {
            constexpr std::size_t kMarkerSize = sizeof(kLogTruncationMarker) - 1;
            for (std::size_t index = 0; index < kMarkerSize; ++index)
            {
                buffer[length + kCapacity - kMarkerSize + index] =
                    kLogTruncationMarker[index];
            }
            messageLength = kCapacity;
        }

        buffer[length + messageLength] = '\n';
        length += messageLength + 1;
        if (priority == LogPriority::breakExecution)
        {
            length = AppendStackTrace(buffer, length);
        }
        WriteLogMessage(priority, buffer.data(), length);
    }
}

template <typename... Args>
void Log(PlainEngine::Core::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    PlainEngine::Core::detail::FormatLogMessage(
        channel, PlainEngine::Core::detail::LogPriority::log, format,
        std::forward<Args>(args)...);
}

template <typename... Args>
void Log(std::format_string<Args...> format, Args&&... args)
{
    Log(PlainEngine::Core::StringId{}, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Info(PlainEngine::Core::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    PlainEngine::Core::detail::FormatLogMessage(
        channel, PlainEngine::Core::detail::LogPriority::info, format,
        std::forward<Args>(args)...);
}

template <typename... Args>
void Info(std::format_string<Args...> format, Args&&... args)
{
    Info(PlainEngine::Core::StringId{}, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Warning(PlainEngine::Core::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    PlainEngine::Core::detail::FormatLogMessage(
        channel, PlainEngine::Core::detail::LogPriority::warning, format,
        std::forward<Args>(args)...);
}

template <typename... Args>
void Warning(std::format_string<Args...> format, Args&&... args)
{
    Warning(PlainEngine::Core::StringId{}, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Error(PlainEngine::Core::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    PlainEngine::Core::detail::FormatLogMessage(
        channel, PlainEngine::Core::detail::LogPriority::error, format,
        std::forward<Args>(args)...);
}

template <typename... Args>
void Error(std::format_string<Args...> format, Args&&... args)
{
    Error(PlainEngine::Core::StringId{}, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Break(PlainEngine::Core::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    PlainEngine::Core::detail::FormatLogMessage(
        channel, PlainEngine::Core::detail::LogPriority::breakExecution, format,
        std::forward<Args>(args)...);
    __debugbreak();
}

template <typename... Args>
void Break(std::format_string<Args...> format, Args&&... args)
{
    Break(PlainEngine::Core::StringId{}, format, std::forward<Args>(args)...);
}

#else

#define Log(...)
#define Info(...)
#define Warning(...)
#define Error(...)
#define Break(...)

#endif
