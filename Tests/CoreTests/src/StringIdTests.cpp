#include <stringId.h>

#include <format>
#include <gtest/gtest.h>

using PlainEngine::Core::StringId;

static constexpr StringId kConstexprStringId{"alpha"};
static_assert("alpha"_id == kConstexprStringId);
static_assert(kConstexprStringId == StringId{"alpha"});
static_assert(StringId{"alpha"} == StringId{"alpha"});
static_assert(StringId{"alpha"} != StringId{"beta"});
static_assert(StringId{"alpha"} < StringId{"beta"});

TEST(StringIdTests, DefaultValueIsEmpty)
{
    const StringId id;

    EXPECT_TRUE(id.view().empty());
    EXPECT_FALSE(id);
}

TEST(StringIdTests, ConvertsAndComparesValues)
{
    const StringId first{"alpha"};
    const StringId same{"alpha"};
    const StringId next{"beta"};

    EXPECT_EQ(first, same);
    EXPECT_NE(first, next);
    EXPECT_LT(first, next);

    EXPECT_EQ(first.view(), "alpha");
    EXPECT_TRUE(first);
}

TEST(StringIdTests, StoresMaximumLength)
{
    const StringId id{"123456789012345"};

    EXPECT_EQ(id.view(), "123456789012345");
    EXPECT_EQ(id.view().size(), StringId::kMaxLength);
}

TEST(StringIdTests, StopsAtEmbeddedNull)
{
    const StringId id{"alpha\0ignored"};

    EXPECT_EQ(id.view(), "alpha");
}

TEST(StringIdTests, FormatsAsText)
{
    EXPECT_EQ(std::format("{}", "GameLaunch"_id), "GameLaunch");
    EXPECT_EQ(std::format("{:>12}", "GameLaunch"_id), "  GameLaunch");
}
