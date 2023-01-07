/// @file
/// @brief  Vse - Asio Output Device
/// @author (C) 2023 ttsuki

#pragma once

#include "./IOutputDevice.h"
#include <memory>
#include <vector>

namespace vse
{
    struct AsioDeviceDescription
    {
        GUID Id;
        std::string DeviceName;
    };

    std::vector<AsioDeviceDescription> EnumerateAllAsioDevices();

    class AsioOutputDevice : public virtual IOutputDevice
    {
    private:
        [[nodiscard]] bool Open() override = 0;

    public:
        /// Open ASIO device
        [[nodiscard]] virtual bool Open(int sampling_frequency, int output_channel_count) = 0;

        /// Open ASIO control panel
        virtual void OpenControlPanel() = 0;

        /// Get output latency in samples
        [[nodiscard]] virtual int GetLatency() const = 0;
    };

    std::shared_ptr<AsioOutputDevice> CreateAsioOutputDevice(GUID id);
}
