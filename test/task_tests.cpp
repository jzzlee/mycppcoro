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

    // awaiting default-constructed task throws broken_promise
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

    // task of reference type
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

    // passing parameter by value to task coroutine calls move-constructor exactly once
    {
        counted::reset_counts();

        auto f = [](counted arg) -> cppcoro::task<>
        {
            co_return;
        };

        counted c;

        assert(counted::active_count() == 1);
        assert(counted::default_construction_count == 1);
        assert(counted::copy_construction_count == 0);
        assert(counted::move_construction_count == 0);
        assert(counted::destruction_count == 0);

        {
            auto t = f(c);
            assert(counted::copy_construction_count == 1);
            assert(counted::default_construction_count == 1);
            assert(counted::move_construction_count == 1);
            assert(counted::destruction_count == 1);
            assert(counted::active_count() == 2);
        }

        assert(counted::active_count() == 1);
    }

    // task<void> fmap pipe operator
    {
        using cppcoro::fmap;

        cppcoro::single_consumer_event event;

        auto f = [&]() -> cppcoro::task<>
        {
            co_await event;
            co_return;
        };

        auto t = f() | fmap([] {return 123;});

        cppcoro::sync_wait(
            cppcoro::when_all_ready(
                [&]() -> cppcoro::task<>
                {
                    assert(co_await t == 123);
                }(),
                [&]() -> cppcoro::task<>
                {
                    event.set();
                    co_return;
                }()));
    }

    // task<int> fmap pipe operator
    {
        using cppcoro::task;
        using cppcoro::fmap;
        using cppcoro::sync_wait;
        using cppcoro::make_task;

        auto one = [&]() -> task<int>
        {
            co_return 1;
        };

        // subcase: r-value fmap / r-value lambda
        {
            auto t = one() | fmap([delta = 1](auto i) { return i + delta; });
            assert(sync_wait(t) == 2);
        }

        // subcase: r-value fmap / l-value lambda
        {
            using namespace std::string_literals;

            auto t = [&]
            {
                auto f = [prefix = "pfx"s](int x)
                {
                    return prefix + std::to_string(x);
                };

                return one() | fmap(f);
            }();

            assert(sync_wait(t) == "pfx1");
        }

        // subcase: l-value fmap / r-value lambda
        {
            using namespace std::string_literals;

            auto t = [&]
            {
                auto addprefix = fmap([prefix = "a really really long prefix that prevents small string optimisation"s](int x)
                {
                    return prefix + std::to_string(x);
                });

                return one() | addprefix;
            }();

            assert(sync_wait(t) == "a really really long prefix that prevents small string optimisation1");
        }

        // subcase: l-value fmap / l-value lambda
        {
            using namespace std::string_literals;

            task<std::string> t;
            {
                auto lambda = [prefix = "a really really long prefix that prevents small string optimisation"s](int x)
                {
                    return prefix + std::to_string(x);
                };

                auto addprefix = fmap(lambda);
                t = make_task(one() | addprefix); 
            }

            assert(!t.is_ready());
            assert(sync_wait(t) == "a really really long prefix that prevents small string optimisation1");
        }
    }
    
    // chained fmap pipe operations
    {
        using namespace std::string_literals;
        using cppcoro::task;
        using cppcoro::sync_wait;

        auto prepend = [](std::string s)
        {
            using cppcoro::fmap;
            return fmap([s = std::move(s)](const std::string& value) { return s + value; });
        };

        auto append = [](std::string s)
        {
            using cppcoro::fmap;
            return fmap([s = std::move(s)](const std::string& value) { return value + s; });
        };

        auto asyncString = [](std::string s) -> task<std::string>
        {
            co_return std::move(s);
        };

        auto t = asyncString("base"s) | prepend("pre_"s) | append("_post"s);

        assert(sync_wait(t) == "pre_base_post");
    }

    // lots of synchronous completions doestn't result in stack-overflow
    {
        auto completeSynchronously = []() -> cppcoro::task<int>
        {
            co_return 1;
        };

        auto run = [&]() -> cppcoro::task<>
        {
            int sum = 0;
            for (int i = 0; i < 1'000'000; ++i)
            {
                sum += co_await completeSynchronously();
            }
            assert(sum == 1'000'000);
        };

        cppcoro::sync_wait(run());
    }

    return 0;
}
