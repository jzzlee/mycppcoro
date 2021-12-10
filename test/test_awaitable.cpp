#include "cppcoro/is_awaitable.hpp"

#include <iostream>
#include <string>

using namespace cppcoro;
using namespace cppcoro::detail;

struct awaitable1
{ };

struct awaitable2
{
    struct awaiter; 
    awaiter operator co_await() noexcept;
};

struct awaitable2::awaiter {
    awaiter(const awaitable2& a): _ma(a) {}
    const awaitable2& _ma;
};

awaitable2::awaiter awaitable2::operator co_await() noexcept {
    return awaiter(*this); 
}

struct awaitable3
{

};

void operator co_await(const awaitable3& awaitable)
{ }

struct is_awaiter1 {
    bool await_ready() {return false;}
    void await_suspend(std::coroutine_handle<> h) {}
    void await_resume() {}
};

struct not_awaiter1 {
    bool await_ready() {return false;}
    int await_suspend(std::coroutine_handle<> h) {return 11;}
    void await_resume() {}
};

struct is_awaiter2 {
    bool await_ready() {return false;}
    bool await_suspend(std::coroutine_handle<> h) {return false;}
    void await_resume() {}
};

struct is_awaiter3 {
    bool await_ready() {return false;}
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) { return h;}
    void await_resume() {}
};


int main()
{
    static_assert(is_awaitable_v<awaitable1> == false);
    static_assert(is_awaitable_v<awaitable2> == true);
    static_assert(is_awaitable_v<awaitable3> == true);
    static_assert(is_awaitable_v<is_awaiter1> == true);
    static_assert(is_awaitable_v<not_awaiter1> == false);
    static_assert(is_awaitable_v<is_awaiter2> == true);
    static_assert(is_awaitable_v<is_awaiter3> == true);

    return 0;
}
