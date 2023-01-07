#include <Windows.h>

#include <iostream>
#include <sstream>
#include <chrono>
#include <optional>
#include <queue>
#include <thread>

#include "../vse/base/IWaveSource.h"
#include "../vse/output/AudioRenderingThread.h"
#include "../vse/output/WasapiOutputDevice.h"

#include "../vse/loader/WaveFileLoader.h"
#include "../vse/processing/WaveFormatConverter.h"
#include "../vse/processing/WaveSourceWithProcessing.h"
#include "../vse/processing/DirectSoundAudioEffectDsp.h"
#include "../vse/processing/HardLimiter.h"
#include "../vse/pipeline/VolumeCalculation.h"
#include "../vse/pipeline/SimpleVoice.h"
#include "../vse/pipeline/StereoWaveMixer.h"

#include "utils/WaveFiles.h"

static std::shared_ptr<vse::IRandomAccessWaveBuffer> CreateBassSound(int note_number, vse::PcmWaveFormat format, size_t length)
{
    if (format.SampleType() != vse::SampleType::S16) throw std::invalid_argument("SampleType");
    if (format.ChannelCount() != 2) throw std::invalid_argument("ChannelCount");

    constexpr float pi = 3.1415926535897932384626433832795f;
    constexpr float freq_a4 = 440.0f;
    constexpr int note_a4 = 69;
    const float omega = pow(2.0f, static_cast<float>(note_number - note_a4) / 12.0f) * freq_a4 / static_cast<float>(format.SamplingFrequency()) * pi * 2;

    auto buffer = vse::AllocateWaveBuffer(format);
    for (size_t i = 0; i < length; i++)
    {
        float theta = omega * static_cast<float>(i);
        float v = 0;
        v += sin(theta * 1) * 0.5f;
        v += sin(theta * 2) * 0.25f;
        v += sin(theta * 3) * 0.125f;
        v += sin(theta * 4) * 0.125f;
        auto s = static_cast<vse::S16>(std::clamp<int>(static_cast<int>(v * 32767.0f), -32767, +32767));
        const auto sample = vse::S16Stereo{s, s};
        (void)buffer->Write(&sample, i * sizeof(sample), sizeof(sample));
    }

    return buffer;
}

class VoicePool
{
    std::queue<std::shared_ptr<vse::SimpleVoice>> pool_{};
    float base_volume_{};
    float base_pan_{};
public:
    VoicePool(
        const std::shared_ptr<vse::IStereoWaveMixer>& target_mixer,
        const std::shared_ptr<vse::IRandomAccessWaveBuffer>& source_buffer,
        int duplicate_count,
        float base_volume = 1.0f,
        float base_pan = 0.0f)
        : base_volume_(base_volume)
        , base_pan_(base_pan)
    {
        for (int i = 0; i < duplicate_count; i++)
            pool_.push(vse::CreateVoice(source_buffer, target_mixer));
    }

    void Play(float volume = 1.0f, float pan = 0.0f)
    {
        auto b = std::move(pool_.front());
        pool_.pop();
        b->SetVolume(volume * base_volume_);
        b->SetPan(pan + base_pan_);
        b->Play();
        pool_.push(b);
    }
};

