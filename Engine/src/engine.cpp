#include "engine.h"
#include "log.h"
#include "consoleVariables.h"

using namespace PlainEngine;

void Engine::ConfigLogsUnsafe(const Config& config)
{
    config.forEachSection("log"_id, [](StringId name, const Config::Section& section)
        {
            const auto* record = section.findRecord("priority"_id);
            if (!record)
                return;

            const int* integerValue = std::get_if<int>(&record->value);
            if (!integerValue)
                return;

            const LogPriority prio = static_cast<LogPriority>(*integerValue);
            if (prio > LogPriority::breakExecution)
                return;

            LogSettings::SetMinimalPriorityUnsafe(name, prio);
        });
}

void Engine::initialize()
{
    _config = Config::Read("config");
    ConsoleVariableManager::Get().useConfig(_config);
    ConfigLogsUnsafe(_config);
}
