#pragma once

#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace PlainEngine::Core
{
    template <typename Key, typename Value>
    class LinearMap final
    {
    public:
        Value& findOrAdd(Key key, bool* alreadyExisted = nullptr)
        {
            const auto position = std::lower_bound(_keys.begin(), _keys.end(), key);
            const std::size_t index =
                static_cast<std::size_t>(position - _keys.begin());
            if (position != _keys.end() && !(key < *position))
            {
                if (alreadyExisted != nullptr)
                {
                    *alreadyExisted = true;
                }
                return _values[index];
            }

            if (alreadyExisted != nullptr)
            {
                *alreadyExisted = false;
            }

            _keys.insert(position, std::move(key));
            _values.insert(
                _values.begin() + static_cast<std::ptrdiff_t>(index), Value{});
            return _values[index];
        }

        std::optional<Value> find(Key key) const
        {
            const auto position = std::lower_bound(_keys.begin(), _keys.end(), key);
            if (position == _keys.end() || key < *position)
            {
                return std::nullopt;
            }

            const std::size_t index =
                static_cast<std::size_t>(position - _keys.begin());
            return _values[index];
        }

    private:
        std::vector<Key> _keys;
        std::vector<Value> _values;
    };
}
