/// @file
/// @brief  xtl::memory_byte_stream
/// @author ttsuki

#pragma once
#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>
#include <algorithm>

#include "xtl_rastream.h"
#include "xtl_spin_lock_mutex.h"

namespace vse::xtl
{
    /// On-memory byte stream.
    class random_access_memory_stream final
    {
        static inline constexpr size_t alignment = 32;
        static inline constexpr size_t block_size = 65536;

        struct alignas(alignment) block
        {
            std::byte data[block_size];
        };

        mutable xtl::recursive_spin_lock_mutex mutex_{};
        std::vector<std::unique_ptr<block>> memory_{};
        size_t length_{};

    public:
        random_access_memory_stream() = default;

        [[nodiscard]] auto lock_guard() const noexcept { return xtl::lock_guard(mutex_); }

        [[nodiscard]] size_t read(void* buffer, size_t cursor, size_t length) const
        {
            auto lock = lock_guard();
            auto slen = length_ - cursor;
            auto size = std::min(slen, length);

            // copy data
            auto dst = static_cast<std::byte*>(buffer);
            auto rem = size;
            while (rem)
            {
                auto s = cursor;
                auto e = std::min((s + block_size) / block_size * block_size, s + rem);
                auto i = s / block_size;
                auto p = s % block_size;
                auto l = e - s;
                memcpy(dst, memory_.at(i)->data + p, l);
                cursor += l;
                dst += l;
                rem -= l;
            }
            return size;
        }

        [[nodiscard]] size_t write(const void* data, size_t cursor, size_t length)
        {
            auto lock = lock_guard();
            auto slen = (length_ >= cursor + length ? length_ : resize(cursor + length)) - cursor;
            auto size = std::min(slen, length);

            // copy data
            auto src = static_cast<const std::byte*>(data);
            auto rem = size;
            while (rem)
            {
                auto s = cursor;
                auto e = std::min((s + block_size) / block_size * block_size, s + rem);
                auto i = s / block_size;
                auto p = s % block_size;
                auto l = e - s;

                memcpy(memory_.at(i)->data + p, src, l);
                cursor += l;
                src += l;
                rem -= l;
            }

            return size;
        }

        [[nodiscard]] size_t size() const
        {
            auto lock = lock_guard();
            return length_;
        }

        [[nodiscard]] size_t resize(size_t length)
        {
            auto lock = lock_guard();

            // allocates more memory block if needed.
            const size_t new_block_count = (length + block_size - 1) / block_size;
            for (size_t i = memory_.size(); i < new_block_count; i++)
                memory_.push_back(std::make_unique<block>());

            length_ = length;
            return length;
        }
    };

    /// On-memory byte stream.
    struct memory_stream : private iorastream<std::shared_ptr<random_access_memory_stream>>
    {
        using io = iorastream<std::shared_ptr<random_access_memory_stream>>;

    public:
        memory_stream() : io(std::make_shared<random_access_memory_stream>()) {}
        explicit memory_stream(std::shared_ptr<random_access_memory_stream> base) : io(base) {}
        [[nodiscard]] auto lock_guard() noexcept { return get_base_stream()->lock_guard(); }
        using io::get_base_stream;
        using io::size;
        using io::read;
        using io::tellg;
        using io::seekg;
        using io::write;
        using io::tellp;
        using io::seekp;
    };
}
