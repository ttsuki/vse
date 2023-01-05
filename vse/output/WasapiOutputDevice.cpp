/// @file
/// @brief  Vse - WASAPI Output Device
/// @author (C) 2022 ttsuki

#include "WasapiOutputDevice.h"

#include <Windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>

#include <memory>
#include <chrono>
#include <optional>

#include "../base/win32/debug.h"
#include "../base/win32/event.h"
#include "../base/win32/com_ptr.h"
#include "../base/win32/com_task_mem_ptr.h"

#define EXPECT_SUCCESS VSE_EXPECT_SUCCESS
#define DEBUG_LOG VSE_DEBUG_LOG("vse::WasapiOutputDevice: ")
#define DEBUG_BREAK() (void(VSE_EXPECT_SUCCESS E_UNEXPECTED))

namespace vse
{
    class WasapiOutputDeviceImpl final : public virtual WasapiOutputDevice
    {
        WAVEFORMATEXTENSIBLE device_format_{};
        AUDCLNT_SHAREMODE device_sharing_mode_{};

        win32::manual_reset_event buffer_switch_event_{};
        win32::com_ptr<IAudioClient> audio_client_{};
        win32::com_ptr<IAudioRenderClient> audio_render_client_{};

    public:
        WasapiOutputDeviceImpl() { }

        ~WasapiOutputDeviceImpl() override { Close(); }

        bool Open() override
        {
            return Open(AUDCLNT_SHAREMODE_SHARED, {}, std::nullopt);
        }

        bool Open(const WAVEFORMATEXTENSIBLE& format) override
        {
            return Open(AUDCLNT_SHAREMODE_SHARED, format, std::nullopt);
        }

        bool OpenExclusive(const WAVEFORMATEXTENSIBLE& format) override
        {
            return Open(AUDCLNT_SHAREMODE_EXCLUSIVE, format, std::nullopt);
        }

        bool OpenExclusive(const WAVEFORMATEXTENSIBLE& format, std::chrono::microseconds device_period) override
        {
            using REFERENCE_TIME_duration = std::chrono::duration<REFERENCE_TIME, std::ratio<1, 10000000>>;
            return Open(AUDCLNT_SHAREMODE_EXCLUSIVE, format, std::chrono::duration_cast<REFERENCE_TIME_duration>(device_period).count());
        }

        bool Open(AUDCLNT_SHAREMODE deviceMode, const WAVEFORMATEXTENSIBLE& format, std::optional<REFERENCE_TIME> period)
        {
            // Create IAudioClient instance.
            DEBUG_LOG << "Creating IAudioClient instance....";
            win32::com_ptr<IAudioClient> audio_client{};
            {
                DEBUG_LOG << "  enumerator = CoCreateInstance(IMMDeviceEnumerator)...";
                win32::com_ptr<IMMDeviceEnumerator> enumerator{};
                win32::com_ptr<IMMDevice> device{};
                EXPECT_SUCCESS ::CoCreateInstance(
                    __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                    __uuidof(IMMDeviceEnumerator), enumerator.put_void());

                DEBUG_LOG << "  device = enumerator->GetDefaultAudioEndpoint...";
                if (HRESULT hr = EXPECT_SUCCESS enumerator->GetDefaultAudioEndpoint(eRender, eConsole, device.put());
                    FAILED(hr) || !device)
                {
                    DEBUG_LOG << "Failed to open device. : Failed to GetDefaultAudioEndpoint. HRESULT=0x" << std::hex << std::setfill('0') << std::setw(8) << hr;
                    return false;
                }

                DEBUG_LOG << "  device->Activate...";
                if (HRESULT hr = EXPECT_SUCCESS device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, audio_client.put_void());
                    FAILED(hr) || !audio_client)
                {
                    DEBUG_LOG << "Failed to open device. : Failed to Actiave AudioClient. HRESULT=0x" << std::hex << std::setfill('0') << std::setw(8) << hr;
                    return false;
                }
                DEBUG_LOG << "IAudioClient is ready.";
            }

            // GetDevicePeriod.
            DEBUG_LOG << "GetDevicePeriod:";
            REFERENCE_TIME default_device_period = 0;
            REFERENCE_TIME minimum_device_period = 0;
            EXPECT_SUCCESS audio_client->GetDevicePeriod(&default_device_period, &minimum_device_period);
            DEBUG_LOG << "  MinimumDevicePeriod: " << (static_cast<double>(minimum_device_period) / 10000.0) << "ms.";
            DEBUG_LOG << "  DefaultDevicePeriod: " << (static_cast<double>(default_device_period) / 10000.0) << "ms.";

            WAVEFORMATEXTENSIBLE device_format{};

