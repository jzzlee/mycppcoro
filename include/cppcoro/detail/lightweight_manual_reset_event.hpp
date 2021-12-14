#ifndef CPPCORO_DETAIL_LIGHTWEIGHT_MANUAL_RESET_EVENT_HPP_INCLUDED
#define CPPCORO_DETAIL_LIGHTWEIGHT_MANUAL_RESET_EVENT_HPP_INCLUDED

#include <atomic>
#include <cstdint>

namespace cppcoro
{
    namespace detail
    {
        class lightweight_manual_reset_event
        {
        public:
            lightweight_manual_reset_event(bool initiallySet = false);
            ~lightweight_manual_reset_event();

            void set() noexcept;
            void reset() noexcept;
            void wait() noexcept;

        private:
            std::atomic<int> m_value;
        };
    }
}

#endif
