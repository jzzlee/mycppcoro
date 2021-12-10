#include <string>
#include <iostream>

#include "cppcoro/detail/is_awaiter.hpp"

using namespace cppcoro;
using namespace cppcoro::detail;

struct awaiter1 {

};

struct awaiter2 {
    bool await_ready() {return false;}
    void await_suspend() {}
    void await_resume() {}
};

struct awaiter3 {
    bool await_ready() {return false;}
    void await_suspend(std::coroutine_handle<> h) {}
    void await_resume() {}
};

struct awaiter4 {
    bool await_ready() {return false;}
    int await_suspend(std::coroutine_handle<> h) {return 11;}
    void await_resume() {}
};

struct awaiter5 {
    bool await_ready() {return false;}
    bool await_suspend(std::coroutine_handle<> h) {return false;}
    void await_resume() {}
};

struct awaiter6 {
    bool await_ready() {return false;}
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) { return h;}
    void await_resume() {}
};


int main() 
{
    static_assert(is_awaiter<awaiter1>::value == false);
    static_assert(is_awaiter<awaiter2>::value == false);
    static_assert(is_awaiter<awaiter3>::value == true);
    static_assert(is_awaiter<awaiter4>::value == false);
    static_assert(is_awaiter<awaiter5>::value == true);
    static_assert(is_awaiter<awaiter6>::value == true);

    return 0;
}
