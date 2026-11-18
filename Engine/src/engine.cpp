#include <engine.h>

#include <consoleVariables.h>

namespace PlainEngine
{
    void Engine::initialize()
    {
        _config = Config::Read("config");
        ConsoleVariableManager::Get().useConfig(_config);
    }
}
