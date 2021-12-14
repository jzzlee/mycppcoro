#ifndef CPPCORO_SYNC_WAIT_TASK_INCLUDED
#define CPPCORO_SYNC_WAIT_TASK_INCLUDED

#include <cassert>
#include <exception>
#include <utility>
#include <coroutine>

#include <cppcoro/awaitable_traits.hpp>
#include <cppcoro/detail/lightweight_manual_reset_event.hpp>

namespace cppcoro
{
    namespace detail
    {
        template<typename RESULT>
        class sync_wait_task;

        template<typename RESULT>
        class sync_wait_task_promise final
        {
            using coroutine_handle_t = std::coroutine_handle<sync_wait_task_promise<RESULT>>;
        
        public:
            using reference = RESULT&&;

            sync_wait_task_promise() noexcept {}

            void start(detail::lightweight_manual_reset_event& event)
            {
                m_event = &event;
                coroutine_handle_t::from_promise(*this).resume();
            }

            auto get_return_object() noexcept
            {
                return coroutine_handle_t::from_promise(*this);
            }

            auto initial_suspend() noexcept
            {
                return std::suspend_always{};
            }

            auto final_suspend() noexcept
            {
                struct completion_notifier
                {
                    bool await_ready() const noexcept { return false; }
                    void await_suspend(coroutine_handle_t coroutine) const noexcept
                    {
                        coroutine.promise().m_event->set();
                    }
                    void await_resume() noexcept {}
                };

                return completion_notifier{};
            }

            auto yield_value(reference result) noexcept
            {
                m_result = std::addressof(result);
                return final_suspend();
            }

            void return_void() noexcept
            {
                assert(false);
            }

            void unhandled_exception()
            {
                m_exception = std::current_exception();
            }

            reference result()
            {
                if (m_exception)
                {
                    std::rethrow_exception(m_exception);
                }
                return static_cast<reference>(*m_result);
            }

        private:
            lightweight_manual_reset_event* m_event;
            std::remove_reference_t<RESULT>* m_result;
            std::exception_ptr m_exception;
        };

        template<>
        class sync_wait_task_promise<void>
        {
            using coroutine_handle_t = std::coroutine_handle<sync_wait_task_promise<void>>;
        
        public:
            sync_wait_task_promise() noexcept {}

            void start(lightweight_manual_reset_event& event)
            {
                m_event = &event;
                coroutine_handle_t::from_promise(*this).resume();
            }

            auto get_return_object() noexcept
            {
                return coroutine_handle_t::from_promise(*this);
            }

            auto initial_suspend() noexcept
            {
                return std::suspend_always{};
            }

            auto final_suspend() noexcept
            {
                struct completion_notifier
                {
                    bool await_ready() const noexcept { return false; }
                    void await_suspend(coroutine_handle_t coroutine) const noexcept
                    {
                        coroutine.promise().m_event->set();
                    }
                    void await_resume() noexcept {}
                };

                return completion_notifier{};
            }

            void return_void() {}
            void unhandled_exception()
            {
                m_exception = std::current_exception();
            }

            void result()
            {
                if (m_exception)
                {
                    std::rethrow_exception(m_exception);
                }
            }

        private:
            lightweight_manual_reset_event* m_event;
            std::exception_ptr m_exception;
        };


        template<typename RESULT>
        class sync_wait_task final
        {
        public:
            using promise_type = sync_wait_task_promise<RESULT>;
            using coroutine_handle_t = std::coroutine_handle<promise_type>;

            sync_wait_task(coroutine_handle_t coroutine) noexcept
                : m_coroutine(coroutine) {}
            
            sync_wait_task(sync_wait_task&& task) noexcept
                : m_coroutine(std::exchange(task.m_coroutine, coroutine_handle_t{}))
                {}
            
            ~sync_wait_task() 
            {
                if (m_coroutine)
                    m_coroutine.destroy();
            }

            sync_wait_task(const sync_wait_task&) = delete;
            sync_wait_task& operator=(const sync_wait_task&) = delete;

            void start(lightweight_manual_reset_event& event) noexcept
            {
                m_coroutine.promise().start(event);
            }

            decltype(auto) result()
            {
                return m_coroutine.promise().result();
            }

        private:
            coroutine_handle_t m_coroutine;
        };

        template<cppcoro::awaitable_traits_concept AWAITABLE>
        auto make_sync_wait_task(AWAITABLE&& awaitable)
            -> sync_wait_task<typename cppcoro::awaitable_traits<AWAITABLE&&>::await_result_t> 
        {
            using RESULT = typename cppcoro::awaitable_traits<AWAITABLE&&>::await_result_t;
            if constexpr (!std::is_void_v<RESULT>)
            {
                co_yield co_await std::forward<AWAITABLE>(awaitable);
            }
            else
            {
                co_await std::forward<AWAITABLE>(awaitable);
            }
        }
    }
}

#endif
