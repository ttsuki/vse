/// @file
/// @brief  Vse - DirectSound Output Device
/// @author (C) 2022 ttsuki

#pragma once

#include "IOutputDevice.h"
#include <memory>

namespace vse
{
    class DirectSoundOutputDevice : public IOutputDevice
    {
    public:
        [[nodiscard]] virtual bool Open(const WAVEFORMATEXTENSIBLE& format) = 0;
    };

    std::shared_ptr<DirectSoundOutputDevice> CreateDirectSoundOutputDevice();
}
