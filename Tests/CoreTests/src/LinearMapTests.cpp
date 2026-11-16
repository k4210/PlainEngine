#include <linearMap.h>

#include <gtest/gtest.h>
#include <string>

using PlainEngine::Core::LinearMap;

TEST(LinearMapTests, InsertsInAnyOrderAndFindsValues)
{
    LinearMap<int, std::string> map;
    map.findOrAdd(8) = "eight";
    map.findOrAdd(2) = "two";
    map.findOrAdd(5) = "five";

    EXPECT_EQ(map.find(2), std::optional<std::string>{"two"});
    EXPECT_EQ(map.find(5), std::optional<std::string>{"five"});
    EXPECT_EQ(map.find(8), std::optional<std::string>{"eight"});
    EXPECT_FALSE(map.find(3));
}

TEST(LinearMapTests, ReportsWhetherFindOrAddFoundAnExistingKey)
{
    LinearMap<int, int> map;
    bool alreadyExisted = true;

    map.findOrAdd(4, &alreadyExisted) = 40;
    EXPECT_FALSE(alreadyExisted);

    map.findOrAdd(4, &alreadyExisted) = 41;
    EXPECT_TRUE(alreadyExisted);
    EXPECT_EQ(map.find(4), 41);
}
