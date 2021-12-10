///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCORO_AWAITABLE_TRAITS_HPP_INCLUDED
#define CPPCORO_AWAITABLE_TRAITS_HPP_INCLUDED

#include <cppcoro/is_awaitable.hpp>

#include <type_traits>

namespace cppcoro
{
    template<typename T>
    concept await_traits_concept = is_awaitable_concept<T> && 
    requires (T x) {
        { cppcoro::detail::get_awaiter(x).await_resume() };
    };

    template<await_traits_concept T>
    struct awaitable_traits
    {
        using awaiter_t = decltype(cppcoro::detail::get_awaiter(std::declval<T>()));

        using await_result_t = decltype(std::declval<awaiter_t>().await_resume());
    };
}

#endif
