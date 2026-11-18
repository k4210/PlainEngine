#pragma once

#include <array>
#include <cassert>
#include <compare>
#include <cstddef>
#include <cstdlib>
#include <format>
#include <string_view>

namespace PlainEngine
{
    struct StringId
    {
        static constexpr std::size_t kBufferSize = 16;
        static constexpr std::size_t kMaxLength = kBufferSize - 1;

        constexpr StringId()
            : _characters{}
        {
        }

        template <std::size_t N>
        consteval explicit StringId(const char (&value)[N])
            : _characters{}
        {
            static_assert(N <= kBufferSize, "StringId literal exceeds 15 characters");
            for (std::size_t index = 0;
                index < N - 1 && value[index] != '\0';
                ++index)
            {
                _characters[index] = value[index];
            }
        }

        constexpr explicit StringId(const char* value)
            : _characters{}
        {
            if (value == nullptr)
            {
                return;
            }

            std::size_t length = 0;
            while (length < kMaxLength && value[length] != '\0')
            {
                _characters[length] = value[length];
                ++length;
            }

            assert(value[length] == '\0' && "StringId input exceeds 15 characters");
        }

        constexpr std::string_view view() const
        {
            return std::string_view{_characters.data()};
        }

        constexpr explicit operator bool() const
        {
            return !view().empty();
        }

        friend constexpr auto operator<=>(const StringId&, const StringId&) = default;

    private:
        std::array<char, kBufferSize> _characters;
    };
}

consteval PlainEngine::StringId operator""_id(
    const char* value,
    std::size_t length)
{
    if (length > PlainEngine::StringId::kMaxLength)
    {
        std::abort();
    }
    return PlainEngine::StringId{value};
}

namespace std
{
    template <>
    struct formatter<PlainEngine::StringId, char>
        : formatter<string_view, char>
    {
        template <typename FormatContext>
        auto format(
            const PlainEngine::StringId& value,
            FormatContext& context) const
        {
            return formatter<string_view, char>::format(
                value.view(), context);
        }
    };
}
