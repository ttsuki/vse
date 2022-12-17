#pragma once

#include <cmath>

#include "../../vse/base/WaveFormat.h"
#include "../../vse/base/IWaveSource.h"

namespace vse_tests
{
    class SineWave16bitStereoSource final : public vse::IWaveSource
    {
        const vse::PcmWaveFormat format_;
        const double omega_ = 0;
        double theta_ = 0;

    public:
        explicit SineWave16bitStereoSource(float frequency = 440.0f)
            : format_(vse::SampleType::S16, vse::SpeakerBit::SpeakerSet_2_0ch, 44100)
            , omega_(static_cast<double>(frequency) * (2 * std::acos(-1.0)) / format_.SamplingFrequency()) { }

        [[nodiscard]] vse::PcmWaveFormat GetFormat() const override
        {
            return format_;
        }

        [[nodiscard]] size_t Read(void* buffer, size_t buffer_length) override
        {
            auto buf = static_cast<vse::S16Stereo*>(buffer);
            auto cnt = buffer_length / sizeof(vse::S16Stereo);

            auto omega = omega_; // load from member
            auto theta = theta_; // load from member

            for (size_t i = 0; i < cnt; i++)
            {
                buf[i].l = static_cast<vse::S16>(std::sin(theta) * 16384.0);
                buf[i].r = static_cast<vse::S16>(std::sin(theta) * 16384.0);
                theta += omega;
            }

            theta_ = theta; // store to member
            return buffer_length;
        }
    };
}
