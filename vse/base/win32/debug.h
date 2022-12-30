/// @file
/// @brief  win32 debug
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>
#include <iomanip>

#include "../xtl/xtl_ostream.h"
#include "../xtl/xtl_timestamp.h"

namespace vse::debug_helper
{
    template <class T, class F>
    static constexpr auto make_percent_operator_redirection(F&& f)
    {
        struct percent_operator_redirection
        {
            F f;
            constexpr explicit percent_operator_redirection(F f) : f(std::move(f)) {}
            constexpr T operator %(T r) const { return f(r), r; }
        };

        return percent_operator_redirection{std::forward<F>(f)};
    }
}

#ifndef NDEBUG
#define VSE_DEBUG_LOG(PREFIX) ::vse::xtl::make_stream_with_prefix(&::OutputDebugStringA, "[" + ::vse::xtl::timestamp::now().to_localtime_string() + ("] " PREFIX))
#define VSE_EXPECT_SUCCESS (::vse::debug_helper::make_percent_operator_redirection<::HRESULT>([](::HRESULT r) { if (FAILED(r)) { VSE_DEBUG_LOG("VSE_EXPECT_SUCCESS FAILED! at ") << __FILE__ << ":" << __LINE__ << " HRESULT=0x" << ::std::hex << ::std::setw(8) << r ; if (::IsDebuggerPresent()) { ::DebugBreak(); } } }))%
#define VSE_DEBUG_BREAK() (::IsDebuggerPresent() ? ::DebugBreak() : void(0))
#else

namespace vse::xtl
{
    struct null_ostream{};
    template <class T> null_ostream operator <<(null_ostream, T&&) { return {}; }
}

#define VSE_DEBUG_LOG(PREFIX) (::vse::xtl::null_ostream{})
#define VSE_EXPECT_SUCCESS
#define VSE_DEBUG_BREAK() void(0)
#endif
