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

#include "stringId.h"

namespace PlainEngine
{
    struct Config;

    inline constexpr std::size_t kLogBufferSize = 2048;

	enum class LogPriority : uint8_t
    {
        log = 0,
        info = 1,
        warning = 2,
        error = 4,
        breakExecution = 5
    };

    std::size_t AppendLogPrefix(
        StringId channel,
        LogPriority priority,
        std::array<char, kLogBufferSize>& buffer);

    std::size_t AppendStackTrace(std::array<char, kLogBufferSize>& buffer, std::size_t length);

    std::size_t ApplyTruncationMarker(
        std::array<char, kLogBufferSize>& buffer,
        std::size_t startOffset,
        std::size_t available);

    template <typename... Args>
    std::size_t AppendArgs(
        std::array<char, kLogBufferSize>& buffer,
        std::size_t startOffset,
        std::size_t requiredFreeSpace,
        std::format_string<Args...> format,
        Args&&... args)
    {
        if (requiredFreeSpace >= kLogBufferSize)
        {
            return startOffset;
        }

        const std::size_t usableLength = kLogBufferSize - requiredFreeSpace;
        if (startOffset >= usableLength)
        {
            return startOffset;
        }

        const std::size_t available = usableLength - startOffset;
        const auto result = std::format_to_n(
            buffer.data() + startOffset,
            available,
            format,
            std::forward<Args>(args)...);
        const std::size_t formattedLength = static_cast<std::size_t>(result.size);

        if (formattedLength > available)
        {
            return ApplyTruncationMarker(buffer, startOffset, available);
        }

        return startOffset + formattedLength;
    }

    void WriteLogMessage(LogPriority priority, const char* message, std::size_t length);

    struct LogSettings
    {
        static void SetMinimalPriorityUnsafe(StringId channel, LogPriority priority);
        static LogPriority MinimalPriority(StringId channel);
    };

    template <typename... Args>
    void FormatLogMessage(
        StringId channel,
        LogPriority priority,
        std::format_string<Args...> format,
        Args&&... args)
    {
        if (priority < LogSettings::MinimalPriority(channel))
            return;

        std::array<char, kLogBufferSize> buffer{};
        const std::size_t prefixLength = AppendLogPrefix(channel, priority, buffer);
        std::size_t length = AppendArgs(
            buffer,
            prefixLength,
            2,
            format,
            std::forward<Args>(args)...);
        buffer[length++] = '\n';

        if (priority == LogPriority::breakExecution)
        {
            length = AppendStackTrace(buffer, length);
        }
        WriteLogMessage(priority, buffer.data(), length);
    }
}

template <typename... Args>
void Log(PlainEngine::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    PlainEngine::FormatLogMessage(
        channel, PlainEngine::LogPriority::log, format,
        std::forward<Args>(args)...);
}

template <typename... Args>
void Log(std::format_string<Args...> format, Args&&... args)
{
    Log(PlainEngine::StringId{}, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Info(PlainEngine::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    PlainEngine::FormatLogMessage(
        channel, PlainEngine::LogPriority::info, format,
        std::forward<Args>(args)...);
}

template <typename... Args>
void Info(std::format_string<Args...> format, Args&&... args)
{
    Info(PlainEngine::StringId{}, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Warning(PlainEngine::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    PlainEngine::FormatLogMessage(
        channel, PlainEngine::LogPriority::warning, format,
        std::forward<Args>(args)...);
}

template <typename... Args>
void Warning(std::format_string<Args...> format, Args&&... args)
{
    Warning(PlainEngine::StringId{}, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Error(PlainEngine::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    PlainEngine::FormatLogMessage(
        channel, PlainEngine::LogPriority::error, format,
        std::forward<Args>(args)...);
}

template <typename... Args>
void Error(std::format_string<Args...> format, Args&&... args)
{
    Error(PlainEngine::StringId{}, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Break(PlainEngine::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    PlainEngine::FormatLogMessage(
        channel, PlainEngine::LogPriority::breakExecution, format,
        std::forward<Args>(args)...);
    __debugbreak();
}

template <typename... Args>
void Break(std::format_string<Args...> format, Args&&... args)
{
    Break(PlainEngine::StringId{}, format, std::forward<Args>(args)...);
}

template <typename... Args>
void CLog(bool condition, PlainEngine::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    if (!condition)
        return;
    Log(channel, format, std::forward<Args>(args)...);
}

template <typename... Args>
void CLog(bool condition, std::format_string<Args...> format, Args&&... args)
{
    if (!condition)
        return;
    Log(format, std::forward<Args>(args)...);
}

template <typename... Args>
void CInfo(bool condition, PlainEngine::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    if (!condition)
        return;
    Info(channel, format, std::forward<Args>(args)...);
}

template <typename... Args>
void CInfo(bool condition, std::format_string<Args...> format, Args&&... args)
{
    if (!condition)
        return;
    Info(format, std::forward<Args>(args)...);
}

template <typename... Args>
void CWarning(bool condition, PlainEngine::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    if (!condition)
        return;
    Warning(channel, format, std::forward<Args>(args)...);
}

template <typename... Args>
void CWarning(bool condition, std::format_string<Args...> format, Args&&... args)
{
    if (!condition)
        return;
    Warning(format, std::forward<Args>(args)...);
}

template <typename... Args>
void CError(bool condition, PlainEngine::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    if (!condition)
        return;
    Error(channel, format, std::forward<Args>(args)...);
}

template <typename... Args>
void CError(bool condition, std::format_string<Args...> format, Args&&... args)
{
    if (!condition)
        return;
    Error(format, std::forward<Args>(args)...);
}

template <typename... Args>
void CBreak(bool condition, PlainEngine::StringId channel, std::format_string<Args...> format, Args&&... args)
{
    if (!condition)
        return;
    Break(channel, format, std::forward<Args>(args)...);
}

template <typename... Args>
void CBreak(bool condition, std::format_string<Args...> format, Args&&... args)
{
    if (!condition)
        return;
    Break(format, std::forward<Args>(args)...);
}

#else

#define Log(...) do { } while (0)
#define Info(...)  do { } while (0)
#define Warning(...)  do { } while (0)
#define Error(...)  do { } while (0)
#define Break(...)  do { } while (0)
#define CLog(...) do { } while (0)
#define CInfo(...) do { } while (0)
#define CWarning(...) do { } while (0)
#define CError(...) do { } while (0)
#define CBreak(...) do { } while (0)

#endif

#define Assert(expr) CBreak(!(expr), "{}", #expr)
