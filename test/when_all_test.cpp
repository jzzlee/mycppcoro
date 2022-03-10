
#include "cppcoro/when_all.hpp"

#include "cppcoro/config.hpp"
#include "cppcoro/async_manual_reset_event.hpp"
#include "cppcoro/async_mutex.hpp"
#include "cppcoro/fmap.hpp"
#include "cppcoro/shared_task.hpp"
#include "cppcoro/sync_wait.hpp"
#include "cppcoro/task.hpp"

#include "counted.hpp"

#include <functional>
#include <string>
#include <vector>

namespace 
{
    template<template<typename T> class TASK, typename T>
    TASK<T> when_event_set_return(cppcoro::async_manual_reset_event& event, T value)
    {
        co_await event;
        co_return std::move(value);
    }
}

int main()
{
    set_global_log_level(INFO);

    // when_all() with no args completes immediately
    {
        [[maybe_unused]] std::tuple<> result = cppcoro::sync_wait(cppcoro::when_all());
    }

    // when_all() with one arg
    {
        bool started = false;
        bool finished = false;
        auto f = [&](cppcoro::async_manual_reset_event& event) -> cppcoro::task<std::string>
        {
            started = true;
            co_await event;
            finished = true;
            co_return "foo";
        };

        cppcoro::async_manual_reset_event event;

        auto whenAllTask = cppcoro::when_all(f(event));
        assert(!started);

        cppcoro::sync_wait(cppcoro::when_all_ready(
            [&]() -> cppcoro::task<>
            {
                auto [s] = co_await whenAllTask;
                assert(s == "foo");
            }(),
            [&]() -> cppcoro::task<>
            {
                assert(started);
                assert(!finished);
                event.set();
                assert(finished);
                co_return;
            }()));
    }


    return 0;
}