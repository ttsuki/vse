/// @file
/// @brief  TTsukiGameSDK::bms::BmsEvent
/// @author (C) 2022 ttsuki

/// Copied from TTsukiGameSDK
/// MIT License: (C) 2022 ttsuki

#pragma once

#include <chrono>
#include <variant>

#include "BmsFile.h"

namespace TTsukiGameSdk::bms
{
    using Tick = int64_t;
    using Timing = std::chrono::duration<double>;
    static constexpr Tick TimeBase = 3840;

    struct BarEvent
    {
        MeasureNumber MeasureNumber;
        Tick Duration;
    };

    struct BpmChangeEvent
    {
        double NewBpm;
    };

    struct StopSequenceEvent
    {
        Tick StopTickCount; // in bms::TimeBase
    };

    struct ChorusPlayEvent
    {
        int WaveNumber; // [0..1296)
    };

    struct HiddenNoteEvent
    {
        int Channel;    // [0..32)
        int WaveNumber; // [0..1296)
    };

    struct NoteEvent
    {
        int Channel;    // [0..32)
        int WaveNumber; // [0..1296)
        Tick Duration;  // =0: normal note, >0: long note
    };

    struct BmpChangeEvent
    {
        enum struct LayerIndex
        {
            Bga,
            BgaLayer1,
            BgaLayer2,
            BgaLayer3,
            BgaPoor,
        } Layer;

        int BmpNumber; // [0..1296)
    };

    struct TextChangeEvent
    {
        int TextNumber; // [0..1296)
    };

    struct BmsEvent
    {
        Tick Tick{};
        std::variant<
            BarEvent,
            BpmChangeEvent,
            ChorusPlayEvent,
            HiddenNoteEvent,
            NoteEvent,
            BmpChangeEvent,
            TextChangeEvent,
            StopSequenceEvent
        > EventData{};
        template <class T> [[nodiscard]] T* EventDataIs() { return std::get_if<T>(&EventData); }
        template <class T> [[nodiscard]] const T* EventDataIs() const { return std::get_if<T>(&EventData); }

        Timing Timing{};
    };

    using BmsEventList = std::vector<BmsEvent>;

