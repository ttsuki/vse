/// @file
/// @brief  TTsukiGameSDK::bms::BmsPlaybackContext
/// @author (C) 2022 ttsuki

/// Copied from TTsukiGameSDK
/// MIT License: (C) 2022 ttsuki

#pragma once

#include <cstddef>
#include <chrono>
#include <type_traits>
#include <algorithm>

#include "BmsFile.h"
#include "BmsEvent.h"

namespace TTsukiGameSdk::bms
{
    struct BmsPlaybackContext
    {
        BmsPlaybackContext(const bms::BmsFile* file, const bms::BmsEventList* events)
            : BmsFile(file), BmsEvents(events)
            , TotalEventCount(events->size())
            , TotalTick(!events->empty() ? events->back().Tick : Tick{})
            , TotalTime(!events->empty() ? events->back().Timing : Timing{})
            , TotalMeasures(!events->empty() ? std::find_if(events->rbegin(), events->rend(), [](const BmsEvent& e) -> bool { return e.EventDataIs<BarEvent>(); })->EventDataIs<BarEvent>()->MeasureNumber : MeasureNumber{})
        {
            CurrentBpm = file->Information.InitialBpm;
        }

        using Clock = std::chrono::high_resolution_clock;

        const BmsFile* const BmsFile{};
        const BmsEventList* const BmsEvents{};
        size_t const TotalEventCount{};
        Tick const TotalTick{};
        Timing const TotalTime{};
        MeasureNumber const TotalMeasures{};

        size_t Cursor{};
        Bpm CurrentBpm{};

        struct Measure
        {
            MeasureNumber Number{};
            Tick StartsAt{};
            Tick Length{};
        } CurrentMeasure{};

        template <class T, std::enable_if_t<std::is_invocable_v<T, const BmsEvent&>>* = nullptr>
        void Update(Timing now, T&& event_callback)
        {
            while (Cursor < BmsEvents->size() && BmsEvents->at(Cursor).Timing < now)
            {
                const BmsEvent& event = BmsEvents->at(Cursor);

                if (auto* event_data = event.EventDataIs<BarEvent>()) { CurrentMeasure = {event_data->MeasureNumber, event.Tick, event_data->Duration}; }
                if (auto* event_data = event.EventDataIs<BpmChangeEvent>()) { CurrentBpm = event_data->NewBpm; }
                event_callback(event);
                Cursor++;
            }
        }

        [[nodiscard]] bool EndOfFile() const
        {
            return Cursor == TotalEventCount;
        }

        Tick GetCurrentTimingAsTick(Timing now) const
        {
            const double units_per_min = Timing(std::chrono::minutes(1)).count();
            const double beats_per_unit = CurrentBpm / units_per_min;
            const double ticks_per_unit = beats_per_unit * TimeBase / 4;

            if (Cursor == 0) { return 0; }
            if (BmsEvents->empty()) { return 0; }
            auto& last_event = BmsEvents->at(Cursor - 1);
            auto delta_time = (now - last_event.Timing).count();
            auto offset_tick = static_cast<Tick>(delta_time * ticks_per_unit);
            if (auto stop = last_event.EventDataIs<StopSequenceEvent>()) { offset_tick = std::max(Tick{0}, offset_tick - stop->StopTickCount); }
            return last_event.Tick + offset_tick;
        }
    };
}
