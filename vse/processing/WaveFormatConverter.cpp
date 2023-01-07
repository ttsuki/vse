/// @file
/// @brief  Vse - Wave Format converter
/// @author (C) 2022 ttsuki

#include "WaveFormatConverter.h"

#include <Windows.h>
#include <propsys.h>
#include <mediaobj.h>
#include <wmcodecdsp.h>
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")

#include <memory>
#include <algorithm>
#include <stdexcept>

#include "../base/xtl/xtl_temp_memory_buffer.h"
#include "../base/win32/debug.h"

#include "./DmoWaveProcessor.h"
#include "./WaveformProcessing.h"
#include "./WaveSourceWithProcessing.h"

namespace vse
{
    std::shared_ptr<IWaveProcessor> CreateThruProcessor(PcmWaveFormat format)
    {
        class ThruProcessorImpl final : public IWaveProcessor
        {
            PcmWaveFormat format_{};

        public:
            explicit ThruProcessorImpl(const PcmWaveFormat& format) : format_(format) {}
            [[nodiscard]] PcmWaveFormat GetInputFormat() const override { return format_; }
            [[nodiscard]] PcmWaveFormat GetOutputFormat() const override { return format_; }

            [[nodiscard]] size_t Process(
                size_t (*read_source)(void* context, void* buffer, size_t buffer_length), void* context,
                void* destination_buffer, size_t destination_buffer_length) override
            {
                return read_source(context, destination_buffer, destination_buffer_length);
            }
        };

        return std::make_shared<ThruProcessorImpl>(format);
    }
}

namespace vse
{
    template <class TSourceSampleType, class TDestinationSampleType>
    class BitDepthConvertProcessorImpl final : public IWaveProcessor
    {
        using SrcType = TSourceSampleType;
        using DstType = TDestinationSampleType;

        PcmWaveFormat input_format_{};
        PcmWaveFormat output_format_{};
        xtl::temp_memory_buffer buffer_{};

    public:
        BitDepthConvertProcessorImpl(const PcmWaveFormat& input_format, const PcmWaveFormat& output_format)
            : input_format_(input_format)
            , output_format_(output_format)
        {
            //
        }

        [[nodiscard]] PcmWaveFormat GetInputFormat() const override { return input_format_; }
        [[nodiscard]] PcmWaveFormat GetOutputFormat() const override { return output_format_; }

        [[nodiscard]] size_t Process(
            size_t (*read_source)(void* context, void* buffer, size_t buffer_length), void* context,
            void* destination_buffer, size_t destination_buffer_length) noexcept override
        {
            // calculates maximum sample count.
            size_t max_count = destination_buffer_length / sizeof(DstType);

            // allocates temporally buffer.
            auto* tmp = buffer_.get<SrcType>(max_count);

            // reads source.
            auto read_bytes = read_source(context, tmp, max_count * sizeof(SrcType));

            size_t count = read_bytes / sizeof(SrcType);

            // converts.
            processing::ConvertCopy(static_cast<DstType*>(destination_buffer), tmp, count);

            // returns processed size.
            return count * sizeof(DstType);
        }
    };

    std::shared_ptr<IWaveProcessor> CreateBitDepthConverter(PcmWaveFormat input_format, SampleType output_format)
    {
        if (input_format.SampleType() == output_format) return CreateThruProcessor(input_format);
        if (input_format.SampleType() == SampleType::S16 && output_format == SampleType::S16) return std::make_shared<BitDepthConvertProcessorImpl<S16, S16>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::S16 && output_format == SampleType::S24) return std::make_shared<BitDepthConvertProcessorImpl<S16, S24>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::S16 && output_format == SampleType::S32) return std::make_shared<BitDepthConvertProcessorImpl<S16, S32>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::S16 && output_format == SampleType::F32) return std::make_shared<BitDepthConvertProcessorImpl<S16, F32>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::S24 && output_format == SampleType::S16) return std::make_shared<BitDepthConvertProcessorImpl<S24, S16>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::S24 && output_format == SampleType::S24) return std::make_shared<BitDepthConvertProcessorImpl<S24, S24>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::S24 && output_format == SampleType::S32) return std::make_shared<BitDepthConvertProcessorImpl<S24, S32>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::S24 && output_format == SampleType::F32) return std::make_shared<BitDepthConvertProcessorImpl<S24, F32>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::S32 && output_format == SampleType::S16) return std::make_shared<BitDepthConvertProcessorImpl<S32, S16>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::S32 && output_format == SampleType::S24) return std::make_shared<BitDepthConvertProcessorImpl<S32, S24>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::S32 && output_format == SampleType::S32) return std::make_shared<BitDepthConvertProcessorImpl<S32, S32>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::S32 && output_format == SampleType::F32) return std::make_shared<BitDepthConvertProcessorImpl<S32, F32>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::F32 && output_format == SampleType::S16) return std::make_shared<BitDepthConvertProcessorImpl<F32, S16>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::F32 && output_format == SampleType::S24) return std::make_shared<BitDepthConvertProcessorImpl<F32, S24>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::F32 && output_format == SampleType::S32) return std::make_shared<BitDepthConvertProcessorImpl<F32, S32>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        if (input_format.SampleType() == SampleType::F32 && output_format == SampleType::F32) return std::make_shared<BitDepthConvertProcessorImpl<F32, F32>>(input_format, PcmWaveFormat{output_format, input_format.channels_, input_format.frequency_});
        throw std::runtime_error("not supported format!");
    }
}

