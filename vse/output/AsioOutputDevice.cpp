/// @file
/// @brief  Vse - Asio Output Device
/// @author (C) 2023 ttsuki

#include "AsioOutputDevice.h"

#include <Windows.h>
#include <avrt.h>
#pragma comment(lib, "Avrt.lib")

#include <memory>
#include <atomic>
#include <utility>
#include <string>
#include <vector>

#include "winasio/asio.h"
#include "winasio/asio-host.h"
#include "winasio/asio-enumerator.h"
#include "winasio/asio-data-transfering.h"

#include "../base/win32/debug.h"
#include "../base/win32/com_base.h"
#include "../base/win32/com_ptr.h"

#include "../base/xtl/xtl_temp_memory_buffer.h"
#include "../base/xtl/xtl_manual_reset_event.h"
#include "../base/xtl/xtl_single_thread.h"

#define DEBUG_LOG VSE_DEBUG_LOG("vse::AsioDevice: ")
#define DEBUG_BREAK() VSE_DEBUG_BREAK()

namespace vse
{
    class AsioOutputDeviceImpl final : public virtual AsioOutputDevice
    {
        const GUID clsid_{};
        xtl::single_thread host_thread_{};

        std::unique_ptr<asio::AsioHost> driver_{};
        const asio::AsioHost::DeviceInfo* device_{};
        const asio::AsioHost::BufferInfo* asio_buffers_{};

        WAVEFORMATEXTENSIBLE local_buffer_format_{};
        int local_buffer_size_{};
        xtl::temp_memory_buffer local_buffer_{};
        std::byte* local_buffer_locked_{};

        std::atomic_int next_buffer_index_{};
        xtl::manual_reset_event buffer_switched_event_{};
        xtl::manual_reset_event buffer_processed_event_{};

        enum struct SchedulingMode
        {
            DoubleBuffered,
            TripleBuffered,
        } const processing_mode_ = SchedulingMode::DoubleBuffered;

    public:
        explicit AsioOutputDeviceImpl(const GUID& device_clsid)
            : clsid_(device_clsid)
        {
            host_thread_.invoke([]
            {
                // Some ASIO drivers need to be on STAThread
                win32::CoInitializeSTA();

                DWORD task_index = 0;
                ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
                ::AvSetMmThreadCharacteristicsW(L"Pro Audio", &task_index);
            });
        }

        AsioOutputDeviceImpl(const AsioOutputDeviceImpl& other) = delete;
        AsioOutputDeviceImpl(AsioOutputDeviceImpl&& other) noexcept = delete;
        AsioOutputDeviceImpl& operator=(const AsioOutputDeviceImpl& other) = delete;
        AsioOutputDeviceImpl& operator=(AsioOutputDeviceImpl&& other) noexcept = delete;
        ~AsioOutputDeviceImpl() override { AsioOutputDeviceImpl::Close(); }

        bool Open() override
        {
            return Open(0, 2);
        }

        bool Open(int sampling_frequency, int channel_count) override
        {
            using namespace asio;

            // some devices need retry to set sampling rate.
            for (int retry = 0; retry < 3; Sleep(1000), ++retry)
            {
                auto driver = std::make_unique<AsioHost>(clsid_);
                double driver_sampling_rate{};
                const AsioHost::DeviceInfo* device{};
                const AsioHost::BufferInfo* buffers{};

                bool ready = host_thread_.invoke([&]() -> bool
                {
                    try
                    {
                        DEBUG_LOG << "Initializing driver... " << win32::to_string(clsid_) << "\n";
                        driver->Initialize();
                    }
                    catch (const std::runtime_error& e)
                    {
                        DEBUG_LOG << "FAILED: An error is occurred: " << e.what();
                        return false;
                    }

                    try
                    {
                        if (sampling_frequency != 0)
                        {
                            DEBUG_LOG << "Checking driver supports sampling frequency " << sampling_frequency << "Hz...";
                            if (!driver->CanSamplingRate(sampling_frequency))
                            {
                                DEBUG_LOG << "FAILED: Sampling frequency is not supported by device!";
                                return false;
                            }

                            DEBUG_LOG << "Setting sampling frequency to " << sampling_frequency << "Hz... ";
                            if (!driver->SetSamplingRate(sampling_frequency))
                            {
                                DEBUG_LOG << "FAILED: Sampling frequency is not supported by driver!";
                                return false;
                            }
                        }

                        DEBUG_LOG << "Getting current sampling frequency.";
                        if (!driver->GetSamplingRate(&driver_sampling_rate))
                        {
                            DEBUG_LOG << "FAILED to get current sampling frequency.";
                            return false;
                        }
                    }
                    catch (const std::runtime_error& e)
                    {
                        DEBUG_LOG << "FAILED: An error is occurred: " << e.what();
                        return false;
                    }

                    try
                    {
                        DEBUG_LOG << "Preparing buffers...\n";
                        buffers = driver->PrepareBuffers(0, channel_count);
                    }
                    catch (const std::runtime_error& e)
                    {
                        DEBUG_LOG << "FAILED: An error is occurred: " << e.what();
                        return false;
                    }

                    std::atomic_thread_fence(std::memory_order_release);
                    return true;
                });
                if (!ready) continue;
                std::atomic_thread_fence(std::memory_order_acquire);

                // Ready.
                DEBUG_LOG << "READY.\n";
                driver_ = std::move(driver);
                device_ = device;
                asio_buffers_ = buffers;

                local_buffer_format_ = PcmWaveFormat{SampleType::S32, DefaultChannelMask(channel_count), static_cast<int>(std::round(driver_sampling_rate))};
                local_buffer_size_ = static_cast<int>(channel_count * sizeof(S32) * buffers->sample_count);
                (void)local_buffer_.get(local_buffer_size_);
                DEBUG_LOG << "Local buffer size = " << local_buffer_size_ << " bytes allocated.";

                break;
            }

            return driver_ != nullptr;
        }

