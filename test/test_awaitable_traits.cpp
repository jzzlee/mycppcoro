#include <type_traits>

#include "cppcoro/awaitable_traits.hpp"
#include "defines/define_awaitables.hpp"

using namespace cppcoro;
using namespace cppcoro::detail;

template<typename T>
struct traits_tester : std::false_type {};

template<typename T>
requires requires (T x) { awaitable_traits<T>{}; }
struct traits_tester<T> : std::true_type {};

template<typename T>
constexpr bool traits_tester_v = traits_tester<T>::value;

int main()
{
    static_assert(traits_tester_v<is_awaiter1> == true);
    static_assert(traits_tester_v<is_awaiter2> == true);
    static_assert(traits_tester_v<not_awaitable1> == false);
    static_assert(traits_tester_v<is_awaitable1> == false);

    return 0;
}
