#include <Windows.h>
#include <mfapi.h>

#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

#include "../vse/base/IWaveSource.h"
#include "../vse/output/AudioRenderingThread.h"
#include "../vse/output/AsioOutputDevice.h"

#include "../vse/loader/WaveFileLoader.h"
#include "../vse/base/win32/com_base.h"

#include "./utils/OutputDebugStringIATHook.h"

int wmain(int argc, const wchar_t* argv[])
{
    HookOutputDebugStringA([](LPCSTR debug)
    {
        Original_OutputDebugStringA(debug);
        std::clog << "DEBUG: " << debug;
    });

    if (argc <= 1)
    {
        std::clog << "Please give file path to play command line.\n";
        return 1;
    }

    ::CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
    ::MFStartup(MF_VERSION, MFSTARTUP_LITE);

    // Select an asio device.
    vse::AsioDeviceDescription device_desc{};
    {
        std::clog << "Enumerating ASIO drivers... \n";
        const auto all_asio_devices = vse::EnumerateAllAsioDevices();
        if (all_asio_devices.empty())
        {
            std::clog << "FATAL: There is no ASIO device.\n";
            return 1;
        }

        for (size_t i = 0; i < all_asio_devices.size(); ++i)
        {
            auto&& desc = all_asio_devices[i];
            std::wclog << L"  - driver[" << std::to_wstring(i) << "] = " << desc.DeviceName << " " << vse::win32::to_wstring(desc.Id) << L"\n";
        }
        std::wclog << L"---\n" << std::flush;

        size_t device_index = 0;
        std::wcout << L"Select driver index: " << std::flush;
        if (argc > 2)
        {
            std::wistringstream(argv[2]) >> device_index;
            std::wcout << device_index << std::endl;
        }
        else
        {
            std::wcin >> device_index;
        }

        if (device_index >= all_asio_devices.size())
        {
            std::wclog << "Index out of range.\n";
            device_index = 0;
        }

        device_desc = all_asio_devices[device_index]; // selected
    }

    {
        std::wclog << L"Loading device driver... " << device_desc.DeviceName << L"\n";
        auto device = vse::CreateAsioOutputDevice(device_desc.Id);

        std::clog << "Opening control panel...\n";
        device->OpenControlPanel();

        std::clog << "Opening device...\n";
        if (!device->Open(44100, 2))
        {
            std::clog << "FAILED to Open device!" << std::endl;
            return 1;
        }

        std::clog << "Device initialized... " << vse::ToString(device->GetFormat()) << "\n"
            << " / BufferSize: " << device->GetBufferSize() / device->GetFormat().Format.nBlockAlign << " Samples. (" << (device->GetBufferSize() / device->GetFormat().Format.nBlockAlign * 1000.0 / device->GetFormat().Format.nSamplesPerSec) << "ms)\n"
            << " / Latency: " << device->GetLatency() << " Samples (" << (device->GetLatency() * 1000.0 / device->GetFormat().Format.nSamplesPerSec) << "ms)\n";

        auto device_format_pcm = vse::PcmWaveFormat::Parse(device->GetFormat());
        auto file_format_pcm = vse::PcmWaveFormat(vse::SampleType::S16, device_format_pcm.channels_, device_format_pcm.frequency_);

        // Loading file
        std::clog << "Opening wave file... \n";
        std::shared_ptr<vse::ISeekableWaveSource> file = vse::AllocateReadCursor(vse::LoadAudioFile(argv[1], file_format_pcm));

        std::clog << "Loaded. "
            << " TotalSampleCount=" << file->GetTotalSampleCount() << "."
            << " memory=" << file->GetTotalSampleCount() * file->GetFormat().BlockAlign() / 1024 << "KB."
            << " time=" << std::fixed << std::setprecision(3) << (static_cast<double>(file->GetTotalSampleCount()) / file->GetFormat().SamplingFrequency()) << "s."
            << "\n";

        std::shared_ptr<vse::IWaveSource> src = file;

        {
            std::clog << "Starting rendering thread... \n";
            auto thread = vse::CreateAudioRenderingThread(vse::ConvertWaveFormat(src, vse::PcmWaveFormat::Parse(device->GetFormat())), device);
            thread->Start();

            std::clog << "Starting playback... \n";
            while (file->GetSampleCursor() < file->GetTotalSampleCount())
            {
                std::clog << (std::stringstream()
                    << "Playing wave..."
                    << " Play time=" << std::fixed << std::setprecision(3) << (static_cast<double>(file->GetSampleCursor()) / file->GetFormat().SamplingFrequency()) << "s."
                    << "\r").str();

                if (::GetAsyncKeyState(VK_ESCAPE)) break;
                std::this_thread::yield();
            }
            std::clog << "\n";

            std::clog << "Stopping rendering thread...\n";
            thread->Stop();
        }

        std::clog << "Exit... \n";
    }

    ::MFShutdown();
    ::CoUninitialize();

    return 0;
}
