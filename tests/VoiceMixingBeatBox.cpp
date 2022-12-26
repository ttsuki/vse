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
#include "../vse/pipeline/VolumeCalculation.h"
#include "../vse/pipeline/SimpleVoice.h"
#include "../vse/pipeline/StereoWaveMixer.h"

#include "utils/WaveFiles.h"

int wmain(int argc, const wchar_t* argv[])
{
    ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
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

        // Build mixing pipeline
        auto mixer = vse::CreateStereoWaveMixer(vse::PcmWaveFormat(vse::SampleType::F32, device_format_pcm.channels_, device_format_pcm.frequency_));
        auto mixer_out_converter = vse::CreateFormatConverter(mixer->GetFormat(), device_format_pcm);
        auto device_source = vse::CreateSourceWithProcessing(mixer, mixer_out_converter);

        // Load file
        std::clog << "Loading wave files... \n";
        auto file_format_pcm = vse::PcmWaveFormat(vse::SampleType::S16, device_format_pcm.channels_, device_format_pcm.frequency_);
        auto ogg36 = vse::CreateVoice(vse::ReadOutToMemory(vse::CreateWaveSourceForFile(wave_files::Open_36_ogg(), file_format_pcm)), mixer);
        auto ogg40 = vse::CreateVoice(vse::ReadOutToMemory(vse::CreateWaveSourceForFile(wave_files::Open_40_ogg(), file_format_pcm)), mixer);
        auto ogg42 = vse::CreateVoice(vse::ReadOutToMemory(vse::CreateWaveSourceForFile(wave_files::Open_42_ogg(), file_format_pcm)), mixer);
        std::clog << "key[1] ogg36: " << ogg36->GetUpstreamBuffer()->Size() << " bytes\n";
        std::clog << "key[2] ogg40: " << ogg40->GetUpstreamBuffer()->Size() << " bytes\n";
        std::clog << "key[3] ogg42: " << ogg42->GetUpstreamBuffer()->Size() << " bytes\n";

        auto bgm_voice = ogg36;

        if (argc > 1 && argv[1])
        {
            try
            {
                auto bgm = vse::LoadAudioFile(argv[1], file_format_pcm);
                bgm_voice = vse::CreateVoice(bgm, mixer);
                bgm_voice->SetVolume(vse::DecibelToLinear(-6.02f));
                bgm_voice->SetPan(-0.5f);
                std::clog << "key[SPACE] BGM Loaded."
                    << " TotalSampleCount=" << (bgm->Size() / bgm->GetFormat().BlockAlign()) << "."
                    << " memory=" << bgm->Size() / 1024 << "KB."
                    << " time=" << std::fixed << std::setprecision(3) << (static_cast<double>(bgm->Size()) / bgm->GetFormat().AvgBytesPerSec()) << "s."
                    << "\n";
            }
            catch (const std::runtime_error& e)
            {
                std::clog << "FAILED to load audio file: " << e.what() << "\n";
            }
        }

        {
            std::clog << "Starting rendering thread... \n";
            auto thread = vse::CreateAudioRenderingThread(device_source, device);
            thread->Start();

            std::clog << "Started. \n";
            std::clog << "Press ESCAPE key to exit.\n";

            do
            {
                // Switch effector.
                if (::GetAsyncKeyState('1') & 1) { ogg36->Play(); }
                if (::GetAsyncKeyState('2') & 1) { ogg40->Play(); }
                if (::GetAsyncKeyState('3') & 1) { ogg42->Play(); }
                if (::GetAsyncKeyState(VK_SPACE) & 1) { !bgm_voice->IsPlaying() ? bgm_voice->Play() : bgm_voice->Stop(); }
                if (::GetAsyncKeyState(VK_LEFT) & 1) { bgm_voice->SetPlayPositionInSeconds(bgm_voice->GetPlayPositionInSeconds() - 5.0); }
                if (::GetAsyncKeyState(VK_RIGHT) & 1) { bgm_voice->SetPlayPositionInSeconds(bgm_voice->GetPlayPositionInSeconds() + 5.0); }
                if (::GetAsyncKeyState(VK_ESCAPE) & 1) { break; }

                std::clog <<
                (std::stringstream()
                    << "BGM time=" << std::fixed << std::setprecision(3) << bgm_voice->GetPlayPositionInSeconds() << "s."
                    << "\r").str();
            }
            while (true);

            std::clog << "Stopping rendering thread...\n";
            thread->Stop();
        }

        std::clog << "Exit. \n";
    }

    ::MFShutdown();
    ::CoUninitialize();
    return 0;
}
