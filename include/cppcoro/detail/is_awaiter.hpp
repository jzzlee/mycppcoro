
#ifndef CPPCORO_DETAIL_IS_AWAITER_HPP_INCLUDED
#define CPPCORO_DETAIL_IS_AWAITER_HPP_INCLUDED

#include <type_traits>
#include <concepts>
#include <coroutine>

namespace cppcoro
{
    namespace detail
    {
        template<typename T, typename... Types>
        struct is_any_type
        {
            static constexpr bool value = ((std::is_same_v<T, Types> || ...));
        };

        template<typename T, typename... Types> concept return_type_concept = 
            is_any_type<T, Types...>::value;

        template<typename T> concept is_awaiter_concept =
        requires(T x) {
            {x.await_ready()} -> std::same_as<bool>;
            {x.await_suspend(std::declval<std::coroutine_handle<>>())} -> 
                return_type_concept<bool, void, std::coroutine_handle<>>;
            {x.await_resume()};
        };

        template<typename T>
        struct is_awaiter: std::false_type {};

        template<is_awaiter_concept T>
        struct is_awaiter<T>: std::true_type {};

        template<typename T>
        constexpr bool is_awaiter_v = is_awaiter<T>::value;
    }
}

#endif
