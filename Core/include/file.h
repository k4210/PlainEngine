#pragma once

#include <cstddef>
#include <expected.h>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace PlainEngine::Core::FileSystem
{
    Expected<std::vector<std::filesystem::path>, std::error_code> FindFiles(
        const std::filesystem::path& directory,
        std::string_view extension,
        bool recursive = false);

    Expected<std::string, std::error_code> ReadTextFile(
        const std::filesystem::path& path);

    Expected<std::vector<std::byte>, std::error_code> ReadBinaryFile(
        const std::filesystem::path& path);
}
