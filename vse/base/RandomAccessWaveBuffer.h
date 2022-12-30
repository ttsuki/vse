/// @file
/// @brief  Vse - Random Access Wave Buffer
/// @author (C) 2022 ttsuki

#pragma once

#include <memory>
#include "WaveFormat.h"
#include "IWaveSource.h"

namespace vse
{
    [[nodiscard]] std::shared_ptr<IRandomAccessWaveBuffer> AllocateWaveBuffer(PcmWaveFormat format);
    [[nodiscard]] std::shared_ptr<IRandomAccessWaveBuffer> ReadOutToMemory(std::shared_ptr<IWaveSource> input);
    [[nodiscard]] std::shared_ptr<IRandomAccessWaveBuffer> DuplicateBuffer(std::shared_ptr<IRandomAccessWaveBuffer> source);
    [[nodiscard]] std::shared_ptr<ISeekableWaveSource> AllocateReadCursor(std::shared_ptr<IRandomAccessWaveBuffer> buffer);
}
