#include "define_awaiters.hpp"

struct not_awaitable1
{ };

struct is_awaitable1
{
    struct awaiter; 
    awaiter operator co_await() noexcept;
};

struct is_awaitable1::awaiter {
    awaiter(const is_awaitable1& a): _ma(a) {}
    const is_awaitable1& _ma;
};

is_awaitable1::awaiter is_awaitable1::operator co_await() noexcept {
    return awaiter(*this); 
}

struct is_awaitable2
{

};

void operator co_await(const is_awaitable2& awaitable)
{ }

