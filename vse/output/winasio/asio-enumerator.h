/// @file
/// @brief  winasio - Driver Enumerator
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>
#include <combaseapi.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <utility>

namespace asio::util
{
    namespace win32::registry
    {
        using registry_key_unique_handle = std::unique_ptr<std::remove_pointer_t<HKEY>, decltype(&RegCloseKey)>;

        static registry_key_unique_handle OpenKey(HKEY parent, const wchar_t* sub_key_name)
        {
            HKEY key{};
            if (::RegOpenKeyW(parent, sub_key_name, &key) != ERROR_SUCCESS) return {nullptr, &RegCloseKey};
            return {key, &RegCloseKey};
        }

        static std::optional<std::wstring> EnumKeyName(HKEY parent, size_t index)
        {
            WCHAR sub_key_name[256] = {};
            DWORD len = static_cast<DWORD>(std::size(sub_key_name)) - 1;
            if (::RegEnumKeyExW(parent, static_cast<DWORD>(index), sub_key_name, &len, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) return std::nullopt;
            return sub_key_name;
        }

        static std::optional<std::wstring> ReadStringValue(HKEY key, const wchar_t* value_name)
        {
            WCHAR val[256] = {};
            DWORD len = sizeof val - sizeof val[0];
            DWORD type{};
            if (::RegQueryValueExW(key, value_name, nullptr, &type, reinterpret_cast<LPBYTE>(val), &len) != ERROR_SUCCESS) return std::nullopt;
            if (type != REG_SZ && type != REG_EXPAND_SZ) return std::nullopt;
            return val;
        }

        static std::optional<GUID> ReadGuidValue(HKEY key, const wchar_t* value_name)
        {
            GUID val{};
            auto str = ReadStringValue(key, value_name);
            if (!str) return std::nullopt;
            if (FAILED(::CLSIDFromString(str->c_str(), &val))) return std::nullopt;
            return val;
        }
    }

    // pair<clsid, name>
    static std::vector<std::pair<CLSID, std::wstring>> EnumerateAsioDrivers()
    {
        std::vector<std::pair<CLSID, std::wstring>> result{};

        const auto root = win32::registry::OpenKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Asio");
        if (!root) return result; // No Asio drivers installed.

        for (size_t i = 0; ; ++i)
        {
            // get drv_key name
            const auto drv_key_name = win32::registry::EnumKeyName(root.get(), i);
            if (!drv_key_name) break; // no more keys

            // open drv key
            const auto drv_key = win32::registry::OpenKey(root.get(), drv_key_name->c_str());
            if (!drv_key) continue; // failed?

            // get clsid, desc
            const auto clsid = win32::registry::ReadGuidValue(drv_key.get(), L"clsid");
            const auto desc = win32::registry::ReadStringValue(drv_key.get(), L"description");
            if (!clsid || !desc) continue; // failed?

            result.emplace_back(*clsid, *desc);
        }

        return result;
    }
}
