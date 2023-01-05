/// @file
/// @brief  winasio - AsioCallbackAllocator
/// @author (C) 2022 ttsuki

#pragma once

#include <memory>
#include <array>
#include <mutex>

#include "asio.h"

// callback dispatching helper
namespace asio::util
{
    struct AsioCallbackFunctions
    {
        void* transparent;
        void (*buffer_switched)(void* transparent, AsioLong buffer_index, AsioBool direct_processing_supported);
        void (*sampling_rate_changed)(void* transparent, AsioSamplingRate new_sampling_rate);
        AsioLong(*handle_message)(void* transparent, AsioMessage selector, AsioLong value, void* message, double* opt);
        AsioTime* (*buffer_switched_with_timeinfo)(void* transparent, AsioTime* params, AsioLong buffer_index, AsioBool direct_processing_supported);
    };

    template <class Tag>
    static AsioCallbacks* InitCallbackAnchor(AsioCallbackFunctions functions)
    {
        static AsioCallbackFunctions anchor_point{};
        static AsioCallbacks callback = {
            [](AsioLong buffer_index, AsioBool direct_processing_supported) { return anchor_point.buffer_switched(anchor_point.transparent, buffer_index, direct_processing_supported); },
            [](AsioSamplingRate new_sampling_rate) { return anchor_point.sampling_rate_changed(anchor_point.transparent, new_sampling_rate); },
            [](AsioMessage selector, AsioLong value, void* message, double* opt) { return anchor_point.handle_message(anchor_point.transparent, selector, value, message, opt); },
            [](AsioTime* params, AsioLong buffer_index, AsioBool direct_processing_supported) { return anchor_point.buffer_switched_with_timeinfo(anchor_point.transparent, params, buffer_index, direct_processing_supported); },
        };

        anchor_point = functions; // copy
        return &callback;
    }

    class AsioCallbackAllocator
    {
        template <size_t> class Slot {};

        static inline std::mutex mutex_;
        static inline std::array<AsioCallbacks*, 16> allocated_{};

    public:
        static AsioCallbacks* Allocate(AsioCallbackFunctions f)
        {
            std::lock_guard lock(mutex_);
            for (size_t i = 0; i < std::size(allocated_); ++i)
            {
                if (!allocated_[i])
                {
                    switch (i) // dispatches runtime index to template parameter
                    {
                    case 0: return allocated_[0] = InitCallbackAnchor<Slot<0>>(f);
                    case 1: return allocated_[1] = InitCallbackAnchor<Slot<1>>(f);
                    case 2: return allocated_[2] = InitCallbackAnchor<Slot<2>>(f);
                    case 3: return allocated_[3] = InitCallbackAnchor<Slot<3>>(f);
                    case 4: return allocated_[4] = InitCallbackAnchor<Slot<4>>(f);
                    case 5: return allocated_[5] = InitCallbackAnchor<Slot<5>>(f);
                    case 6: return allocated_[6] = InitCallbackAnchor<Slot<6>>(f);
                    case 7: return allocated_[7] = InitCallbackAnchor<Slot<7>>(f);
                    case 8: return allocated_[8] = InitCallbackAnchor<Slot<8>>(f);
                    case 9: return allocated_[9] = InitCallbackAnchor<Slot<9>>(f);
                    case 10: return allocated_[10] = InitCallbackAnchor<Slot<10>>(f);
                    case 11: return allocated_[11] = InitCallbackAnchor<Slot<11>>(f);
                    case 12: return allocated_[12] = InitCallbackAnchor<Slot<12>>(f);
                    case 13: return allocated_[13] = InitCallbackAnchor<Slot<13>>(f);
                    case 14: return allocated_[14] = InitCallbackAnchor<Slot<14>>(f);
                    case 15: return allocated_[15] = InitCallbackAnchor<Slot<15>>(f);
                    default: static_assert(std::size(allocated_) == 16);
                    }
                }
            }
            throw std::bad_alloc();
        }

        static void Free(AsioCallbacks* p)
        {
            static_assert(std::is_trivial_v<AsioCallbacks>);
            static_assert(std::is_trivial_v<AsioCallbackFunctions>);

            std::lock_guard lock(mutex_);
            for (size_t i = 0; i < std::size(allocated_); ++i)
                if (allocated_[i] == p)
                    allocated_[i] = nullptr;
        }

        static std::shared_ptr<AsioCallbacks> AllocateShared(AsioCallbackFunctions f)
        {
            return { Allocate(f), &Free };
        }
    };
}
