#include <cppcoro/task.hpp>
#include <cppcoro/sync_wait.hpp>

#include <string>
#include <type_traits>
#include <iostream>

int main()
{
    bool started = false;
    auto func = [&]() -> cppcoro::task<>
	{
		started = true;
		co_return;
	};

    cppcoro::sync_wait([&]() -> cppcoro::task<>
    {
        auto t = func();
        assert(!started);
        std::cout<<started<<std::endl;

        co_await t;

        assert(started);
        std::cout<<started<<std::endl;

    }());

    return 0;
}
