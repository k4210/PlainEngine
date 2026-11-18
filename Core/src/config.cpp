#include <config.h>

#include <file.h>
#include <log.h>

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

        void ParseConfigFile(
            StringId file,
            std::string_view content,
            std::unordered_map<ConfigKey, ConfigValue>& records)
        {
            StringId currentSection;
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
                        MakeStringId(sectionName, currentSection);
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
                if (configValue)
                {
                    records.insert_or_assign(
                        ConfigKey{file, currentSection, key}, *configValue);
                }
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

            ParseConfigFile(fileId, *content, config._records);
        }

        return config;
    }

    std::optional<ConfigValue> Config::find(
        StringId filename,
        StringId section,
        StringId key) const
    {
        const auto value = _records.find(ConfigKey{filename, section, key});
        if (value == _records.end())
        {
            return std::nullopt;
        }

        return value->second;
    }
}
