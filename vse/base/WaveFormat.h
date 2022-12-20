/// @file
/// @brief  Vse - Wave Format
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>
#include <mmreg.h>

#include <cstddef>
#include <string>
#include <bitset>

#include "CommonTypes.h"

namespace vse
{
    enum WaveFormatTag : WORD
    {
        WaveFormatTag_Pcm = WAVE_FORMAT_PCM,
        WaveFormatTag_IeeeFloat = WAVE_FORMAT_IEEE_FLOAT,
    };

    enum struct SampleType : DWORD
    {
        Unknown = 0,
        S16 = WaveFormatTag_Pcm << 16 | 16,
        S32 = WaveFormatTag_Pcm << 16 | 32,
        F32 = WaveFormatTag_IeeeFloat << 16 | 32,
    };

    enum struct SpeakerBit : DWORD
    {
        FrontLeft = SPEAKER_FRONT_LEFT,
        FrontRight = SPEAKER_FRONT_RIGHT,
        FrontCenter = SPEAKER_FRONT_CENTER,
        LowFrequency = SPEAKER_LOW_FREQUENCY,
        BackLeft = SPEAKER_BACK_LEFT,
        BackRight = SPEAKER_BACK_RIGHT,
        FrontLeftOfCenter = SPEAKER_FRONT_LEFT_OF_CENTER,
        FrontRightOfCenter = SPEAKER_FRONT_RIGHT_OF_CENTER,
        BackCenter = SPEAKER_BACK_CENTER,
        SideLeft = SPEAKER_SIDE_LEFT,
        SideRight = SPEAKER_SIDE_RIGHT,
        TopCenter = SPEAKER_TOP_CENTER,
        TopFrontLeft = SPEAKER_TOP_FRONT_LEFT,
        TopFrontCenter = SPEAKER_TOP_FRONT_CENTER,
        TopFrontRight = SPEAKER_TOP_FRONT_RIGHT,
        TopBackLeft = SPEAKER_TOP_BACK_LEFT,
        TopBackCenter = SPEAKER_TOP_BACK_CENTER,
        TopBackRight = SPEAKER_TOP_BACK_RIGHT,

        FrontPair = FrontLeft | FrontRight,
        CenterPair = FrontCenter | LowFrequency,
        BackPair = BackLeft | BackRight,
        SidePair = SideLeft | SideRight,

        None = 0,
        SpeakerSet_1_0ch = FrontCenter,
        SpeakerSet_2_0ch = FrontPair,
        SpeakerSet_2_1ch = FrontPair | CenterPair,
        SpeakerSet_4_0ch = FrontPair | BackPair,
        SpeakerSet_5_1ch = FrontPair | CenterPair | BackPair,
        SpeakerSet_7_1ch = FrontPair | CenterPair | BackPair | SidePair,
    };

    static inline constexpr std::underlying_type_t<SpeakerBit> operator +(SpeakerBit rhs) { return static_cast<std::underlying_type_t<SpeakerBit>>(rhs); }
    static inline constexpr SpeakerBit operator ~(SpeakerBit rhs) { return static_cast<SpeakerBit>(~+rhs); }
    static inline constexpr SpeakerBit operator |(SpeakerBit lhs, SpeakerBit rhs) { return static_cast<SpeakerBit>(+lhs | +rhs); }
    static inline constexpr SpeakerBit operator &(SpeakerBit lhs, SpeakerBit rhs) { return static_cast<SpeakerBit>(+lhs & +rhs); }
    static inline constexpr SpeakerBit operator ^(SpeakerBit lhs, SpeakerBit rhs) { return static_cast<SpeakerBit>(+lhs ^ +rhs); }
    static inline constexpr SpeakerBit& operator |=(SpeakerBit& lhs, SpeakerBit rhs) { return lhs = lhs | rhs; }
    static inline constexpr SpeakerBit& operator &=(SpeakerBit& lhs, SpeakerBit rhs) { return lhs = lhs & rhs; }
    static inline constexpr SpeakerBit& operator ^=(SpeakerBit& lhs, SpeakerBit rhs) { return lhs = lhs ^ rhs; }