int main()
{
    ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);

    {
        auto device = vse::CreateWasapiOutputDevice();
        if (!device->Open())
        {
            std::clog << "FAILED to Open device!" << std::endl;
            return 1;
        }

        std::clog << "Device initialized: " << vse::ToString(device->GetFormat()) << "\n";
        auto device_format_pcm = vse::PcmWaveFormat::Parse(device->GetFormat());

        // Build mixing pipeline graph
        //   crsh -→ reverb_mixer -→ reverb_effector -------┐
        //   hiht ----↗  ↑                                  |
        //   shot -------┘                                  ↓
        //   kick -→ cmpres_mixer -→ cmpres_effector -→ master_mixer →┐
        //   bass ------------------------------------↗               |
        //                                                            |
        //    ┌-------------------------------------------------------┘
        //    └→ masterCompressor -→ masterLimiter -> outputDevice 🔊
        std::clog << "Building mixing pipeline... \n";
        vse::PcmWaveFormat processing_format = {vse::SampleType::F32, device_format_pcm.channels_, device_format_pcm.frequency_};
        auto reverb_mixer = CreateStereoWaveMixer(processing_format);
        auto cmpres_mixer = CreateStereoWaveMixer(processing_format);
        auto reverb_effector = CreateSourceWithProcessing(reverb_mixer, CreateWavesReverbEffector(processing_format, {0.0f, -9.0f, 3000.0f, 0.100f}));
        auto cmpres_effector = CreateSourceWithProcessing(cmpres_mixer, CreateCompressorEffector(processing_format, {9.0f, 1.0f, 50.0f, -24.0f, 10.0f, 0.0f}));
        auto master_mixer = CreateStereoWaveMixer(processing_format);
        master_mixer->RegisterSource(reverb_effector);
        master_mixer->RegisterSource(cmpres_effector);
        auto master_compressor = CreateSourceWithProcessing(master_mixer, CreateCompressorEffector(processing_format, {-4.0f, 10.0f, 50.0f, -24.0f, 4.0f, 0.0f}));
        auto master_limiter = CreateSourceWithProcessing(master_compressor, CreateHardLimiter(processing_format, {1.0f, 0.98f}));
        auto master_output = vse::ConvertWaveFormat(master_limiter, device_format_pcm);

        // Load voice
        std::clog << "Loading wave files... \n";
        int transpose = 1;
        auto file_format_pcm = vse::PcmWaveFormat(vse::SampleType::S16, device_format_pcm.channels_, device_format_pcm.frequency_);
        auto voice_drum_crsh = VoicePool(reverb_mixer, LoadAudioFile(wave_files::Open_49_ogg(), file_format_pcm), 1, vse::DecibelToLinear(12.0f), +0.33f);
        auto voice_drum_crs2 = VoicePool(reverb_mixer, LoadAudioFile(wave_files::Open_57_ogg(), file_format_pcm), 1, vse::DecibelToLinear(12.0f), -0.44f);
        auto voice_drum_hiht = VoicePool(reverb_mixer, LoadAudioFile(wave_files::Open_42_ogg(), file_format_pcm), 2, vse::DecibelToLinear(+3.0f), -0.40f);
        auto voice_drum_shot = VoicePool(reverb_mixer, LoadAudioFile(wave_files::Open_40_ogg(), file_format_pcm), 2, vse::DecibelToLinear(+2.0f), -0.20f);
        auto voice_drum_kick = VoicePool(cmpres_mixer, LoadAudioFile(wave_files::Open_36_ogg(), file_format_pcm), 1, vse::DecibelToLinear(+0.0f), +0.00f);
        auto voice_bass_c = VoicePool(master_mixer, CreateBassSound(36 + transpose, file_format_pcm, 6000), 1, vse::DecibelToLinear(-3.0f), +0.00f);
        auto voice_bass_d = VoicePool(master_mixer, CreateBassSound(38 + transpose, file_format_pcm, 6000), 1, vse::DecibelToLinear(-3.0f), +0.00f);
        auto voice_bass_f = VoicePool(master_mixer, CreateBassSound(41 + transpose, file_format_pcm, 6000), 1, vse::DecibelToLinear(-3.0f), +0.00f);
        auto voice_bass_g = VoicePool(master_mixer, CreateBassSound(43 + transpose, file_format_pcm, 6000), 1, vse::DecibelToLinear(-3.0f), +0.00f);
        auto voice_bass_a = VoicePool(master_mixer, CreateBassSound(45 + transpose, file_format_pcm, 6000), 1, vse::DecibelToLinear(-3.0f), +0.00f);

        // Start playback
        std::clog << "Starting rendering thread... \n";
        auto thread = vse::CreateAudioRenderingThread(master_output, device);
        thread->Start();

        std::clog << "\n";
        std::clog << "Keyboard: \n";
        std::clog << "[ESCAPE]: exit \n";
        std::clog << "[Y]: Bass C# [U]: Bass D# [I]: Bass F# [O]: Bass G# [P]: Bass A# \n";
        std::clog << "[Z]: Kick    [X]: Snare   [C]: HiHat   [D]: Crash 1 [F]: Crash 2 \n";

        using demo_sequence_clock = std::chrono::high_resolution_clock;
        auto demo_sequence_next_update = demo_sequence_clock::time_point::max();
        auto demo_sequence_cursor = 0;

        while (::GetAsyncKeyState(VK_ESCAPE) == 0)
        {
            if (::GetAsyncKeyState('Z') & 1) voice_drum_kick.Play();
            if (::GetAsyncKeyState('X') & 1) voice_drum_shot.Play();
            if (::GetAsyncKeyState('C') & 1) voice_drum_hiht.Play();
            if (::GetAsyncKeyState('D') & 1) voice_drum_crsh.Play();
            if (::GetAsyncKeyState('F') & 1) voice_drum_crs2.Play();
            if (::GetAsyncKeyState('Y') & 1) voice_bass_c.Play();
            if (::GetAsyncKeyState('U') & 1) voice_bass_d.Play();
            if (::GetAsyncKeyState('I') & 1) voice_bass_f.Play();
            if (::GetAsyncKeyState('O') & 1) voice_bass_g.Play();
            if (::GetAsyncKeyState('P') & 1) voice_bass_a.Play();

            if (::GetAsyncKeyState(VK_SPACE) & 1)
            {
                demo_sequence_cursor = 0;
                demo_sequence_next_update = demo_sequence_next_update == demo_sequence_clock::time_point::max()
                                                ? demo_sequence_clock::now()
                                                : demo_sequence_clock::time_point::max();
            }

            // Play demo sequence
            if (demo_sequence_clock::now() >= demo_sequence_next_update)
            {
                static const auto bpm = 130;
                static const auto seq_length = 4 * 8 * 16;
                static const auto seq_cymb = std::string_view("C.h.h.h.h.h.h.hh" "h.h.h.h.h.h.h.hh" "h.h.h.h.h.h.h.hh" "h.h.h.h.h.h.h.h." "C.h.h.h.h.h.h.hh" "h.h.h.h.h.h.h.hh" "h.h.h.h.h.h.h.hh" "C...h.hhh.h.R...");
                static const auto seq_shot = std::string_view("....S..s.S..S..." "....S..s.S..S..s" "....S..s.S..S..." "....S..s.S..Ss.S" "....S..s.S..S..." "....S..s.S..S..s" "....S..s.S..S..." "..s.S.sS.S..SSsS");
                static const auto seq_kick = std::string_view("K...K...K...K..." "K...K...K...K..k" "K...K...K...K.K." "K...K..kkK.kK.K." "K...K...K...K..." "K...K...K...K..k" "K...K...K...K.Kk" "K.K.K.K.KkkkKkkK");
                static const auto seq_bass = std::string_view("d..d..d..d..d.c." "d..d..f..f..g.g." "d..d..d..d..d.c." "d..d..f..f..c.c." "d..d..d..d..d.c." "d..d..f..f..g.g." "d..d..d..d..d.c." "d..f..g..a..c.c.");

                if (auto i = demo_sequence_cursor++; i < seq_length)
                {
                    if (seq_cymb[i % seq_cymb.size()] == 'C') voice_drum_crsh.Play();
                    if (seq_cymb[i % seq_cymb.size()] == 'R') voice_drum_crs2.Play();
                    if (seq_cymb[i % seq_cymb.size()] == 'h') voice_drum_hiht.Play();
                    if (seq_shot[i % seq_shot.size()] == 'S') voice_drum_shot.Play();
                    if (seq_shot[i % seq_shot.size()] == 's') voice_drum_shot.Play(vse::DecibelToLinear(-2.0f));
                    if (seq_kick[i % seq_kick.size()] == 'K') voice_drum_kick.Play();
                    if (seq_kick[i % seq_kick.size()] == 'k') voice_drum_kick.Play(vse::DecibelToLinear(-5.0f));
                    if (seq_kick[i % seq_kick.size()] == 'k') voice_drum_kick.Play(vse::DecibelToLinear(-5.0f));
                    if (seq_bass[i % seq_bass.size()] == 'c') voice_bass_c.Play();
                    if (seq_bass[i % seq_bass.size()] == 'd') voice_bass_d.Play();
                    if (seq_bass[i % seq_bass.size()] == 'f') voice_bass_f.Play();
                    if (seq_bass[i % seq_bass.size()] == 'g') voice_bass_g.Play();
                    if (seq_bass[i % seq_bass.size()] == 'a') voice_bass_a.Play();

                    double swing = (i % 2 == 0 ? 1.20 : 0.80);
                    demo_sequence_next_update += std::chrono::duration_cast<demo_sequence_clock::duration>(std::chrono::duration<double>(60.000 / bpm / 4 * swing));
                }
                else if (i == seq_length)
                {
                    voice_drum_crsh.Play();
                    voice_drum_kick.Play();
                    voice_bass_d.Play();
                    demo_sequence_next_update = demo_sequence_clock::time_point::max();
                }
            }
            std::clog << (std::string("[Space]: Playback demo sequence (cursor: ") + std::to_string(demo_sequence_cursor / 16 + 1) + "." + std::to_string(demo_sequence_cursor % 16 / 4 + 1) + "." + std::to_string(demo_sequence_cursor % 16) + ")    \r");
        }

        std::clog << "\n";
        std::clog << "Stopping rendering thread...\n";
        thread->Stop();

        std::clog << "Exit. \n";
    }

    ::CoUninitialize();
    return 0;
}
