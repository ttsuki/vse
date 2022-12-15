/// @file
/// @brief  win32 memory
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>

#include <cstring>
#include <type_traits>
#include <memory>
#include <algorithm>

namespace vse::win32
{
    class memory_block final
    {
        struct virt
        {
            static size_t round_up_to_page_size(size_t size) noexcept { return size + 4095 & ~4095; }
            static void* reserve_memory_address(size_t size) noexcept { return ::VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS); }
            static void* reserve_memory_address(void* base, size_t start, size_t size) noexcept { return ::VirtualAlloc(static_cast<std::byte*>(base) + start, size, MEM_RESERVE, PAGE_NOACCESS); }
            static void* commit_memory(void* base, size_t start, size_t size) noexcept { return ::VirtualAlloc(static_cast<std::byte*>(base) + start, size, MEM_COMMIT, PAGE_READWRITE); }
            static void decommit_memory(void* base, size_t start, size_t size) noexcept { ::VirtualFree(static_cast<std::byte*>(base) + start, size, MEM_DECOMMIT); }
            static void free_memory(void* base) noexcept { ::VirtualFree(base, 0, MEM_RELEASE); }
        };

        struct deleter_t
        {
            void operator()(void* p) const noexcept { if (p) virt::free_memory(p); }
        };

        using ptr_t = std::unique_ptr<void, deleter_t>;

        ptr_t base_address_{};
        size_t reserved_size_{};
        size_t committed_size_{};
        size_t size_{};

    public:
        memory_block() = default;

        explicit memory_block(size_t initial_size, size_t initial_reserved_size = 0, size_t initial_reserved_address_space_size = 0)
        {
            initial_reserved_size = std::max(initial_size, initial_reserved_size);
            initial_reserved_address_space_size = std::max(initial_reserved_address_space_size, initial_reserved_size);
            this->reserve_address_space(initial_reserved_address_space_size, false);
            this->reserve(initial_reserved_size, false);
            this->resize(initial_size, false);
        }

        memory_block(const memory_block& other) = delete;
        memory_block(memory_block&& other) noexcept = default;
        memory_block& operator=(const memory_block& other) = delete;
        memory_block& operator=(memory_block&& other) = default;
        ~memory_block() = default;

        [[nodiscard]] void* get() const noexcept { return base_address_.get(); }
        [[nodiscard]] const void* data() const noexcept { return base_address_.get(); }
        [[nodiscard]] void* data() noexcept { return base_address_.get(); }

        [[nodiscard]] size_t size() const noexcept { return size_; }

        void reserve_address_space(size_t size, bool keep_data)
        {
            if (size > reserved_size_) // need to re-alloc
            {
                size = virt::round_up_to_page_size(size);

                // try to reserve adjacent memory address
                ptr_t r{virt::reserve_memory_address(base_address_.get(), reserved_size_, size - reserved_size_)};
                if (!r) // if failed
                {
                    // try to reserve new memory
                    r = ptr_t{virt::reserve_memory_address(nullptr, 0, size)};
                    if (!r) throw std::bad_alloc();

                    // try to commit new memory
                    void* k = virt::commit_memory(r.get(), 0, committed_size_);
                    if (!k) throw std::bad_alloc();

                    // copy data to new memory
                    if (keep_data) std::memcpy(k, base_address_.get(), size_);
                }

                base_address_ = std::exchange(r, {});
                reserved_size_ = size;
            }
        }

        void reserve(size_t size, bool keep_data)
        {
            reserve_address_space(size, keep_data);

            if (size > committed_size_)
            {
                size = virt::round_up_to_page_size(size);

                void* k = virt::commit_memory(base_address_.get(), 0, size);
                if (!k) throw std::bad_alloc();

                committed_size_ = size;
            }
        }

        void resize(size_t size, bool keep_data = true)
        {
            reserve(size, keep_data);
            size_ = size;
        }

        void shrink_to_fit()
        {
            size_t r = virt::round_up_to_page_size(size_);
            virt::decommit_memory(base_address_.get(), r, committed_size_ - r);
            committed_size_ = r;
        }
    };

    static_assert(std::is_nothrow_move_assignable_v<memory_block>);
    static_assert(std::is_nothrow_move_constructible_v<memory_block>);
}
