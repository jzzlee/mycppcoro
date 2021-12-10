///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCORO_IS_AWAITABLE_HPP_INCLUDED
#define CPPCORO_IS_AWAITABLE_HPP_INCLUDED

#include <cppcoro/detail/get_awaiter.hpp>

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
}

#endif