namespace vse
{
    std::shared_ptr<IWaveProcessor> CreateAudioResamplingDsp(
        PcmWaveFormat input_format,
        PcmWaveFormat output_format,
        int quality_level)
    {
        win32::com_ptr<IMediaObject> media_object{};

        // Audio Resampler DSP
        // The Audio Resampler performs one or both of the following actions on an audio stream.
        // https://learn.microsoft.com/en-us/windows/win32/medfound/audioresampler
        //static constexpr GUID CLSID_CResamplerMediaObject = {0xf447b69e, 0x1884, 0x4a7e, {0x80, 0x55, 0x34, 0x6f, 0x74, 0xd6, 0xed, 0xb3}};

        WAVEFORMATEXTENSIBLE input_wfx = input_format;
        WAVEFORMATEXTENSIBLE output_wfx = output_format;

        if (HRESULT hr = VSE_EXPECT_SUCCESS CreateMediaObject(
                CLSID_CResamplerMediaObject,
                reinterpret_cast<WAVEFORMATEX*>(&input_wfx),
                reinterpret_cast<WAVEFORMATEX*>(&output_wfx),
                media_object.put());
            FAILED(hr) || !media_object)
        {
            throw std::runtime_error("FAILED to CreateAudioResamplerDsp(...)");
        }

        if (win32::com_ptr<IPropertyStore> props = media_object)
        {
            // constexpr PROPERTYKEY MFPKEY_WMRESAMP_FILTERQUALITY = {{0xaf1adc73, 0xa210, 0x4b05, {0x96, 0x6e, 0x54, 0x91, 0xcf, 0xf4, 0x8b, 0x1d}}, 0x01};
            // Specifies the quality of the output.
            // The valid range of this property is 1 to 60, inclusive.
            // Higher values produce higher-quality output, but introduce more latency.
            // A value of 1 is essentially the same as linear interpolation.
            PROPVARIANT value{};
            value.vt = VT_I4;
            value.intVal = quality_level;
            VSE_EXPECT_SUCCESS props->SetValue(MFPKEY_WMRESAMP_FILTERQUALITY, value);
        }

        if (win32::com_ptr<IPropertyStore> props = media_object)
        {
            // constexpr PROPERTYKEY MFPKEY_WMRESAMP_LOWPASS_BANDWIDTH = {{0xaf1adc73, 0xa210, 0x4b05, {0x96, 0x6e, 0x54, 0x91, 0xcf, 0xf4, 0x8b, 0x1d}}, 0x03};
            // Specifies the low-pass filter bandwidth, as a percentage of the destination sample rate.
            // The valid range of this property is 0.0 to 1.0, inclusive
            PROPVARIANT value{};
            value.vt = VT_R4;
            value.fltVal = std::min(1.0f, static_cast<float>(input_format.SamplingFrequency()) / static_cast<float>(output_format.SamplingFrequency()));
            VSE_EXPECT_SUCCESS props->SetValue(MFPKEY_WMRESAMP_LOWPASS_BANDWIDTH, value);
        }

        if (win32::com_ptr<IPropertyStore> props = media_object)
        {
            // constexpr PROPERTYKEY MFPKEY_WMRESAMP_CHANNELMTX = {{0xaf1adc73, 0xa210, 0x4b05, {0x96, 0x6e, 0x54, 0x91, 0xcf, 0xf4, 0x8b, 0x1d}}, 0x02};
            // Specifies the channel matrix, which is used to convert the source channels into the destination channels (for example, to convert 5.1 to stereo).
            // The value of the property is a matrix of Ns x Nd coefficients, where Ns is the number of source channels and Nd is the number of destination channels.
            // Coefficients are specified in decibels using the following formula:
            //   (int)(65536 * 20 * log10(dB))
            // The matrix is stored as an array in row - major order where the row number is the source channel and the column number is the destination channel.
            PROPVARIANT propvar{};
            if (HRESULT hr = VSE_EXPECT_SUCCESS props->GetValue(MFPKEY_WMRESAMP_CHANNELMTX, &propvar); SUCCEEDED(hr))
            {
                // TODO: Support conversion with channel matrix.
                PropVariantClear(&propvar);
            }
        }

        return CreateDmoWaveProcessor(media_object.get(), input_format, output_format);
    }
}

namespace vse
{
    std::shared_ptr<IWaveProcessor> CreateFormatConverter(
        const WAVEFORMATEXTENSIBLE& input_format,
        const WAVEFORMATEXTENSIBLE& output_format)
    {
        auto in = PcmWaveFormat::Parse(input_format);
        auto out = PcmWaveFormat::Parse(output_format);

        // If source and destination formats are same, no need to convert source.
        if (in && out && in == out)
        {
            return CreateThruProcessor(in);
        }

        // If sampling-rates and channel-counts are same, only bit-depth is need to convert.
        if (in && out
            && in.SamplingFrequency() == out.SamplingFrequency()
            && in.ChannelCount() == out.ChannelCount())
        {
            return CreateBitDepthConverter(in, out.SampleType());
        }

        try
        {
            return CreateAudioResamplingDsp(in, out);
        }
        catch (...) {}

        throw std::runtime_error("Not supported format.");
    }

    std::shared_ptr<IWaveSource> ConvertWaveFormat(
        std::shared_ptr<IWaveSource> source,
        const WAVEFORMATEXTENSIBLE& desired_format)
    {
        if (!source) return nullptr;

        if (auto desired = PcmWaveFormat::Parse(desired_format);
            source->GetFormat() == desired)
            return source;

        return CreateSourceWithProcessing(source, CreateFormatConverter(source->GetFormat(), desired_format));
    }
}
