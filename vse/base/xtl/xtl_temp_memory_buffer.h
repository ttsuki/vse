/// @file
/// @brief  xtl::temporally memory buffer
/// @author ttsuki

#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>

namespace vse::xtl
{
    class temp_memory_buffer final
    {
        static inline constexpr size_t alignment_ = 64;
        static inline constexpr size_t before_ = 1;
        static inline constexpr size_t after_ = 1;

        struct alignas(alignment_) block_t
        {
            std::byte e[alignment_];
        };

        std::unique_ptr<block_t[]> ptr_{};
        size_t capacity_{};

    public:
        template <class T = std::byte, std::enable_if_t<std::is_trivial_v<T>>* = nullptr>
        [[nodiscard]] T* get(size_t count)
        {
            // increase a bit to reduce re-allocation
            const auto requested_bytes = sizeof(T) * count;
            const auto allocation_bytes = requested_bytes + std::max(requested_bytes / 4, alignment_ * 4);
            if (allocation_bytes > capacity_)
            {
                const size_t requested_unit_count = (allocation_bytes + alignment_ - 1) / alignment_;
                ptr_ = std::make_unique<block_t[]>(before_ + requested_unit_count + after_);
                capacity_ = requested_unit_count * alignment_;
            }

            return reinterpret_cast<T*>(ptr_.get() + before_);
        }
    };
}
