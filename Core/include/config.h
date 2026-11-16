#pragma once

#include <linearMap.h>

#include <optional>
#include <string_view>
#include <variant>

#include <stringId.h>

namespace PlainEngine::Core::Config
{
    using AnyValue = std::variant<int, float>;

    struct Config final
    {
        using Sections = LinearMap<
            StringId,
            LinearMap<StringId, AnyValue>>;

        static Config Read(std::string_view relativeFolderPath);

        std::optional<AnyValue> find(
            StringId filename,
            StringId section,
            StringId key) const;

    private:
        LinearMap<StringId, Sections> _files;
    };
}
