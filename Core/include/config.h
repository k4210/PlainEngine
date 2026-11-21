#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>
#include <variant>

#include "stringId.h"
#include "hash.h"

namespace PlainEngine
{
    using ConfigValue = std::variant<int, float>;

    struct ConfigKey
    {
        StringId file;
        StringId section;
        StringId key;

        friend bool operator==(const ConfigKey&, const ConfigKey&) = default;
    };
}

namespace std
{
    template <>
    struct hash<PlainEngine::ConfigKey>
    {
        size_t operator()(
            const PlainEngine::ConfigKey& key) const
        {
            size_t seed = 0;
            PlainEngine::hash_combine(seed, key.file.view());
            PlainEngine::hash_combine(seed, key.section.view());
            PlainEngine::hash_combine(seed, key.key.view());
            return seed;
        }
    };
}

namespace PlainEngine
{
    struct Config
    {
        static Config Read(std::string_view relativeFolderPath);

        std::optional<ConfigValue> find(
            StringId filename,
            StringId section,
            StringId key) const;

    private:
        std::unordered_map<ConfigKey, ConfigValue> _records;
    };
}
