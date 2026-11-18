#pragma once

#include <config.h>

namespace PlainEngine
{
    class Engine
    {
    public:
        void initialize();

    private:
        Config _config;
    };
}
