#include <file.h>

#include "TestFileHelpers.h"

#include <cstddef>
#include <filesystem>
#include <gtest/gtest.h>
#include <string_view>

using PlainEngine::Core::FileSystem::FindFiles;
using PlainEngine::Core::FileSystem::ReadBinaryFile;
using PlainEngine::Core::FileSystem::ReadTextFile;

TEST(FileTests, FindsFilesByExtensionWithOptionalRecursion)
{
    const CoreTests::TemporaryDirectory directory;
    std::filesystem::create_directories(directory.path() / "nested");
    ASSERT_TRUE(CoreTests::WriteFile(directory.path() / "root.txt", "root"));
    ASSERT_TRUE(CoreTests::WriteFile(directory.path() / "root.bin", "binary"));
    ASSERT_TRUE(CoreTests::WriteFile(
        directory.path() / "nested" / "nested.txt", "nested"));

    const auto rootFiles = FindFiles(directory.path(), ".txt");
    ASSERT_TRUE(rootFiles);
    ASSERT_EQ(rootFiles->size(), 1);
    EXPECT_EQ(rootFiles->front().filename(), "root.txt");

    const auto allFiles = FindFiles(directory.path(), ".txt", true);
    ASSERT_TRUE(allFiles);
    ASSERT_EQ(allFiles->size(), 2);
}

TEST(FileTests, ReadsTextAndBinaryContents)
{
    const CoreTests::TemporaryDirectory directory;
    const auto textPath = directory.path() / "text.txt";
    const auto binaryPath = directory.path() / "binary.bin";
    const auto emptyPath = directory.path() / "empty.txt";
    ASSERT_TRUE(CoreTests::WriteFile(textPath, "first line\nsecond line"));
    ASSERT_TRUE(CoreTests::WriteFile(emptyPath, std::string_view{}));
    const char binaryContents[]{'\0', '\x01', '\x7f', static_cast<char>(0xff)};
    ASSERT_TRUE(CoreTests::WriteFile(
        binaryPath, std::string_view{binaryContents, sizeof(binaryContents)}));

    const auto text = ReadTextFile(textPath);
    ASSERT_TRUE(text);
    EXPECT_EQ(*text, "first line\nsecond line");

    const auto emptyText = ReadTextFile(emptyPath);
    ASSERT_TRUE(emptyText);
    EXPECT_TRUE(emptyText->empty());

    const auto binary = ReadBinaryFile(binaryPath);
    ASSERT_TRUE(binary);
    ASSERT_EQ(binary->size(), sizeof(binaryContents));
    for (std::size_t index = 0; index < binary->size(); ++index)
    {
        EXPECT_EQ((*binary)[index], static_cast<std::byte>(
            static_cast<unsigned char>(binaryContents[index])));
    }
}

TEST(FileTests, ReportsReadAndSearchErrors)
{
    const CoreTests::TemporaryDirectory directory;
    const auto text = ReadTextFile(directory.path() / "missing.txt");
    ASSERT_FALSE(text);
    EXPECT_TRUE(text.error());

    const auto files = FindFiles(directory.path() / "missing", ".txt");
    ASSERT_FALSE(files);
    EXPECT_TRUE(files.error());
}
