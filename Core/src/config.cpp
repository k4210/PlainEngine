#include "config.h"

#include "file.h"
#include "log.h"

#include <charconv>
#include <cctype>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace PlainEngine
{
    namespace
    {
        bool IsWhitespace(char value)
        {
            return std::isspace(static_cast<unsigned char>(value)) != 0;
        }

        std::string_view Trim(std::string_view value)
        {
            while (!value.empty() && IsWhitespace(value.front()))
            {
                value.remove_prefix(1);
            }
            while (!value.empty() && IsWhitespace(value.back()))
            {
                value.remove_suffix(1);
            }
            return value;
        }

        bool MakeStringId(std::string_view value, StringId& result)
        {
            if (value.size() > StringId::kMaxLength)
            {
                return false;
            }

            const std::string ownedValue{value};
            result = StringId{ownedValue.c_str()};
            return true;
        }

        std::optional<ConfigValue> ParseNumericValue(std::string_view value)
        {
            int integerValue = 0;
            const auto integerResult = std::from_chars(
                value.data(), value.data() + value.size(), integerValue);
            if (integerResult.ec == std::errc{} &&
                integerResult.ptr == value.data() + value.size())
            {
                return integerValue;
            }

            float floatingPointValue = 0.0f;
            const auto floatingPointResult = std::from_chars(
                value.data(),
                value.data() + value.size(),
                floatingPointValue,
                std::chars_format::general);
            if (floatingPointResult.ec == std::errc{} &&
                floatingPointResult.ptr == value.data() + value.size())
            {
                return floatingPointValue;
            }
            return std::nullopt;
        }

    }

    const Config::Record* Config::Section::findRecord(StringId recordName) const
    {
        for (const Record& record : records)
        {
            if (record.name == recordName)
            {
                return &record;
            }
        }
        return nullptr;
    }

    const Config::Section* Config::FileData::findSection(StringId sectionName) const
    {
        for (const Section& section : sections)
        {
            if (section.name == sectionName)
            {
                return &section;
            }
        }
        return nullptr;
    }

    void Config::ParseConfigFile(FileData& file, std::string_view content)
    {
        StringId currentSectionName;
        Section* currentSection = nullptr;
        bool currentSectionIsValid = true;
        std::size_t lineStart = 0;
        while (lineStart < content.size())
        {
            const std::size_t lineEnd = content.find('\n', lineStart);
            const std::size_t length = lineEnd == std::string_view::npos
                ? content.size() - lineStart
                : lineEnd - lineStart;
            const std::string_view line = Trim(content.substr(lineStart, length));
            lineStart = lineEnd == std::string_view::npos
                ? content.size()
                : lineEnd + 1;

            if (line.empty() || line.front() == ';' || line.front() == '#')
            {
                continue;
            }

            if (line.front() == '[' && line.back() == ']')
            {
                const std::string_view sectionName =
                    Trim(line.substr(1, line.size() - 2));
                currentSectionIsValid =
                    MakeStringId(sectionName, currentSectionName);
                currentSection = nullptr;
                continue;
            }

            if (!currentSectionIsValid)
            {
                continue;
            }

            const std::size_t separator = line.find('=');
            if (separator == std::string_view::npos)
            {
                continue;
            }

            const std::string_view keyName = Trim(line.substr(0, separator));
            const std::string_view value = Trim(line.substr(separator + 1));
            StringId key;
            if (keyName.empty() || !MakeStringId(keyName, key))
            {
                continue;
            }

            const std::optional<ConfigValue> configValue =
                ParseNumericValue(value);
            if (!configValue)
            {
                continue;
            }

            if (currentSection == nullptr)
            {
                for (Section& section : file.sections)
                {
                    if (section.name == currentSectionName)
                    {
                        currentSection = &section;
                        break;
                    }
                }

                if (currentSection == nullptr)
                {
                    file.sections.emplace_back();
                    currentSection = &file.sections.back();
                    currentSection->name = currentSectionName;
                }
            }

            Record* record = nullptr;
            for (Record& existing : currentSection->records)
            {
                if (existing.name == key)
                {
                    record = &existing;
                    break;
                }
            }

            if (record != nullptr)
            {
                record->value = *configValue;
            }
            else
            {
                currentSection->records.push_back({key, *configValue});
            }
        }
    }

    Config Config::Read(std::string_view relativeFolderPath)
    {
        Config config;
        const auto paths = FindFiles(
            std::filesystem::path{relativeFolderPath}, ".ini");
        if (!paths)
        {
            Error(
                "Config"_id,
                "Could not enumerate config files in {}: {}",
                relativeFolderPath,
                paths.error().message());
            return config;
        }

        for (const std::filesystem::path& path : *paths)
        {
            const std::string fileName = path.stem().string();
            StringId fileId;
            if (!MakeStringId(fileName, fileId))
            {
                Warning(
                    "Config"_id,
                    "Ignoring config filename longer than 15 characters: {}",
                    fileName);
                continue;
            }

            Assert(config._fileCount < config._files.size());
            if (config._fileCount >= config._files.size())
            {
                return config;
            }

            const auto content = ReadTextFile(path);
            if (!content)
            {
                Error(
                    "Config"_id,
                    "Could not read {}: {}",
                    path.string(),
                    content.error().message());
                return Config{};
            }

            FileData& file = config._files[config._fileCount++];
            file.name = fileId;
            ParseConfigFile(file, *content);
        }

        return config;
    }

    std::optional<ConfigValue> Config::find(
        StringId filename,
        StringId section,
        StringId key) const
    {
        for (std::size_t index = 0; index < _fileCount; ++index)
        {
            const FileData& file = _files[index];
            if (file.name != filename)
            {
                continue;
            }

            const Section* foundSection = file.findSection(section);
            if (foundSection == nullptr)
            {
                return std::nullopt;
            }

            const Record* record = foundSection->findRecord(key);
            return record != nullptr
                ? std::optional<ConfigValue>{record->value}
                : std::nullopt;
        }

        return std::nullopt;
    }

    void Config::forEachSection(
        StringId fileName,
        std::function<void(StringId, const Section&)> func) const
    {
        for (std::size_t index = 0; index < _fileCount; ++index)
        {
            const FileData& file = _files[index];
            if (file.name != fileName)
            {
                continue;
            }

            for (const Section& section : file.sections)
            {
                func(section.name, section);
            }
            return;
        }
    }
}
