#include "file.h"

#include <cstdint>
#include <fstream>
#include <limits>
#include <string>

namespace PlainEngine
{
    namespace
    {
        Expected<std::vector<std::byte>, std::error_code> ReadFile(
            const std::filesystem::path& path)
        {
            std::ifstream file{path, std::ios::binary | std::ios::ate};
            if (!file)
            {
                return MakeUnexpected(std::make_error_code(std::errc::io_error));
            }

            const std::streamoff fileSize = file.tellg();
            if (fileSize < 0 ||
                static_cast<std::uintmax_t>(fileSize) >
                    std::vector<std::byte>{}.max_size() ||
                static_cast<std::uintmax_t>(fileSize) >
                    static_cast<std::uintmax_t>(
                        std::numeric_limits<std::streamsize>::max()))
            {
                return MakeUnexpected(
                    std::make_error_code(std::errc::file_too_large));
            }

            std::vector<std::byte> content(static_cast<std::size_t>(fileSize));
            file.seekg(0, std::ios::beg);
            if (!file)
            {
                return MakeUnexpected(std::make_error_code(std::errc::io_error));
            }

            if (!content.empty())
            {
                file.read(
                    reinterpret_cast<char*>(content.data()),
                    static_cast<std::streamsize>(content.size()));
                if (!file)
                {
                    return MakeUnexpected(std::make_error_code(std::errc::io_error));
                }
            }

            return content;
        }
    }

    Expected<std::vector<std::filesystem::path>, std::error_code> FindFiles(
        const std::filesystem::path& directory,
        std::string_view extension,
        bool recursive)
    {
        std::error_code error;
        error.clear();
        std::vector<std::filesystem::path> files;
        const auto addFileIfMatching = [&files, extension, &error](
            const std::filesystem::directory_entry& entry)
        {
            if (entry.is_regular_file(error) &&
                entry.path().extension() == extension)
            {
                files.push_back(entry.path());
            }
        };

        if (recursive)
        {
            std::filesystem::recursive_directory_iterator iterator{directory, error};
            const std::filesystem::recursive_directory_iterator end;
            while (!error && iterator != end)
            {
                addFileIfMatching(*iterator);
                if (!error)
                {
                    iterator.increment(error);
                }
            }
        }
        else
        {
            std::filesystem::directory_iterator iterator{directory, error};
            const std::filesystem::directory_iterator end;
            while (!error && iterator != end)
            {
                addFileIfMatching(*iterator);
                if (!error)
                {
                    iterator.increment(error);
                }
            }
        }

        if (error)
        {
            return MakeUnexpected(error);
        }

        return files;
    }

    Expected<std::string, std::error_code> ReadTextFile(
        const std::filesystem::path& path)
    {
        const auto content = ReadFile(path);
        if (!content)
        {
            return MakeUnexpected(content.error());
        }
        if (content->empty())
        {
            return std::string{};
        }

        return std::string{
            reinterpret_cast<const char*>(content->data()),
            content->size()};
    }

    Expected<std::vector<std::byte>, std::error_code> ReadBinaryFile(
        const std::filesystem::path& path)
    {
        return ReadFile(path);
    }
}
