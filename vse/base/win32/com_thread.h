/// @file
/// @brief  win32 com thread
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>
#include <combaseapi.h>

#include <type_traits>
#include <functional>
#include <future>
#include <mutex>

#include "./thread.h"
#include "./com_base.h"

namespace vse::win32
{
    class STAThread final
    {
    public:
        STAThread() = default;
        STAThread(const STAThread& other) = delete;
        STAThread(STAThread&& other) noexcept = delete;
        STAThread& operator=(const STAThread& other) = delete;
        STAThread& operator=(STAThread&& other) noexcept = delete;
        ~STAThread() { this->Enqueue(nullptr, nullptr); }

        void Post(void (*f)(void* ctx), void* ctx)
        {
            if (f)
                this->Enqueue(f, ctx);
        }

        template <class F, std::enable_if_t<std::is_invocable_v<F>>* = nullptr>
        auto Invoke(F&& f)
        {
            using R = std::invoke_result_t<F>;

            std::promise<R> p;
            struct ctx_t
            {
                std::reference_wrapper<std::remove_reference_t<F>> f;
                std::reference_wrapper<std::promise<R>> r;
            } ctx{std::ref(f), std::ref(p)};

            this->Post([](void* ctx)
            {
                if constexpr (std::is_same_v<R, void>)
                {
                    static_cast<ctx_t*>(ctx)->f.get()();
                    static_cast<ctx_t*>(ctx)->r.get().set_value();
                }
                else
                {
                    static_cast<ctx_t*>(ctx)->r.get().set_value(
                        static_cast<ctx_t*>(ctx)->f.get()()
                    );
                }
            }, &ctx);

            return p.get_future().get();
        }

    private:
        std::mutex mutex_{};
        std::condition_variable cv_{};
        bool calling_{};
        void* ctx_{};
        void (*func_)(void* ctx){};

        win32::thread host_thread_{
            [this]
            {
                win32::CoInitializeSTA();

                std::unique_lock l(mutex_);
                while (true)
                {
                    cv_.wait(l, [&] { return calling_; });
                    if (func_) { func_(ctx_); }
                    else { break; }
                    calling_ = false;
                }

                win32::CoUninitialize();
            },
            thread::join_on_destructor_flag::join_on_destructor
        };

        void Enqueue(void (*f)(void* ctx), void* ctx)
        {
            std::unique_lock l(mutex_);
            calling_ = true;
            ctx_ = ctx;
            func_ = f;
            cv_.notify_one();
        }
    };
}
