/// @file
/// @brief  Vse - Wave Decoder (Media Foundation)
/// @author (C) 2022 ttsuki

#include "WaveFileLoader.h"

#include <Windows.h>
#include <mmreg.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")

#include "../base/win32/debug.h"
#include "../base/win32/com_ptr.h"
#include "../base/win32/com_task_mem_ptr.h"
#include "../base/xtl/xtl_spin_lock_mutex.h"

#define HR_DEBUG_BREAK VSE_EXPECT_SUCCESS
#define EXPECT_SUCCESS VSE_EXPECT_SUCCESS

namespace vse
{
    /// (Utility class)
    /// proxies IMFByteStream interface to a simple stream implementation.
    class MFByteStreamProxy final : public IMFByteStream
    {
    public:
        // Implement by user.
        struct Impl
        {
            std::function<size_t(void* buf, size_t size)> read;
            std::function<size_t(const void* buf, size_t size)> write;
            std::function<size_t(ptrdiff_t size, int origin)> seek;
            std::function<size_t()> size;
            std::function<size_t(size_t size)> resize;
        };

    private:
        LONG ref_count_{1};
        QWORD current_position_{};
        bool end_of_stream_{};
        Impl impl_{};
        xtl::spin_lock_mutex mutex_;
        static const inline GUID ASYNC_READ_OP_RESULT = {0xa35ba80e, 0x6f1d, 0x482f, {0x97, 0xf0, 0xc, 0xbb, 0x24, 0x58, 0x60, 0x9a}};
        static const inline GUID ASYNC_WRITE_OP_RESULT = {0x46ccfe40, 0xfd57, 0x4f49, {0x99, 0xce, 0x4b, 0xb4, 0x1b, 0x50, 0x1f, 0xe}};

        MFByteStreamProxy(Impl impl) : impl_(impl) {}

