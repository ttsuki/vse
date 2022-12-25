/// @file
/// @brief  Vse - Source Switcher
/// @author (C) 2022 ttsuki

#include "SourceSwitcher.h"

#include <memory>
#include <vector>

#include "../base/WaveFormat.h"
#include "../base/IWaveSource.h"

#include "../base/xtl/xtl_spin_lock_mutex.h"

namespace vse
{
    std::shared_ptr<ISourceSwitcher> CreateSourceSwitcher(PcmWaveFormat format)
    {
        class SourceSwitcherImpl : public ISourceSwitcher
        {
            xtl::spin_lock_mutex mutex_{};
            PcmWaveFormat format_{};
            std::shared_ptr<IWaveSource> current_{};
        public:
            explicit SourceSwitcherImpl(PcmWaveFormat format) : format_(std::move(format)) {}

            [[nodiscard]] PcmWaveFormat GetFormat() const override { return format_; }

            [[nodiscard]] size_t Read(void* buffer, size_t buffer_length) override
            {
                xtl::lock_guard lock(mutex_);
                return current_ ? current_->Read(buffer, buffer_length) : 0;
            }

            std::shared_ptr<IWaveSource> Current() override
            {
                return current_;
            }

            void Assign(std::shared_ptr<IWaveSource> source) override
            {
                xtl::lock_guard lock(mutex_);
                current_ = source;
            }
        };

        return std::make_shared<SourceSwitcherImpl>(format);
    }

    std::shared_ptr<ISourceSwitcher> CreateSourceSwitcher(std::shared_ptr<IWaveSource> initial_source)
    {
        auto sw = CreateSourceSwitcher(initial_source->GetFormat());
        sw->Assign(initial_source);
        return sw;
    }
}
