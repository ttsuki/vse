/// @file
/// @brief  Vse - Format converter
/// @author (C) 2022 ttsuki

#pragma once

#include "../base/IWaveProcessor.h"

namespace vse
{
    /// Creates thru (no-operation thru input to output) processor
    /// @param format source/destination format
    [[nodiscard]] std::shared_ptr<IWaveProcessor> CreateThru(
        PcmWaveFormat format);

    /// Creates bit-depth converter
    /// @param input_format source format
    /// @param output_format destination format
    [[nodiscard]] std::shared_ptr<IWaveProcessor> CreateBitDepthConverter(
        PcmWaveFormat input_format,
        SampleType output_format);

    /// Creates Microsoft Audio Resampling DSP.
    /// @param input_format source format
    /// @param output_format destination format
    /// @param quality_level 1-60 inclusive, 1 is linear interpolation, 60 is most high quality.
    [[nodiscard]] std::shared_ptr<IWaveProcessor> CreateAudioResamplingDsp(
        PcmWaveFormat input_format,
        PcmWaveFormat output_format,
        int quality_level = 60);


    [[nodiscard]] std::shared_ptr<IWaveProcessor> CreateFormatConverter(
        const WAVEFORMATEXTENSIBLE& input_format,
        const WAVEFORMATEXTENSIBLE& output_format);
}
