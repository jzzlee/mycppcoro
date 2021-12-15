#ifndef CPPCORO_SYNC_WAIT_TASK_INCLUDED
#define CPPCORO_SYNC_WAIT_TASK_INCLUDED

#include <cassert>
#include <exception>
#include <utility>
#include <coroutine>

#include <cppcoro/awaitable_traits.hpp>
#include <cppcoro/detail/lightweight_manual_reset_event.hpp>
#include <cppcoro/logging.hpp>

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

            sync_wait_task_promise() noexcept 
            {
                DLOG << "call sync_wait_task_promise " << __FUNCTION__ << " " << this;
            }

            void start(detail::lightweight_manual_reset_event& event)
            {
                DLOG << "call sync_wait_task_promise " << __FUNCTION__ << " " << this;
                m_event = &event;
                coroutine_handle_t::from_promise(*this).resume();
            }

            auto get_return_object() noexcept
            {
                DLOG << "call sync_wait_task_promise " << __FUNCTION__ << " " << this;
                return coroutine_handle_t::from_promise(*this);
            }

            auto initial_suspend() noexcept
            {
                DLOG << "call sync_wait_task_promise " << __FUNCTION__ << " " << this;
                return std::suspend_always{};
            }

            auto final_suspend() noexcept
            {
                DLOG << "call sync_wait_task_promise " << __FUNCTION__ << " " << this;
                struct completion_notifier
                {
                    bool await_ready() const noexcept {
                        DLOG << "call sync_wait_task_promise::completion_notifier " << __FUNCTION__ << " " << this;
                        return false; 
                    }
                    void await_suspend(coroutine_handle_t coroutine) const noexcept
                    {
                        DLOG << "call sync_wait_task_promise::completion_notifier " << __FUNCTION__ << " " << this;
                        coroutine.promise().m_event->set();
                    }
                    void await_resume() noexcept 
                    {
                        DLOG << "call sync_wait_task_promise::completion_notifier " << __FUNCTION__ << " " << this;
                    }
                };

                return completion_notifier{};
            }

            auto yield_value(reference result) noexcept
            {
                DLOG << "call sync_wait_task_promise " << __FUNCTION__ << " " << this;
                m_result = std::addressof(result);
                return final_suspend();
            }

            void return_void() noexcept
            {
                DLOG << "call sync_wait_task_promise " << __FUNCTION__ << " " << this;
                assert(false);
            }

            void unhandled_exception()
            {
                DLOG << "call sync_wait_task_promise " << __FUNCTION__ << " " << this;
                m_exception = std::current_exception();
            }

            reference result()
            {
                DLOG << "call sync_wait_task_promise " << __FUNCTION__ << " " << this;
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
            sync_wait_task_promise() noexcept 
            {
                DLOG << "call sync_wait_task_promise<void> " << __FUNCTION__ << " " << this;
            }

            void start(lightweight_manual_reset_event& event)
            {
                DLOG << "call sync_wait_task_promise<void> " << __FUNCTION__ << " " << this;
                m_event = &event;
                coroutine_handle_t::from_promise(*this).resume();
            }

            auto get_return_object() noexcept
            {
                DLOG << "call sync_wait_task_promise<void> " << __FUNCTION__ << " " << this << " " << coroutine_handle_t::from_promise(*this).address();
                return coroutine_handle_t::from_promise(*this);
            }

            auto initial_suspend() noexcept
            {
                DLOG << "call sync_wait_task_promise<void> " << __FUNCTION__ << " " << this;
                return std::suspend_always{};
            }

            auto final_suspend() noexcept
            {
                DLOG << "call sync_wait_task_promise<void> " << __FUNCTION__ << " " << this;
                struct completion_notifier
                {
                    bool await_ready() const noexcept 
                    { 
                        DLOG << "call sync_wait_task_promise<void>::completion_notifier " << __FUNCTION__;
                        return false; 
                    }
                    void await_suspend(coroutine_handle_t coroutine) const noexcept
                    {
                        DLOG << "call sync_wait_task_promise<void>::completion_notifier " << __FUNCTION__;
                        coroutine.promise().m_event->set();
                    }
                    void await_resume() noexcept 
                    {
                        DLOG << "call sync_wait_task_promise<void>::completion_notifier " << __FUNCTION__;
                    }
                };

                return completion_notifier{};
            }

            void return_void() 
            {
                DLOG << "call sync_wait_task_promise<void> " << __FUNCTION__ << " " << this;
            }
            void unhandled_exception()
            {
                DLOG << "call sync_wait_task_promise<void> " << __FUNCTION__ << " " << this;
                m_exception = std::current_exception();
            }

            void result()
            {
                DLOG << "call sync_wait_task_promise<void> " << __FUNCTION__ << " " << this;
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
                : m_coroutine(coroutine) 
            {
                DLOG << "call sync_wait_task construct with coro " << __FUNCTION__ << " " << this << " " << coroutine.address();
            }
            
            sync_wait_task(sync_wait_task&& task) noexcept
                : m_coroutine(std::exchange(task.m_coroutine, coroutine_handle_t{}))
            {
                DLOG << "call sync_wait_task construct with task " << __FUNCTION__ << " " << this;
            }
            
            ~sync_wait_task() 
            {
                DLOG << "call sync_wait_task " << __FUNCTION__ << " " << this;
                if (m_coroutine)
                    DLOG << "call sync_wait_task " << m_coroutine.address();
                else
                    DLOG << "call sync_wait_task no m_coroutine";
                if (m_coroutine)
                    m_coroutine.destroy();
            }

            sync_wait_task(const sync_wait_task&) = delete;
            sync_wait_task& operator=(const sync_wait_task&) = delete;

            void start(lightweight_manual_reset_event& event) noexcept
            {
                DLOG << "call sync_wait_task " << __FUNCTION__ << " " << this << " " << m_coroutine.address();
                m_coroutine.promise().start(event);
            }

            decltype(auto) result()
            {
                DLOG << "call sync_wait_task " << __FUNCTION__ << " " << this << " " << m_coroutine.address();
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
                DLOG << "call " << __FUNCTION__ << " result is not void";
                co_yield co_await std::forward<AWAITABLE>(awaitable);
            }
            else
            {
                DLOG << "call " << __FUNCTION__ << " result is void";
                co_await std::forward<AWAITABLE>(awaitable);
            }
        }
    }
}

#endif
