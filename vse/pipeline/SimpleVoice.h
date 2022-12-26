/// @file
/// @brief  Vse - SimpleVoice
/// @author (C) 2022 ttsuki

#pragma once

#include <memory>

#include "../base/WaveFormat.h"
#include "../base/IWaveSource.h"
#include "./StereoWaveMixer.h"

namespace vse
{
    class SimpleVoice : protected virtual Interface
    {
    public:
        virtual std::shared_ptr<IRandomAccessWaveBuffer> GetUpstreamBuffer() = 0;
        virtual std::shared_ptr<IStereoWaveMixer> GetTargetMixer() = 0;

        // play/stop control
        virtual void Play() noexcept = 0;
        virtual void PlayIfNotPlaying() noexcept = 0;
        virtual void Pause() noexcept = 0;
        virtual void Stop() noexcept = 0;
        virtual bool IsPlaying() const noexcept = 0;

        // cursor control
        virtual void SetPlayPosition(ptrdiff_t samples) noexcept = 0;
        virtual ptrdiff_t GetPlayPosition() const noexcept = 0;
        virtual void SetPlayPositionInSeconds(double seconds) noexcept = 0;
        virtual double GetPlayPositionInSeconds() const noexcept = 0;

        // volume control
        virtual float GetVolume() const = 0;   // [ 0.0 .. 1.0 .. +inf]
        virtual void SetVolume(float vol) = 0; // [ 0.0 .. 1.0 .. +inf]
        virtual float GetPan() const = 0;      // [-1.0 .. 0.0 .. +1.0]
        virtual void SetPan(float pan) = 0;    // [-1.0 .. 0.0 .. +1.0]

        // get exclusive lock from other threads
        virtual void lock() noexcept = 0;
        virtual void unlock() noexcept = 0;
    };

    std::shared_ptr<SimpleVoice> CreateVoice(
        std::shared_ptr<IRandomAccessWaveBuffer> source_buffer,
        std::shared_ptr<IStereoWaveMixer> target_mixer);
}
