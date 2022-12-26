/// @file
/// @brief  Vse - Dmo Wave Processor
/// @author (C) 2022 ttsuki

#include "DmoWaveProcessor.h"

#include <Windows.h>
#include <mediaobj.h>
#include <uuids.h>

#include <new>
#include <algorithm>

#include "../base/win32/com_base.h"
#include "../base/win32/com_ptr.h"
#include "../base/win32/debug.h"

#pragma comment(lib, "strmiids.lib")

namespace vse
{
    DMO_MEDIA_TYPE MakeDmoMediaType(const WAVEFORMATEX* wfx) noexcept
    {
        return DMO_MEDIA_TYPE{
            MEDIATYPE_Audio,
            wfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE
                ? reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(wfx)->SubFormat
                : GUID{wfx->wFormatTag, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}},
            TRUE,
            FALSE,
            static_cast<ULONG>(wfx->wBitsPerSample / 8),
            FORMAT_WaveFormatEx,
            NULL,
            static_cast<ULONG>(sizeof(WAVEFORMATEX) + wfx->cbSize),
            reinterpret_cast<BYTE*>(const_cast<WAVEFORMATEX*>(wfx))
        };
    }

    size_t SuggestInputBufferSizeForOutputBufferSize(
        const PcmWaveFormat& input_format,
        const PcmWaveFormat& output_format,
        size_t output_size) noexcept
    {
        size_t read_size = 2048; // default read callback size in bytes

        if (input_format && output_format)
        {
            read_size = static_cast<size_t>(
                static_cast<uintmax_t>(output_size)
                * input_format.AvgBytesPerSec()
                / output_format.AvgBytesPerSec());

            read_size -= read_size % input_format.BlockAlign();
            read_size = std::max(size_t{16} * input_format.BlockAlign(), read_size);
        }

        return read_size;
    }

    HRESULT CreateMediaBuffer(DWORD capacity, IMediaBuffer** pp_media_buffer) noexcept
    {
        if (!pp_media_buffer) return E_POINTER;

        static constexpr std::align_val_t buffer_alignment_ = std::align_val_t{64};

        class MediaBufferImpl final : public IMediaBuffer
        {
        private:
            void* const buffer_{};
            DWORD const capacity_{};
            DWORD length_{};
            LONG ref_count_{1};

        private:
            static void* AllocateMemory(size_t size) { return ::operator new(size, buffer_alignment_, std::nothrow); }
            static void FreeMemory(void* p) { return ::operator delete(p, buffer_alignment_, std::nothrow); }
            static constexpr inline size_t RoundUpAlignment(size_t size) { return size + (static_cast<size_t>(buffer_alignment_) - 1) & ~(static_cast<size_t>(buffer_alignment_) - 1); }
            static constexpr inline size_t OffsetToBuffer() { return RoundUpAlignment(sizeof(MediaBufferImpl)); }

        private:
            MediaBufferImpl(void* buffer, DWORD capacity) noexcept : buffer_(buffer), capacity_(capacity) { }

        public:
            static MediaBufferImpl* CreateInstance(DWORD capacity) noexcept
            {
                if (auto p = AllocateMemory(OffsetToBuffer() + RoundUpAlignment(capacity)))
                    return new(p) MediaBufferImpl(static_cast<std::byte*>(p) + OffsetToBuffer(), capacity);

                return nullptr;
            }

        private:
            static void DeleteInstance(MediaBufferImpl* p) noexcept
            {
                p->~MediaBufferImpl();
                FreeMemory(p);
            }

        public:
            // IUnknown::QueryInterface
            STDMETHOD(QueryInterface(REFIID riid, void** ppv)) noexcept override
            {
                if (!ppv) { return E_POINTER; }

                if (riid == __uuidof(IUnknown))
                {
                    AddRef();
                    *ppv = static_cast<IUnknown*>(this);
                    return S_OK;
                }

                if (riid == __uuidof(IMediaBuffer))
                {
                    AddRef();
                    *ppv = static_cast<IMediaBuffer*>(this);
                    return S_OK;
                }

                return E_NOINTERFACE;
            }

            // IUnknown::AddRef
            STDMETHOD_(ULONG, AddRef()) noexcept override
            {
                return InterlockedIncrement(&ref_count_);
            }

            // IUnknown::Release
            STDMETHOD_(ULONG, Release()) noexcept override
            {
                LONG ref = InterlockedDecrement(&ref_count_);
                if (ref == 0) DeleteInstance(this);
                return ref;
            }

            // IMediaBuffer::GetBufferAndLength
            STDMETHOD(GetBufferAndLength(BYTE** ppBuffer, DWORD* pcbLength)) noexcept override
            {
                if (!ppBuffer && !pcbLength) return VSE_EXPECT_SUCCESS E_POINTER;
                if (ppBuffer) *ppBuffer = static_cast<BYTE*>(buffer_);
                if (pcbLength) *pcbLength = length_;
                return S_OK;
            }

            // IMediaBuffer::GetMaxLength
            STDMETHOD(GetMaxLength(DWORD* pcbMaxLength)) noexcept override
            {
                if (!pcbMaxLength) return VSE_EXPECT_SUCCESS E_POINTER;
                *pcbMaxLength = capacity_;
                return S_OK;
            }

            // IMediaBuffer::SetLength
            STDMETHOD(SetLength(DWORD cbLength)) noexcept override
            {
                if (cbLength > capacity_) return VSE_EXPECT_SUCCESS E_INVALIDARG;
                length_ = cbLength;
                return S_OK;
            }
        };

        *pp_media_buffer = MediaBufferImpl::CreateInstance(capacity);
        return VSE_EXPECT_SUCCESS (*pp_media_buffer ? S_OK : E_OUTOFMEMORY);
    }

    HRESULT CreateMediaObject(
        const CLSID& clsid,
        const WAVEFORMATEX* input_format,
        const WAVEFORMATEX* output_format,
        IMediaObject** pp_media_object) noexcept
    {
        if (!pp_media_object) return VSE_EXPECT_SUCCESS E_POINTER;

        win32::com_ptr<IMediaObject> object = nullptr;
        if (HRESULT hr = VSE_EXPECT_SUCCESS ::CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(object.put())); FAILED(hr)) return FAILED(hr);

        const DMO_MEDIA_TYPE input_media_type = MakeDmoMediaType(input_format);
        const DMO_MEDIA_TYPE output_media_type = MakeDmoMediaType(output_format);
        if (HRESULT hr = VSE_EXPECT_SUCCESS object->SetInputType(0, &input_media_type, 0); FAILED(hr)) return hr;
        if (HRESULT hr = VSE_EXPECT_SUCCESS object->SetOutputType(0, &output_media_type, 0); FAILED(hr)) return hr;

        *pp_media_object = object.detach();
        return S_OK;
    }

    DmoWaveProcessor::DmoWaveProcessor(IMediaObject* media_object, const PcmWaveFormat& input_format, const PcmWaveFormat& output_format)
        : media_object_(media_object)
        , input_format_(input_format)
        , output_format_(output_format)
    {
        //
    }

    size_t DmoWaveProcessor::Process(
        size_t (*read_source)(void* context, void* buffer, size_t buffer_length), void* context,
        void* buffer, size_t buffer_length)
    {
        if (!continuity_.test_and_set())
            VSE_EXPECT_SUCCESS media_object_->Discontinuity(0);

        size_t rest = buffer_length;
        while (rest)
        {
            if (need_more_input_ && end_of_input_)
                break; // no more samples

            // Processes input
            if (need_more_input_)
            {
                DWORD read_size = static_cast<DWORD>(SuggestInputBufferSizeForOutputBufferSize(input_format_, output_format_, rest));

                win32::com_ptr<IMediaBuffer> src_buffer;
                if (HRESULT hr = VSE_EXPECT_SUCCESS CreateMediaBuffer(read_size, src_buffer.put()); FAILED(hr) || !src_buffer) break;
                BYTE* buf{};
                DWORD len{};
                if (HRESULT hr = VSE_EXPECT_SUCCESS src_buffer->GetBufferAndLength(&buf, &len); FAILED(hr)) break;

                size_t read = read_source(context, buf, read_size);
                if (read > 0)
                {
                    if (HRESULT hr = VSE_EXPECT_SUCCESS src_buffer->SetLength(static_cast<DWORD>(read)); FAILED(hr)) break;
                    if (HRESULT hr = VSE_EXPECT_SUCCESS media_object_->ProcessInput(0, src_buffer.get(), 0, 0, 0); FAILED(hr)) break;
                }
                else
                {
                    end_of_input_ = true;
                    if (HRESULT hr = VSE_EXPECT_SUCCESS media_object_->Discontinuity(0); FAILED(hr)) break;
                }
            }

            // Processes output
            {
                win32::com_ptr<IMediaBuffer> dst_buffer;
                if (HRESULT hr = VSE_EXPECT_SUCCESS CreateMediaBuffer(static_cast<DWORD>(rest), dst_buffer.put()); FAILED(hr) || !dst_buffer) break;
                DMO_OUTPUT_DATA_BUFFER out = {dst_buffer.get(), 0, 0, 0};
                DWORD status{};
                if (HRESULT hr = VSE_EXPECT_SUCCESS media_object_->ProcessOutput(0, 1, &out, &status); FAILED(hr)) break;

                BYTE* buf{};
                DWORD len{};
                if (HRESULT hr = VSE_EXPECT_SUCCESS dst_buffer->GetBufferAndLength(&buf, &len); FAILED(hr)) break;

                memcpy(buffer, buf, len);
                buffer = static_cast<std::byte*>(buffer) + len;
                rest -= len;

                need_more_input_ = !(out.dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE);
            }
        }

        return buffer_length - rest;
    }

    void DmoWaveProcessor::Discontinuity()
    {
        continuity_.clear();
    }
}
