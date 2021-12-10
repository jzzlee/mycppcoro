///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCORO_DETAIL_GET_AWAITER_HPP_INCLUDED
#define CPPCORO_DETAIL_GET_AWAITER_HPP_INCLUDED

#include <cppcoro/detail/is_awaiter.hpp>
#include <cppcoro/detail/any.hpp>

namespace cppcoro
{
	namespace detail
	{
		template<typename T>
		requires requires (T t) { t.operator co_await(); }
		auto get_awaiter_impl(T&& value)
		 noexcept(noexcept(static_cast<T&&>(value).operator co_await()))
		 -> decltype(static_cast<T&&>(value).operator co_await())
		{
			return static_cast<T&&>(value).operator co_await();
		}

		template<typename T>
		requires requires (T t) { operator co_await(static_cast<T&&>(t)); }
		auto get_awaiter_impl(T&& value)
			noexcept(noexcept(operator co_await(static_cast<T&&>(value))))
			-> decltype(operator co_await(static_cast<T&&>(value)))
		{
			return operator co_await(static_cast<T&&>(value));
		}

		template<typename T>
		requires is_awaiter_concept<T>
		T&& get_awaiter_impl(T&& value) noexcept
		{
			return static_cast<T&&>(value);
		}

		template<typename T>
		auto get_awaiter(T&& value)
			noexcept(noexcept(detail::get_awaiter_impl(static_cast<T&&>(value))))
			-> decltype(detail::get_awaiter_impl(static_cast<T&&>(value)))
		{
			return detail::get_awaiter_impl(static_cast<T&&>(value));
		}
	}
}

#endif
