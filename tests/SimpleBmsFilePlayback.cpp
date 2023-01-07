#include <Windows.h>
#include <mmsystem.h>
#include <mfapi.h>
#include <io.h>
#include <fcntl.h>

#include <iostream>
#include <sstream>
#include <chrono>
#include <future>
#include <thread>

#include "../vse/base/xtl/xtl_ostream.h"
#include "../vse/base/xtl/xtl_timestamp.h"

#include "../vse/base/IWaveSource.h"
#include "../vse/output/AudioRenderingThread.h"
#include "../vse/output/WasapiOutputDevice.h"

#include "../vse/loader/WaveFileLoader.h"
#include "../vse/processing/HardLimiter.h"
#include "../vse/processing/WaveFormatConverter.h"
#include "../vse/processing/WaveSourceWithProcessing.h"
#include "../vse/processing/DirectSoundAudioEffectDsp.h"
#include "../vse/pipeline/VolumeCalculation.h"
#include "../vse/pipeline/SimpleVoice.h"
#include "../vse/pipeline/StereoWaveMixer.h"
#include "../vse/pipeline/SourceSwitcher.h"

#include "./utils/BmsFile.h"
#include "./utils/BmsEvent.h"
#include "./utils/BmsPlaybackContext.h"
#include "./utils/Jcode.h"

using namespace TTsukiGameSdk;
using namespace Tsukikage::util;

static char EMPTY_CHAR = '|';
static char BAR_CHAR = '+';
static char NOTE_CHAR = '#';
static double RENDER_TIMING_OFFSET = +1.0; // rendering timing offset in seconds
static int RENDERING_RESOLUTION = 32;      // rendering grid in n-th note

static std::string as_fixed_status_string(const std::string& status_message);

