/// @file
/// @brief  Vse - WaveProcessingWaveSource
/// @author (C) 2022 ttsuki

#pragma once

#include <memory>

#include "../base/IWaveSource.h"
#include "../base/IWaveProcessor.h"

namespace vse
{
    class IWaveSourceWithProcessing : public IWaveSource
    {
    public:
        [[nodiscard]] virtual std::shared_ptr<IWaveSource> GetUpstreamWaveSource() = 0;
        [[nodiscard]] virtual std::shared_ptr<IWaveProcessor> GetAttachedProcessor() = 0;
    };

    [[nodiscard]] std::shared_ptr<IWaveSourceWithProcessing> CreateSourceWithProcessing(
        std::shared_ptr<IWaveSource> source,
        std::shared_ptr<IWaveProcessor> processor);
}
