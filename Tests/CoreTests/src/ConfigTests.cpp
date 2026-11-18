#include <config.h>
#include <stringId.h>

#include "TestFileHelpers.h"

#include <filesystem>
#include <gtest/gtest.h>
#include <variant>

using PlainEngine::Config;

TEST(ConfigTests, LoadsNumericValuesFromIniFiles)
{
    const PlainEngine::TemporaryDirectory directory;
    std::filesystem::create_directories(directory.path() / "config");
    ASSERT_TRUE(PlainEngine::WriteFile(
        directory.path() / "config" / "settings.ini",
        "; comment\n"
        "[window]\n"
        "width = 1280\n"
        "scale = 1.5\n"
        "invalid = 12px\n"
        "[audio]\n"
        "volume = 0.75\n"));

    const std::filesystem::path originalPath = std::filesystem::current_path();
    std::filesystem::current_path(directory.path());
    const auto config = Config::Read("config");
    std::filesystem::current_path(originalPath);

    const auto width = config.find("settings"_id, "window"_id, "width"_id);
    ASSERT_TRUE(width);
    EXPECT_EQ(std::get<int>(*width), 1280);

    const auto scale = config.find("settings"_id, "window"_id, "scale"_id);
    ASSERT_TRUE(scale);
    EXPECT_FLOAT_EQ(std::get<float>(*scale), 1.5f);

    const auto volume = config.find("settings"_id, "audio"_id, "volume"_id);
    ASSERT_TRUE(volume);
    EXPECT_FLOAT_EQ(std::get<float>(*volume), 0.75f);

    EXPECT_FALSE(config.find("settings"_id, "window"_id, "invalid"_id));
    EXPECT_FALSE(config.find("settings.ini"_id, "window"_id, "width"_id));
    EXPECT_FALSE(config.find("missing"_id, "window"_id, "width"_id));
}
