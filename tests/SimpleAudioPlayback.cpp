#include <Windows.h>
#include <combaseapi.h>

#include <iostream>
#include <chrono>
#include <thread>

#include "../vse/base/IWaveSource.h"
#include "../vse/output/AudioRenderingThread.h"
#include "../vse/output/WasapiOutputDevice.h"

#include "utils/SineWave16bitStereoSource.h"

int main()
{
    ::CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);

    {
        std::shared_ptr<vse::IWaveSource> src = std::make_shared<vse_tests::SineWave16bitStereoSource>(440.0f);

        std::clog << "Initializing device... " << vse::ToString(src->GetFormat()) << "\n";
        auto device = vse::CreateWasapiOutputDevice();
        if (!device->Open(src->GetFormat()))
        {
            std::clog << "FAILED to Open device!" << std::endl;
            return 1;
        }

        {
            std::clog << "Starting rendering thread... \n";
            auto thread = vse::CreateAudioRenderingThread(src, device);
            thread->Start();

            std::clog << "Playing sine wave... \n";
            std::this_thread::sleep_for(std::chrono::seconds(3));

            std::clog << "Stopping rendering thread...\n";
            thread->Stop();
        }

        std::clog << "Exit... \n";
    }

    ::CoUninitialize();
    return 0;
}
