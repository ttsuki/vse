/// @file
/// @brief  win32 com base
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>
#include <combaseapi.h>

#include <optional>
#include <string>
#include <algorithm>
#include <iterator>

namespace vse::win32
{
    static inline std::wstring to_wstring(GUID guid)
    {
        OLECHAR buf[64]{};
        auto wrote = StringFromGUID2(guid, buf, 63);
        return std::wstring(buf, buf + wrote - 1);
    }

    static inline std::string to_string(GUID guid)
    {
        std::wstring o = to_wstring(guid); // assumes no non-ascii character.
        std::string s;
        std::transform(o.begin(), o.end(), std::back_insert_iterator(s), [](wchar_t w) { return static_cast<char>(w); });
        return s;
    }

    static inline std::optional<GUID> from_wstring(const wchar_t* guid)
    {
        GUID val{};
        if (FAILED(::IIDFromString(guid, &val))) return std::nullopt;
        return val;
    }

    static inline std::optional<GUID> from_string(const char* guid)
    {
        std::wstring in(guid, guid + strlen(guid)); // assumes no mb character.
        return from_wstring(in.c_str());
    }

    static inline void CoInitializeSTA()
    {
        ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
    }

    static inline void CoInitializeMTA()
    {
        ::CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
    }

    static inline void CoUninitialize()
    {
        ::CoUninitialize();
    }
}
