/// @file
/// @brief  Vse - DirectSound Output Device
/// @author (C) 2022 ttsuki

#include "DirectSoundOutputDevice.h"

#include <Windows.h>
#include <dsound.h>
#pragma comment(lib, "dsound.lib")
MIDL_INTERFACE("b0210783-89cd-11d0-af08-00a0c925cd16") IDirectSoundNotify;

#include <cassert>

#include "../base/win32/debug.h"
#include "../base/win32/event.h"
#include "../base/win32/com_ptr.h"

#define EXPECT_SUCCESS VSE_EXPECT_SUCCESS
#define DEBUG_LOG VSE_DEBUG_LOG("vse::DirectSoundOutput: ")
#define DEBUG_BREAK() (void(VSE_EXPECT_SUCCESS E_UNEXPECTED))

namespace vse
{
    class DirectSoundOutputDeviceImpl final : public DirectSoundOutputDevice
    {
        static inline constexpr DWORD buffer_size_in_milliseconds_ = 64;
        static inline constexpr DWORD buffer_count_ = 2;

        WAVEFORMATEXTENSIBLE device_format_{};
        win32::com_ptr<IDirectSound8> device_{};
        win32::com_ptr<IDirectSoundBuffer> buffer_{};
        win32::manual_reset_event buffer_switch_event_{};
        DWORD buffer_length_{};
        DWORD emitted_buffer_count_{};
        void* locked_ptr_{};
        DWORD locked_size_{};

    public:
        DirectSoundOutputDeviceImpl() = default;
        DirectSoundOutputDeviceImpl(const DirectSoundOutputDeviceImpl& other) = delete;
        DirectSoundOutputDeviceImpl(DirectSoundOutputDeviceImpl&& other) noexcept = delete;
        DirectSoundOutputDeviceImpl& operator=(const DirectSoundOutputDeviceImpl& other) = delete;
        DirectSoundOutputDeviceImpl& operator=(DirectSoundOutputDeviceImpl&& other) noexcept = delete;
        ~DirectSoundOutputDeviceImpl() override { Close(); }

        bool Open() override
        {
            return Open(WAVEFORMATEXTENSIBLE{});
        }

        bool Open(const WAVEFORMATEXTENSIBLE& format) override
        {
            DEBUG_LOG << "Opening at " << ToString(format);

            // Create device.
            win32::com_ptr<IDirectSound8> device;
            {
                if (HRESULT hr = EXPECT_SUCCESS DirectSoundCreate8(nullptr, device.put(), nullptr); FAILED(hr))
                {
                    DEBUG_LOG << "FAILED to DirectSoundCreate8";
                    return false;
                }

                if (HRESULT hr = EXPECT_SUCCESS device->SetCooperativeLevel(GetConsoleWindow(), DSSCL_PRIORITY); FAILED(hr))
                {
                    DEBUG_LOG << "FAILED to device->SetCooperativeLevel";
                    return false;
                }
            }

            // Create primary buffer.
            {
                win32::com_ptr<IDirectSoundBuffer> primary_buffer;

                DSBUFFERDESC desc{};
                desc.dwSize = sizeof(DSBUFFERDESC);
                desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

                if (HRESULT hr = EXPECT_SUCCESS device->CreateSoundBuffer(&desc, primary_buffer.put(), NULL); FAILED(hr) || !primary_buffer)
                    return DEBUG_LOG << "FAILED to create Primary buffer!", false;

                if (HRESULT hr = primary_buffer->SetFormat(&format.Format); FAILED(hr))
                    return DEBUG_LOG << "FAILED to set format to Primary buffer!", false;

                primary_buffer.reset();
                DEBUG_LOG << "Primary buffer created.";
            }

            // Create secondary buffer (but treated as primary buffer)
            win32::com_ptr<IDirectSoundBuffer> buffer;
            win32::manual_reset_event buffer_switch_event{false};

            const DWORD buffer_length_in_samples = (buffer_size_in_milliseconds_ * format.Format.nSamplesPerSec / 1000) + 63 & ~63;
            const DWORD buffer_length = buffer_length_in_samples * format.Format.nBlockAlign;

            {
                DSBUFFERDESC desc{};
                WAVEFORMATEXTENSIBLE f = format;
                desc.dwSize = sizeof(DSBUFFERDESC);
                desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
                desc.dwBufferBytes = buffer_count_ * buffer_length;
                desc.lpwfxFormat = &f.Format;

                if (auto hr = EXPECT_SUCCESS device->CreateSoundBuffer(&desc, buffer.put(), NULL); FAILED(hr))
                    return DEBUG_LOG << "FAILED to create Secondary buffer!", false;

                // Clear buffer.
                {
                    void* data{};
                    DWORD len{};

                    if (HRESULT hr = EXPECT_SUCCESS buffer->Lock(0, buffer_length, &data, &len, nullptr, nullptr, DSBLOCK_ENTIREBUFFER); FAILED(hr))
                        return DEBUG_LOG << "FAILED to lock buffer.", false;

                    memset(data, 0, len);

                    if (HRESULT hr = EXPECT_SUCCESS buffer->Unlock(data, len, nullptr, 0); FAILED(hr))
                        return DEBUG_LOG << "FAILED to unlock buffer.", false;
                }

                // Install notification event
                {
                    win32::com_ptr<IDirectSoundNotify> notify = buffer.query_interface<IDirectSoundNotify>();
                    if (!notify)
                        return DEBUG_LOG << "FAILED to query IDirectSoundNotify interface.", false;

                    auto position_notifies = std::make_unique<DSBPOSITIONNOTIFY[]>(buffer_count_);
                    for (DWORD i = 0; i < buffer_count_; i++)
                        position_notifies[i] = {buffer_length * i, buffer_switch_event.handle()};

                    if (HRESULT hr = EXPECT_SUCCESS notify->SetNotificationPositions(buffer_count_, position_notifies.get()); FAILED(hr))
                        return DEBUG_LOG << "FAILED to SetNotificationPositions.", false;
                }

                DEBUG_LOG << "Secondary buffer created.";
            }

            // Ok.
            buffer_switch_event_ = std::move(buffer_switch_event);
            device_format_ = format;
            device_ = std::move(device);
            buffer_ = std::move(buffer);
            buffer_length_ = buffer_length;

            DEBUG_LOG << "Ready.";
            return true;
        }