int wmain(int argc, const wchar_t* argv[])
{
    if (argc <= 1)
    {
        std::clog << "Please give file path to play command line.\n";
        return 1;
    }

    // Setup Windows console window to understand VT100 escape sequences.
    {
        DWORD console_mode{};
        ::GetConsoleMode(::GetStdHandle(STD_OUTPUT_HANDLE), &console_mode);
        ::SetConsoleMode(::GetStdHandle(STD_OUTPUT_HANDLE), console_mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    ::CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
    ::MFStartup(MF_VERSION, MFSTARTUP_LITE);

    {
        std::clog << "Initializing audio device... ";
        auto device = vse::CreateWasapiOutputDevice();
        if (!device->Open())
        {
            std::clog << "FAILED to Open device!" << std::endl;
            return 1;
        }
        std::clog << "[OK]: " << vse::ToString(device->GetFormat()) << "\n";

        // Build mixing pipeline
        const vse::PcmWaveFormat device_format = vse::PcmWaveFormat::Parse(device->GetFormat());
        const vse::PcmWaveFormat buffer_format = vse::PcmWaveFormat(vse::SampleType::S32, device_format.channels_, device_format.frequency_);
        const vse::PcmWaveFormat mixer_format = vse::PcmWaveFormat(vse::SampleType::F32, device_format.channels_, device_format.frequency_);
        std::shared_ptr<vse::IStereoWaveMixer> mixer = vse::CreateStereoWaveMixer(mixer_format);
        std::shared_ptr<vse::IWaveSourceWithProcessing> limiter = vse::CreateSourceWithProcessing(mixer, vse::CreateHardLimiter(mixer->GetFormat(), {vse::DecibelToLinear(-6.0f), vse::DecibelToLinear(-1.0f)}));
        std::shared_ptr<vse::IWaveSource> mixer_block_out = limiter;

        // effector block
        std::shared_ptr<vse::IWaveSource> effector_in = mixer_block_out;
        std::shared_ptr<vse::IWaveSource> s0 = effector_in;
        std::shared_ptr<vse::IWaveSourceWithProcessing> s1 = vse::CreateSourceWithProcessing(effector_in, vse::CreateEchoEffector(effector_in->GetFormat()));
        std::shared_ptr<vse::IWaveSourceWithProcessing> s2 = vse::CreateSourceWithProcessing(effector_in, vse::CreateGargleEffector(effector_in->GetFormat()));
        std::shared_ptr<vse::IWaveSourceWithProcessing> s3 = vse::CreateSourceWithProcessing(effector_in, vse::CreateDistortionEffector(effector_in->GetFormat()));
        std::shared_ptr<vse::IWaveSourceWithProcessing> s4 = vse::CreateSourceWithProcessing(effector_in, vse::CreateWavesReverbEffector(effector_in->GetFormat()));
        std::shared_ptr<vse::IWaveSourceWithProcessing> s5 = vse::CreateSourceWithProcessing(effector_in, vse::CreateChorusEffector(effector_in->GetFormat()));
        std::shared_ptr<vse::IWaveSourceWithProcessing> s6 = vse::CreateSourceWithProcessing(effector_in, vse::CreateFlangerEffector(effector_in->GetFormat()));
        std::shared_ptr<vse::ISourceSwitcher> effector_switch = vse::CreateSourceSwitcher(s0);
        std::shared_ptr<vse::IWaveSourceWithProcessing> post_limiter = vse::CreateSourceWithProcessing(effector_switch, vse::CreateHardLimiter(effector_switch->GetFormat(), {vse::DecibelToLinear(-0.0f), vse::DecibelToLinear(-0.1f)}));
        std::shared_ptr<vse::IWaveSource> effector_out = post_limiter;
        std::shared_ptr<vse::IWaveSource> device_source_in = vse::CreateSourceWithProcessing(effector_out, vse::CreateFormatConverter(effector_out->GetFormat(), device->GetFormat()));

        // Loading seq file
        auto loading_start_timestamp = std::chrono::high_resolution_clock::now();
        std::clog << "Loading sequence...\n";
        bms::BmsFile source_file;
        jcode::encoding source_encoding{};
        bms::BmsEventList event_list;
        {
            std::filesystem::path BmsFilePath = argv[1];
            int random_seed = static_cast<int>(std::random_device{}());
            auto source_stream = vse::LoadFile(BmsFilePath);
            std::string source_code(source_stream->Size(), '\0');
            (void)source_stream->Read(source_code.data(), source_code.size());
            source_encoding = jcode::guess_encoding(source_code);

            std::stringstream parser_log;
            auto null_out = vse::xtl::o_null_stream{};
            auto clog_out = vse::xtl::make_stream_with_prefix([&](const char* line) { parser_log << line; }, std::string("  "));
            source_file = bms::BmsFile::Parse(source_code, random_seed, clog_out, clog_out, null_out);
            event_list = BuildEventList(source_file, clog_out, clog_out, null_out);

            {
                std::wclog << std::flush;
                int old_mode = _setmode(_fileno(stderr), _O_WTEXT);
                std::wclog << jcode::convert_to_wstring(parser_log.str(), source_encoding) << std::flush;
                _setmode(_fileno(stderr), old_mode);
            }

            std::clog << "Loaded." << " events=" << event_list.size() << "." << " time=" << event_list.back().Timing.count() << "s." << "\n";
        }

        // Loading assets
        std::clog << "Loading assets...\n";
        std::array<std::shared_ptr<vse::SimpleVoice>, 1296> voice_bank;
        {
            std::filesystem::path DirectoryPath = std::filesystem::path(argv[1]).parent_path();
            std::map<std::string, std::shared_future<std::shared_ptr<vse::IRandomAccessWaveBuffer>>, std::less<>> loading;
            for (auto&& wav_file_name : source_file.WaveTable)
            {
                if (!wav_file_name.empty() && loading.count(wav_file_name) == 0)
                    loading[wav_file_name] = std::async(std::launch::async, [&]
                    {
                        auto path = DirectoryPath / std::filesystem::path(jcode::convert_to_wstring(wav_file_name, source_encoding));
                        if (!exists(path)) path = path.replace_extension("wav");
                        if (!exists(path)) path = path.replace_extension("ogg");
                        if (!exists(path)) path = path.replace_extension("mp3");
                        if (!exists(path)) path = path.replace_extension("wma");
                        if (!exists(path)) throw std::runtime_error("file not found");
                        return vse::LoadAudioFile(path, buffer_format);
                    }).share();
            }

            for (size_t i = 0; i < source_file.WaveTable.size(); ++i)
            {
                auto&& wav_file_name = source_file.WaveTable[i];
                if (auto it = loading.find(wav_file_name); it != loading.end())
                {
                    try
                    {
                        voice_bank[i] = vse::CreateVoice(it->second.get(), mixer);
                    }
                    catch (const std::runtime_error& e)
                    {
                        auto path = std::filesystem::path();
                        std::clog << "  Failed to load wave file: " << std::flush;

                        {
                            std::wclog << std::flush;
                            int old_mode = _setmode(_fileno(stderr), _O_WTEXT);
                            std::wclog << jcode::convert_to_wstring(wav_file_name, source_encoding) << std::flush;
                            _setmode(_fileno(stderr), old_mode);
                        }

                        std::clog << ": " << e.what() << std::endl;
                    }
                }
            }

            size_t total_buffer_memory = 0;
            for (auto&& [k, buffer] : loading)
            {
                try
                {
                    if (auto p = buffer.get())
                        total_buffer_memory += p->Size();
                }
                catch (const std::runtime_error&) { }
            }

            std::clog << "Total WaveBufferMemory: " << total_buffer_memory / 1024 << "KB.\n";
        }

        auto loading_end_timestamp = std::chrono::high_resolution_clock::now();
        std::clog << "BMS Loaded: ElapsedTime: " << std::chrono::duration<double, std::milli>(loading_end_timestamp - loading_start_timestamp).count() << " ms.\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::clog << "Starting rendering thread... \n";
        auto audio_rendering_thread = vse::CreateAudioRenderingThread(device_source_in, device);
        audio_rendering_thread->Start();

        // Prepare contexts
        std::clog << "Preparing contexts... \n";

        // audio context
        auto audio_playback = std::make_unique<bms::BmsPlaybackContext>(&source_file, &event_list);
        bms::MeasureNumber current_measure_number = 0;
        bms::Tick current_measure_started_at = 0;

        // input assigning context
        auto input_assigning = std::make_unique<bms::BmsPlaybackContext>(&source_file, &event_list);
        bool manual_play = false;
        std::vector<std::shared_ptr<vse::SimpleVoice>> voice_assign(32);

        // graphic context
        auto screen_rendering = std::make_unique<bms::BmsPlaybackContext>(&source_file, &event_list);
        uint64_t video_rendered_frame = 0;
        std::string video_rendering_target(32, EMPTY_CHAR);
        std::string video_rendering_target_meta;
        const auto TickToFrameNumber = [](bms::Tick t) { return static_cast<uint64_t>(t / (bms::TimeBase / RENDERING_RESOLUTION)); };
        const auto RenderLine = [&]
        {
            // layout
            std::stringstream graphic;
            graphic << "  |";
            graphic << "\x1b[0m|";
            graphic << "\x1b[0m" << (video_rendering_target[0x06] == NOTE_CHAR ? "\x1b[31;101m" : "\x1b[31;49m") << video_rendering_target[0x06] << video_rendering_target[0x06] << video_rendering_target[0x06] << video_rendering_target[0x06] << video_rendering_target[0x06];
            graphic << "\x1b[0m" << (video_rendering_target[0x01] == NOTE_CHAR ? "\x1b[37;107m" : "\x1b[37;49m") << video_rendering_target[0x01] << video_rendering_target[0x01] << video_rendering_target[0x01];
            graphic << "\x1b[0m" << (video_rendering_target[0x02] == NOTE_CHAR ? "\x1b[34;104m" : "\x1b[34;49m") << video_rendering_target[0x02] << video_rendering_target[0x02];
            graphic << "\x1b[0m" << (video_rendering_target[0x03] == NOTE_CHAR ? "\x1b[37;107m" : "\x1b[37;49m") << video_rendering_target[0x03] << video_rendering_target[0x03] << video_rendering_target[0x03];
            graphic << "\x1b[0m" << (video_rendering_target[0x04] == NOTE_CHAR ? "\x1b[34;104m" : "\x1b[34;49m") << video_rendering_target[0x04] << video_rendering_target[0x04];
            graphic << "\x1b[0m" << (video_rendering_target[0x05] == NOTE_CHAR ? "\x1b[37;107m" : "\x1b[37;49m") << video_rendering_target[0x05] << video_rendering_target[0x05] << video_rendering_target[0x05];
            graphic << "\x1b[0m" << (video_rendering_target[0x08] == NOTE_CHAR ? "\x1b[34;104m" : "\x1b[34;49m") << video_rendering_target[0x08] << video_rendering_target[0x08];
            graphic << "\x1b[0m" << (video_rendering_target[0x09] == NOTE_CHAR ? "\x1b[37;107m" : "\x1b[37;49m") << video_rendering_target[0x09] << video_rendering_target[0x09] << video_rendering_target[0x09];
            graphic << "\x1b[0m|  |";
            graphic << "\x1b[0m" << (video_rendering_target[0x11] == NOTE_CHAR ? "\x1b[37;107m" : "\x1b[37;49m") << video_rendering_target[0x11] << video_rendering_target[0x11] << video_rendering_target[0x11];
            graphic << "\x1b[0m" << (video_rendering_target[0x12] == NOTE_CHAR ? "\x1b[34;104m" : "\x1b[34;49m") << video_rendering_target[0x12] << video_rendering_target[0x12];
            graphic << "\x1b[0m" << (video_rendering_target[0x13] == NOTE_CHAR ? "\x1b[37;107m" : "\x1b[37;49m") << video_rendering_target[0x13] << video_rendering_target[0x13] << video_rendering_target[0x13];
            graphic << "\x1b[0m" << (video_rendering_target[0x14] == NOTE_CHAR ? "\x1b[34;104m" : "\x1b[34;49m") << video_rendering_target[0x14] << video_rendering_target[0x14];
            graphic << "\x1b[0m" << (video_rendering_target[0x15] == NOTE_CHAR ? "\x1b[37;107m" : "\x1b[37;49m") << video_rendering_target[0x15] << video_rendering_target[0x15] << video_rendering_target[0x15];
            graphic << "\x1b[0m" << (video_rendering_target[0x18] == NOTE_CHAR ? "\x1b[34;104m" : "\x1b[34;49m") << video_rendering_target[0x18] << video_rendering_target[0x18];
            graphic << "\x1b[0m" << (video_rendering_target[0x19] == NOTE_CHAR ? "\x1b[37;107m" : "\x1b[37;49m") << video_rendering_target[0x19] << video_rendering_target[0x19] << video_rendering_target[0x19];
            graphic << "\x1b[0m" << (video_rendering_target[0x16] == NOTE_CHAR ? "\x1b[31;101m" : "\x1b[31;49m") << video_rendering_target[0x16] << video_rendering_target[0x16] << video_rendering_target[0x16] << video_rendering_target[0x16] << video_rendering_target[0x16];
            graphic << "\x1b[0m|";
            graphic << "|  ";
            graphic << "\x1b[0;93m" << video_rendering_target_meta;
            std::clog << graphic.str() << "\x1b[0m\n";

            video_rendering_target.assign(32, EMPTY_CHAR); // clear
            video_rendering_target_meta.clear();           // clear
        };

        // Ready.
        std::clog << "READY. \n";
        //std::clog << "PRESS [ENTER] key to start: ";
        //(void)std::getchar();

        const bms::BmsPlaybackContext::Clock::time_point started_at = bms::BmsPlaybackContext::Clock::now();
        while (true)
        {
            const bms::Timing clock = bms::BmsPlaybackContext::Clock::now() - started_at;
            const bms::Timing video_clock = clock + std::chrono::duration<double>(RENDER_TIMING_OFFSET);

            // audio context // TODO: from audio thread
            audio_playback->Update(clock, [&](const bms::BmsEvent& e)
            {
                if (auto data = e.EventDataIs<bms::BarEvent>())
                {
                    current_measure_number = data->MeasureNumber;
                    current_measure_started_at = e.Tick;
                }

                if (auto data = e.EventDataIs<bms::ChorusPlayEvent>())
                {
                    if (auto& voice = voice_bank[data->WaveNumber])
                        voice->Play();
                }

                if (auto data = e.EventDataIs<bms::NoteEvent>())
                {
                    if (auto& voice = voice_bank[data->WaveNumber])
                        if (!manual_play) voice->Play();
                }
            });

            // input assigning context
            input_assigning->Update(clock + bms::Timing(0.2), [&](const bms::BmsEvent& e)
            {
                // TODO: search for nearest note
                if (auto data = e.EventDataIs<bms::NoteEvent>())
                {
                    voice_assign[data->Channel] = voice_bank[data->WaveNumber];
                }
            });

            // graphic context
            screen_rendering->Update(video_clock, [&](const bms::BmsEvent& e)
            {
                // TODO: sync BPM change, STOP sequence with audio
                // (can't support them by this architecture construction)
                for (auto g = TickToFrameNumber(e.Tick); video_rendered_frame < g; ++video_rendered_frame)
                    RenderLine();

                if (auto data = e.EventDataIs<bms::BarEvent>())
                {
                    for (auto&& chr : video_rendering_target) if (chr == EMPTY_CHAR)chr = BAR_CHAR;
                    video_rendering_target_meta += " :: MEASURE ::= " + std::to_string(data->MeasureNumber);
                }

                if (auto data = e.EventDataIs<bms::BpmChangeEvent>())
                {
                    video_rendering_target_meta += " :: BPM CHANGE ::= " + std::to_string(data->NewBpm);
                }

                if (auto data = e.EventDataIs<bms::StopSequenceEvent>())
                {
                    video_rendering_target_meta += " :: STOP SEQUENCE ::= " + std::to_string(data->StopTickCount);
                    RenderLine(); // force render line.
                }

                if (auto data = e.EventDataIs<bms::NoteEvent>())
                {
                    video_rendering_target.at(data->Channel) = NOTE_CHAR;
                    // TODO: supports long note (data->Duration)
                }
            });
            for (auto current_frame_number = TickToFrameNumber(screen_rendering->GetCurrentTimingAsTick(video_clock)); video_rendered_frame < current_frame_number; ++video_rendered_frame)
                RenderLine();

            // Display audio Playback status
            {
                constexpr auto mmssfff = [](bms::Timing duration) -> std::string
                {
                    return (std::ostringstream()
                        << std::setfill('0') << std::setw(2) << std::chrono::duration_cast<std::chrono::minutes>(duration).count()
                        << ":" << std::setfill('0') << std::setw(2) << std::chrono::duration_cast<std::chrono::seconds>(duration).count() % 60
                        << ":" << std::setfill('0') << std::setw(3) << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000
                    ).str();
                };

                constexpr auto mmbbfff = [](bms::MeasureNumber measure, bms::Tick fractal)
                {
                    constexpr bms::Tick timebase = bms::TimeBase / 4;
                    return (std::ostringstream()
                        << std::setw(3) << measure
                        << "." << std::setw(1) << (fractal / timebase + 1)
                        << "." << std::setw(3) << std::setfill('0') << fractal % timebase
                    ).str();
                };

                std::ostringstream message;
                message << "[" << vse::xtl::timestamp::now().to_localtime_string() << "]\n";
                message << "Playing... AUTOPLAY:" << (manual_play ? "OFF" : "\x1b[31mON\x1b[0m") << " <- [Q]key to Toggle\n";
                message << "   BPM: " << std::setprecision(3) << std::fixed << audio_playback->CurrentBpm << "\n";
                message << "  Time: " << mmssfff(clock) << " / " << mmssfff(audio_playback->TotalTime) << "\n";
                message << "  Tick: " << std::setw(9) << audio_playback->GetCurrentTimingAsTick(clock) << " / " << std::setw(9) << audio_playback->TotalTick << "\n";
                message << "  Beat: " << mmbbfff(current_measure_number, audio_playback->GetCurrentTimingAsTick(clock) - current_measure_started_at) << " / " << mmbbfff(audio_playback->TotalMeasures, 0) << "\n";
                message << "  Event:" << std::setw(9) << audio_playback->Cursor << " / " << std::setw(9) << audio_playback->TotalEventCount << "\n";
                std::clog << as_fixed_status_string(message.str()) << std::flush;
            }

            // Input: switch effector.
            if (::GetAsyncKeyState('0') & 1) { effector_switch->Assign(s0), std::clog << "\x1b[K Effector is switched to OFF." << "\n"; }
            if (::GetAsyncKeyState('1') & 1) { effector_switch->Assign(s1), s1->GetAttachedProcessor()->Discontinuity(), std::clog << "\x1b[K Effector is switched to ECHO." << "\n"; }
            if (::GetAsyncKeyState('2') & 1) { effector_switch->Assign(s2), s2->GetAttachedProcessor()->Discontinuity(), std::clog << "\x1b[K Effector is switched to GARGLE." << "\n"; }
            if (::GetAsyncKeyState('3') & 1) { effector_switch->Assign(s3), s3->GetAttachedProcessor()->Discontinuity(), std::clog << "\x1b[K Effector is switched to DISTORTION." << "\n"; }
            if (::GetAsyncKeyState('4') & 1) { effector_switch->Assign(s4), s4->GetAttachedProcessor()->Discontinuity(), std::clog << "\x1b[K Effector is switched to REVERB." << "\n"; }
            if (::GetAsyncKeyState('5') & 1) { effector_switch->Assign(s5), s5->GetAttachedProcessor()->Discontinuity(), std::clog << "\x1b[K Effector is switched to CHORUS." << "\n"; }
            if (::GetAsyncKeyState('6') & 1) { effector_switch->Assign(s6), s6->GetAttachedProcessor()->Discontinuity(), std::clog << "\x1b[K Effector is switched to FLANGER." << "\n"; }

            // Input: key on. // TODO: from input thread
            if (::GetAsyncKeyState('Q') & 1) { manual_play ^= true; }
            if (::GetAsyncKeyState(VK_LSHIFT) & 1) { if (auto& v = voice_assign[0x06]) v->Play(); }
            if (::GetAsyncKeyState('Z') & 1) { if (auto& v = voice_assign[0x01]) v->Play(); }
            if (::GetAsyncKeyState('S') & 1) { if (auto& v = voice_assign[0x02]) v->Play(); }
            if (::GetAsyncKeyState('X') & 1) { if (auto& v = voice_assign[0x03]) v->Play(); }
            if (::GetAsyncKeyState('D') & 1) { if (auto& v = voice_assign[0x04]) v->Play(); }
            if (::GetAsyncKeyState('C') & 1) { if (auto& v = voice_assign[0x05]) v->Play(); }
            if (::GetAsyncKeyState('F') & 1) { if (auto& v = voice_assign[0x08]) v->Play(); }
            if (::GetAsyncKeyState('V') & 1) { if (auto& v = voice_assign[0x09]) v->Play(); }
            if (::GetAsyncKeyState('N') & 1) { if (auto& v = voice_assign[0x11]) v->Play(); }
            if (::GetAsyncKeyState('J') & 1) { if (auto& v = voice_assign[0x12]) v->Play(); }
            if (::GetAsyncKeyState('M') & 1) { if (auto& v = voice_assign[0x13]) v->Play(); }
            if (::GetAsyncKeyState('K') & 1) { if (auto& v = voice_assign[0x14]) v->Play(); }
            if (::GetAsyncKeyState(VK_OEM_COMMA) & 1) { if (auto& v = voice_assign[0x15]) v->Play(); }
            if (::GetAsyncKeyState('L') & 1) { if (auto& v = voice_assign[0x18]) v->Play(); }
            if (::GetAsyncKeyState(VK_OEM_PERIOD) & 1) { if (auto& v = voice_assign[0x19]) v->Play(); }
            if (::GetAsyncKeyState(VK_RSHIFT) & 1) { if (auto& v = voice_assign[0x16]) v->Play(); }

            // Wait for all active sounds reaching to end.
            if (audio_playback->EndOfFile())
            {
                if (bool all_sound_is_stopped = std::all_of(
                    voice_bank.begin(), voice_bank.end(),
                    [](auto&& p) { return !p || !p->IsPlaying(); }))
                    break;
            }
        }


        std::clog << std::string(10, '\n');

        std::clog << "Stopping rendering thread...\n";
        audio_rendering_thread->Stop();

        std::clog << "Exit... \n";
    }

    ::MFShutdown();
    ::CoUninitialize();
    return 0;
}

static std::string as_fixed_status_string(const std::string& status_message)
{
    int lines = 0;
    std::string body;
    body.reserve(status_message.size() + 64);

    std::string buffer = "\x1b[K";
    buffer.reserve(256);

    const size_t prefix_length = buffer.size();
    for (auto c : status_message)
    {
        buffer += c;
        if (c == '\n')
        {
            body += buffer;
            buffer.resize(prefix_length);
            lines++;
        }
    }

    if (buffer.size() != prefix_length)
    {
        body += buffer;
        body += '\n';
        lines++;
    }

    std::string out;
    out.reserve(body.size() + 7 * lines);

    for (int i = 0; i < lines; i++) out += "\n";
    for (int i = 0; i < lines; i++) out += "\x1b[A";
    out += body;
    for (int i = 0; i < lines; i++) out += "\x1b[A";
    return out;
}
