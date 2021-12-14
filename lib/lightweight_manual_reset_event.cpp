#include <system_error>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <linux/futex.h>
#include <cerrno>
#include <climits>
#include <cassert>

#include <limits>

#include <cppcoro/detail/lightweight_manual_reset_event.hpp>


namespace 
{
    namespace local
    {
        int futex(
            int* user_addr, 
            int futex_operation, 
            int value, 
            const struct timespec* timeout, 
            int* user_addr2,
            int value3)
        {
            return syscall(
                SYS_futex,
                user_addr,
                futex_operation,
                value,
                timeout,
                user_addr2,
                value3);
        }
    }
}

cppcoro::detail::lightweight_manual_reset_event::lightweight_manual_reset_event(bool initiallySet)
    : m_value(initiallySet ? 1 : 0) {}

cppcoro::detail::lightweight_manual_reset_event::~lightweight_manual_reset_event() { }

void cppcoro::detail::lightweight_manual_reset_event::set() noexcept
{
    m_value.store(1, std::memory_order_release);
    constexpr int waiter_number = std::numeric_limits<int>::max();
    [[maybe_unused]] int wake_number = local::futex(
        reinterpret_cast<int*>(&m_value),
        FUTEX_WAKE_PRIVATE,
        waiter_number,
        nullptr,
        nullptr,
        0);
    
    assert(wake_number != -1);
}

void cppcoro::detail::lightweight_manual_reset_event::reset() noexcept
{
    m_value.store(0, std::memory_order_relaxed);
}

void cppcoro::detail::lightweight_manual_reset_event::wait() noexcept
{
    int old_value = m_value.load(std::memory_order_acquire);
    while(old_value == 0)
    {
        int result = local::futex(
            reinterpret_cast<int*>(&m_value),
            FUTEX_WAKE_PRIVATE,
            old_value,
            nullptr,
            nullptr,
            0);
        
        if (result == -1)
        {
            if (errno == EAGAIN)
            {
                return;
            }
        }
        else
        {
            old_value = m_value.load(std::memory_order_acquire);
        }
    }
}
