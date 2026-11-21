#pragma once

#include "config.h"

namespace PlainEngine
{
    class Engine
    {
    public:
        void initialize();

    private:
        static void ConfigLogsUnsafe(const Config& config);

        Config _config;
    };
}