        WAVEFORMATEXTENSIBLE GetFormat() const override
        {
            return device_format_;
        }

        int GetBufferSize() const override
        {
            return static_cast<int>(buffer_length_);
        }

        bool Start() override
        {
            if (!buffer_) return DEBUG_BREAK(), false;
            HRESULT hr = EXPECT_SUCCESS buffer_->Play(0, 0, DSBPLAY_LOOPING);
            emitted_buffer_count_ = 0;
            return SUCCEEDED(hr);
        }

        void* LockBuffer(int buffer_size) override
        {
            if (!buffer_) return DEBUG_BREAK(), nullptr;    // not allocated.
            if (locked_ptr_) return DEBUG_BREAK(), nullptr; // already locked.

            void* ptr = nullptr;
            DWORD size{};

            if (buffer_switch_event_.wait_signal(1000))
            {
                buffer_switch_event_.reset_signal_state();

                {
                    DWORD play_pos{}, write_pos{};
                    while (true)
                    {
                        auto current = emitted_buffer_count_ % buffer_count_;
                        if (HRESULT hr = EXPECT_SUCCESS buffer_->GetCurrentPosition(&play_pos, &write_pos); FAILED(hr)) break;
                        if (play_pos >= current * buffer_length_ && play_pos < (current + 1) * buffer_length_) break;
                    }
                }

                HRESULT hr = EXPECT_SUCCESS buffer_->Lock(
                    (++emitted_buffer_count_ % buffer_count_) * buffer_length_,
                    buffer_length_,
                    &ptr, &size,
                    nullptr, nullptr,
                    0);

                assert(size == buffer_length_);

                if (FAILED(hr)) return nullptr;
                if (size != static_cast<DWORD>(buffer_size)) return nullptr;

                // OK.
                locked_ptr_ = ptr;
                locked_size_ = size;
                return ptr;
            }

            return nullptr;
        }

        void UnlockBuffer(int wrote_bytes) override
        {
            if (!buffer_) return DEBUG_BREAK();     // not allocated.
            if (!locked_ptr_) return DEBUG_BREAK(); // not locked.

            if (static_cast<DWORD>(wrote_bytes) < locked_size_)
            {
                memset(static_cast<byte*>(locked_ptr_) + wrote_bytes, 0, locked_size_ - wrote_bytes);
            }

            if (HRESULT hr = EXPECT_SUCCESS buffer_->Unlock(locked_ptr_, locked_size_, nullptr, 0); SUCCEEDED(hr))
            {
                locked_ptr_ = nullptr;
                locked_size_ = 0;
            }
        }

        void Reset() override
        {
            if (!buffer_) return DEBUG_BREAK();

            buffer_->Stop();
            emitted_buffer_count_ = 0;
        }

        void Close() override
        {
            buffer_.reset();
            device_.reset();
        }
    };

    std::shared_ptr<DirectSoundOutputDevice> CreateDirectSoundOutputDevice()
    {
        return std::make_shared<DirectSoundOutputDeviceImpl>();
    }
}
