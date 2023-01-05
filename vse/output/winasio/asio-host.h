/// @file
/// @brief  winasio - AsioHost
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>
#include <combaseapi.h>

#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <functional>

#include "asio.h"
#include "asio-callback-allocator.h"

namespace asio
{
    class AsioHost final
    {
    public:
        struct DeviceInfo
        {
            char driver_name[256]{};
            AsioLong driver_version{};

            AsioLong input_channel_count{};
            AsioLong output_channel_count{};
            AsioLong minimum_buffer_size{};
            AsioLong maximum_buffer_size{};
            AsioLong preferred_buffer_size{};
            AsioLong buffer_size_granularity{};
            AsioSamplingRate default_sampling_rate{};
            AsioBool supports_output_ready_optimization{};
        };

        struct BufferInfo
        {
            AsioLong channel_number;
            AsioBool for_input;
            AsioSampleType sample_type;
            AsioLong sample_count;
            AsioLong latency;
            void* buffers[2];
        };

        using BufferSwitchedCallbackFunction = std::function<
            void(const BufferInfo* channels, size_t channel_count, int buffer_index, bool do_process_now)>;

    private:
        struct default_com_delete
        {
            void operator()(IUnknown* p) const noexcept { if (p) p->Release(); }
        };

        template <class T> using ComPtr = std::unique_ptr<T, default_com_delete>;

        CLSID driver_clsid_{};
        ComPtr<IAsio> driver_{};
        DeviceInfo device_info_{};

        std::shared_ptr<AsioCallbacks> buffer_callback_{};
        std::vector<BufferInfo> buffer_info_{};
        BufferSwitchedCallbackFunction buffer_switched_callback_{};

    public:
        explicit AsioHost(CLSID clsid) : driver_clsid_(clsid) {}
        AsioHost(const AsioHost& other) = delete;
        AsioHost(AsioHost&& other) noexcept = delete;
        AsioHost& operator=(const AsioHost& other) = delete;
        AsioHost& operator=(AsioHost&& other) noexcept = delete;
        ~AsioHost() { Close(); }

        const DeviceInfo* Initialize()
        {
            if (driver_)
                throw std::logic_error("driver already initialized.");

            ComPtr<IAsio> driver{};
            DeviceInfo device_info{};

            // Instantiate driver
            {
                // REQUIREMENT: calling thread is STAThread.
                IAsio* instance;
                if (HRESULT hr = ::CoCreateInstance(
                        driver_clsid_,
                        nullptr, CLSCTX_INPROC_SERVER,
                        driver_clsid_, reinterpret_cast<void**>(&instance));
                    FAILED(hr) || !instance)
                {
                    throw std::runtime_error("FAILED to load the driver! HR=" + std::to_string(hr));
                }
                driver.reset(instance);
            }

            // Initialize driver
            {
                char error_message[256]{};
                if (!driver->Initialize(nullptr))
                {
                    driver->GetErrorMessage(error_message);
                    throw std::runtime_error("FAILED to Initialize driver: " + std::string(error_message));
                }

                driver->GetDriverName(device_info.driver_name);
                device_info.driver_version = driver->GetDriverVersion();
            }

            // Gather driver properties
            {
                // get the number of available channels
                if (AsioError result = driver->GetChannels(
                        &device_info.input_channel_count,
                        &device_info.output_channel_count);
                    result != ASE_OK)
                {
                    throw std::runtime_error("FAILED to driver->GetChannels(): E=" + std::to_string(result));
                }

                // get the usable buffer sizes
                if (AsioError result = driver->GetBufferSize(
                        &device_info.minimum_buffer_size,
                        &device_info.maximum_buffer_size,
                        &device_info.preferred_buffer_size,
                        &device_info.buffer_size_granularity);
                    result != ASE_OK)
                {
                    throw std::runtime_error("FAILED to driver->GetBufferSize(): E=" + std::to_string(result));
                }

                // get the default sample rate
                if (AsioError result = driver->GetSampleRate(&device_info.default_sampling_rate);
                    result != ASE_OK)
                {
                    driver->SetSampleRate(44100.0); // try to set 44.1KHz
                    if (result = driver->GetSampleRate(&device_info.default_sampling_rate);
                        result != ASE_OK)
                    {
                        throw std::runtime_error("FAILED to driver->GetSampleRate(): E=" + std::to_string(result));
                    }
                }

                // check whether the driver requires the AsioOutputReady() optimization
                device_info.supports_output_ready_optimization = driver->NotifyOutputReady() == ASE_OK;
            }

            driver_ = std::move(driver);
            device_info_ = std::move(device_info);
            return &device_info_;
        }

