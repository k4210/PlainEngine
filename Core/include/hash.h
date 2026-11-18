#pragma once

#include <cstddef>
#include <functional>

namespace PlainEngine
{
    template <typename T>
    void hash_combine(std::size_t& seed, const T& value)
    {
        seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}
