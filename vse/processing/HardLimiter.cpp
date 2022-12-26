/// @file
/// @brief  Vse - Hard Limiter
/// @author (C) 2022 ttsuki

#include "HardLimiter.h"

#include "./WaveformProcessing.h"

namespace vse
{
    namespace
    {
        template <class TSample>
        class HardLimitImpl final : public virtual IHardLimiter
        {
            PcmWaveFormat format_;
            std::atomic<HardLimiterParameters> params_;

        public:
            HardLimitImpl(PcmWaveFormat format, HardLimiterParameters params) : format_(format), params_(params) {}

            [[nodiscard]] PcmWaveFormat GetInputFormat() const override { return format_; }
            [[nodiscard]] PcmWaveFormat GetOutputFormat() const override { return format_; }

            [[nodiscard]] size_t Process(size_t (*read_source)(void* context, void* buffer, size_t buffer_length), void* context, void* destination_buffer, size_t destination_buffer_length) override
            {
                size_t sz = read_source(context, destination_buffer, destination_buffer_length);
                auto param = GetParameters();
                auto dst = static_cast<TSample*>(destination_buffer);

                processing::ProcessHardLimit(dst, dst, sz / sizeof(TSample), param.PreAmpMultiplier, param.LimiterAbs);

                return sz;
            }

            [[nodiscard]] HardLimiterParameters GetParameters() const override { return params_.load(std::memory_order_acquire); }
            void SetParameters(HardLimiterParameters parameters) override { params_.store(parameters, std::memory_order_release); }
        };
    }

    std::shared_ptr<IHardLimiter> CreateHardLimiter(PcmWaveFormat format, HardLimiterParameters initialParameters)
    {
        // select implementation
        //if (format.SampleType() == SampleType::S16) return std::make_shared<HardLimitImpl<S16>>(format, initialParameters);
        //if (format.SampleType() == SampleType::S32) return std::make_shared<HardLimitImpl<S32>>(format, initialParameters);
        if (format.SampleType() == SampleType::F32) return std::make_shared<HardLimitImpl<F32>>(format, initialParameters);

        // not implemented yet.
        throw std::invalid_argument("not implemented");
    }
}
