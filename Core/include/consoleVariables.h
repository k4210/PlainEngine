#pragma once

#include <config.h>

#include <atomic>
#include <compare>
#include <shared_mutex>
#include <vector>

namespace PlainEngine::Core::ConsoleVariables
{
    struct ConsoleVariableKey final
    {
        StringId file;
        StringId section;
        StringId key;

        friend constexpr auto operator<=>(
            const ConsoleVariableKey&,
            const ConsoleVariableKey&) = default;
    };

    enum class ConsoleVariableType
    {
        integer,
        floatingPoint
    };

    class ConsoleVariableManager;

    class ConsoleVariableBase
    {
    public:
        virtual ~ConsoleVariableBase() = default;

        ConsoleVariableBase(const ConsoleVariableBase&) = delete;
        ConsoleVariableBase& operator=(const ConsoleVariableBase&) = delete;
        ConsoleVariableBase(ConsoleVariableBase&&) = delete;
        ConsoleVariableBase& operator=(ConsoleVariableBase&&) = delete;

    protected:
        explicit ConsoleVariableBase(ConsoleVariableType type);

        void unregister();

    private:
        friend class ConsoleVariableManager;

        virtual bool trySet(const Config::AnyValue& value) = 0;

        ConsoleVariableType _type;
    };

    class ConsoleVariableInt final : public ConsoleVariableBase
    {
    public:
        ConsoleVariableInt(
            StringId file,
            StringId section,
            StringId key,
            int defaultValue);

        ~ConsoleVariableInt() override;

        int get() const;
        void set(int newValue);

    private:
        bool trySet(const Config::AnyValue& value) override;

        std::atomic<int> _value;
    };

    class ConsoleVariableFloat final : public ConsoleVariableBase
    {
    public:
        ConsoleVariableFloat(
            StringId file,
            StringId section,
            StringId key,
            float defaultValue);

        ~ConsoleVariableFloat() override;

        float get() const;
        void set(float newValue);

    private:
        bool trySet(const Config::AnyValue& value) override;

        std::atomic<float> _value;
    };

    class ConsoleVariableManager final
    {
    public:
        static ConsoleVariableManager& Get();

        ConsoleVariableManager(const ConsoleVariableManager&) = delete;
        ConsoleVariableManager& operator=(const ConsoleVariableManager&) = delete;
        ConsoleVariableManager(ConsoleVariableManager&&) = delete;
        ConsoleVariableManager& operator=(ConsoleVariableManager&&) = delete;

        void registerVariable(
            StringId file,
            StringId section,
            StringId key,
            ConsoleVariableBase& variable);

        ConsoleVariableInt* findInt(
            StringId file,
            StringId section,
            StringId key) const;

        ConsoleVariableFloat* findFloat(
            StringId file,
            StringId section,
            StringId key) const;

        void useConfig(const Config::Config& config);

    private:
        struct Entry final
        {
            ConsoleVariableKey key;
            ConsoleVariableBase* variable;
        };

        ConsoleVariableManager() = default;
        void unregisterVariable(ConsoleVariableBase& variable);

        mutable std::shared_mutex _mutex;
        std::vector<Entry> _variables;

        friend class ConsoleVariableInt;
        friend class ConsoleVariableFloat;
        friend class ConsoleVariableBase;
    };
}