    static std::vector<BmsEvent> BuildEventList(const BmsFile& parsed_input, std::ostream& cerr, std::ostream& cinfo, std::ostream& cdebug)
    {
        std::vector<BmsEvent> output;

        [[maybe_unused]] std::ostream& cerr_ = cerr;
        [[maybe_unused]] std::ostream& cinfo_ = cinfo;
        [[maybe_unused]] std::ostream& cdebug_ = cdebug;

        // calculates measure offset/length
        struct measure_info_t
        {
            Tick start_at;
            Tick length;
        };

        std::vector<measure_info_t> measure_info{};
        {
            MeasureNumber last_measure = 0;
            for (auto&& e : parsed_input.RawEvents)
                last_measure = std::max(last_measure, e.Measure);

            Tick current = 0;
            for (MeasureNumber i = 0; i <= last_measure + 1; i++)
            {
                Tick length = TimeBase;
                if (auto it = parsed_input.MeasureLength.find(i); it != parsed_input.MeasureLength.end())
                    length = static_cast<Tick>(std::round(static_cast<double>(length) * it->second));

                measure_info.push_back({current, length});
                output.push_back({current, BarEvent{i, length}});

                current += length;
            }
        }

        // convert all raw events to BmsEvent
        {
            // context
            const int LNOBJ = parsed_input.LNOBJ;            // for long note parsing
            std::array<size_t, 32> last_normal_note_index{}; // for long note parsing
            std::array<size_t, 32> last_long_note_index{};   // for long note parsing

            for (auto&& [measure_number, channel, events] : parsed_input.RawEvents)
            {
                using BmsEventCode = BmsFile::BmsEventCode;
                const auto& measure = measure_info[measure_number];

                for (size_t i = 0; i < events.size(); ++i)
                {
                    Tick tick = measure.start_at + (i * measure.length / events.size());
                    int value = events[i];
                    if (value == 0) continue;

                    switch (channel)
                    {
                    case BmsEventCode::PlayBackChorus:
                        output.push_back({tick, ChorusPlayEvent{value}});
                        break;

                    case BmsEventCode::ChangeBPM:
                        output.push_back({tick, BpmChangeEvent{static_cast<double>(value)}});
                        break;

                    case BmsEventCode::ChangeBPMFromTable:
                        if (Bpm bpm = parsed_input.BpmTable[value]; bpm != Bpm{})
                            output.push_back({tick, BpmChangeEvent{bpm}});
                        else
                            cerr_ << "Build Error: ChangeBPMFromTable index=" << value << " is not defined or zero. ignore.";
                        break;

                    case BmsEventCode::ChangeBMP:
                        output.push_back({tick, BmpChangeEvent{BmpChangeEvent::LayerIndex::Bga, value}});
                        break;

                    case BmsEventCode::ChangeBMPPoor:
                        output.push_back({tick, BmpChangeEvent{BmpChangeEvent::LayerIndex::BgaPoor, value}});
                        break;

                    case BmsEventCode::ChangeBMPLayer:
                        output.push_back({tick, BmpChangeEvent{BmpChangeEvent::LayerIndex::BgaLayer1, value}});
                        break;

                    case BmsEventCode::ChangeBMPLayer2:
                        output.push_back({tick, BmpChangeEvent{BmpChangeEvent::LayerIndex::BgaLayer2, value}});
                        break;

                    case BmsEventCode::StopSequenceFromTable:
                        if (Tick stop = parsed_input.StopTable[value] * TimeBase / 192; stop != 0)
                            output.push_back({tick, StopSequenceEvent{stop}});
                        else
                            cerr_ << "Build Error: StopSequenceFromTable index=" << value << " is not defined or zero. ignore.";
                        break;

                    case BmsEventCode::ChangeTextFromList:
                        output.push_back({tick, TextChangeEvent{value}});
                        break;

                    case BmsEventCode::NormalNote00:
                    case BmsEventCode::NormalNote01:
                    case BmsEventCode::NormalNote02:
                    case BmsEventCode::NormalNote03:
                    case BmsEventCode::NormalNote04:
                    case BmsEventCode::NormalNote05:
                    case BmsEventCode::NormalNote06:
                    case BmsEventCode::NormalNote07:
                    case BmsEventCode::NormalNote08:
                    case BmsEventCode::NormalNote09:
                    case BmsEventCode::NormalNote0A:
                    case BmsEventCode::NormalNote0B:
                    case BmsEventCode::NormalNote0C:
                    case BmsEventCode::NormalNote0D:
                    case BmsEventCode::NormalNote0E:
                    case BmsEventCode::NormalNote0F:
                    case BmsEventCode::NormalNote10:
                    case BmsEventCode::NormalNote11:
                    case BmsEventCode::NormalNote12:
                    case BmsEventCode::NormalNote13:
                    case BmsEventCode::NormalNote14:
                    case BmsEventCode::NormalNote15:
                    case BmsEventCode::NormalNote16:
                    case BmsEventCode::NormalNote17:
                    case BmsEventCode::NormalNote18:
                    case BmsEventCode::NormalNote19:
                    case BmsEventCode::NormalNote1A:
                    case BmsEventCode::NormalNote1B:
                    case BmsEventCode::NormalNote1C:
                    case BmsEventCode::NormalNote1D:
                    case BmsEventCode::NormalNote1E:
                    case BmsEventCode::NormalNote1F:
                        if (int lane = static_cast<int>(channel) - static_cast<int>(BmsEventCode::NormalNote00); value != LNOBJ)
                        {
                            // normal note entry.
                            last_normal_note_index[lane] = output.size();
                            output.push_back({tick, NoteEvent{lane, value, 0}});
                        }
                        else if (last_normal_note_index[lane])
                        {
                            // note off is found, change last normal note to long note.
                            auto& t = output[last_normal_note_index[lane]];
                            std::get<NoteEvent>(t.EventData).Duration = tick;
                            last_normal_note_index[lane] = 0;
                        }
                        else
                        {
                            cerr_ << "Build Error: Note off found without corresponding note on. ignore." << std::endl;
                        }
                        break;

                    case BmsEventCode::HiddenNote00:
                    case BmsEventCode::HiddenNote01:
                    case BmsEventCode::HiddenNote02:
                    case BmsEventCode::HiddenNote03:
                    case BmsEventCode::HiddenNote04:
                    case BmsEventCode::HiddenNote05:
                    case BmsEventCode::HiddenNote06:
                    case BmsEventCode::HiddenNote07:
                    case BmsEventCode::HiddenNote08:
                    case BmsEventCode::HiddenNote09:
                    case BmsEventCode::HiddenNote0A:
                    case BmsEventCode::HiddenNote0B:
                    case BmsEventCode::HiddenNote0C:
                    case BmsEventCode::HiddenNote0D:
                    case BmsEventCode::HiddenNote0E:
                    case BmsEventCode::HiddenNote0F:
                    case BmsEventCode::HiddenNote10:
                    case BmsEventCode::HiddenNote11:
                    case BmsEventCode::HiddenNote12:
                    case BmsEventCode::HiddenNote13:
                    case BmsEventCode::HiddenNote14:
                    case BmsEventCode::HiddenNote15:
                    case BmsEventCode::HiddenNote16:
                    case BmsEventCode::HiddenNote17:
                    case BmsEventCode::HiddenNote18:
                    case BmsEventCode::HiddenNote19:
                    case BmsEventCode::HiddenNote1A:
                    case BmsEventCode::HiddenNote1B:
                    case BmsEventCode::HiddenNote1C:
                    case BmsEventCode::HiddenNote1D:
                    case BmsEventCode::HiddenNote1E:
                    case BmsEventCode::HiddenNote1F:
                        if (int lane = static_cast<int>(channel) - static_cast<int>(BmsEventCode::HiddenNote00); true)
                            output.push_back({tick, HiddenNoteEvent{lane, value}});
                        break;

                    case BmsEventCode::LongNote00:
                    case BmsEventCode::LongNote01:
                    case BmsEventCode::LongNote02:
                    case BmsEventCode::LongNote03:
                    case BmsEventCode::LongNote04:
                    case BmsEventCode::LongNote05:
                    case BmsEventCode::LongNote06:
                    case BmsEventCode::LongNote07:
                    case BmsEventCode::LongNote08:
                    case BmsEventCode::LongNote09:
                    case BmsEventCode::LongNote0A:
                    case BmsEventCode::LongNote0B:
                    case BmsEventCode::LongNote0C:
                    case BmsEventCode::LongNote0D:
                    case BmsEventCode::LongNote0E:
                    case BmsEventCode::LongNote0F:
                    case BmsEventCode::LongNote10:
                    case BmsEventCode::LongNote11:
                    case BmsEventCode::LongNote12:
                    case BmsEventCode::LongNote13:
                    case BmsEventCode::LongNote14:
                    case BmsEventCode::LongNote15:
                    case BmsEventCode::LongNote16:
                    case BmsEventCode::LongNote17:
                    case BmsEventCode::LongNote18:
                    case BmsEventCode::LongNote19:
                    case BmsEventCode::LongNote1A:
                    case BmsEventCode::LongNote1B:
                    case BmsEventCode::LongNote1C:
                    case BmsEventCode::LongNote1D:
                    case BmsEventCode::LongNote1E:
                    case BmsEventCode::LongNote1F:
                        if (int lane = static_cast<int>(channel) - static_cast<int>(BmsEventCode::LongNote00); last_long_note_index[lane] == 0)
                        {
                            // note on.
                            last_long_note_index[lane] = output.size();
                            output.push_back({tick, NoteEvent{lane, value, 0}});
                        }
                        else
                        {
                            // note off.
                            auto& t = output[last_long_note_index[lane]];
                            std::get<NoteEvent>(t.EventData).Duration = tick;
                            last_long_note_index[lane] = 0;
                        }
                        break;

                    default:
                    case BmsEventCode::ChangeBarLength:
                    case BmsEventCode::ExtChr:
                        // ignore.
                        break;
                    }
                }
            }
        }

        // sort all events 
        std::stable_sort(output.begin(), output.end(), [](const BmsEvent& a, const BmsEvent& b)
        {
            return a.Tick != b.Tick
                       ? a.Tick < b.Tick
                       : a.EventData.index() < b.EventData.index();
        });

        // calculate timing (in seconds) by playback emulation
        {
            using Time = double;

            double bpm = parsed_input.Information.InitialBpm;
            Tick ref_tick = 0;
            Time ref_time = 0.0;
            Tick stop_tick = 0;

            for (auto& e : output)
            {
                double ticks_per_second = (bpm / 60.0) * (TimeBase / 4);
                Tick offset_tick = (e.Tick + stop_tick) - ref_tick;
                Time current_time = static_cast<double>(offset_tick) / ticks_per_second + ref_time;
                e.Timing = Timing(std::chrono::duration<double>(current_time));

                // Update reference tick/time on BPM Change event
                if (BpmChangeEvent* ee = std::get_if<BpmChangeEvent>(&e.EventData))
                {
                    bpm = ee->NewBpm;
                    ref_tick = e.Tick;
                    ref_time = current_time;
                    stop_tick = 0;
                }

                // Update stop tick
                if (StopSequenceEvent* ee = std::get_if<StopSequenceEvent>(&e.EventData))
                {
                    stop_tick += ee->StopTickCount;
                }
            }
        }

        return output;
    }
}
