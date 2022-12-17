/// @file
/// @brief  Vse - Audio Rendering Thread
/// @author (C) 2022 ttsuki

#pragma once

#include "../base/Interface.h"
#include "../base/IWaveSource.h"
#include "./IOutputDevice.h"

namespace vse
{
    class AudioRenderingThread : protected virtual Interface
    {
    public:
        /// Starts rendering.
        virtual void Start() = 0;

        /// Stops rendering.
        virtual void Stop() = 0;
    };

    /// Create a thread that pumps audio data from source source to destination output device.
    /// @param source source source.
    /// @param destination destination output device.
    /// @returns AudioRenderingThread
    std::shared_ptr<AudioRenderingThread> CreateAudioRenderingThread(
        std::shared_ptr<IWaveSource> source,
        std::shared_ptr<IOutputDevice> destination);
}
