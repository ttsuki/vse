/// @file
/// @brief  Vse - Wave Decoder (Ogg Vorbis)
/// @author (C) 2022 ttsuki

#include "WaveFileLoader.h"

#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

#pragma comment(lib, "ogg.lib")
#pragma comment(lib, "vorbis.lib")
#pragma comment(lib, "vorbisfile.lib")

#include "../base/IByteStream.h"
#include "../base/IWaveSource.h"

#include "../base/win32/debug.h"

namespace vse
{
    std::shared_ptr<IWaveSource> CreateWaveSourceOggVorbis(std::shared_ptr<ISeekableByteStream> file)
    {
        using SrcStream = ISeekableByteStream;
        static constexpr ov_callbacks callback
        {
            /*  read: */ [](void* ptr, size_t size, size_t nmemb, void* datasource) -> size_t { return static_cast<SrcStream*>(datasource)->Read(ptr, size * nmemb); },
            /*  seek: */ [](void* datasource, ogg_int64_t offset, int whence) -> int
            {
                [file = static_cast<SrcStream*>(datasource)](ptrdiff_t offset, int whence)
                {
                    switch (whence)
                    {
                    case SEEK_SET: return (void)file->Seek(offset);
                    case SEEK_CUR: return (void)file->Seek(file->Tell() + offset);
                    case SEEK_END: return (void)file->Seek(file->Size() + offset);
                    default: return (void)file->Tell();
                    }
                }(static_cast<ptrdiff_t>(offset), whence);
                return 0;
            },

            /* close: */ [](void* datasource [[maybe_unused]]) -> int { return 0; },
            /*  tell: */ [](void* datasource) -> long { return static_cast<long>(static_cast<ISeekableByteStream*>(datasource)->Tell()); },
        };

        auto ovf = std::shared_ptr<OggVorbis_File>(
            new OggVorbis_File(),
            [](OggVorbis_File* p)
            {
                ov_clear(p);
                delete p;
            });

        if (ov_open_callbacks(file.get(), ovf.get(), nullptr, 0, callback) < 0)
        {
            return nullptr; // throw std::runtime_error("DecodeAudioFileOggVorbis: Input does not appear to be an Ogg bitstream.");
        }

        vorbis_info* info = ov_info(ovf.get(), -1);
        PcmWaveFormat format = PcmWaveFormat{SampleType::S16, DefaultChannelMask(info->channels), static_cast<int>(info->rate)};

        class OggVorbisWaveSourceImpl final : public ISeekableWaveSource
        {
            std::shared_ptr<SrcStream> file_{};
            std::shared_ptr<OggVorbis_File> vf_{};
            PcmWaveFormat format_{};

            int curSection_{};

        public:
            OggVorbisWaveSourceImpl(
                std::shared_ptr<SrcStream> file,
                std::shared_ptr<OggVorbis_File> vf,
                PcmWaveFormat format)
                : file_(file), vf_(vf), format_(format) { }

            [[nodiscard]] PcmWaveFormat GetFormat() const override { return format_; }

            [[nodiscard]] size_t Read(void* buffer, size_t buffer_length) override
            {
                size_t wrote = 0;
                while (buffer_length - wrote)
                {
                    // TODO: Correct channel order more than two.
                    /// The output channels are in stream order and not remapped. Vorbis I defines channel order as follows:
                    /// - one channel - the stream is monophonic
                    /// - two channels - the stream is stereo. channel order: left, right
                    /// - three channels - the stream is a 1d - surround encoding. channel order: left, center, right
                    /// - four channels - the stream is quadraphonic surround. channel order: front left, front right, rear left, rear right
                    /// - five channels - the stream is five - channel surround. channel order: front left, center, front right, rear left, rear right
                    /// - six channels - the stream is 5.1 surround. channel order: front left, center, front right, rear left, rear right, LFE
                    /// - seven channels - the stream is 6.1 surround. channel order: front left, center, front right, side left, side right, rear center, LFE
                    /// - eight channels - the stream is 7.1 surround. channel order: front left, center, front right, side left, side right, rear left, rear right, LFE
                    /// - greater than eight channels - channel use and order is undefined
                    // Windows assumes channel order as: left, right, center, LFE, rear left, rear right, side left, side right

                    long ret = ov_read(
                        vf_.get(),
                        static_cast<char*>(buffer) + wrote,
                        static_cast<int>(buffer_length - wrote),
                        0, 2, 1,
                        &curSection_);

                    if (ret > 0) { wrote += ret; }

                    if (ret == 0)
                    {
                        break;
                    }
                }

                return wrote;
            }

            [[nodiscard]] size_t GetTotalSampleCount() const override { return static_cast<size_t>(ov_pcm_total(vf_.get(), -1)); }
            [[nodiscard]] size_t GetSampleCursor() const override { return static_cast<size_t>(ov_pcm_tell(vf_.get())); }
            [[nodiscard]] size_t SetSampleCursor(size_t newPosition) override { return ov_pcm_seek(vf_.get(), static_cast<ogg_int64_t>(newPosition)), GetSampleCursor(); }
        };

        return std::make_shared<OggVorbisWaveSourceImpl>(file, ovf, format);
    }
}
