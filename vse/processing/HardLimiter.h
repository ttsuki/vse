/// @file
/// @brief  Vse - Hard Limiter
/// @author (C) 2022 ttsuki

#pragma once

#include "../base/WaveFormat.h"
#include "../base/IWaveProcessor.h"

namespace vse
{
    struct HardLimiterParameters
    {
        float PreAmpMultiplier = 1.0f;
        float LimiterAbs = 0.999f;
    };

    using IHardLimiter = IParametricWaveProcessor<HardLimiterParameters>;

    std::shared_ptr<IHardLimiter> CreateHardLimiter(
        PcmWaveFormat format,
        HardLimiterParameters initialParameters = HardLimiterParameters{});
}
