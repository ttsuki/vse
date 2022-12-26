/// @file
/// @brief  Vse - StereoWaveMixer
/// @author (C) 2022 ttsuki

#include "StereoWaveMixer.h"

#include <memory>
#include <set>
#include <vector>
#include <mutex>
#include <algorithm>
#include <utility>
#include <stdexcept>

#include "../base/xtl/xtl_spin_lock_mutex.h"
#include "../base/xtl/xtl_temp_memory_buffer.h"
#include "../processing/WaveformProcessing.h"

namespace vse
{
    template <class TSample>
    class StereoWaveMixerImpl : public virtual IStereoWaveMixer
    {
        xtl::spin_lock_mutex mutex_{};
        PcmWaveFormat format_{};
        std::vector<std::shared_ptr<IWaveSource>> incoming_{};
        std::vector<std::shared_ptr<IWaveSource>> outgoing_{};
        std::set<std::shared_ptr<IWaveSource>> running_{};
        xtl::temp_memory_buffer source_buffer_{};
        xtl::temp_memory_buffer mixing_buffer_{};

    public:
        explicit StereoWaveMixerImpl(PcmWaveFormat format)
            : format_(format) { }

        [[nodiscard]] PcmWaveFormat GetFormat() const override
        {
            return format_;
        }

        size_t Read(void* buffer, size_t buffer_length) override
        {
            if (std::unique_lock lock(mutex_, std::try_to_lock); lock.owns_lock())
            {
                for (auto& i : outgoing_) running_.erase(i);
                for (auto& i : incoming_) running_.emplace(std::move(i));
                incoming_.clear();
                outgoing_.clear();
            }

            auto* src = source_buffer_.get<TSample>(buffer_length / sizeof(TSample));
            auto* mix = mixing_buffer_.get<TSample>(buffer_length / sizeof(TSample));
            std::memset(mix, 0, buffer_length);

            for (auto&& i : running_)
            {
                float lch_mix = 1.0f;
                float rch_mix = 1.0f;

                size_t bytes = 0;
                if (auto stereo = dynamic_cast<IStereoWaveSource*>(i.get()))
                    bytes = stereo->Read(src, buffer_length, &lch_mix, &rch_mix);
                else
                    bytes = i->Read(src, buffer_length);

                processing::MixStereo(mix, src, bytes / sizeof(TSample), lch_mix, rch_mix);
            }

            std::memcpy(buffer, mix, buffer_length);
            return buffer_length;
        }

        void RegisterSource(std::shared_ptr<IWaveSource> source) override
        {
            if (source->GetFormat() != format_)
                throw std::runtime_error("invalid format");

            xtl::lock_guard lock(mutex_);
            incoming_.emplace_back(std::move(source));
        }

        void DeregisterSource(std::shared_ptr<IWaveSource> source) override
        {
            xtl::lock_guard lock(mutex_);

            if (auto it = std::find(incoming_.begin(), incoming_.end(), source); it != incoming_.end())
                incoming_.erase(it);

            if (auto it = running_.find(source); it != running_.end())
                outgoing_.emplace_back(std::move(source));
        }
    };

    std::shared_ptr<IStereoWaveMixer> CreateStereoWaveMixer(PcmWaveFormat format)
    {
        //if (format.SampleType() == SampleType::S16 && format.ChannelCount() == 2) return std::make_shared<StereoMixerImpl<S16>>(format);
        //if (format.SampleType() == SampleType::S32 && format.ChannelCount() == 2) return std::make_shared<StereoMixerImpl<S32>>(format);
        if (format.SampleType() == SampleType::F32 && format.ChannelCount() == 2) return std::make_shared<StereoWaveMixerImpl<F32Stereo>>(format);
        throw std::runtime_error("not implemented for given format.");
    }
}
