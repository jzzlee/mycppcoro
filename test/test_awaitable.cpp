#include "cppcoro/is_awaitable.hpp"
#include "defines/define_awaitables.hpp"

using namespace cppcoro;
using namespace cppcoro::detail;


int main()
{
    static_assert(is_awaitable_v<not_awaitable1> == false);
    static_assert(is_awaitable_v<is_awaitable1> == true);
    static_assert(is_awaitable_v<is_awaitable2> == true);
    static_assert(is_awaitable_v<is_awaiter1> == true);
    static_assert(is_awaitable_v<not_awaiter1> == false);
    static_assert(is_awaitable_v<is_awaiter2> == true);
    static_assert(is_awaitable_v<is_awaiter3> == true);

    static_assert(is_all_awaitables_v<is_awaiter1, not_awaitable1> == false);
    static_assert(is_all_awaitables_v<is_awaiter1, is_awaiter2> == true);

    return 0;
}
