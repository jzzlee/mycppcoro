
#include <coroutine>

struct not_awaiter1 {

};

struct not_awaiter2 {
    bool await_ready() {return false;}
    void await_suspend() {}
    void await_resume() {}
};

struct is_awaiter1 {
    bool await_ready() {return false;}
    void await_suspend(std::coroutine_handle<> h) {}
    void await_resume() {}
};

struct not_awaiter3 {
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
