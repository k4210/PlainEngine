#include <log.h>
#include <stringId.h>

#include <gtest/gtest.h>

TEST(LogTests, FormatsWithAndWithoutChannel)
{
    Log("Core"_id, "value {}", 42);
    Log("StringId {}", "GameLaunch"_id);
    Info("started");
    Warning("Input"_id, "key {}", 7);
    Error("failed");
}
