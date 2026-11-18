#pragma once

#include <cassert>
#include <type_traits>
#include <utility>
#include <variant>

namespace PlainEngine
{
    template <typename E>
    class Unexpected
    {
    public:
        constexpr explicit Unexpected(E error)
            : _error(std::move(error))
        {
        }

        constexpr E& error() &
        {
            return _error;
        }

        constexpr const E& error() const&
        {
            return _error;
        }

        constexpr E&& error() &&
        {
            return std::move(_error);
        }

    private:
        E _error;
    };

    template <typename E>
    Unexpected(E) -> Unexpected<E>;

    template <typename E>
    constexpr auto MakeUnexpected(E&& error)
    {
        return Unexpected<std::decay_t<E>>{std::forward<E>(error)};
    }

    template <typename T, typename E>
    class Expected
    {
    public:
        constexpr Expected()
            requires std::is_default_constructible_v<T>
            : _storage(std::in_place_index<0>)
        {
        }

        constexpr Expected(const T& value)
            : _storage(std::in_place_index<0>, value)
        {
        }

        constexpr Expected(T&& value)
            : _storage(std::in_place_index<0>, std::move(value))
        {
        }

        constexpr Expected(const Unexpected<E>& error)
            : _storage(std::in_place_index<1>, error)
        {
        }

        constexpr Expected(Unexpected<E>&& error)
            : _storage(std::in_place_index<1>, std::move(error))
        {
        }

        constexpr bool has_value() const
        {
            return _storage.index() == 0;
        }

        constexpr explicit operator bool() const
        {
            return has_value();
        }

        constexpr T& value() &
        {
            assert(has_value());
            return *std::get_if<0>(&_storage);
        }

        constexpr const T& value() const&
        {
            assert(has_value());
            return *std::get_if<0>(&_storage);
        }

        constexpr T&& value() &&
        {
            assert(has_value());
            return std::move(*std::get_if<0>(&_storage));
        }

        constexpr E& error() &
        {
            assert(!has_value());
            return std::get_if<1>(&_storage)->error();
        }

        constexpr const E& error() const&
        {
            assert(!has_value());
            return std::get_if<1>(&_storage)->error();
        }

        constexpr E&& error() &&
        {
            assert(!has_value());
            return std::move(*std::get_if<1>(&_storage)).error();
        }

        constexpr T& operator*() &
        {
            return value();
        }

        constexpr const T& operator*() const&
        {
            return value();
        }

        constexpr T* operator->()
        {
            return &value();
        }

        constexpr const T* operator->() const
        {
            return &value();
        }

    private:
        std::variant<T, Unexpected<E>> _storage;
    };

    template <typename E>
    class Expected<void, E>
    {
    public:
        constexpr Expected() = default;

        constexpr Expected(const Unexpected<E>& error)
            : _storage(std::in_place_index<1>, error)
        {
        }

        constexpr Expected(Unexpected<E>&& error)
            : _storage(std::in_place_index<1>, std::move(error))
        {
        }

        constexpr bool has_value() const
        {
            return _storage.index() == 0;
        }

        constexpr explicit operator bool() const
        {
            return has_value();
        }

        constexpr void value() const
        {
            assert(has_value());
        }

        constexpr E& error() &
        {
            assert(!has_value());
            return std::get_if<1>(&_storage)->error();
        }

        constexpr const E& error() const&
        {
            assert(!has_value());
            return std::get_if<1>(&_storage)->error();
        }

        constexpr E&& error() &&
        {
            assert(!has_value());
            return std::move(*std::get_if<1>(&_storage)).error();
        }

    private:
        std::variant<std::monostate, Unexpected<E>> _storage;
    };
}
