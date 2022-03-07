#include "cppcoro/task.hpp"
#include "cppcoro/single_consumer_event.hpp"
#include "cppcoro/sync_wait.hpp"
#include "cppcoro/when_all_ready.hpp"
#include "cppcoro/logging.hpp"
#include "counted.hpp"

#include <string>
#include <type_traits>
#include <iostream>

int main()
{
    set_global_log_level(INFO);

    // task doesn't start until awaited
    {
        bool started = false;
        auto func = [&]() -> cppcoro::task<>
        {
            DLOG << "enter func";
            started = true;
            co_return;
        };

        cppcoro::sync_wait([&]() -> cppcoro::task<>
        {
            DLOG << "enter main lambda function";
            auto t = func();
            DLOG << "after call func function";
            assert(!started);
            DLOG << "before co_await t";

            co_await t;

            assert(started);
            DLOG << "after co_await t";

        }());

        DLOG << "before main finished";
    }

    // task doesn't start until awaited
    {
        cppcoro::sync_wait([&]() -> cppcoro::task<>
        {
            bool flag = false;
            cppcoro::task<> t;
            try
            {
                co_await t;
            }
            catch (cppcoro::broken_promise& e)
            {
                DLOG << "case2: get broken_promise exception";
                flag = true;
            }
            assert(flag);
        }());
    }

    // awaiting task that completes asynchronously
    {
        bool reach_before_event = false;
        bool reach_after_event = false;
        cppcoro::single_consumer_event event;
        auto f = [&]() -> cppcoro::task<>
        {
            reach_before_event = true;
            co_await event;
            reach_after_event = true;
        };

        cppcoro::sync_wait([&]() -> cppcoro::task<>
        {
            auto t = f();

            assert(!reach_before_event);
            (void)co_await cppcoro::when_all_ready(
                [&]() -> cppcoro::task<>
                {
                    co_await t;
                    assert(reach_before_event);
                    assert(reach_after_event);
                }(),
                [&]() -> cppcoro::task<>
                {
                    assert(reach_before_event);
                    assert(!reach_after_event);
                    event.set();
                    assert(reach_after_event);
                    co_return;
                }());
        }());
    }

    // destroying task that was never awaited destroys captured args
    {
        counted::reset_counts();

        auto f = [](counted c) -> cppcoro::task<counted>
        {
            co_return c;
        };

        assert(counted::active_count() == 0);

        {
            auto t = f(counted{});
            assert(counted::active_count() == 1);
        }

        assert(counted::active_count() == 0);
    }

    // task destructor destroys result
    {
        counted::reset_counts();

        auto f = []() -> cppcoro::task<counted>
        {
            co_return counted{};
        };

        {
            auto t = f();
            assert(counted::active_count() == 0);

            auto& result = cppcoro::sync_wait(t);

            assert(counted::active_count() == 1);
            assert(result.id == 0);
        }

        assert(counted::active_count() == 0);
    }

    // task of reference  type
    {
        int value = 3;
        auto f = [&]() -> cppcoro::task<int&>
        {
            co_return value;
        };

        cppcoro::sync_wait([&]() -> cppcoro::task<>
        {
            // subcase: awaiting rvalue task
            {
                decltype(auto) result = co_await f();
                static_assert(
                    std::is_same<decltype(result), int&>::value,
                    "co_await r-value reference of task<int&> should result in an int&");
                assert(&result == &value);
            }
            // subcase: awaiting lvalue task
            {
                auto t = f();
                decltype(auto) result = co_await t;
                static_assert(
                    std::is_same<decltype(result), int&>::value,
                    "co_await l-value reference of task<int&> should result in an int&");
                assert(&result == &value);
            }
        }());
    }

    return 0;
}