            // Get default device format
            DEBUG_LOG << "Get default device format:";
            win32::com_task_mem_ptr<WAVEFORMATEX> default_device_format{};
            {
                EXPECT_SUCCESS audio_client->GetMixFormat(default_device_format.put());
                device_format_ = PcmWaveFormat::Parse(default_device_format.get());
                DEBUG_LOG << "  DefaultDeviceFormat: " << ToString(default_device_format.get());

                if (default_device_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
                {
                    device_format = *reinterpret_cast<WAVEFORMATEXTENSIBLE*>(default_device_format.get());
                }
                else
                {
                    device_format.Format = *default_device_format;
                    device_format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
                    device_format.Format.cbSize = static_cast<WORD>(sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX));
                    device_format.Samples.wValidBitsPerSample = device_format.Format.wBitsPerSample;
                    device_format.SubFormat = GUID{DEFINE_WAVEFORMATEX_GUID(default_device_format->wFormatTag)};
                    device_format.dwChannelMask = static_cast<DWORD>(DefaultChannelMask(default_device_format->nChannels));
                }
            }

            // Decide the device format.
            DWORD stream_flags = 0;
            stream_flags |= AUDCLNT_STREAMFLAGS_NOPERSIST;
            stream_flags |= AUDCLNT_STREAMFLAGS_EVENTCALLBACK;

            HRESULT initialization_result = E_FAIL;

            if (deviceMode == AUDCLNT_SHAREMODE_EXCLUSIVE)
            {
                // -- EXCLUSIVE MODE --
                // Check the format is supported by the device.
                DEBUG_LOG << "Cheking Exclusive mode @ " << ToString(format) << "...";

                if (HRESULT hr = audio_client->IsFormatSupported(
                        AUDCLNT_SHAREMODE_EXCLUSIVE,
                        &format.Format,
                        NULL);
                    FAILED(hr))
                {
                    DEBUG_LOG << "Exclusive mode @ " << ToString(format) << " is not supported.";
                    DEBUG_LOG << "Failed to open device.";
                    return false;
                }

                device_format = format;
            }
            else
            {
                // -- SHARED MODE --
                DEBUG_LOG << "Checking Shared mode...";

                // Queries IAudioClient3 interface that supported since Windows 10.
                if (win32::com_ptr<IAudioClient3> audio_client3 = audio_client; audio_client3)
                {
                    DEBUG_LOG << "IAudioClient3 is supported.";
                    if (format.Format.wFormatTag != WAVE_FORMAT_UNKNOWN)
                    {
                        device_format = format;
                    }

                    UINT32 defaultPeriod, fundamental, minimumPeriod, maximumPeriod;
                    if (HRESULT hr = audio_client3->GetSharedModeEnginePeriod(
                            reinterpret_cast<const WAVEFORMATEX*>(&device_format),
                            &defaultPeriod,
                            &fundamental,
                            &minimumPeriod,
                            &maximumPeriod);
                        FAILED(hr))
                    {
                        DEBUG_LOG << "Failed to GetSharedModeEnginePeriod. HRESULT = 0x" << std::hex << std::setfill('0') << std::setw(8) << hr;
                        DEBUG_LOG << "Falling back to default initialization...";
                    }
                    else
                    {
                        DEBUG_LOG << "Using DevicePeriod: " << (static_cast<double>(minimumPeriod) / device_format.Format.nSamplesPerSec) * 1000.0 << "ms.";
                        DEBUG_LOG << "InitializeSharedAudioStream @ " << ToString(device_format) << ".";
                        if (initialization_result = audio_client3->InitializeSharedAudioStream(
                                AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                minimumPeriod,
                                &device_format.Format,
                                nullptr);
                            FAILED(initialization_result))
                        {
                            DEBUG_LOG << "Failed to InitializeSharedAudioStream. HRESULT = 0x" << std::hex << std::setfill('0') << std::setw(8) << initialization_result;
                            DEBUG_LOG << "Falling back to default initialization...";
                        }
                    }
                }
                else
                {
                    DEBUG_LOG << "IAudioClient3 is NOT supported.";
                }

                if (format.Format.wFormatTag != WAVE_FORMAT_UNKNOWN)
                {
                    device_format = format;
                    stream_flags |= AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM; // from Windows 10.0.14393.0
                    DEBUG_LOG << "Using auto PCM format converion.";
                }
            }

