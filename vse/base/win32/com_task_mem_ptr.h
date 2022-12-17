/// @file
/// @brief  win32 com_task_mem_ptr
/// @author (C) 2022 ttsuki


#pragma once

#include <Windows.h>
#include <combaseapi.h>

#include <type_traits>

namespace vse::win32
{
    template <class Type>
    class com_task_mem_ptr final
    {
        void* ptr_{};

    public:
        static com_task_mem_ptr<Type> allocate() { return com_task_mem_ptr<Type>(::CoTaskMemAlloc(sizeof(Type))); }

        constexpr com_task_mem_ptr(nullptr_t = nullptr) noexcept {}
        constexpr explicit com_task_mem_ptr(Type* ptr) noexcept : ptr_(ptr) {}
        com_task_mem_ptr(const com_task_mem_ptr& other) = delete;
        com_task_mem_ptr(com_task_mem_ptr&& other) noexcept : ptr_(std::exchange(other.ptr_, nullptr)) {}
        com_task_mem_ptr& operator=(const com_task_mem_ptr& other) = delete;

        com_task_mem_ptr& operator=(com_task_mem_ptr&& other) noexcept
        {
            if (this->ptr_ != other.ptr_) { this->reset(other.detach()); }
            return *this;
        }

        ~com_task_mem_ptr() { reset(nullptr); }

        [[nodiscard]] Type* get() const noexcept { return static_cast<Type*>(ptr_); }
        [[nodiscard]] Type** put() noexcept { return reinterpret_cast<Type**>(&ptr_); }
        [[nodiscard]] void** put_void() noexcept { return &ptr_; }
        Type* operator ->() const noexcept { return get(); }
        Type& operator *() const noexcept { return *get(); }

        Type* detach() noexcept { return static_cast<Type*>(std::exchange(ptr_, nullptr)); }

        void reset(Type* ptr = nullptr) noexcept
        {
            if (ptr_) { ::CoTaskMemFree(std::exchange(ptr_, nullptr)); }
            this->ptr_ = ptr;
        }
    };

    static_assert(std::is_nothrow_move_assignable_v<com_task_mem_ptr<int>>);
    static_assert(std::is_nothrow_move_constructible_v<com_task_mem_ptr<int>>);
}
