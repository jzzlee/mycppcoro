#include <cppcoro/task.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/logging.hpp>

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


    return 0;
}
