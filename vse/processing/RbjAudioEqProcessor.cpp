/// @file
/// @brief  Vse - RBJ's Audio-EQ-Cookbook
/// @author (C) 2022 ttsuki
///
/// An implementation of RBJ's Audio-EQ-Cookbook
/// https://webaudio.github.io/Audio-EQ-Cookbook/Audio-EQ-Cookbook.txt

#include "RbjAudioEqProcessor.h"

namespace vse
{
    std::shared_ptr<IWaveProcessor> CreateBiquadIirFilter(PcmWaveFormat format, rbj_audio_eq::Coefficients coefficients)
    {
        if (format.SampleType() != SampleType::F32 || format.ChannelCount() > 8)
            throw std::invalid_argument("not supported.");

        class BiquadIirFilterImpl : public IWaveProcessor
        {
            const PcmWaveFormat format_{};
            const rbj_audio_eq::Coefficients parameters_{};
            std::array<rbj_audio_eq::ShiftRegister, 8> registers_{};

        public:
            BiquadIirFilterImpl(PcmWaveFormat format, rbj_audio_eq::Coefficients parameters) : format_(format), parameters_(parameters) { }

            [[nodiscard]] PcmWaveFormat GetInputFormat() const override { return format_; }
            [[nodiscard]] PcmWaveFormat GetOutputFormat() const override { return format_; }

            [[nodiscard]] size_t Process(
                size_t (* read_source)(void* context, void* buffer, size_t buffer_length), void* context,
                void* destination_buffer, size_t destination_buffer_length) override
            {
                const auto coeff = parameters_;
                const int channels = format_.ChannelCount();
                const int block_align = format_.BlockAlign();
                const size_t bytes = read_source(context, destination_buffer, destination_buffer_length / block_align * block_align);

                auto* buf = static_cast<F32*>(destination_buffer);
                const size_t sample_count = bytes / block_align;
                for (int i = 0; i < channels; ++i)
                    registers_[i] = rbj_audio_eq::ProcessSingleChannel(registers_[i], coeff, buf + i, buf + i, sample_count, channels);

                return bytes;
            }
        };

        return std::make_shared<BiquadIirFilterImpl>(format, coefficients);
    }
}
