/// @file
/// @brief  Vse - RBJ's Audio-EQ-Cookbook
/// @author (C) 2022 ttsuki
///
/// An implementation of RBJ's Audio-EQ-Cookbook
/// https://webaudio.github.io/Audio-EQ-Cookbook/Audio-EQ-Cookbook.txt

#pragma once

#include <cstddef>
#include <cmath>

#include "../base/WaveFormat.h"
#include "../base/IWaveProcessor.h"

namespace vse
{
    namespace rbj_audio_eq
    {
        constexpr float Pi = static_cast<float>(3.1415926535897932384626433832795);

        static inline float A(float dbGain) { return pow(10.0f, dbGain / 40.0f); }
        static inline float w0(float f0, float Fs) { return 2.0f * Pi * f0 / Fs; }
        static inline float alphaQ(float Q, float w0) { return sin(w0) / (2 * Q); }
        static inline float alphaBW(float BW, float w0) { return sin(w0) * sinh(log(2.0f) / 2 * BW * w0 / sin(w0)); }
        static inline float alphaS(float S, float A, float w0) { return sin(w0) / 2 * sqrt((A + 1 / A) * (1 / S - 1) + 2); }

        struct Coefficients
        {
            float b0a0, b1a0, b2a0, a1a0, a2a0;

            static Coefficients from(float b0, float b1, float b2, float a0, float a1, float a2)
            {
                return {b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0};
            }
        };

        static inline Coefficients LowPassFilterCoefficients(float w0, float alpha)
        {
            // H(s) = 1 / (s^2 + s/Q + 1)
            // @formatter:off
            return Coefficients::from
            ( /* b0 */  (1 - cos(w0))/2
            , /* b1 */   1 - cos(w0)
            , /* b2 */  (1 - cos(w0))/2
            , /* a0 */   1 + alpha
            , /* a1 */  -2*cos(w0)
            , /* a2 */   1 - alpha
            );
            // @formatter:on
        }

        static inline Coefficients HighPassFilterCoefficients(float w0, float alpha)
        {
            // H(s) = s^2 / (s^2 + s/Q + 1)
            // @formatter:off
            return Coefficients::from
            ( /* b0 */  (1 + cos(w0))/2
            , /* b1 */ -(1 + cos(w0))
            , /* b2 */  (1 + cos(w0))/2
            , /* a0 */   1 + alpha
            , /* a1 */  -2*cos(w0)
            , /* a2 */   1 - alpha
            );
            // @formatter:on
        }

        static inline Coefficients BandPassFilterCoefficients_ConstantSkirtGain(float w0, float alpha)
        {
            // H(s) = s / (s^2 + s/Q + 1)  (constant skirt gain, peak gain = Q)
            // @formatter:off
            return Coefficients::from
            ( /* b0 */   sin(w0)/2
            , /* b1 */   0
            , /* b2 */  -sin(w0)/2
            , /* a0 */   1 + alpha
            , /* a1 */  -2*cos(w0)
            , /* a2 */   1 - alpha
            );
            // @formatter:on
        }

        static inline Coefficients BandPassFilterCoefficients_Constant0dBPeakGain(float w0, float alpha)
        {
            // H(s) = (s/Q) / (s^2 + s/Q + 1)      (constant 0 dB peak gain)
            // @formatter:off
            return Coefficients::from
            ( /* b0 */   alpha
            , /* b1 */   0
            , /* b2 */  -alpha
            , /* a0 */   1 + alpha
            , /* a1 */  -2*cos(w0)
            , /* a2 */   1 - alpha
            );
            // @formatter:on
        }

        static inline Coefficients NotchFilterCoefficients(float w0, float alpha)
        {
            // H(s) = (s^2 + 1) / (s^2 + s/Q + 1)
            // @formatter:off
            return Coefficients::from
            (  /* b0 */   1
            ,  /* b1 */  -2*cos(w0)
            ,  /* b2 */   1
            ,  /* a0 */   1 + alpha
            ,  /* a1 */  -2*cos(w0)
            ,  /* a2 */   1 - alpha
            );
            // @formatter:on
        }

        static inline Coefficients AllPassFilterCoefficients(float w0, float alpha)
        {
            // H(s) = (s^2 - s/Q + 1) / (s^2 + s/Q + 1)
            // @formatter:off
            return Coefficients::from
            ( /* b0 */   1 - alpha
            , /* b1 */  -2*cos(w0)
            , /* b2 */   1 + alpha
            , /* a0 */   1 + alpha
            , /* a1 */  -2*cos(w0)
            , /* a2 */   1 - alpha
            );
            // @formatter:on
        }