        WAVEFORMATEXTENSIBLE GetFormat() const override
        {
            return local_buffer_format_;
        }

        int GetBufferSize() const override
        {
            return local_buffer_size_;
        }

        bool Start() override
        {
            if (!driver_) return DEBUG_BREAK(), false;

            return host_thread_.invoke([this]
            {
                try
                {
                    DEBUG_LOG << "Starting...";
                    if (!driver_->Start(
                        [this](const asio::AsioHost::BufferInfo*, size_t, int buffer_index, bool do_process_now)
                        {
                            next_buffer_index_.store(buffer_index, std::memory_order_release);
                            buffer_switched_event_.notify_signal();

                            if (do_process_now)
                            {
                                if (buffer_processed_event_.wait_for(std::chrono::milliseconds(20)))
                                    buffer_processed_event_.reset_signal();
                            }
                        }
                    ))
                    {
                        DEBUG_LOG << "Failed to start.";
                        return false;
                    }
                }
                catch (const std::runtime_error& e)
                {
                    DEBUG_LOG << "FAILED: An error is occurred: " << e.what();
                    return false;
                }
                DEBUG_LOG << "Started.";
                return true;
            });
        }


        void* LockBuffer(int buffer_size) override
        {
            if (!driver_) return DEBUG_BREAK(), nullptr;                          // not initialized.
            if (local_buffer_locked_) return DEBUG_BREAK(), nullptr;              // already locked.
            if (buffer_size != local_buffer_size_) return DEBUG_BREAK(), nullptr; // invalid size.
            if (processing_mode_ == SchedulingMode::DoubleBuffered)
            {
                // wait for ready to write
                if (buffer_switched_event_.wait_for(std::chrono::milliseconds(1000)))
                    buffer_switched_event_.reset_signal();
            }

            return local_buffer_locked_ = local_buffer_.get(local_buffer_size_);
        }

        void UnlockBuffer(int wrote_bytes) override
        {
            if (!driver_) return DEBUG_BREAK();              // not initialized.
            if (!local_buffer_locked_) return DEBUG_BREAK(); // not locked yet.

            if (wrote_bytes < local_buffer_size_)
                memset(local_buffer_locked_ + wrote_bytes, 0, local_buffer_size_ - wrote_bytes);

            if (processing_mode_ == SchedulingMode::TripleBuffered)
            {
                // wait for ready to write
                if (buffer_switched_event_.wait_for(std::chrono::milliseconds(1000)))
                    buffer_switched_event_.reset_signal();
            }

            // transfer data to device buffer from input buffer
            {
                const int channel_count = local_buffer_format_.Format.nChannels;
                const int buffer_index = next_buffer_index_.load(std::memory_order_acquire);

                for (int i = 0; i < channel_count; i++)
                {
                    asio::util::TransferSingleChannelFromInterleavedSource(
                        asio_buffers_[i].buffers[buffer_index],
                        asio_buffers_[i].sample_type,
                        reinterpret_cast<const int32_t*>(local_buffer_locked_),
                        channel_count,
                        i,
                        asio_buffers_[i].sample_count);
                }
            }

            // notify buffer writing is completed
            buffer_processed_event_.notify_signal();
            driver_->NotifyOutputBufferReady();

            local_buffer_locked_ = nullptr;
        }

        void Reset() override
        {
            if (!driver_) return DEBUG_BREAK(), void{};

            (void)host_thread_.invoke([this]
            {
                try
                {
                    DEBUG_LOG << "Stopping...";
                    if (!driver_->Stop())
                    {
                        DEBUG_LOG << "Failed to stop.";
                        return false;
                    }
                    DEBUG_LOG << "Stopped.";
                    return true;
                }
                catch (const std::runtime_error& e)
                {
                    DEBUG_LOG << "FAILED: An error is occurred: " << e.what();
                    return false;
                }
            });
        }

        void Close() override
        {
            if (driver_)
            {
                host_thread_.invoke([this] { driver_->Close(); });
                driver_.reset();
            }
        }

        void OpenControlPanel() override
        {
            host_thread_.invoke([this]
            {
                try
                {
                    if (!driver_)
                    {
                        if (auto driver = std::make_unique<asio::AsioHost>(clsid_))
                        {
                            DEBUG_LOG << "Initialize driver to open control panel...";
                            driver->Initialize();
                            DEBUG_LOG << "Opening ASIO control panel...";
                            driver->OpenControlPanel();
                            DEBUG_LOG << "Closed.";
                        }
                    }
                    else
                    {
                        DEBUG_LOG << "Opening ASIO control panel...";
                        driver_->OpenControlPanel();
                        DEBUG_LOG << "Closed.";
                    }
                }
                catch (const std::exception& e)
                {
                    DEBUG_LOG << "FAILED: An error is occurred: " << e.what();
                }
            });
        }

        int GetLatency() const override
        {
            return asio_buffers_ ? asio_buffers_->latency : 0;
        }
    };

    std::shared_ptr<AsioOutputDevice> CreateAsioOutputDevice(GUID id)
    {
        return std::make_shared<AsioOutputDeviceImpl>(id);
    }


    std::vector<AsioDeviceDescription> EnumerateAllAsioDevices()
    {
        std::vector<AsioDeviceDescription> result;

        for (auto&& [clsid, name] : asio::util::EnumerateAsioDrivers())
            result.push_back({clsid, name});

        return result;
    }
}
