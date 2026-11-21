#include "engine.h"
#include "log.h"

int main()
{
    PlainEngine::Engine engine;
    engine.initialize();

    Log("Game"_id, "Starting game session");
    Info("Game"_id, "Loading level '{}'", "Ashwood Forest");
    Log("Core"_id, "This Core log should be filtered");
    Info("Core"_id, "Core systems initialized");
    Info("UI"_id, "Main menu opened");
    Warning("Renderer"_id, "Frame time exceeded {:.1f} ms", 18.4f);
    Warning("Physics"_id, "This warning should be shown");
    Error("Physics"_id, "Missing collision shape '{}'", "crate");

    return 0;
}