        static inline Coefficients PeakingEqFilterCoefficients(float w0, float alpha, float A)
        {
            // H(s) = (s^2 + s*(A/Q) + 1) / (s^2 + s/(A*Q) + 1)
            // @formatter:off
            return Coefficients::from
            ( /* b0 */   1 + alpha*A
            , /* b1 */  -2*cos(w0)
            , /* b2 */   1 - alpha*A
            , /* a0 */   1 + alpha/A
            , /* a1 */  -2*cos(w0)
            , /* a2 */   1 - alpha/A
            );
            // @formatter:on
        }

        static inline Coefficients LowShelfFilterCoefficients(float w0, float alpha, float A)
        {
            // H(s) = A * (s^2 + (sqrt(A)/Q)*s + A)/(A*s^2 + (sqrt(A)/Q)*s + 1)
            // @formatter:off
            return Coefficients::from
            ( /* b0 */    A*( (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha )
            , /* b1 */  2*A*( (A-1) - (A+1)*cos(w0)                   )
            , /* b2 */    A*( (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha )
            , /* a0 */        (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha
            , /* a1 */   -2*( (A-1) + (A+1)*cos(w0)                   )
            , /* a2 */        (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha
            );
            // @formatter:on
        }

        static inline Coefficients HighShelfFilterCoefficients(float w0, float alpha, float A)
        {
            // H(s) = A * (A*s^2 + (sqrt(A)/Q)*s + 1)/(s^2 + (sqrt(A)/Q)*s + A)
            // @formatter:off
            return Coefficients::from
            ( /* b0 */    A*( (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha )
            , /* b1 */ -2*A*( (A-1) + (A+1)*cos(w0)                   )
            , /* b2 */    A*( (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha )
            , /* a0 */        (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha
            , /* a1 */    2*( (A-1) - (A+1)*cos(w0)                   )
            , /* a2 */        (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha
            );
            // @formatter:on
        }

        struct ShiftRegister
        {
            float y1, y2, x1, x2;
        };

        static inline ShiftRegister ProcessSingleChannel(
            ShiftRegister state, const Coefficients& coeff,
            float* output, const float* input, size_t count, size_t stride = 1)
        {
            float y0 = 0;
            float y1 = state.y1;
            float y2 = state.y2;
            float x0 = 0;
            float x1 = state.x1;
            float x2 = state.x2;

            for (size_t i = 0; i < count / 3 * 3; i += 3)
            {
                y0 = *output = coeff.b0a0 * (x0 = *input) + coeff.b1a0 * x1 + coeff.b2a0 * x2 - coeff.a1a0 * y1 - coeff.a2a0 * y2;
                output += stride;
                input += stride;

                y2 = *output = coeff.b0a0 * (x2 = *input) + coeff.b1a0 * x0 + coeff.b2a0 * x1 - coeff.a1a0 * y0 - coeff.a2a0 * y1;
                output += stride;
                input += stride;

                y1 = *output = coeff.b0a0 * (x1 = *input) + coeff.b1a0 * x2 + coeff.b2a0 * x0 - coeff.a1a0 * y2 - coeff.a2a0 * y0;
                output += stride;
                input += stride;
            }
            if (count % 3 == 0) return {y1, y2, x1, x2};

            y0 = *output = coeff.b0a0 * (x0 = *input) + coeff.b1a0 * x1 + coeff.b2a0 * x2 - coeff.a1a0 * y1 - coeff.a2a0 * y2;
            output += stride;
            input += stride;
            if (count % 3 == 1) return {y0, y1, x0, x1};

            y2 = *output = coeff.b0a0 * (x2 = *input) + coeff.b1a0 * x0 + coeff.b2a0 * x1 - coeff.a1a0 * y0 - coeff.a2a0 * y1;
            output += stride;
            input += stride;
            return {y2, y0, x2, x0};
        }
    }

    /// Creates Audio EQ Cookbook's Biquad Filter
    /// @param format source format
    /// @param coefficients filter parameter.
    std::shared_ptr<IWaveProcessor> CreateBiquadIirFilter(
        PcmWaveFormat format,
        rbj_audio_eq::Coefficients coefficients);
}
