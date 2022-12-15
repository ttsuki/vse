/// @file
/// @brief  win32 handle
/// @author (C) 2022 ttsuki

#pragma once
#include <Windows.h>

#include <memory>

namespace vse::win32
{
    struct default_handle_closer
    {
        void operator()(HANDLE p) const noexcept { if (p) ::CloseHandle(p); }
    };

    template <class HANDLE, class CLOSER> using unique_handle_t = std::unique_ptr<std::remove_pointer_t<HANDLE>, CLOSER>;

    using unique_handle = unique_handle_t<HANDLE, default_handle_closer>;
}
