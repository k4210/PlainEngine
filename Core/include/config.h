#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

#include "stringId.h"

namespace PlainEngine
{
    using ConfigValue = std::variant<int, float>;
}

namespace PlainEngine
{
    struct Config
    {
        struct Record
        {
            StringId name;
            ConfigValue value;
        };

        struct Section
        {
            const Record* findRecord(StringId recordName) const;

        private:
            friend struct Config;

            StringId name;
            std::vector<Record> records;
        };

        struct FileData
        {
            const Section* findSection(StringId sectionName) const;

        private:
            friend struct Config;

            StringId name;
            std::vector<Section> sections;
        };

        static Config Read(std::string_view relativeFolderPath);

        std::optional<ConfigValue> find(
            StringId filename,
            StringId section,
            StringId key) const;

        void forEachSection(
            StringId file,
            std::function<void(StringId, const Section&)> func) const;

    private:
        static void ParseConfigFile(FileData& file, std::string_view content);

        std::array<FileData, 16> _files{};
        std::size_t _fileCount = 0;
    };
}