    [[nodiscard]] static constexpr inline SpeakerBit DefaultChannelMask(size_t channel_count)
    {
        switch (channel_count)
        {
        case 1: return SpeakerBit::SpeakerSet_1_0ch;
        case 2: return SpeakerBit::SpeakerSet_2_0ch;
        case 4: return SpeakerBit::SpeakerSet_4_0ch;
        case 6: return SpeakerBit::SpeakerSet_5_1ch;
        case 8: return SpeakerBit::SpeakerSet_7_1ch;
        default: return SpeakerBit::None;
        }
    }

    // Represents Simple PCM/IEEE_FLOAT WAVEFORMATEXTENSIBLE
    struct PcmWaveFormat
    {
        SampleType format_{};
        SpeakerBit channels_{};
        int frequency_{};

        PcmWaveFormat() = default;
        PcmWaveFormat(SampleType format, SpeakerBit channels, int frequency) : format_(format), channels_(channels), frequency_(frequency) {}

        [[nodiscard]] SampleType SampleType() const noexcept { return format_; }
        [[nodiscard]] SpeakerBit ChannelMask() const noexcept { return channels_; }
        [[nodiscard]] int SamplingFrequency() const noexcept { return frequency_; }

        [[nodiscard]] int ChannelCount() const noexcept { return static_cast<int>(std::bitset<32>(+ChannelMask()).count()); }
        [[nodiscard]] int BitsPerSample() const noexcept { return static_cast<int>(SampleType()) & 0xFF; }
        [[nodiscard]] int BlockAlign() const noexcept { return (BitsPerSample() / 8) * ChannelCount(); }
        [[nodiscard]] int AvgBytesPerSec() const noexcept { return BlockAlign() * SamplingFrequency(); }

        explicit operator bool() const noexcept { return format_ != SampleType::Unknown; }
        bool operator ==(const PcmWaveFormat& rhs) const noexcept { return format_ != SampleType::Unknown && std::tie(format_, frequency_, channels_) == std::tie(rhs.format_, rhs.frequency_, rhs.channels_); }
        bool operator !=(const PcmWaveFormat& rhs) const noexcept { return format_ == SampleType::Unknown || std::tie(format_, frequency_, channels_) != std::tie(rhs.format_, rhs.frequency_, rhs.channels_); }

        static PcmWaveFormat Parse(const WAVEFORMATEX* wf) noexcept;
        static PcmWaveFormat Parse(const WAVEFORMATEX& f) noexcept { return Parse(&f); }
        static PcmWaveFormat Parse(const WAVEFORMATEXTENSIBLE& f) noexcept { return Parse(&f.Format); }

        operator WAVEFORMATEX() const noexcept;
        operator WAVEFORMATEXTENSIBLE() const noexcept;

        [[nodiscard]] std::string ToString() const;
    };

    // WAVEFORMATEX helpers

    [[nodiscard]] bool Equals(const WAVEFORMATEX* lhs, const WAVEFORMATEX* rhs) noexcept;
    [[nodiscard]] static inline bool operator ==(const WAVEFORMATEX& lhs, const WAVEFORMATEX& rhs) noexcept { return Equals(&lhs, &rhs); }
    [[nodiscard]] static inline bool operator !=(const WAVEFORMATEX& lhs, const WAVEFORMATEX& rhs) noexcept { return !(lhs == rhs); }
    [[nodiscard]] static inline bool operator ==(const WAVEFORMATEXTENSIBLE& lhs, const WAVEFORMATEXTENSIBLE& rhs) noexcept { return Equals(&lhs.Format, &rhs.Format); }
    [[nodiscard]] static inline bool operator !=(const WAVEFORMATEXTENSIBLE& lhs, const WAVEFORMATEXTENSIBLE& rhs) noexcept { return !(lhs == rhs); }

    [[nodiscard]] std::string ToString(const WAVEFORMATEX* f);
    [[nodiscard]] static inline std::string ToString(const WAVEFORMATEX& f) { return ToString(&f); }
    [[nodiscard]] static inline std::string ToString(const WAVEFORMATEXTENSIBLE& f) { return ToString(&f.Format); }
    [[nodiscard]] static inline std::string ToString(const PcmWaveFormat& f) { return f.ToString(); }
}
