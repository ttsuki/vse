/// @file
/// @brief  Vse - Volume calculation helper
/// @author (C) 2022 ttsuki

#pragma once

#include <cstddef>
#include <cmath>
#include <limits>
#include <array>
#include <algorithm>
#include <bitset>
#include <stdexcept>

namespace vse
{
    /// Convert decibel to linear amplitude ratio.
    /// Examples:
    ///   -ininity dB (  0%) to 0.00.
    ///   -12.0412 dB ( 25%) to 0.25.
    ///    -6.0206 dB ( 50%) to 0.50.
    ///     0.0000 dB (100%) to 1.00.
    ///    +6.0206 dB (200%) to 2.00.
    ///   +12.0412 dB (400%) to 4.00.
    inline float DecibelToLinear(float decibel)
    {
        return std::powf(10.0f, decibel / 20.0f);
    }

    /// Convert linear amplitude ratio to decibel.
    /// Examples:
    ///   0.00 to -ininity dB (  0%).
    ///   0.25 to -12.0412 dB ( 25%).
    ///   0.50 to  -6.0206 dB ( 50%).
    ///   1.00 to   0.0000 dB (100%).
    ///   2.00 to  +6.0206 dB (200%).
    ///   4.00 to +12.0412 dB (400%).
    inline float LinearToDecibel(float linear_amplitude)
    {
        return linear_amplitude > 0.0f ? 20.0f * std::log10(linear_amplitude) : -std::numeric_limits<float>::infinity();
    }

    /// Calculates stereo mix volume.
    /// @param volume Volume (in linear) - Mute [0.0 .. 1.0 .. +∞ ] ∞
    /// @param pan Panning - Left [-1.0 .. 0.0 .. +1.0] Right
    /// @param [out]lch_mix - Left mix value
    /// @param [out]rch_mix - Right mix value
    inline void CalculateStereoVolume(float volume, float pan, float* lch_mix, float* rch_mix)
    {
        // at vol=100%
        //    pan=-1.0 -> l=141.4% r=  0.0%
        //    pan=-0.5 -> l=130.7% r= 54.1%
        //    pan= 0.0 -> l=100.0% r=100.0%
        //    pan=+0.5 -> l= 54.1% r=130.7%
        //    pan=+1.0 -> l=  0.0% r=141.4%

        auto pi = std::acos(-1.0f);
        auto sqrt2 = std::sqrt(2.0f);

        // theta = project pan[-1..0..+1] to [0deg..45deg..90deg] (in radian).
        float theta = (std::clamp(pan, -1.0f, 1.0f) + 1.0f) / 4.0f * pi;
        if (lch_mix) *lch_mix = volume * std::cos(theta) * sqrt2;
        if (rch_mix) *rch_mix = volume * std::sin(theta) * sqrt2;
    }

    /// VolumeCalculationTable
    class VolumeCalculationTable
    {
        static constexpr inline int master_volume_bit_count_ = 8;
        static constexpr inline size_t table_size_ = size_t{1} << master_volume_bit_count_;
        static constexpr inline unsigned bit_mask_ = static_cast<unsigned>(table_size_ - 1);

        std::array<float, table_size_> table_{};

    public:
        VolumeCalculationTable()
        {
            std::fill(table_.begin(), table_.end(), 1.0f);
        }

        /// Sets volume for bit
        /// @param bit Target bit.
        /// @param volume Volume in linear amplitude ratio.
        void SetMultiplierForBit(unsigned bit, float volume)
        {
            if (bit & ~bit_mask_)throw std::invalid_argument("invalid bit is set.");
            if (std::bitset<master_volume_bit_count_>(bit).count() != 1) throw std::invalid_argument("bit must be only single bit");

            table_[bit & bit_mask_] = volume;

            // Re calculates volume for all bitset value
            for (size_t i = 0; i < table_size_; i++)
            {
                float v = 1.0f;
                for (int j = 0; j < master_volume_bit_count_; j++)
                    if (i & size_t{1} << j)
                        v *= table_[size_t{1} << j];

                table_[i] = v;
            }
        }

        /// Get volume for bitset 
        /// @param bit_set Target bit set.
        /// @returns Calculated volume in linear amplitude ratio.
        float CalcurateVolumeForBitSet(unsigned bit_set)
        {
            return table_[bit_set & bit_mask_];
        }
    };
}
