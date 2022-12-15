/// @file
/// @brief  win32 debug
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>
#include <iomanip>

#include "../xtl/xtl_ostream.h"
#include "../xtl/xtl_timestamp.h"

#ifndef NDEBUG
#define VSE_DEBUG_LOG(PREFIX) ::vse::xtl::make_stream_with_prefix(&::OutputDebugStringA, "[" + ::vse::xtl::timestamp::now().to_localtime_string() + ("] " PREFIX))
#define VSE_EXPECT_SUCCESS (([]() { struct X { ::HRESULT operator %(::HRESULT r) const { if (FAILED(r)) { VSE_DEBUG_LOG("VSE_EXPECT_SUCCESS FAILED! at ") << __FILE__ << ":" << __LINE__ << " HRESULT=0x" << ::std::hex << ::std::setw(8) << r ; if (::IsDebuggerPresent()) { ::DebugBreak(); } } return r; } }; return X(); })())%
#else

namespace vse::xtl
{
    struct null_ostream{};
    template <class T> null_ostream operator <<(null_ostream, T&&) { return {}; }
}

#define VSE_DEBUG_LOG(PREFIX) (::vse::xtl::null_ostream{})
#define VSE_EXPECT_SUCCESS
#endif
