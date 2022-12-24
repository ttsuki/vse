/// @file
/// @brief  Vse - WaveProcessingWaveSource
/// @author (C) 2022 ttsuki

#include "WaveSourceWithProcessing.h"

#include <memory>
#include <stdexcept>

#include "../base/WaveFormat.h"
#include "../base/IWaveSource.h"
#include "../base/IWaveProcessor.h"

namespace vse
{
    std::shared_ptr<IWaveSource> CreateSourceWithProcessing(
        std::shared_ptr<IWaveSource> source,
        std::shared_ptr<IWaveProcessor> processor)
    {
        class Proc : public IWaveSource
        {
        public:
            std::shared_ptr<IWaveSource> source_;
            std::shared_ptr<IWaveProcessor> processor_;

            Proc(std::shared_ptr<IWaveSource> source, std::shared_ptr<IWaveProcessor> processor)
                : source_(std::move(source))
                , processor_(std::move(processor)) {}

            [[nodiscard]] PcmWaveFormat GetFormat() const override
            {
                return processor_->GetOutputFormat();
            }

            [[nodiscard]] size_t Read(void* buffer, size_t buffer_length) override
            {
                return processor_->Process([&](void* buf, size_t sz) { return source_->Read(buf, sz); }, buffer, buffer_length);
            }
        };

        if (source->GetFormat() != processor->GetInputFormat())
            throw std::logic_error("The source format and the processor input format isn't match.");

        return std::make_shared<Proc>(std::move(source), std::move(processor));
    }
}
