/// @file
/// @brief  Vse - Audio Rendering Thread
/// @author (C) 2022 ttsuki

#include "AudioRenderingThread.h"

#include <Windows.h>
#include <avrt.h>
#pragma comment(lib, "Avrt.lib")

#include <atomic>
#include <memory>

#include "../base/win32/com_base.h"
#include "../base/win32/event.h"
#include "../base/win32/thread.h"

namespace vse
{
    std::shared_ptr<AudioRenderingThread> CreateAudioRenderingThread(std::shared_ptr<IWaveSource> source, std::shared_ptr<IOutputDevice> destination)
    {
        class AudioRenderingThreadImpl final : public virtual AudioRenderingThread
        {
            std::shared_ptr<IWaveSource> input_{};
            std::shared_ptr<IOutputDevice> output_{};
            std::atomic_flag running_{};
            win32::thread thread_{};

        public:
            AudioRenderingThreadImpl(std::shared_ptr<IWaveSource> input, std::shared_ptr<IOutputDevice> output)
                : input_(std::move(input))
                , output_(std::move(output))
            {
                //
            }

            AudioRenderingThreadImpl(const AudioRenderingThreadImpl& other) = delete;
            AudioRenderingThreadImpl(AudioRenderingThreadImpl&& other) noexcept = delete;
            AudioRenderingThreadImpl& operator=(const AudioRenderingThreadImpl& other) = delete;
            AudioRenderingThreadImpl& operator=(AudioRenderingThreadImpl&& other) noexcept = delete;

            ~AudioRenderingThreadImpl() override
            {
                AudioRenderingThreadImpl::Stop();
            }

            void Start() override
            {
                if (thread_.joinable()) throw std::logic_error("thread is already started.");

                running_.test_and_set();
                thread_ = win32::thread([this] { this->AudioRenderThreadProc(); }, 8 * 1048576);
            }

            void Stop() override
            {
                if (thread_.joinable())
                {
                    running_.clear();
                    thread_.join();
                }
            }

        private:
            unsigned AudioRenderThreadProc()
            {
                win32::CoInitializeMTA();

                DWORD task_index = 0;
                win32::unique_handle_t task_handle{::AvSetMmThreadCharacteristicsW(L"Pro Audio", &task_index), &::AvRevertMmThreadCharacteristics};

                if (task_handle)
                {
                    ::AvSetMmThreadPriority(task_handle.get(), AVRT_PRIORITY_CRITICAL);
                }

                if (output_->Start())
                {
                    while (running_.test_and_set())
                    {
                        const int bufferSize = output_->GetBufferSize();

                        if (auto* buf = static_cast<std::byte*>(output_->LockBuffer(bufferSize)))
                        {
                            size_t wrote = input_ ? input_->Read(buf, bufferSize) : 0;
                            memset(buf + wrote, 0, bufferSize - wrote);
                            output_->UnlockBuffer(bufferSize);
                        }
                    }

                    output_->Reset();
                }

                task_handle.reset();

                win32::CoUninitialize();
                return 0;
            }
        };

        return std::make_shared<AudioRenderingThreadImpl>(std::move(source), std::move(destination));
    }
}
