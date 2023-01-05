/// @file
/// @brief  winasio - platform header
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>
#include <combaseapi.h>

namespace asio
{
    typedef LONG AsioLong;
    typedef LONGLONG AsioLongLong;

    typedef AsioLong AsioBool;
    typedef double AsioSamplingRate;
    typedef AsioLongLong AsioSamples;
    typedef AsioLongLong AsioTimestamp;
    typedef void AsioTime;

    enum AsioError : AsioLong
    {
        ASE_OK = 0
    };

    enum struct AsioSampleType : AsioLong
    {
        Int16MSB = 0,
        Int24MSB = 1,
        Int32MSB = 2,
        Float32MSB = 3,
        Float64MSB = 4,
        Int32MSB16 = 8,
        Int32MSB18 = 9,
        Int32MSB20 = 10,
        Int32MSB24 = 11,
        Int16LSB = 16,
        Int24LSB = 17,
        Int32LSB = 18,
        Float32LSB = 19,
        Float64LSB = 20,
        Int32LSB16 = 24,
        Int32LSB18 = 25,
        Int32LSB20 = 26,
        Int32LSB24 = 27,
    };

    struct AsioClockSource
    {
        AsioLong index;
        AsioLong associated_channel;
        AsioLong associated_group;
        AsioBool is_current_source;
        char name[32];
    };

    struct AsioChannelInfo
    {
        AsioLong channel;
        AsioBool is_input;
        AsioBool is_active;
        AsioLong channel_group;
        AsioSampleType type;
        char name[32];
    };

    struct AsioBufferInfo
    {
        AsioBool is_input;
        AsioLong channel_number;
        void* buffers[2];
    };

    enum struct AsioMessage : AsioLong
    {
        SelectorSupported = 1,
        EngineVersion,
        ResetRequest,
        BufferSizeChange,
        ResyncRequest,
        LatenciesChanged,
        SupportsTimeInfo,
        SupportsTimeCode,
    };

    struct AsioCallbacks
    {
        void (__cdecl* buffer_switched)(AsioLong buffer_index, AsioBool do_direct_processing);
        void (__cdecl* sampling_rate_changed)(AsioSamplingRate new_sampling_rate);
        AsioLong (__cdecl* handle_message)(AsioMessage selector, AsioLong value, void* message, double* opt);
        AsioTime* (__cdecl* buffer_switched_with_timeinfo)(AsioTime* params, AsioLong buffer_index, AsioBool do_direct_processing);
    };

    interface __declspec(novtable) IAsio : IUnknown
    {
        virtual __declspec(nothrow) AsioBool __thiscall Initialize(HWND main_window) = 0;
        virtual __declspec(nothrow) void __thiscall GetDriverName(char* name) = 0;
        virtual __declspec(nothrow) AsioLong __thiscall GetDriverVersion() = 0;
        virtual __declspec(nothrow) void __thiscall GetErrorMessage(char* string) = 0;
        virtual __declspec(nothrow) AsioError __thiscall Start() = 0;
        virtual __declspec(nothrow) AsioError __thiscall Stop() = 0;
        virtual __declspec(nothrow) AsioError __thiscall GetChannels(AsioLong* input_channel_count, AsioLong* output_channel_count) = 0;
        virtual __declspec(nothrow) AsioError __thiscall GetLatencies(AsioLong* input_latency, AsioLong* output_latency) = 0;
        virtual __declspec(nothrow) AsioError __thiscall GetBufferSize(AsioLong* minimum_size, AsioLong* maximum_size, AsioLong* preferred_size, AsioLong* granularity) = 0;
        virtual __declspec(nothrow) AsioError __thiscall CanSampleRate(AsioSamplingRate sampling_rate) = 0;
        virtual __declspec(nothrow) AsioError __thiscall GetSampleRate(AsioSamplingRate* sampling_rate) = 0;
        virtual __declspec(nothrow) AsioError __thiscall SetSampleRate(AsioSamplingRate sampling_rate) = 0;
        virtual __declspec(nothrow) AsioError __thiscall GetClockSources(AsioClockSource* clocks, AsioLong* numSources) = 0;
        virtual __declspec(nothrow) AsioError __thiscall SetClockSource(AsioLong reference) = 0;
        virtual __declspec(nothrow) AsioError __thiscall GetSamplePosition(AsioSamples* position, AsioTimestamp* timestamp) = 0;
        virtual __declspec(nothrow) AsioError __thiscall GetChannelInfo(AsioChannelInfo* info) = 0;
        virtual __declspec(nothrow) AsioError __thiscall CreateBuffers(AsioBufferInfo* buffer_info, AsioLong channel_count, AsioLong buffer_size, AsioCallbacks* callbacks) = 0;
        virtual __declspec(nothrow) AsioError __thiscall DisposeBuffers() = 0;
        virtual __declspec(nothrow) AsioError __thiscall OpenControlPanel() = 0;
        virtual __declspec(nothrow) AsioError __thiscall Future(AsioLong selector, void* opt) = 0;
        virtual __declspec(nothrow) AsioError __thiscall NotifyOutputReady() = 0;
    };
}
