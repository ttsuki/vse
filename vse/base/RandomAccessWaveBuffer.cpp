/// @file
/// @brief  Vse - Random Access Wave Buffer
/// @author (C) 2022 ttsuki

#include "RandomAccessWaveBuffer.h"

#include <cstddef>
#include <memory>
#include <utility>
#include <algorithm>

#include "xtl/xtl_memory_stream.h"
#include "xtl/xtl_temp_memory_buffer.h"

namespace vse
{
    std::shared_ptr<IRandomAccessWaveBuffer> AllocateWaveBuffer(PcmWaveFormat format)
    {
        class RandomAccessWaveBufferImpl : public IRandomAccessWaveBuffer
        {
            PcmWaveFormat format_{};
            xtl::random_access_memory_stream ms_{};

        public:
            RandomAccessWaveBufferImpl(const PcmWaveFormat& format) : format_(format) {}
            [[nodiscard]] PcmWaveFormat GetFormat() const override { return format_; }
            [[nodiscard]] size_t Read(void* buffer, size_t cursor, size_t length) const noexcept override { return ms_.read(buffer, cursor, length); }
            [[nodiscard]] size_t Write(const void* buffer, size_t cursor, size_t length) noexcept override { return ms_.write(buffer, cursor, length); }
            [[nodiscard]] size_t Size() const override { return ms_.size(); }
            [[nodiscard]] size_t Resize(size_t length) override { return ms_.resize(length); }
        };

        return std::make_shared<RandomAccessWaveBufferImpl>(format);
    }

    std::shared_ptr<IRandomAccessWaveBuffer> ReadOutToMemory(std::shared_ptr<IWaveSource> input)
    {
        if (!input) return nullptr;

        auto destination = AllocateWaveBuffer(input->GetFormat());
        auto temp_buffer = xtl::temp_memory_buffer();
        while (true)
        {
            constexpr size_t buf_size = 65536;
            void* p = temp_buffer.get(buf_size);
            size_t sz = sz = input->Read(p, buf_size);
            (void)destination->Write(p, destination->Size(), sz);
            if (sz == 0) break; // end
        }

        return destination;
    }

    std::shared_ptr<IRandomAccessWaveBuffer> DuplicateBuffer(std::shared_ptr<IRandomAccessWaveBuffer> source)
    {
        if (!source) return nullptr;

        auto destination = AllocateWaveBuffer(source->GetFormat());

        auto temp_buffer = xtl::temp_memory_buffer();
        for (size_t i = 0; i < source->Size();)
        {
            constexpr size_t buf_size = 65536;
            void* p = temp_buffer.get(buf_size);
            size_t sz = sz = source->Read(p, i, buf_size);
            (void)destination->Write(p, i, sz);
            i += sz;
            if (sz == 0) break; // ?
        }

        return destination;
    }

    std::shared_ptr<ISeekableWaveSource> AllocateReadCursor(std::shared_ptr<IRandomAccessWaveBuffer> buffer)
    {
        if (!buffer) return nullptr;

        class WaveCursorImpl : public ISeekableWaveSource
        {
            std::shared_ptr<IRandomAccessWaveBuffer> target_{};
            PcmWaveFormat format_{};
            size_t cursor_{};

        public:
            WaveCursorImpl(std::shared_ptr<IRandomAccessWaveBuffer> target) : target_(std::move(target)), format_(target_->GetFormat()) {}

            [[nodiscard]] PcmWaveFormat GetFormat() const override { return format_; }

            [[nodiscard]] size_t Read(void* buffer, size_t buffer_length) override
            {
                size_t rd = target_->Read(buffer, cursor_, buffer_length);
                cursor_ += rd;
                return rd;
            }

            [[nodiscard]] size_t GetTotalSampleCount() const override { return target_->Size() / format_.BlockAlign(); }
            [[nodiscard]] size_t GetSampleCursor() const override { return cursor_ / format_.BlockAlign(); }
            size_t SetSampleCursor(size_t new_position) override { return cursor_ = std::min(new_position * format_.BlockAlign(), target_->Size()); }
        };

        return std::make_shared<WaveCursorImpl>(std::move(buffer));
    }
}
