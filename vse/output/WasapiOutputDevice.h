/// @file
/// @brief  Vse - WASAPI Output Device
/// @author (C) 2022 ttsuki

#pragma once

#include "IOutputDevice.h"
#include <memory>
#include <chrono>

namespace vse
{
    class WasapiOutputDevice : public virtual IOutputDevice
    {
    public:
        /// Open the device with WASAPI Shared mode.
        /// The buffer format will set to the device default format.
        /// The format is available via OutputDevice::GetFormat API.
        using IOutputDevice::Open;

        /// Open the device with WASAPI Shared mode with auto format conversion.
        /// Auto format conversion is supported since Windows 10.0.14393.0
        [[nodiscard]]
        virtual bool Open(const WAVEFORMATEXTENSIBLE& format) = 0;

        /// Open the device with WASAPI Exclusive mode.
        /// If the device does not support the format, the function will fail.
        [[nodiscard]]
        virtual bool OpenExclusive(const WAVEFORMATEXTENSIBLE& format) = 0;

        /// Open the device with WASAPI Exclusive mode.
        /// If the device does not support the format, the function will fail.
        [[nodiscard]]
        virtual bool OpenExclusive(const WAVEFORMATEXTENSIBLE& format, std::chrono::microseconds device_period) = 0;
    };

    std::shared_ptr<WasapiOutputDevice> CreateWasapiOutputDevice();
}
