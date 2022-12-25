#include <Windows.h>
#include <mfapi.h>

#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

#include "../vse/base/IWaveSource.h"
#include "../vse/output/AudioRenderingThread.h"
#include "../vse/output/WasapiOutputDevice.h"

#include "../vse/decoding/WaveFileLoader.h"
#include "../vse/processing/WaveFormatConverter.h"
#include "../vse/processing/WaveSourceWithProcessing.h"

int main(int args, const char* argv[])
{
    if (args <= 1)
    {
        std::clog << "Please give file path to play command line.\n";
        return 1;
    }

    ::CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
    ::MFStartup(MF_VERSION, MFSTARTUP_LITE);

    {
        auto device = vse::CreateWasapiOutputDevice();
        if (!device->Open())
        {
            std::clog << "FAILED to Open device!" << std::endl;
            return 1;
        }

        std::clog << "Device initialized... " << vse::ToString(device->GetFormat()) << "\n";
        auto device_format_pcm = vse::PcmWaveFormat::Parse(device->GetFormat());
        auto file_format_pcm = vse::PcmWaveFormat(vse::SampleType::S16, device_format_pcm.channels_, device_format_pcm.frequency_);

        // Loading file
        std::clog << "Loading wave file... \n";
        std::shared_ptr<vse::ISeekableWaveSource> file = vse::LoadAudioFile(argv[1], file_format_pcm);

        std::clog << "Loaded. "
            << " TotalSampleCount=" << file->GetTotalSampleCount() << "."
            << " memory=" << file->GetTotalSampleCount() * file->GetFormat().BlockAlign() / 1024 << "KB."
            << " time=" << std::fixed << std::setprecision(3) << (static_cast<double>(file->GetTotalSampleCount()) / file->GetFormat().SamplingFrequency()) << "s."
            << "\n";

        std::shared_ptr<vse::IWaveSource> src = file;
        src = vse::CreateSourceWithProcessing(src, vse::CreateFormatConverter(src->GetFormat(), device->GetFormat()));

        {
            std::clog << "Starting rendering thread... \n";
            auto thread = vse::CreateAudioRenderingThread(src, device);
            thread->Start();

            std::clog << "Start... \n";

            do
            {
                std::clog <<
                (std::stringstream()
                    << "Playing wave..."
                    << " Play time=" << std::fixed << std::setprecision(3) << (static_cast<double>(file->GetSampleCursor()) / file->GetFormat().SamplingFrequency()) << "s."
                    << "\r").str();
            }
            while (file->GetSampleCursor() < file->GetTotalSampleCount());

            std::clog << "Stopping rendering thread...\n";
            thread->Stop();
        }

        std::clog << "Exit... \n";
    }

    ::CoUninitialize();
    return 0;
}
