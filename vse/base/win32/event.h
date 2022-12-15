/// @file
/// @brief  win32 event
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>

#include <type_traits>
#include <memory>
#include <new>
#include <stdexcept>

#include "./unique_handle.h"

namespace vse::win32
{
    template <bool AutoReset>
    class event final
    {
        unique_handle handle_{};

    public:
        explicit event(bool initial_state = false)
        {
            MemoryBarrier();
            handle_.reset(::CreateEventW(nullptr, AutoReset, initial_state, nullptr));
            if (!handle_) throw std::bad_alloc();
        }

        event(const event& other) = delete;
        event(event&& other) noexcept = default;
        event& operator=(const event& other) = delete;
        event& operator=(event&& other) noexcept = default;

        HANDLE handle() const noexcept { return handle_.get(); }

        void notify_signal() { ::SetEvent(this->handle()); }

        void reset_signal_state() { ::ResetEvent(this->handle()); }

        bool wait_signal(DWORD milliseconds = INFINITE)
        {
            if (!handle()) throw std::logic_error("invalid call");

            auto result = ::WaitForSingleObject(handle(), milliseconds);
            if (result == WAIT_OBJECT_0) return true;
            if (result == WAIT_TIMEOUT) return false;
            if (result == WAIT_ABANDONED) throw std::runtime_error("handle abandoned");
            throw std::runtime_error("object corrupted");
        }
    };

    using auto_reset_event = event<true>;
    using manual_reset_event = event<false>;

    static_assert(std::is_nothrow_move_assignable_v<manual_reset_event>);
    static_assert(std::is_nothrow_move_constructible_v<manual_reset_event>);
}
