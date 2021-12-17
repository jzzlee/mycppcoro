#ifndef CPPCORO_WHEN_ALL_READY_HPP_INCLUDED
#define CPPCORO_WHEN_ALL_READY_HPP_INCLUDED

#include <tuple>
#include <utility>
#include <vector>
#include <type_traits>

#include "cppcoro/awaitable_traits.hpp"
#include "cppcoro/is_awaitable.hpp"
#include "cppcoro/detail/when_all_task.hpp"
#include "cppcoro/detail/when_all_ready_awaitable.hpp"
#include "cppcoro/detail/unwrap_reference.hpp"


namespace cppcoro
{
    template<typename... AWAITABLES>
    requires is_all_awaitables_concept<AWAITABLES...>
    [[nodiscard]]
    inline auto when_all_ready(AWAITABLES... awaitables)
    {
        return detail::when_all_ready_awaitable<std::tuple<detail::when_all_task<
            typename awaitable_traits<detail::unwrap_reference_t<std::remove_reference_t<AWAITABLES>>>::await_result_t>...>>(
                std::make_tuple(detail::make_when_all_task(std::forward<AWAITABLES>(awaitables))...));
    }

    template<typename AWAITABLE>
    requires is_awaitable_concept<AWAITABLE>
    [[nodiscard]]
    auto when_all_ready(std::vector<AWAITABLE> awaitables)
    {
        using result_t = typename awaitable_traits<detail::unwrap_reference_t<AWAITABLE>>::await_result_t;

        std::vector<detail::when_all_task<result_t>> tasks;
        tasks.reserve(awaitables.size());

        for(auto& awaitable: awaitables)
        {
            tasks.emplace_back(detail::make_when_all_task(std::move(awaitable)));
        }

        return detail::when_all_ready_awaitable<std::vector<detail::when_all_task<result_t>>>(
            std::move(tasks));
    }
}

#endif
