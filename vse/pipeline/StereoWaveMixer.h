/// @file
/// @brief  Vse - StereoWaveMixer
/// @author (C) 2022 ttsuki

#pragma once

#include "../base/IWaveSource.h"

namespace vse
{
    class IStereoWaveSource : public virtual IWaveSource
    {
    public:
        using IWaveSource::GetFormat;
        using IWaveSource::Read;
        [[nodiscard]] virtual size_t Read(void* buffer, size_t buffer_length, float* lch_mix, float* rch_mix) = 0;
    };

    class IStereoWaveMixer : public IWaveSource
    {
    public:
        virtual void RegisterSource(std::shared_ptr<IWaveSource> source) = 0;
        virtual void DeregisterSource(std::shared_ptr<IWaveSource> source) = 0;
    };

    std::shared_ptr<IStereoWaveMixer> CreateStereoWaveMixer(PcmWaveFormat format);
}
