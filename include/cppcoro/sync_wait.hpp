#ifndef CPPCORO_SYNC_WAIT_HPP_INCLUDED
#define CPPCORO_SYNC_WAIT_HPP_INCLUDED

#include <cstdint>
#include <atomic>

#include "cppcoro/detail/lightweight_manual_reset_event.hpp"
#include "cppcoro/detail/sync_wait_task.hpp"
#include "cppcoro/awaitable_traits.hpp"
#include "cppcoro/logging.hpp"

namespace cppcoro
{
    template<awaitable_traits_concept AWAITABLE>
    auto sync_wait(AWAITABLE&& awaitable)
        -> typename cppcoro::awaitable_traits<AWAITABLE&&>::await_result_t
    {
        DLOG << "enter sync_wait function";
        auto task = detail::make_sync_wait_task(std::forward<AWAITABLE>(awaitable));
        detail::lightweight_manual_reset_event event;
        DLOG << "before event start";
        task.start(event);
        DLOG << "after event start";
        event.wait();
        DLOG << "after event waited";
        return task.result();
    }
}

#endif
