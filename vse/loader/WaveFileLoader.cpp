/// @file
/// @brief  Vse - Wave File Loader
/// @author (C) 2022 ttsuki

#include "WaveFileLoader.h"

#include <filesystem>
#include <fstream>

#include "../base/xtl/xtl_temp_memory_buffer.h"
#include "../base/xtl/xtl_memory_stream.h"
#include "../base/xtl/xtl_fixed_memory_stream.h"

#include "../base/IWaveSource.h"
#include "../base/IWaveProcessor.h"
#include "../processing/WaveFormatConverter.h"
#include "../processing/WaveSourceWithProcessing.h"

namespace vse
{
    std::shared_ptr<ISeekableByteStream> OpenFile(const std::filesystem::path& path)
    {
        class StdIFStream : public ISeekableByteStream
        {
            mutable std::ifstream stream_;
            mutable size_t size_;

        public:
            explicit StdIFStream(std::ifstream ifs)
                : stream_(std::move(ifs))
                , size_(static_cast<size_t>(stream_.seekg(0, std::ios::end).tellg()))
            {
                stream_.seekg(0);
            }

            [[nodiscard]] size_t Read(void* buffer, size_t buffer_length) override
            {
                stream_.clear();
                stream_.read(static_cast<char*>(buffer), buffer_length);
                return stream_.gcount();
            }

            [[nodiscard]] size_t Size() const override { return size_; }
            [[nodiscard]] size_t Tell() const override { return stream_.tellg(); }
            [[nodiscard]] size_t Seek(size_t new_position) override { return Seek(new_position, std::ios::beg); }

            [[nodiscard]] size_t Seek(ptrdiff_t new_position, int whence) override
            {
                stream_.clear();
                return stream_.seekg(new_position, whence).tellg();
            }
        };

        std::ifstream stream(path, std::ios::in | std::ios::binary | std::ios::ate);
        if (!stream) throw std::runtime_error("File can't be open.");
        return std::make_shared<StdIFStream>(std::move(stream));
    }

    std::shared_ptr<ISeekableByteStream> OpenFile(std::shared_ptr<const void> file_image, size_t length)
    {
        class XtlFixedMemStream : public ISeekableByteStream
        {
            std::shared_ptr<const void> memory_;
            xtl::fixed_memory_stream_ro ms_;

        public:
            XtlFixedMemStream(std::shared_ptr<const void> memory, size_t length) : memory_(std::move(memory)), ms_(memory_.get(), length) {}
            [[nodiscard]] size_t Size() const override { return ms_.size(); }
            [[nodiscard]] size_t Read(void* buffer, size_t buffer_length) override { return ms_.read(buffer, buffer_length); }
            [[nodiscard]] size_t Tell() const override { return ms_.tellg(); }
            [[nodiscard]] size_t Seek(ptrdiff_t new_position, int whence) override { return ms_.seekg(new_position, whence); }
        };

        return std::make_shared<XtlFixedMemStream>(std::move(file_image), length);
    }

    std::shared_ptr<ISeekableByteStream> ReadOutToMemory(std::shared_ptr<ISeekableByteStream> stream)
    {
        if (!stream) return nullptr;

        class XtlMemStream : public ISeekableByteStream
        {
            xtl::memory_stream ms_;

        public:
            XtlMemStream() = default;
            [[nodiscard]] size_t Size() const override { return ms_.size(); }
            [[nodiscard]] size_t Read(void* buffer, size_t buffer_length) override { return ms_.read(buffer, buffer_length); }
            [[nodiscard]] size_t Tell() const override { return ms_.tellg(); }
            [[nodiscard]] size_t Seek(ptrdiff_t new_position, int whence) override { return ms_.seekg(new_position, whence); }

            [[nodiscard]] size_t Write(const void* data, size_t length)
            {
                return ms_.write(data, length);
            }
        };

        auto temp_buffer = xtl::temp_memory_buffer();
        auto destination = std::make_shared<XtlMemStream>();

        (void)stream->Seek(0);
        size_t remains = stream->Size();
        while (remains)
        {
            constexpr size_t max_size = 65536;
            void* p = temp_buffer.get(max_size);
            size_t sz = std::min(remains, max_size);
            sz = stream->Read(p, sz);
            (void)destination->Write(p, sz);
            remains -= sz;
            if (sz == 0) break; // ?
        }

        return destination;
    }

    [[nodiscard]] std::shared_ptr<IWaveSource> CreateWaveSourceForFile(std::shared_ptr<ISeekableByteStream> file)
    {
        DWORD four_cc{};
        if (file->Read(&four_cc, 4) != 4)
            throw std::runtime_error("no audio stream.");

        std::shared_ptr<IWaveSource> pcm_stream{};

        if (!pcm_stream && four_cc == 0x5367674f)
        {
            (void)file->Seek(0);
            pcm_stream = CreateWaveSourceOggVorbis(file);
        }

        if (!pcm_stream)
        {
            (void)file->Seek(0);
            pcm_stream = CreateWaveSourceMediaFoundation(file);
        }

        if (!pcm_stream)
            throw std::runtime_error("no decoder can be decode the file.");

        return pcm_stream;
    }

    std::shared_ptr<IWaveSource> ConvertWaveFormat(std::shared_ptr<IWaveSource> source, PcmWaveFormat desired_format)
    {
        if (!source) return nullptr;
        if (source->GetFormat() == desired_format) return source;
        return CreateSourceWithProcessing(source, CreateFormatConverter(source->GetFormat(), desired_format));
    }
}
