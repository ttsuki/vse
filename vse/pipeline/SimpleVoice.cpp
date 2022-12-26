/// @file
/// @brief  Vse - SimpleVoice
/// @author (C) 2022 ttsuki

#include "SimpleVoice.h"

#include "../base/xtl/xtl_spin_lock_mutex.h"
#include "../decoding/WaveFileLoader.h"
#include "../processing/WaveFormatConverter.h"
#include "VolumeCalculation.h"

namespace vse
{
    std::shared_ptr<SimpleVoice> CreateVoice(std::shared_ptr<IRandomAccessWaveBuffer> source_buffer, std::shared_ptr<IStereoWaveMixer> target_mixer)
    {
        class SimpleVoiceImpl
            : public SimpleVoice
            , public IStereoWaveSource
            , public std::enable_shared_from_this<SimpleVoiceImpl>
        {
            std::shared_ptr<IRandomAccessWaveBuffer> source_{};
            PcmWaveFormat source_format_{};
            std::weak_ptr<IStereoWaveMixer> mixer_{};
            PcmWaveFormat mixing_format_{};
            xtl::recursive_spin_lock_mutex mutex_{};
            std::shared_ptr<IWaveProcessor> format_converter_{};

            bool is_playing_{};
            ptrdiff_t cursor_{0};

            float volume_{1.0f};
            float pan_{0.0f};

        public:
            SimpleVoiceImpl(std::shared_ptr<IRandomAccessWaveBuffer> source, std::shared_ptr<IStereoWaveMixer> mixer)
                : source_(source)
                , source_format_(source->GetFormat())
                , mixer_(mixer)
                , mixing_format_(mixer->GetFormat())
                , format_converter_(CreateFormatConverter(source_format_, mixing_format_)) {}

        private:
            void RegisterToMixer()
            {
                if (auto mixer = mixer_.lock())
                {
                    mixer->RegisterSource(shared_from_this());
                    is_playing_ = true;
                }
            }

            void DeregisterFromMixer()
            {
                if (auto mixer = mixer_.lock())
                {
                    mixer->DeregisterSource(shared_from_this());
                    is_playing_ = false;
                }
            }

            void ResetCursor(size_t cursor_in_bytes)
            {
                cursor_ = cursor_in_bytes;
                format_converter_->Discontinuity();
            }

        public: // SimpleVoice
            std::shared_ptr<IRandomAccessWaveBuffer> GetUpstreamBuffer() override { return source_; }
            std::shared_ptr<IStereoWaveMixer> GetTargetMixer() override { return mixer_.lock(); }

            void Play() noexcept override
            {
                xtl::lock_guard lock(mutex_);
                ResetCursor(0);
                RegisterToMixer();
            }

            void PlayIfNotPlaying() noexcept override
            {
                xtl::lock_guard lock(mutex_);
                RegisterToMixer();
            }

            void Pause() noexcept override
            {
                xtl::lock_guard lock(mutex_);
                DeregisterFromMixer();
            }

            void Stop() noexcept override
            {
                xtl::lock_guard lock(mutex_);
                DeregisterFromMixer();
                ResetCursor(0);
            }

            bool IsPlaying() const noexcept override
            {
                return is_playing_;
            }

            void SetPlayPosition(ptrdiff_t samples) noexcept override
            {
                xtl::lock_guard lock(mutex_);
                if (auto requested = samples * source_format_.BlockAlign();
                    abs(requested - cursor_) > 1)
                    ResetCursor(requested);
            }

            ptrdiff_t GetPlayPosition() const noexcept override { return cursor_ / source_format_.BlockAlign(); }
            void SetPlayPositionInSeconds(double seconds) noexcept override { SetPlayPosition(static_cast<ptrdiff_t>(std::floor(seconds * source_format_.SamplingFrequency()))); }
            double GetPlayPositionInSeconds() const noexcept override { return static_cast<double>(cursor_) / source_format_.AvgBytesPerSec(); }
            float GetVolume() const override { return volume_; }
            void SetVolume(float vol) override { volume_ = vol; }
            float GetPan() const override { return pan_; }
            void SetPan(float pan) override { pan_ = pan; }

            void lock() noexcept override { mutex_.lock(); }
            void unlock() noexcept override { mutex_.unlock(); }

        public: // IStereoWaveSource
            [[nodiscard]] PcmWaveFormat GetFormat() const override { return mixing_format_; }

            [[nodiscard]] size_t Read(void* buffer, size_t length) override
            {
                xtl::lock_guard lock(mutex_);
                auto sz = format_converter_->Process(
                    [this](void* buf, size_t len)
                    {
                        size_t silent = 0;
                        if (cursor_ < 0)
                        {
                            silent = std::min(static_cast<size_t>(- cursor_), len);
                            memset(buf, 0, silent);
                            buf = static_cast<std::byte*>(buf) + silent;
                            len -= silent;
                            cursor_ += static_cast<ptrdiff_t>(silent);
                        }

                        // TODO: loop sequence control
                        size_t read = source_->Read(buf, cursor_, len);
                        cursor_ += static_cast<ptrdiff_t>(read);

                        return silent + read;
                    }, buffer, length);

                if (sz == 0) { Stop(); } // reached to end of stream
                return sz;
            }

            [[nodiscard]] size_t Read(void* buffer, size_t length, float* lch_mix, float* rch_mix) override
            {
                size_t ret = this->Read(buffer, length);
                CalculateStereoVolume(volume_, pan_, lch_mix, rch_mix);
                return ret;
            }
        };

        return std::make_shared<SimpleVoiceImpl>(std::move(source_buffer), std::move(target_mixer));
    }
}
