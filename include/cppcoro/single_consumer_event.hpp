#ifndef CPPCORO_SINGLE_CONSUMER_EVENT_HEAD_INCLUDED
#define CPPCORO_SINGLE_CONSUMER_EVENT_HEAD_INCLUDED

#include <atomic>
#include <coroutine>

namespace cppcoro
{

    class single_consumer_event
    {
    public:
        single_consumer_event(bool initiallySet = false) noexcept
            : m_state(initiallySet ? state::set : state::not_set) 
        {}

        bool is_set() const noexcept
        {
            return m_state.load(std::memory_order_acquire) == state::set;
        }

        void set()
        {
            const state old_state = m_state.exchange(state::set, std::memory_order_acq_rel);
            if (old_state == state::not_set_consumer_waiting)
            {
                m_awaiter.resume();
            }
        }

        void reset() noexcept
        {
            state old_state = state::set;
            m_state.compare_exchange_strong(old_state, state::not_set, std::memory_order_relaxed);
        }

        auto operator co_await() noexcept
        {
            class awaiter
            {
            public:

                awaiter(single_consumer_event& event) : m_event(event) {}

                bool await_ready() const noexcept
                {
                    return m_event.is_set();
                }

                bool await_suspend(std::coroutine_handle<> awaiter)
                {
                    m_event.m_awaiter = awaiter;

                    state old_state = state::not_set;
                    return m_event.m_state.compare_exchange_strong(
                        old_state,
                        state::not_set_consumer_waiting,
                        std::memory_order_release,
                        std::memory_order_acquire);
                }

                void await_resume() noexcept {}

            private:
                single_consumer_event& m_event;
            };

            return awaiter{*this};
        }


    private:
        enum class state: unsigned char
        {
            not_set,
            not_set_consumer_waiting,
            set
        };

        std::atomic<state> m_state;
        std::coroutine_handle<> m_awaiter;
    };
}

#endif
