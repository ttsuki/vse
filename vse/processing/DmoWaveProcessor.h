/// @file
/// @brief  Vse - Dmo Wave Processor
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>
#include <mediaobj.h>

#include "../base/IWaveProcessor.h"
#include "../base/win32/com_ptr.h"

namespace vse
{
    /// (Utility) Creates DMO_MEDIA_TYPE by wrapping WAVEFORMATEX.
    /// @remark Returned DMO_MEDIA_TYPE object does not manage the lifetime of input object.
    DMO_MEDIA_TYPE MakeDmoMediaType(const WAVEFORMATEX* wfx) noexcept;

    /// (Utility) Calculates the input size for output size.
    /// Default implementation uses ratio of the averages bytes per second of the input format's vs the output format's.
    /// @param input_format The input format.
    /// @param output_format The output format.
    /// @param output_size The next output size in bytes.
    /// @returns Suggested next input size in bytes.
    size_t SuggestInputBufferSizeForOutputBufferSize(
        _In_ const PcmWaveFormat& input_format,
        _In_ const PcmWaveFormat& output_format,
        _In_ size_t output_size) noexcept;

    /// (Utility) Creates an instance of an implementation of IMediaBuffer interface.
    /// @returns S_OK
    HRESULT CreateMediaBuffer(
        _In_ DWORD capacity, 
        _Out_ IMediaBuffer** pp_media_buffer) noexcept;

    /// (Utility) Creates DMO instance.
    /// @returns S_OK if media object is created successfully.
    HRESULT CreateMediaObject(
        _In_ const CLSID& clsid,
        _In_ const WAVEFORMATEX* input_format,
        _In_ const WAVEFORMATEX* output_format,
        _Out_ IMediaObject** pp_media_object) noexcept;

    /// DmoWaveProcessor.
    /// Provides basic WaveProcessing function using MediaObject.
    class DmoWaveProcessor : public IWaveProcessor
    {
    protected:
        const win32::com_ptr<IMediaObject> media_object_{};
        const PcmWaveFormat input_format_{};
        const PcmWaveFormat output_format_{};
        bool need_more_input_{true};
        bool end_of_input_{false};

    public:
        DmoWaveProcessor(IMediaObject* media_object, const PcmWaveFormat& input_format, const PcmWaveFormat& output_format);
        [[nodiscard]] PcmWaveFormat GetInputFormat() const override { return input_format_; }
        [[nodiscard]] PcmWaveFormat GetOutputFormat() const override { return output_format_; }

        /// Processes wave samples by MediaObject.
        [[nodiscard]] size_t Process(
            size_t (*read_source)(void* context, void* buffer, size_t buffer_length), void* context,
            void* buffer, size_t buffer_length) override;
    };

    static inline std::shared_ptr<IWaveProcessor> CreateDmoWaveProcessor(IMediaObject* media_object, const PcmWaveFormat& input_format, const PcmWaveFormat& output_format)
    {
        return std::make_shared<DmoWaveProcessor>(media_object, input_format, output_format);
    }
}
