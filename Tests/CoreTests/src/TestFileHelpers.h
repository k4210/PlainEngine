#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>

namespace PlainEngine
{
    class TemporaryDirectory
    {
    public:
        TemporaryDirectory()
            : _path(
                std::filesystem::temp_directory_path() /
                ("PlainEngineCoreTests_" + std::to_string(
                    std::chrono::steady_clock::now().time_since_epoch().count())))
        {
            std::filesystem::create_directories(_path);
        }

        ~TemporaryDirectory()
        {
            std::error_code error;
            std::filesystem::remove_all(_path, error);
        }

        const std::filesystem::path& path() const
        {
            return _path;
        }

    private:
        std::filesystem::path _path;
    };

    inline bool WriteFile(
        const std::filesystem::path& path,
        std::string_view contents)
    {
        std::ofstream file{path, std::ios::binary};
        file.write(contents.data(), static_cast<std::streamsize>(contents.size()));
        file.close();
        return static_cast<bool>(file);
    }
}
