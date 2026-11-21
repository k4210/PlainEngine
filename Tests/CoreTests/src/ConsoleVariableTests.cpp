#include "consoleVariables.h"
#include "config.h"

#include "TestFileHelpers.h"

#include <filesystem>
#include <gtest/gtest.h>
#include <type_traits>

using PlainEngine::Config;
using PlainEngine::ConsoleVariableFloat;
using PlainEngine::ConsoleVariableInt;
using PlainEngine::ConsoleVariableManager;

static_assert(!std::is_copy_constructible_v<ConsoleVariableInt>);
static_assert(!std::is_move_constructible_v<ConsoleVariableInt>);
static_assert(!std::is_copy_assignable_v<ConsoleVariableInt>);
static_assert(!std::is_move_assignable_v<ConsoleVariableInt>);
static_assert(!std::is_copy_constructible_v<ConsoleVariableFloat>);
static_assert(!std::is_move_constructible_v<ConsoleVariableFloat>);
static_assert(!std::is_copy_assignable_v<ConsoleVariableFloat>);
static_assert(!std::is_move_assignable_v<ConsoleVariableFloat>);

TEST(ConsoleVariableTests, RegistersLoadsConfigAndUsesAtomicValues)
{
    const PlainEngine::TemporaryDirectory directory;
    std::filesystem::create_directories(directory.path() / "config");
    ASSERT_TRUE(PlainEngine::WriteFile(
        directory.path() / "config" / "variables.ini",
        "[render]\n"
        "width = 1920\n"
        "scale = 1.5\n"
        "count = 2\n"
        "fraction = 0.25\n"));

    const std::filesystem::path originalPath = std::filesystem::current_path();
    std::filesystem::current_path(directory.path());
    const Config config = Config::Read("config");
    std::filesystem::current_path(originalPath);

    ConsoleVariableInt width{"variables"_id, "render"_id, "width"_id, 1280};
    ConsoleVariableInt fraction{"variables"_id, "render"_id, "fraction"_id, 5};
    ConsoleVariableFloat scale{"variables"_id, "render"_id, "scale"_id, 1.0f};
    ConsoleVariableFloat count{"variables"_id, "render"_id, "count"_id, 0.5f};
    ConsoleVariableManager& manager = ConsoleVariableManager::Get();

    EXPECT_EQ(manager.findInt("variables"_id, "render"_id, "width"_id), &width);
    EXPECT_EQ(
        manager.findInt("variables"_id, "render"_id, "fraction"_id),
        &fraction);
    EXPECT_EQ(manager.findFloat("variables"_id, "render"_id, "scale"_id), &scale);
    EXPECT_EQ(manager.findFloat("variables"_id, "render"_id, "count"_id), &count);
    EXPECT_EQ(manager.findInt("variables"_id, "render"_id, "scale"_id), nullptr);

    manager.useConfig(config);
    EXPECT_EQ(width.get(), 1920);
    EXPECT_EQ(fraction.get(), 5);
    EXPECT_FLOAT_EQ(scale.get(), 1.5f);
    EXPECT_FLOAT_EQ(count.get(), 2.0f);

    width.set(2560);
    scale.set(2.0f);
    EXPECT_EQ(width.get(), 2560);
    EXPECT_FLOAT_EQ(scale.get(), 2.0f);
}

TEST(ConsoleVariableTests, UnregistersVariablesWhenTheyAreDestroyed)
{
    ConsoleVariableManager& manager = ConsoleVariableManager::Get();
    {
        ConsoleVariableInt local{"testFile"_id, "testSection"_id, "local"_id, 7};
        EXPECT_EQ(
            manager.findInt("testFile"_id, "testSection"_id, "local"_id),
            &local);
    }

    EXPECT_EQ(
        manager.findInt("testFile"_id, "testSection"_id, "local"_id),
        nullptr);
}