    public:
        static HRESULT CreateInstanceFor(Impl impl, IMFByteStream** out)
        {
            if (!out) return HR_DEBUG_BREAK E_POINTER;
            *out = new MFByteStreamProxy(std::move(impl));
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(const IID& riid, void** ppvObject) noexcept override
        {
            auto QI = [&](auto* this_) -> HRESULT
            {
                if (riid == __uuidof(this_))
                {
                    *ppvObject = this_;
                    AddRef();
                    return S_OK;
                }
                return E_FAIL;
            };

            if (!ppvObject) return HR_DEBUG_BREAK E_POINTER;
            if (SUCCEEDED(QI(static_cast<IUnknown*>(this)))) return S_OK;
            if (SUCCEEDED(QI(static_cast<IMFByteStream*>(this)))) return S_OK;
            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef() override
        {
            return InterlockedIncrement(&ref_count_);
        }

        ULONG STDMETHODCALLTYPE Release() override
        {
            ULONG i = InterlockedDecrement(&ref_count_);
            if (i == 0) { delete this; }
            return i;
        }

        HRESULT STDMETHODCALLTYPE GetCapabilities(DWORD* pdwCapabilities) override
        {
            if (!pdwCapabilities) return HR_DEBUG_BREAK E_POINTER;
            *pdwCapabilities = 0;
            *pdwCapabilities |= impl_.read ? MFBYTESTREAM_IS_READABLE : 0;
            *pdwCapabilities |= impl_.write ? MFBYTESTREAM_IS_WRITABLE : 0;
            *pdwCapabilities |= impl_.seek ? MFBYTESTREAM_IS_SEEKABLE : 0;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE GetLength(QWORD* pqwLength) override
        {
            if (!pqwLength) return HR_DEBUG_BREAK E_POINTER;

            xtl::lock_guard lock(mutex_);
            *pqwLength = impl_.size ? impl_.size() : -1;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE SetLength(QWORD qwLength [[maybe_unused]]) override
        {
            if (!impl_.resize) return HR_DEBUG_BREAK E_NOTIMPL;

            xtl::lock_guard lock(mutex_);
            impl_.resize(static_cast<size_t>(qwLength));
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE GetCurrentPosition(QWORD* pqwPosition) override
        {
            if (!pqwPosition) return HR_DEBUG_BREAK E_POINTER;

            xtl::lock_guard lock(mutex_);
            *pqwPosition = impl_.seek ? impl_.seek(0, SEEK_CUR) : current_position_;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE SetCurrentPosition(QWORD qwPosition) override
        {
            if (current_position_ == qwPosition) return S_OK;
            if (!impl_.seek) return HR_DEBUG_BREAK E_NOTIMPL;

            xtl::lock_guard lock(mutex_);
            current_position_ = impl_.seek(static_cast<ptrdiff_t>( qwPosition), SEEK_SET);
            return current_position_ == qwPosition ? S_OK : E_INVALIDARG;
        }

        HRESULT STDMETHODCALLTYPE IsEndOfStream(BOOL* pfEndOfStream) override
        {
            if (!pfEndOfStream) return HR_DEBUG_BREAK E_POINTER;

            xtl::lock_guard lock(mutex_);
            *pfEndOfStream = impl_.size ? current_position_ == impl_.size() : end_of_stream_;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE Read(BYTE* pb, ULONG cb, ULONG* pcbRead) override
        {
            if (pcbRead) *pcbRead = 0;
            if (!impl_.read) return HR_DEBUG_BREAK E_NOTIMPL;

            xtl::lock_guard lock(mutex_);
            size_t read = impl_.read(pb, cb);
            current_position_ += read;
            if (read == 0) end_of_stream_ = true;
            if (pcbRead) *pcbRead = static_cast<ULONG>(read);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE BeginRead(BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState) override
        {
            ULONG l{};
            HRESULT hr = this->Read(pb, cb, &l);

            win32::com_ptr<IMFAttributes> attr;
            win32::com_ptr<IMFAsyncResult> r;
            EXPECT_SUCCESS MFCreateAttributes(attr.put(), 1);
            EXPECT_SUCCESS MFCreateAsyncResult(attr.get(), pCallback, punkState, r.put());
            if (attr && r)
            {
                EXPECT_SUCCESS attr->SetUINT64(ASYNC_READ_OP_RESULT, l);
                EXPECT_SUCCESS r->SetStatus(hr);
                return EXPECT_SUCCESS MFInvokeCallback(r.get());
            }
            return EXPECT_SUCCESS E_UNEXPECTED;
        }

        HRESULT STDMETHODCALLTYPE EndRead(IMFAsyncResult* pResult, ULONG* pcbRead) override
        {
            if (pcbRead) *pcbRead = 0;
            if (!pcbRead) { return HR_DEBUG_BREAK E_POINTER; }
            if (!pResult) { return HR_DEBUG_BREAK E_INVALIDARG; }

            win32::com_ptr<IUnknown> obj;
            EXPECT_SUCCESS pResult->GetObject(obj.put());
            if (win32::com_ptr<IMFAttributes> attr = obj)
            {
                UINT64 l{};
                if (SUCCEEDED(EXPECT_SUCCESS attr->GetUINT64(ASYNC_READ_OP_RESULT, &l)))
                {
                    *pcbRead = static_cast<ULONG>(l);
                    return S_OK;
                }
            }
            return EXPECT_SUCCESS E_INVALIDARG;
        }

        HRESULT STDMETHODCALLTYPE Write(const BYTE* pb, ULONG cb, ULONG* pcbWritten) override
        {
            if (pcbWritten) *pcbWritten = 0;
            if (!impl_.write) return E_NOTIMPL;

            xtl::lock_guard lock(mutex_);
            size_t wrote = impl_.write(pb, cb);
            current_position_ += wrote;
            if (pcbWritten) *pcbWritten = static_cast<ULONG>(wrote);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE BeginWrite(const BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState) override
        {
            ULONG l{};
            HRESULT hr = this->Write(pb, cb, &l);

            win32::com_ptr<IMFAttributes> attr;
            win32::com_ptr<IMFAsyncResult> r;
            EXPECT_SUCCESS MFCreateAttributes(attr.put(), 1);
            EXPECT_SUCCESS MFCreateAsyncResult(attr.get(), pCallback, punkState, r.put());
            if (attr && r)
            {
                EXPECT_SUCCESS attr->SetUINT64(ASYNC_WRITE_OP_RESULT, l);
                EXPECT_SUCCESS r->SetStatus(hr);
                return EXPECT_SUCCESS MFInvokeCallback(r.get());
            }
            return EXPECT_SUCCESS E_UNEXPECTED;
        }

        HRESULT STDMETHODCALLTYPE EndWrite(IMFAsyncResult* pResult, ULONG* pcbWritten) override
        {
            if (pcbWritten) *pcbWritten = 0;
            if (!pcbWritten) { return HR_DEBUG_BREAK E_POINTER; }
            if (!pResult) { return HR_DEBUG_BREAK E_INVALIDARG; }

            win32::com_ptr<IUnknown> obj;
            EXPECT_SUCCESS pResult->GetObject(obj.put());
            if (win32::com_ptr<IMFAttributes> attr = obj)
            {
                UINT64 l{};
                if (SUCCEEDED(EXPECT_SUCCESS attr->GetUINT64(ASYNC_WRITE_OP_RESULT, &l)))
                {
                    *pcbWritten = static_cast<ULONG>(l);
                    return S_OK;
                }
            }
            return HR_DEBUG_BREAK E_INVALIDARG;
        }

        HRESULT STDMETHODCALLTYPE Seek(MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG llSeekOffset, [[maybe_unused]] DWORD dwSeekFlags, QWORD* pqwCurrentPosition) override
        {
            if (pqwCurrentPosition) *pqwCurrentPosition = static_cast<QWORD>(current_position_);
            if (SeekOrigin == msoBegin && static_cast<ULONGLONG>(llSeekOffset) == current_position_) return S_OK;
            if (SeekOrigin == msoCurrent && llSeekOffset == 0) return S_OK;
            if (!impl_.seek) return HR_DEBUG_BREAK E_NOTIMPL;

            xtl::lock_guard lock(mutex_);
            switch (SeekOrigin)
            {
            case msoBegin:
                current_position_ = impl_.seek(static_cast<ptrdiff_t>(llSeekOffset), SEEK_SET);
                break;
            case msoCurrent:
                current_position_ = impl_.seek(static_cast<ptrdiff_t>(llSeekOffset), SEEK_CUR);
                break;
            default:
                return HR_DEBUG_BREAK E_INVALIDARG;
            }

            if (pqwCurrentPosition) *pqwCurrentPosition = current_position_;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE Flush() override { return S_OK; }
        HRESULT STDMETHODCALLTYPE Close() override { return S_OK; }
    };

    std::shared_ptr<IWaveSource> CreateWaveSourceMediaFoundation(std::shared_ptr<ISeekableByteStream> file)
    {
        // initialize source_reader
        win32::com_ptr<IMFSourceReader> source_reader;
        {
            win32::com_ptr<IMFByteStream> mf_byte_stream;
            EXPECT_SUCCESS MFByteStreamProxy::CreateInstanceFor(
                MFByteStreamProxy::Impl
                {
                    /*read*/ [file](void* b, size_t l) -> size_t { return file->Read(b, l); },
                    /*write*/ nullptr,
                    /*seek*/ [file](ptrdiff_t p, int w) -> size_t { return file->Seek(p, w); },
                    /*size*/ [file]() -> size_t { return file->Size(); },
                    /*resize*/ nullptr,
                }, mf_byte_stream.put());

            if (HRESULT r = EXPECT_SUCCESS ::MFCreateSourceReaderFromByteStream(mf_byte_stream.get(), NULL, source_reader.put());
                FAILED(r) || !source_reader)
                return nullptr; // throw std::runtime_error("failed to MFCreateSourceReaderFromByteStream");

            EXPECT_SUCCESS source_reader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), FALSE);
            EXPECT_SUCCESS source_reader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), TRUE);
        }

        // Setting Output Media Type.
        PcmWaveFormat reader_output_format{};
        {
            HRESULT hr = S_OK;

            // get native output format
            win32::com_task_mem_ptr<WAVEFORMATEX> native_wfx;
            if (SUCCEEDED(hr))
            {
                win32::com_ptr<IMFMediaType> native_media_type;
                UINT32 native_wfx_length{};
                if (SUCCEEDED(hr)) hr = EXPECT_SUCCESS source_reader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), native_media_type.put());
                if (SUCCEEDED(hr)) hr = EXPECT_SUCCESS MFCreateWaveFormatExFromMFMediaType(native_media_type.get(), native_wfx.put(), &native_wfx_length, MFWaveFormatExConvertFlag_ForceExtensible);
            }

            // request decoded format
            if (SUCCEEDED(hr))
            {
                PcmWaveFormat desired_pcm_format = PcmWaveFormat::Parse(native_wfx.get());
                desired_pcm_format.format_ = SampleType::S16;
                WAVEFORMATEXTENSIBLE desired_wfx = desired_pcm_format;
                win32::com_ptr<IMFAudioMediaType> desired_media_type;
                if (SUCCEEDED(hr)) hr = EXPECT_SUCCESS MFCreateAudioMediaType(reinterpret_cast<const WAVEFORMATEX*>(&desired_wfx), desired_media_type.put());
                if (SUCCEEDED(hr)) hr = EXPECT_SUCCESS source_reader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr, desired_media_type.get());
            }

            // get current output format
            if (SUCCEEDED(hr))
            {
                win32::com_ptr<IMFMediaType> reader_media_type;
                win32::com_task_mem_ptr<WAVEFORMATEX> reader_wfx;
                UINT32 reader_wfx_length{};
                if (SUCCEEDED(hr)) hr = EXPECT_SUCCESS source_reader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), reader_media_type.put());
                if (SUCCEEDED(hr)) hr = EXPECT_SUCCESS::MFCreateWaveFormatExFromMFMediaType(reader_media_type.get(), reader_wfx.put(), &reader_wfx_length, MFWaveFormatExConvertFlag_ForceExtensible);
                if (SUCCEEDED(hr)) reader_output_format = PcmWaveFormat::Parse(reader_wfx.get());
                if (!reader_output_format) hr = EXPECT_SUCCESS E_FAIL;
            }

            if (!reader_output_format)
            {
                return nullptr; // throw std::runtime_error("media foundation can't decode the source to s16 pcm");
            }
        }

        // ready.

        class MediaFoundationSourceReaderWaveSource : public virtual IWaveSource
        {
            win32::com_ptr<IMFSourceReader> source_reader_;
            PcmWaveFormat format_{};

            win32::com_ptr<IMFMediaBuffer> current_buffer_{};
            size_t current_cursor_{};
            bool end_{};

        public:
            MediaFoundationSourceReaderWaveSource(win32::com_ptr<IMFSourceReader> source_reader, PcmWaveFormat output_format)
                : source_reader_(std::move(source_reader))
                , format_(output_format) { }

            [[nodiscard]] PcmWaveFormat GetFormat() const override { return format_; }

            [[nodiscard]] size_t Read(void* buffer, size_t buffer_length) override
            {
                size_t wrote = 0;

                while (!end_ && buffer_length - wrote)
                {
                    if (!current_buffer_)
                    {
                        win32::com_ptr<IMFSample> sample;
                        DWORD flags = 0;
                        if (HRESULT hr = EXPECT_SUCCESS source_reader_->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, NULL, &flags, NULL, sample.put());
                            FAILED(hr))
                        {
                            // TODO
                            end_ = true;
                            break;
                        }

                        if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
                        {
                            HR_DEBUG_BREAK E_UNEXPECTED;
                            end_ = true;
                            break;
                        }

                        if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
                        {
                            end_ = true;
                            break;
                        }

                        EXPECT_SUCCESS sample->ConvertToContiguousBuffer(current_buffer_.put());
                        current_cursor_ = 0;
                    }

                    if (current_buffer_)
                    {
                        BYTE* buf{};
                        DWORD len{};
                        EXPECT_SUCCESS current_buffer_->Lock(&buf, nullptr, &len);
                        size_t reading = std::min(static_cast<size_t>(len) - current_cursor_, buffer_length - wrote);
                        memcpy(static_cast<BYTE*>(buffer) + wrote, buf + current_cursor_, reading);
                        EXPECT_SUCCESS current_buffer_->Unlock();

                        current_cursor_ += reading;
                        wrote += reading;

                        if (current_cursor_ == len)
                            current_buffer_.reset(nullptr);
                    }
                }

                return wrote;
            }
        };

        return std::make_shared<MediaFoundationSourceReaderWaveSource>(std::move(source_reader), reader_output_format);
    }
}
