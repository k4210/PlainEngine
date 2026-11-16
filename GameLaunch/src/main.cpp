#include <log.h>

int main()
{
    Log("GameLaunch test log");
    Info("GameLaunch"_id, "Sample game initialized");
    Warning("GameLaunch"_id, "Test warning int {}, string {}, id {}", 42, "hello", "test"_id);
    Error("GameLaunch"_id, "Test error");
    //Break("GameLaunch"_id, "Test break execution");
    return 0;
}
