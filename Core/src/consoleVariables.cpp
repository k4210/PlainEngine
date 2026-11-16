#include <consoleVariables.h>

#include <config.h>
#include <log.h>

#include <algorithm>
#include <mutex>
#include <variant>

namespace PlainEngine::Core::ConsoleVariables
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

    bool ConsoleVariableInt::trySet(const Config::AnyValue& value)
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

    bool ConsoleVariableFloat::trySet(const Config::AnyValue& value)
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
        const ConsoleVariableKey variableKey{file, section, key};
        bool isDuplicate = false;
        {
            const std::unique_lock lock{_mutex};
            const auto existing = std::find_if(
                _variables.begin(),
                _variables.end(),
                [&variableKey](const Entry& entry)
                {
                    return entry.key == variableKey;
                });
            if (existing != _variables.end())
            {
                if (existing->variable == &variable)
                {
                    return;
                }

                isDuplicate = true;
            }
            else
            {
                _variables.push_back(Entry{variableKey, &variable});
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
        const ConsoleVariableKey variableKey{file, section, key};
        const auto found = std::find_if(
            _variables.begin(),
            _variables.end(),
            [&variableKey](const Entry& entry)
            {
                return entry.key == variableKey;
            });
        if (found == _variables.end() ||
            found->variable->_type != ConsoleVariableType::integer)
        {
            return nullptr;
        }

        return static_cast<ConsoleVariableInt*>(found->variable);
    }

    ConsoleVariableFloat* ConsoleVariableManager::findFloat(
        StringId file,
        StringId section,
        StringId key) const
    {
        const std::shared_lock lock{_mutex};
        const ConsoleVariableKey variableKey{file, section, key};
        const auto found = std::find_if(
            _variables.begin(),
            _variables.end(),
            [&variableKey](const Entry& entry)
            {
                return entry.key == variableKey;
            });
        if (found == _variables.end() ||
            found->variable->_type != ConsoleVariableType::floatingPoint)
        {
            return nullptr;
        }

        return static_cast<ConsoleVariableFloat*>(found->variable);
    }

    void ConsoleVariableManager::useConfig(const Config::Config& config)
    {
        const std::shared_lock lock{_mutex};
        for (const Entry& entry : _variables)
        {
            const auto value = config.find(
                entry.key.file, entry.key.section, entry.key.key);
            if (value && !entry.variable->trySet(*value))
            {
                Warning(
                    "ConsoleVar"_id,
                    "Config value [{}][{}][{}] has an incompatible type",
                    entry.key.file,
                    entry.key.section,
                    entry.key.key);
            }
        }
    }

    void ConsoleVariableManager::unregisterVariable(
        ConsoleVariableBase& variable)
    {
        const std::unique_lock lock{_mutex};
        std::erase_if(
            _variables,
            [&variable](const Entry& entry)
            {
                return entry.variable == &variable;
            });
    }
}
