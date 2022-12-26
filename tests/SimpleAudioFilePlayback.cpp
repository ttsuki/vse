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
#include "../vse/processing/DirectSoundAudioEffectDsp.h"
#include "../vse/pipeline/SourceSwitcher.h"

int wmain(int args, const wchar_t* argv[])
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
        std::shared_ptr<vse::ISeekableWaveSource> file = vse::LoadAudioFileAsWaveSource(argv[1], file_format_pcm);

        std::clog << "Loaded. "
            << " TotalSampleCount=" << file->GetTotalSampleCount() << "."
            << " memory=" << file->GetTotalSampleCount() * file->GetFormat().BlockAlign() / 1024 << "KB."
            << " time=" << std::fixed << std::setprecision(3) << (static_cast<double>(file->GetTotalSampleCount()) / file->GetFormat().SamplingFrequency()) << "s."
            << "\n";

        std::shared_ptr<vse::IWaveSource> src = file;

        std::shared_ptr<vse::IWaveSource> s0 = src;
        std::shared_ptr<vse::IWaveSourceWithProcessing> s1 = vse::CreateSourceWithProcessing(src, vse::CreateEchoEffector(src->GetFormat()));
        std::shared_ptr<vse::IWaveSourceWithProcessing> s2 = vse::CreateSourceWithProcessing(src, vse::CreateGargleEffector(src->GetFormat()));
        std::shared_ptr<vse::IWaveSourceWithProcessing> s3 = vse::CreateSourceWithProcessing(src, vse::CreateDistortionEffector(src->GetFormat()));
        std::shared_ptr<vse::IWaveSourceWithProcessing> s4 = vse::CreateSourceWithProcessing(src, vse::CreateWavesReverbEffector(src->GetFormat()));
        std::shared_ptr<vse::IWaveSourceWithProcessing> s5 = vse::CreateSourceWithProcessing(src, vse::CreateChorusEffector(src->GetFormat()));
        std::shared_ptr<vse::IWaveSourceWithProcessing> s6 = vse::CreateSourceWithProcessing(src, vse::CreateFlangerEffector(src->GetFormat()));
        std::shared_ptr<vse::ISourceSwitcher> sw = vse::CreateSourceSwitcher(s0);
        src = sw;

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

                // Switch effector.
                if (::GetAsyncKeyState('0') & 1) { sw->Assign(s0), std::clog << "\x1b[K Effector is switched to OFF." << "\n"; }
                if (::GetAsyncKeyState('1') & 1) { sw->Assign(s1), s1->GetAttachedProcessor()->Discontinuity(), std::clog << "\x1b[K Effector is switched to ECHO." << "\n"; }
                if (::GetAsyncKeyState('2') & 1) { sw->Assign(s2), s2->GetAttachedProcessor()->Discontinuity(), std::clog << "\x1b[K Effector is switched to GARGLE." << "\n"; }
                if (::GetAsyncKeyState('3') & 1) { sw->Assign(s3), s3->GetAttachedProcessor()->Discontinuity(), std::clog << "\x1b[K Effector is switched to DISTORTION." << "\n"; }
                if (::GetAsyncKeyState('4') & 1) { sw->Assign(s4), s4->GetAttachedProcessor()->Discontinuity(), std::clog << "\x1b[K Effector is switched to REVERB." << "\n"; }
                if (::GetAsyncKeyState('5') & 1) { sw->Assign(s5), s5->GetAttachedProcessor()->Discontinuity(), std::clog << "\x1b[K Effector is switched to CHORUS." << "\n"; }
                if (::GetAsyncKeyState('6') & 1) { sw->Assign(s6), s6->GetAttachedProcessor()->Discontinuity(), std::clog << "\x1b[K Effector is switched to FLANGER." << "\n"; }
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
