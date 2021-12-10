#ifndef CPPCORO_TASK_HPP_INCLUDED
#define CPPCORO_TASK_HPP_INCLUDED

#include <atomic>
#include <exception>
#include <utility>
#include <type_traits>
#include <cstdint>
#include <cassert>
#include <coroutine>

#include <cppcoro/awaitable_traits.hpp>
#include <cppcoro/broken_promise.hpp>
#include <cppcoro/detail/remove_rvalue_reference.hpp>

namespace cppcoro
{
    template<typename T> class task;

    namespace detail
    {
        class task_promise_base
        {
            friend struct final_awaitable;

            struct final_awaitable
            {
                bool await_ready() const noexcept {return false;}
                
                template<typename PROMISE>
                std::coroutine_handle<> await_suspend(std::coroutine_handle<PROMISE> coro) noexcept
                {
                    return coro.promise().m_continuation;
                }

                void await_resume() noexcept { }
            };

        public:
            task_promise_base() noexcept {}

            auto initial_suspend() noexcept
            {
                return std::suspend_always{};
            }

            auto final_suspend() noexcept
            {
                return final_awaitable{};
            }

            void set_continuation(std::coroutine_handle<> continuation) noexcept
            {
                m_continuation = continuation;
            }
        
        private:
            std::coroutine_handle<> m_continuation;
        };

        template<typename T>
        class task_promise final : public task_promise_base
        {
        public:
            task_promise() noexcept {}
            ~task_promise()
            {
                switch (m_result_type)
                {
                    case result_type::value:
                        m_value.~T();
                        break;
                    case result_type::exception:
                        m_exception.~exception_ptr();
                        break;
                    default:
                        break;
                }
            }

            task<T> get_return_object() noexcept;

            void unhanlded_exception() noexcept
            {
                ::new (static_cast<void*>(std::addressof(m_exception))) std::exception_ptr(
                    std::current_exception());
                m_result_type = result_type::exception;
            }

            template<typename VALUE>
            requires (std::is_convertible_v<VALUE&&, T>>)
            void return_value(VALUE&& value) noexcept(std::is_nothrow_constructible_v<T, VALUE&&>)
            {
                ::new (static_cast<void*>(std::addressof(m_value))) T(std::forward<VALUE>(value));
                m_result_type = result_type::value;
            }

            T& result() &
            {
                if (m_result_type == result_type::exception)
                {
                    std::rethrow_exception(m_exception);
                }

                assert(m_result_type == result_type::value);
                return m_value;
            }

            using rvalue_type = std::confitional_t<
            std::is_arithmetic_v<T> || std::is_pointer_v<T>,
            T,
            T&&>;

            rvalue_type result() &&
            {
                if (m_result_type == result_type::exception)
                {
                    std::rethrow_exception(m_exception);
                }

                assert(m_result_type == result_type::value);
                return std::move(m_value);
            }
        
        private:
            enum class result_type : uint8_t { empty, value, exception };

            result_type m_result_type = result_type::empty;

            union
            {
                T m_value;
                std::exception_ptr m_exception;
            }
        };

        template<>
        class task_promise<void>: public task_promise_base
        {
        public:
            task_promise() noexcept = default;
            
            task<void> get_return_object() noexcept;
            void return_void() noexcept {}

            void unhanlded_exception() noexcept
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
            std::exception_ptr m_exception;
        };

        template<typename T>
        class task_promise_base<T&>: public task_promise_base
        {
        public:
            task_promise() noexcept = default;

            task<T&> get_return_object() noexcept;
            
            void unhanlded_exception() noexcept
            {
                m_exception = std::current_exception();
            }

            void return_value(T& value) noexcept
            {
                m_value = std::addressof(value);
            }

            T& result()
            {
                if (m_exception)
                {
                    std::rethrow_exception(m_exception);
                }

                return *m_value;
            }
        
        private:
            T* m_value;
            std::exception_ptr m_exception;
        };
    }

    template<typename T = void>
    class [[nodiscard]] task
    {
    public:
        using promise_type = detail::task_promise<T>;
        using value_type = T;

    private:
        struct awaitable_base
        {
            std::coroutine_handle<promise_type> m_coroutine;

            awaitable_base(std::coroutine_handle<promise_type> coroutine) noexcept
                : m_coroutine(coroutine)
            {}

            bool await_ready() const noexcept
            {
                return !m_coroutine || m_coroutine.done();
            }

            std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaiting_coroutine) noexcept
            {
                m_coroutine.promise().set_continuation(awaiting_coroutine);
                return m_coroutine;
            }
        };

    public:
        task() noexcept
            : m_coroutine(nullptr) {}
        
        explicit task(std::coroutine_handle<promise_type> coroutine)
            : m_coroutine(coroutine) {}
        
        task(task&& t) noexcept
            : m_coroutine(t.m_coroutine)
        {
            t.m_coroutine = nullptr;
        }

        task(const task&) = delete;
        task& operator=(const task&) = delete;

        ~task()
        {
            if (m_coroutine)
            {
                m_coroutine.destroy();
            }
        }

        task& operator=(task&& other) noexcept
        {
            if (std::addressof(other) != this)
            {
                if (m_coroutine)
                {
                    m_coroutine.destroy();
                }

                m_coroutine = other.m_coroutine;
                other.m_coroutine = nullptr;
            }

            return *this;
        }

        bool is_ready() const noexcept
        {
            return !m_coroutine || m_coroutine.done();
        }

        auto operator co_await() const & noexcept
        {
            struct awaitable : awaitable_base
            {
                using awaitable_base::awaitable_base;

                decltype(auto) await_resume()
                {
                    if (!this->m_coroutine)
                    {
                        throw broken_promise{};
                    }
                    return this->m_coroutine.promise().result();
                }
            };

            return awaitable{m_coroutine};
        }

        auto operator co_await() const && noexcept
        {
            struct awaitable : awaitable_base
            {
                using awaitable_base::awaitable_base;

                decltype(auto) await_resume()
                {
                    if (!this->m_coroutine)
                    {
                        throw broken_promise{};
                    }
                    return std::move(this->m_coroutine.promise()).result();
                }
            };

            return awaitable{m_coroutine};
        }

        auto when_ready() const noexcept
        {
            struct awaitable: awaitable_base
            {
                using awaitable_base::awaitable_base;

                void await_resume() const noexcept {}
            };

            return awaitable{m_coroutine};
        }

    private:
        std::coroutine_handle<promise_type> m_coroutine;
    };

    namespace detail
    {
        template<typename T>
        task<T> task_promise<T>::get_return_object() noexcept
        {
            return task<T>{std::coroutine_handle<task_promise>::from_promise(*this)};
        }

        inline task<void> task_promise<void>::get_return_object() noexcept
        {
            return task<void>{std::coroutine_handle<task_promise>::from_promise(*this)};
        }

        template<typename T>
        task<T&> task_promise<T&>::get_return_object() noexcept
        {
            return task<T&>{std::coroutine_handle<task_promise>::from_promise(*this)};
        }
    }

    template<typename AWAITABLE>
    auto make_task(AWAITABLE awaitable)
        -> task<detail::remove_rvalue_reference_t<typename awaitable_traits<AWAITABLE>::await_result_t>>
    {
        co_return co_await static_cast<AWAITABLE>(awaitable);
    }
}

#endif
