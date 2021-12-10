#include <string>
#include <iostream>

#include "cppcoro/detail/is_awaiter.hpp"
#include "defines/define_awaiters.hpp"

using namespace cppcoro;
using namespace cppcoro::detail;


int main() 
{
    static_assert(is_awaiter_v<not_awaiter1> == false);
    static_assert(is_awaiter_v<not_awaiter2> == false);
    static_assert(is_awaiter_v<is_awaiter1> == true);
    static_assert(is_awaiter_v<not_awaiter3> == false);
    static_assert(is_awaiter_v<is_awaiter2> == true);
    static_assert(is_awaiter_v<is_awaiter3> == true);

    return 0;
}
