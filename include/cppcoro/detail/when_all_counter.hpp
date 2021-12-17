#ifndef CPPCORO_WHEN_ALL_COUNTER_HEAD_INCLUDED
#define CPPCORO_WHEN_ALL_COUNTER_HEAD_INCLUDED

#include <coroutine>
#include <atomic>
#include <cstdint>

namespace cppcoro
{
    namespace detail
    {
        class when_all_counter
        {
        public:
            when_all_counter(std::size_t count) noexcept
                : m_count(count + 1)
                , m_awaiting_coroutine(nullptr)
            {}

            bool is_ready() const noexcept
            {
                return static_cast<bool>(m_awaiting_coroutine);
            }

            bool try_await(std::coroutine_handle<> awaiting_coroutine) noexcept
            {
                m_awaiting_coroutine = awaiting_coroutine;
                return m_count.fetch_sub(1, std::memory_order_acq_rel) > 1;
            }

            void notify_awaitable_completed() noexcept
            {
                if (m_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
                {
                    m_awaiting_coroutine.resume();
                }
            }
        
        protected:

            std::atomic<std::size_t> m_count;
            std::coroutine_handle<> m_awaiting_coroutine;
        };
    }
}

#endif
