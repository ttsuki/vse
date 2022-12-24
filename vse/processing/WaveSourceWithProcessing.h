/// @file
/// @brief  Vse - WaveProcessingWaveSource
/// @author (C) 2022 ttsuki

#pragma once

#include <memory>

#include "../base/IWaveSource.h"
#include "../base/IWaveProcessor.h"

namespace vse
{
    [[nodiscard]] std::shared_ptr<IWaveSource> CreateSourceWithProcessing(
        std::shared_ptr<IWaveSource> source,
        std::shared_ptr<IWaveProcessor> processor);
}