            // Initializing...
            if (!SUCCEEDED(initialization_result))
            {
                DEBUG_LOG << "Initializing " << (deviceMode == AUDCLNT_SHAREMODE_EXCLUSIVE ? "Exclusive" : "Shared") << " mode @ " << ToString(device_format) << "...";

                REFERENCE_TIME device_period = period ? *period : deviceMode == AUDCLNT_SHAREMODE_EXCLUSIVE ? minimum_device_period : default_device_period;
                DEBUG_LOG << "Using DevicePeriod: " << static_cast<double>(device_period) / 10000.0 << "ms.";

                initialization_result = audio_client->Initialize(
                    deviceMode,
                    stream_flags,
                    device_period,
                    device_period,
                    &device_format.Format,
                    nullptr);

                if (initialization_result == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED)
                {
                    UINT32 frame = 0;
                    audio_client->GetBufferSize(&frame);

                    const REFERENCE_TIME device_pereod_fixed = static_cast<REFERENCE_TIME>(static_cast<double>(frame) * 10000000.0 / device_format.Format.nSamplesPerSec + .5);
                    DEBUG_LOG << "Using Fixed DevicePeriod: " << static_cast<double>(device_pereod_fixed) / 10000.0 << "ms.";

                    initialization_result = audio_client->Initialize(
                        deviceMode,
                        stream_flags,
                        device_pereod_fixed,
                        device_pereod_fixed,
                        &device_format.Format,
                        NULL);
                }
            }

            if (FAILED(initialization_result))
            {
                DEBUG_LOG << "Failed to open device. : Failed to initialize AudioClient. HRESULT=0x" << std::hex << std::setfill('0') << std::setw(8) << initialization_result;
                return false;
            }

            // OK.
            DEBUG_LOG << "AudioClient is initialized successfully.";

            EXPECT_SUCCESS audio_client->SetEventHandle(buffer_switch_event_.handle());

            // Get IAudioRenderClient interface.
            win32::com_ptr<IAudioRenderClient> audio_reder_client{};
            EXPECT_SUCCESS audio_client->GetService(__uuidof(IAudioRenderClient), audio_reder_client.put_void());

            // Clear the buffer (first data).
            if (deviceMode == AUDCLNT_SHAREMODE_EXCLUSIVE)
            {
                UINT32 frames = 0;
                LPBYTE ptr = nullptr;
                EXPECT_SUCCESS audio_client->GetBufferSize(&frames);
                EXPECT_SUCCESS audio_reder_client->GetBuffer(frames, &ptr);
                if (ptr) { memset(ptr, 0, frames * device_format.Format.nBlockAlign); }
                EXPECT_SUCCESS audio_reder_client->ReleaseBuffer(frames, AUDCLNT_BUFFERFLAGS_SILENT);
            }

            device_format_ = device_format;
            device_sharing_mode_ = deviceMode;
            audio_client_ = std::move(audio_client);
            audio_render_client_ = std::move(audio_reder_client);

            DEBUG_LOG << "WasapiOutputDevice READY.";
            return true;
        }

        WAVEFORMATEXTENSIBLE GetFormat() const override
        {
            return device_format_;
        }

        bool Start() override
        {
            if (!audio_client_) return DEBUG_BREAK(), false; // not initialized

            DEBUG_LOG << "Starting audio client...";

            if (HRESULT hr = EXPECT_SUCCESS audio_client_->Start(); FAILED(hr))
                return (DEBUG_LOG << "Failed to start audio client..."), false;

            DEBUG_LOG << "Audio client started.";
            return true;
        }

        int GetBufferSize() const override
        {
            if (!audio_client_) return DEBUG_BREAK(), 0; // not initialized

            UINT32 frames = 0;
            EXPECT_SUCCESS audio_client_->GetBufferSize(&frames);

            UINT32 padding = 0;
            if (device_sharing_mode_ == AUDCLNT_SHAREMODE_SHARED)
                EXPECT_SUCCESS audio_client_->GetCurrentPadding(&padding);

            return (frames - padding) * device_format_.Format.nBlockAlign;
        }

        void* LockBuffer(int buffer_size) override
        {
            if (!audio_client_) return DEBUG_BREAK(), nullptr; // not initialized

            BYTE* locked{};
            if (device_sharing_mode_ == AUDCLNT_SHAREMODE_SHARED || buffer_switch_event_.wait_signal(1000))
                EXPECT_SUCCESS audio_render_client_->GetBuffer(buffer_size / device_format_.Format.nBlockAlign, &locked);

            return locked;
        }

        void UnlockBuffer(int wrote_bytes) override
        {
            if (!audio_client_) return DEBUG_BREAK(), void{}; // not initialized

            audio_render_client_->ReleaseBuffer(wrote_bytes / device_format_.Format.nBlockAlign, 0);
        }

        void Reset() override
        {
            if (audio_client_)
            {
                DEBUG_LOG << "Resetting audio client...";
                audio_client_->Stop();
                audio_client_->Reset();
            }
        }

        void Close() override
        {
            if (audio_client_)
            {
                DEBUG_LOG << "Closing Audio Client...";
                audio_client_->Stop();
                audio_client_->Reset();
                DEBUG_LOG << "Device is closed.";
            }

            audio_client_.reset();
            audio_render_client_.reset();
        }
    };

    std::shared_ptr<WasapiOutputDevice> CreateWasapiOutputDevice()
    {
        return std::make_shared<WasapiOutputDeviceImpl>();
    }
}