        void Close()
        {
            if (!driver_) return;
            if (driver_) this->Stop();
            if (driver_) this->DisposeBuffers();
            driver_.reset();
        }

        bool OpenControlPanel()
        {
            if (!driver_)
                throw std::logic_error("not initialized.");

            return driver_->OpenControlPanel() == ASE_OK;
        }

        bool CanSamplingRate(double sampling_frequency)
        {
            if (!driver_)
                throw std::logic_error("not initialized.");

            return driver_->CanSampleRate(sampling_frequency) == ASE_OK;
        }

        bool GetSamplingRate(double* sampling_frequency)
        {
            if (!driver_)
                throw std::logic_error("not initialized.");

            AsioSamplingRate got{};
            AsioError result = driver_->GetSampleRate(&got);
            if (sampling_frequency) { *sampling_frequency = got; }
            return result == ASE_OK;
        }

        bool SetSamplingRate(double sampling_frequency)
        {
            if (!driver_)
                throw std::logic_error("not initialized.");

            // Sets sampling rate.
            if (AsioError result = driver_->SetSampleRate(sampling_frequency);
                result != ASE_OK)
            {
                // continue anyway...
            }

            AsioSamplingRate got{};
            bool ret = this->GetSamplingRate(&got);
            return ret && std::abs(got - sampling_frequency) < 1.0;
        }

        const BufferInfo* PrepareBuffers(int input_channel_count, int output_channel_count)
        {
            if (!driver_)
                throw std::logic_error("not initialized.");

            if (buffer_callback_)
                throw std::logic_error("already prepared.");

            if (input_channel_count + output_channel_count == 0)
                throw std::invalid_argument("there are no channels.");

            if (input_channel_count > device_info_.input_channel_count)
                throw std::invalid_argument("input_channel_count is too large.");

            if (output_channel_count > device_info_.output_channel_count)
                throw std::invalid_argument("output_channel_count is too large.");

            const auto buffer_sample_count = device_info_.preferred_buffer_size;
            std::vector<AsioBufferInfo> buffer_info_array;
            for (int i = 0; i < input_channel_count; i++) buffer_info_array.push_back({true, i, {nullptr, nullptr}});
            for (int i = 0; i < output_channel_count; i++) buffer_info_array.push_back({false, i, {nullptr, nullptr}});

            // allocate callback
            std::shared_ptr<AsioCallbacks> buffer_callback = util::AsioCallbackAllocator::AllocateShared({
                this,
                [](void* t, AsioLong buffer_index, AsioBool do_direct_processing) -> void { static_cast<AsioHost*>(t)->BufferSwitchedCallback(buffer_index, do_direct_processing); },
                []([[maybe_unused]] void* t, [[maybe_unused]] AsioSamplingRate new_sampling_rate) -> void {},
                [](void* t, AsioMessage selector, AsioLong value, void* message, double* opt) -> AsioLong { return static_cast<AsioHost*>(t)->HandleMessageCallback(selector, value, message, opt); },
                [](void* t, AsioTime*, AsioLong buffer_index, AsioBool do_direct_processing) -> AsioTime* { return static_cast<AsioHost*>(t)->BufferSwitchedCallback(buffer_index, do_direct_processing), nullptr; },
            });

            // create and activate buffers
            if (AsioError result = driver_->CreateBuffers(
                    buffer_info_array.data(),
                    static_cast<AsioLong>(buffer_info_array.size()),
                    static_cast<AsioLong>(buffer_sample_count),
                    buffer_callback.get());
                result != ASE_OK)
            {
                throw std::runtime_error("FAILED to driver->CreateBuffers(): E= " + std::to_string(result));
            }

            // get channel info
            std::vector<BufferInfo> buffer_info;
            for (const auto& b : buffer_info_array)
            {
                // get channel info
                AsioChannelInfo channel_info{};
                channel_info.channel = b.channel_number;
                channel_info.is_input = b.is_input;
                if (AsioError result = driver_->GetChannelInfo(&channel_info);
                    result != ASE_OK)
                {
                    driver_->DisposeBuffers();
                    throw std::runtime_error("FAILED to driver->GetChannelInfo(): E= " + std::to_string(result));
                }

                // get latency
                AsioLong input_latency{};
                AsioLong output_latency{};
                if (AsioError result = driver_->GetLatencies(&input_latency, &output_latency);
                    result != ASE_OK)
                {
                    driver_->DisposeBuffers();
                    throw std::runtime_error("FAILED to driver->GetLatencies(): E= " + std::to_string(result));
                }

                // store buffer info
                BufferInfo dst{};
                dst.channel_number = b.channel_number;
                dst.for_input = b.is_input;
                dst.sample_type = channel_info.type;
                dst.sample_count = buffer_sample_count;
                dst.buffers[0] = b.buffers[0];
                dst.buffers[1] = b.buffers[1];
                dst.latency = dst.for_input ? input_latency : output_latency;
                buffer_info.push_back(dst);
            }

            // Ready.
            buffer_info_ = std::move(buffer_info);
            buffer_callback_ = std::move(buffer_callback);
            return buffer_info_.data();
        }

