/// @file
/// @brief  win32 thread
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>
#include <process.h>

#include <type_traits>
#include <new>
#include <stdexcept>
#include <future>

#include "./unique_handle.h"
#include "./event.h"

namespace vse::win32
{
    class thread final
    {
    public:
        enum struct join_on_destructor_flag
        {
            none = 0,
            join_on_destructor = 1,
        };

        static inline constexpr join_on_destructor_flag join_on_destructor = join_on_destructor_flag::join_on_destructor;

    private:
        unique_handle thread_handle_{};
        unsigned int thread_id_{};
        bool join_on_destructor_{};

    public:
        thread() = default;

        template <class F, std::enable_if_t<std::is_invocable_v<F>>* = nullptr>
        explicit thread(F function_body, size_t stack_commit_size = 65536)
            : thread(std::move(function_body), join_on_destructor_flag::none, stack_commit_size) { }

        template <class F, std::enable_if_t<std::is_invocable_v<F>>* = nullptr>
        explicit thread(F function_body, join_on_destructor_flag flag, size_t stack_commit_size = 65536)
            : join_on_destructor_(flag == join_on_destructor_flag::join_on_destructor)
        {
            // thread argument
            struct arg_t
            {
                std::reference_wrapper<F> thread_function_body;
                std::promise<void> thread_is_ready{};
            } arg{std::ref(function_body)};

            thread_handle_.reset(reinterpret_cast<HANDLE>(::_beginthreadex(
                nullptr,
                static_cast<unsigned>(stack_commit_size),
                [](void* arg) -> unsigned
                {
                    auto function_body = std::move(static_cast<arg_t*>(arg)->thread_function_body.get()); // move
                    auto& thread_is_ready = static_cast<arg_t*>(arg)->thread_is_ready;                    // reference

                    thread_is_ready.set_value(); // notify to caller that sub-thread is ready.
                    function_body();             // invoke
                    return 0;
                },
                &arg,
                0,
                &thread_id_)));

            if (!thread_handle_)
                throw std::bad_alloc();

            arg.thread_is_ready.get_future().get(); // wait for thread started
        }

        thread(const thread& other) = delete;
        thread(thread&& other) noexcept = default;
        thread& operator=(const thread& other) = delete;
        thread& operator=(thread&& other) noexcept = default;

        ~thread() noexcept(true)
        {
            if (joinable() && join_on_destructor_)
                join();

            if (joinable())
            {
                __pragma(warning(suppress: 4297))                                // C4297: function assumed not to throw an exception but does
                throw std::logic_error("the thread is not joined or detached!"); // causes std::terminate
            }
        }

        [[nodiscard]] bool joinable() const noexcept { return static_cast<bool>(thread_handle_); }

        [[nodiscard]] HANDLE handle() const noexcept { return thread_handle_.get(); }

        [[nodiscard]] DWORD thread_id() const noexcept { return joinable() ? thread_id_ : 0; }

        void detach()
        {
            if (!thread_handle_) throw std::logic_error("invalid call");
            thread_handle_.reset();
            thread_id_ = 0;
        }

        bool join(DWORD milliseconds = INFINITE)
        {
            if (!thread_handle_) throw std::logic_error("invalid call");

            auto result = ::WaitForSingleObject(handle(), milliseconds);
            if (result == WAIT_OBJECT_0)
            {
                thread_handle_.reset();
                thread_id_ = 0;
                return true;
            }
            else if (result == WAIT_TIMEOUT)
            {
                return false;
            }
            else if (result == WAIT_ABANDONED)
            {
                throw std::runtime_error("object abandoned");
            }
            else
            {
                throw std::runtime_error("object corrupted");
            }
        }
    };

    static_assert(std::is_nothrow_move_assignable_v<thread>);
    static_assert(std::is_nothrow_move_constructible_v<thread>);
}
