/// @file
/// @brief  win32 com_ptr
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>
#include <combaseapi.h>

#include <type_traits>
#include <memory>
#include <stdexcept>

namespace vse::win32
{
    template <class TInterface>
    class com_ptr
    {
        TInterface* pointer_{};

    public:
        constexpr com_ptr(nullptr_t = nullptr) noexcept {}

        // with AddRef
        com_ptr(TInterface* ptr) noexcept
        {
            this->reset(ptr);
        }

        // with AddRef
        com_ptr(const com_ptr& other) noexcept
        {
            this->reset(other.get());
        }

        // with AddRef
        com_ptr& operator=(const com_ptr& other) noexcept
        {
            if (this == std::addressof(other)) return *this;
            this->reset(other.get());
            return *this;
        }

        // with AddRef
        void reset(TInterface* ptr = nullptr) noexcept
        {
            if (ptr) { ptr->AddRef(); }
            std::swap(pointer_, ptr);
            if (ptr) { ptr->Release(); }
        }

        // with QueryInterface
        template <class UInterface>
        com_ptr(UInterface* ptr) noexcept
        {
            this->reset<UInterface>(ptr);
        }

        // with QueryInterface
        template <class UInterface>
        com_ptr(const com_ptr<UInterface>& other)
        {
            this->reset<UInterface>(other.get());
        }

        // with QueryInterface
        template <class UInterface>
        com_ptr& operator=(const com_ptr<UInterface>& other)
        {
            this->reset<UInterface>(other.get());
            return *this;
        }

        // with QueryInterface
        template <class UInterface>
        void reset(UInterface* p)
        {
            TInterface* ptr{};
            if (p) { p->QueryInterface(__uuidof(TInterface), reinterpret_cast<void**>(&ptr)); }
            std::swap(pointer_, ptr);
            if (ptr) { ptr->Release(); }
        }

        // without AddRef
        com_ptr(com_ptr&& other) noexcept
        {
            pointer_ = std::exchange(other.pointer_, nullptr);
        }

        // without AddRef
        com_ptr& operator=(com_ptr&& other) noexcept
        {
            if (this == std::addressof(other)) return *this;
            pointer_ = std::exchange(other.pointer_, nullptr);
            return *this;
        }

        // with Release
        ~com_ptr()
        {
            if (pointer_) { pointer_->Release(); }
        }

        // without AddRef
        void attach(TInterface* ptr) noexcept
        {
            std::swap(pointer_, ptr);
            if (ptr) { ptr->Release(); }
        }

        // without Release
        TInterface* detach() noexcept
        {
            return std::exchange(pointer_, nullptr);
        }

        [[nodiscard]] TInterface* get() const noexcept
        {
            return pointer_;
        }

        [[nodiscard]] TInterface** put()
        {
            if (pointer_ != nullptr) throw std::logic_error("pointer is set already.");
            return &pointer_;
        }

        [[nodiscard]] void** put_void()
        {
            return reinterpret_cast<void**>(this->put());
        }

        explicit operator bool() const noexcept { return pointer_; }

        struct __declspec(novtable) InterfaceProxy : public TInterface
        {
        private:
            virtual ULONG STDMETHODCALLTYPE AddRef() override = 0;  // make method private
            virtual ULONG STDMETHODCALLTYPE Release() override = 0; // make method private
        };

        InterfaceProxy* operator ->() const noexcept { return reinterpret_cast<InterfaceProxy*>(pointer_); }

        // with QueryInterface
        template <class UInterface>
        [[nodiscard]] com_ptr<UInterface> query_interface() const noexcept
        {
            if (!pointer_) return nullptr;

            com_ptr<UInterface> result{};
            HRESULT hr = pointer_->QueryInterface(__uuidof(UInterface), result.put_void());
            if (SUCCEEDED(hr)) return result;
            if (hr == E_NOINTERFACE) return nullptr;
            return nullptr; // throw
        }
    };

    static_assert(std::is_nothrow_move_assignable_v<com_ptr<IUnknown>>);
    static_assert(std::is_nothrow_move_constructible_v<com_ptr<IUnknown>>);
}
