///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCORO_IS_AWAITABLE_HPP_INCLUDED
#define CPPCORO_IS_AWAITABLE_HPP_INCLUDED

#include "cppcoro/detail/get_awaiter.hpp"

#include <type_traits>

namespace cppcoro
{
    template<typename T>
    struct is_awaitable: std::false_type {};

    template<typename T> concept is_awaitable_concept = 
    requires (T x) {
        { cppcoro::detail::get_awaiter(x) };
    };
    
    template<is_awaitable_concept T>
    struct is_awaitable<T>: std::true_type {};

    template<typename T>
    constexpr bool is_awaitable_v = is_awaitable<T>::value;

    template<typename... Types>
    struct is_all_awaitables
    {
        static constexpr bool value = ((is_awaitable_v<Types> && ...));
    };

    template<typename... Types>
    constexpr bool is_all_awaitables_v = is_all_awaitables<Types...>::value;

    template<typename... Types> concept is_all_awaitables_concept = is_all_awaitables_v<Types...>;

}

#endif