        void DisposeBuffers()
        {
            if (!driver_)
                throw std::logic_error("not initialized.");

            if (buffer_callback_)
            {
                driver_->DisposeBuffers();
                buffer_callback_.reset();
                buffer_info_.clear();
            }
        }

        bool Start(BufferSwitchedCallbackFunction callback)
        {
            if (!driver_)
                throw std::logic_error("not initialized.");

            buffer_switched_callback_ = std::move(callback);
            std::atomic_thread_fence(std::memory_order_seq_cst);

            if (AsioError result = driver_->Start();
                result != ASE_OK)
            {
                throw std::runtime_error("FAILED to driver->Start(): E=" + std::to_string(result));
            }
            return true;
        }

        bool Stop()
        {
            if (!driver_)
                throw std::logic_error("not initialized.");

            if (AsioError result = driver_->Stop();
                result != ASE_OK)
            {
                throw std::runtime_error("FAILED to driver->Stop(): E=" + std::to_string(result));
            }

            std::atomic_thread_fence(std::memory_order_seq_cst);
            buffer_switched_callback_ = nullptr;
            return true;
        }

        void NotifyOutputBufferReady()
        {
            if (driver_ && device_info_.supports_output_ready_optimization)
                driver_->NotifyOutputReady();
        }

    private:
        AsioLong HandleMessageCallback(AsioMessage selector, AsioLong value, [[maybe_unused]] void* message, [[maybe_unused]] double* opt)
        {
            switch (selector)
            {
            case AsioMessage::SelectorSupported:
                switch (static_cast<AsioMessage>(value))
                {
                case AsioMessage::EngineVersion:
                case AsioMessage::ResetRequest:
                case AsioMessage::ResyncRequest:
                case AsioMessage::LatenciesChanged:
                case AsioMessage::SupportsTimeInfo:
                case AsioMessage::SupportsTimeCode:
                    return 1; // = supported.
                case AsioMessage::SelectorSupported:
                case AsioMessage::BufferSizeChange:
                default:
                    return 0; // = not supported
                }

            case AsioMessage::EngineVersion: return 2;
            case AsioMessage::ResetRequest: return 1;
            case AsioMessage::ResyncRequest: return 1;
            case AsioMessage::LatenciesChanged: return 1;
            case AsioMessage::SupportsTimeInfo: return 0;
            case AsioMessage::SupportsTimeCode: return 0;

            case AsioMessage::BufferSizeChange:
            default:
                break; // = not supported
            }
            return 0;
        }

        void BufferSwitchedCallback(AsioLong buffer_index, AsioBool direct_processing)
        {
            if (buffer_switched_callback_)
                buffer_switched_callback_(
                    this->buffer_info_.data(),
                    this->buffer_info_.size(),
                    static_cast<int>(buffer_index),
                    static_cast<bool>(direct_processing));
        }
    };
}
