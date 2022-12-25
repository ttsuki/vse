/// @file
/// @brief  Vse - Source Switcher
/// @author (C) 2022 ttsuki

#pragma once

#include <cstddef>

#include "../base/WaveFormat.h"
#include "../base/IWaveSource.h"

namespace vse
{
    class ISourceSwitcher : public IWaveSource
    {
    public:
        virtual std::shared_ptr<IWaveSource> Current() = 0;
        virtual void Assign(std::shared_ptr<IWaveSource> source) = 0;
    };

    std::shared_ptr<ISourceSwitcher> CreateSourceSwitcher(PcmWaveFormat format );
    std::shared_ptr<ISourceSwitcher> CreateSourceSwitcher(std::shared_ptr<IWaveSource> initial_source);
}
