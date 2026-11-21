#include "consoleVariables.h"

#include "config.h"
#include "log.h"

#include <mutex>
#include <variant>

namespace PlainEngine
{
    ConsoleVariableBase::ConsoleVariableBase(
        ConsoleVariableType type)
        : _type{type}
    {
    }

    void ConsoleVariableBase::unregister()
    {
        ConsoleVariableManager::Get().unregisterVariable(*this);
    }

    ConsoleVariableInt::ConsoleVariableInt(
        StringId file,
        StringId section,
        StringId key,
        int defaultValue)
        : ConsoleVariableBase{ConsoleVariableType::integer}
        , _value{defaultValue}
    {
        ConsoleVariableManager::Get().registerVariable(file, section, key, *this);
    }

    ConsoleVariableInt::~ConsoleVariableInt()
    {
        unregister();
    }

    int ConsoleVariableInt::get() const
    {
        return _value.load(std::memory_order_relaxed);
    }

    void ConsoleVariableInt::set(int newValue)
    {
        _value.store(newValue, std::memory_order_relaxed);
    }

    bool ConsoleVariableInt::trySet(const ConfigValue& value)
    {
        if (const int* integerValue = std::get_if<int>(&value))
        {
            set(*integerValue);
            return true;
        }

        return false;
    }

    ConsoleVariableFloat::ConsoleVariableFloat(
        StringId file,
        StringId section,
        StringId key,
        float defaultValue)
        : ConsoleVariableBase{ConsoleVariableType::floatingPoint}
        , _value{defaultValue}
    {
        ConsoleVariableManager::Get().registerVariable(file, section, key, *this);
    }

    ConsoleVariableFloat::~ConsoleVariableFloat()
    {
        unregister();
    }

    float ConsoleVariableFloat::get() const
    {
        return _value.load(std::memory_order_relaxed);
    }

    void ConsoleVariableFloat::set(float newValue)
    {
        _value.store(newValue, std::memory_order_relaxed);
    }

    bool ConsoleVariableFloat::trySet(const ConfigValue& value)
    {
        if (const float* floatingPointValue = std::get_if<float>(&value))
        {
            set(*floatingPointValue);
            return true;
        }

        if (const int* integerValue = std::get_if<int>(&value))
        {
            set(static_cast<float>(*integerValue));
            return true;
        }

        return false;
    }

    ConsoleVariableManager& ConsoleVariableManager::Get()
    {
        static ConsoleVariableManager manager;
        return manager;
    }

    void ConsoleVariableManager::registerVariable(
        StringId file,
        StringId section,
        StringId key,
        ConsoleVariableBase& variable)
    {
        const CVarKey variableKey{file, section, key};
        bool isDuplicate = false;
        {
            const std::unique_lock lock{_mutex};
            const auto existing = _variables.find(variableKey);
            if (existing != _variables.end())
            {
                if (existing->second == &variable)
                {
                    return;
                }

                isDuplicate = true;
            }
            else
            {
                _variables.emplace(variableKey, &variable);
            }
        }
        if (isDuplicate)
        {
            Error(
                "ConsoleVar"_id,
                "Duplicate console variable [{}][{}][{}]",
                file,
                section,
                key);
        }
    }

    ConsoleVariableInt* ConsoleVariableManager::findInt(
        StringId file,
        StringId section,
        StringId key) const
    {
        const std::shared_lock lock{_mutex};
        const CVarKey variableKey{file, section, key};
        const auto found = _variables.find(variableKey);
        if (found == _variables.end() ||
            found->second->_type != ConsoleVariableType::integer)
        {
            return nullptr;
        }

        return static_cast<ConsoleVariableInt*>(found->second);
    }

    ConsoleVariableFloat* ConsoleVariableManager::findFloat(
        StringId file,
        StringId section,
        StringId key) const
    {
        const std::shared_lock lock{_mutex};
        const CVarKey variableKey{file, section, key};
        const auto found = _variables.find(variableKey);
        if (found == _variables.end() ||
            found->second->_type != ConsoleVariableType::floatingPoint)
        {
            return nullptr;
        }

        return static_cast<ConsoleVariableFloat*>(found->second);
    }

    void ConsoleVariableManager::useConfig(const Config& config)
    {
        const std::shared_lock lock{_mutex};
        for (const auto& [key, variable] : _variables)
        {
            const auto value = config.find(
                key.file, key.section, key.key);
            if (value && !variable->trySet(*value))
            {
                Warning(
                    "ConsoleVar"_id,
                    "Config value [{}][{}][{}] has an incompatible type",
                    key.file,
                    key.section,
                    key.key);
            }
        }
    }

    void ConsoleVariableManager::unregisterVariable(
        ConsoleVariableBase& variable)
    {
        const std::unique_lock lock{_mutex};
        std::erase_if(
            _variables,
            [&variable](const auto& entry)
            {
                return entry.second == &variable;
            });
    }
}
