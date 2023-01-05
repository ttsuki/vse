/// @file
/// @brief  Vse - Waveform processing functions (Implementation helper)
/// @author (C) 2022 ttsuki

#include "WaveformProcessing.h"

#include <cmath>
#include <algorithm>

#ifdef __RESHARPER__
#define __AVX2__
#endif

#ifdef __AVX2__
#include "../base/arkxmm.h"
using namespace arkana;
#endif

namespace vse::processing
{
    void ConvertCopy(S16* __restrict dst, const S32* __restrict src, size_t count) noexcept
    {
#ifdef __AVX2__
        for (size_t i = 0; i < count / 8; i++)
        {
            xmm::store_u<xmm::vi16x8>(
                dst, xmm::pack_sat_i(
                    xmm::load_u<xmm::vi32x4>(src + 0) >> 16,
                    xmm::load_u<xmm::vi32x4>(src + 4) >> 16));

            src += 8;
            dst += 8;
        }
        count %= 8;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] = static_cast<S16>(src[i] >> 16);
        }
    }

    void ConvertCopy(S16* __restrict dst, const S24* __restrict src, size_t count) noexcept
    {
#ifdef __AVX2__
        for (size_t i = 0; i < count / 16; i++)
        {
            auto x0 = xmm::load_u<xmm::vu8x16>(reinterpret_cast<const std::byte*>(src) + 0);
            auto x1 = xmm::load_u<xmm::vu8x16>(reinterpret_cast<const std::byte*>(src) + 16);
            auto x2 = xmm::load_u<xmm::vu8x16>(reinterpret_cast<const std::byte*>(src) + 32);

            auto y0 = xmm::byte_align_r_128<6>(
                xmm::byte_shuffle_128(x0, xmm::i8x16(-1, -1, -1, -1, -1, -1, 1, 2, 4, 5, 7, 8, 10, 11, 13, 14)),
                xmm::byte_shuffle_128(x1, xmm::i8x16(0, 1, 3, 4, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)));

            auto y1 = xmm::byte_align_r_128<11>(
                xmm::byte_shuffle_128(x1, xmm::i8x16(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 9, 10, 12, 13, 15)),
                xmm::byte_shuffle_128(x2, xmm::i8x16(0, 2, 3, 5, 6, 8, 9, 11, 12, 14, 15, -1, -1, -1, -1, -1)));

            xmm::store_u<xmm::vu8x16>(reinterpret_cast<std::byte*>(dst) + 0, y0);
            xmm::store_u<xmm::vu8x16>(reinterpret_cast<std::byte*>(dst) + 16, y1);

            src += 16;
            dst += 16;
        }
        count %= 16;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] = static_cast<S16>(static_cast<S32>(src[i]) >> 8);
        }
    }

    void ConvertCopy(S16* __restrict dst, const F32* __restrict src, size_t count) noexcept
    {
        const float scale = 0x1p+15f; // = 1 << 15
        const float maxi = std::nextafter(+scale, -1.0f);
        const float mini = std::nextafter(-scale, -1.0f);

#ifdef __AVX2__
        const auto scalev = xmm::f32x4(scale);
        const auto maxiv = xmm::f32x4(maxi);
        const auto miniv = xmm::f32x4(mini);
        for (size_t i = 0; i < count / 8; i++)
        {
            auto x0 = xmm::load_u<xmm::vf32x4>(src + 0);
            auto x1 = xmm::load_u<xmm::vf32x4>(src + 4);
            auto w0 = xmm::clamp(x0 * scalev, miniv, maxiv);
            auto w1 = xmm::clamp(x1 * scalev, miniv, maxiv);
            auto y0 = xmm::convert_cast<xmm::vi32x4>(w0);
            auto y1 = xmm::convert_cast<xmm::vi32x4>(w1);
            xmm::store_u<xmm::vi16x8>(dst, pack_sat_i(y0, y1));

            src += 8;
            dst += 8;
        }
        count %= 8;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] = static_cast<S16>(std::clamp(src[i] * scale, mini, maxi));
        }
    }

    void ConvertCopy(S24* __restrict dst, const S16* __restrict src, size_t count) noexcept
    {
#ifdef __AVX2__
        for (size_t i = 0; i < count / 16; i++)
        {
            auto x0 = xmm::load_u<xmm::vu8x16>(src + 0);
            auto x1 = xmm::load_u<xmm::vu8x16>(src + 8);

            auto y0 = xmm::byte_shuffle_128(x0, xmm::i8x16(-1, 0, 1, -1, 2, 3, -1, 4, 5, -1, 6, 7, -1, 8, 9, -1));
            auto y1 = xmm::byte_shuffle_128(xmm::byte_align_r_128<8>(x0, x1), xmm::i8x16(2, 3, -1, 4, 5, -1, 6, 7, -1, 8, 9, -1, 10, 11, -1, 12));
            auto y2 = xmm::byte_shuffle_128(x1, xmm::i8x16(5, -1, 6, 7, -1, 8, 9, -1, 10, 11, -1, 12, 13, -1, 14, 15));

            xmm::store_u<xmm::vu8x16>(reinterpret_cast<std::byte*>(dst) + 0, y0);
            xmm::store_u<xmm::vu8x16>(reinterpret_cast<std::byte*>(dst) + 16, y1);
            xmm::store_u<xmm::vu8x16>(reinterpret_cast<std::byte*>(dst) + 32, y2);

            src += 16;
            dst += 16;
        }
        count %= 16;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] = S24(src[i] << 8);
        }
    }

    void ConvertCopy(S24* __restrict dst, const S32* __restrict src, size_t count) noexcept
    {
#ifdef __AVX2__
        for (size_t i = 0; i < count / 16; i++)
        {
            auto x0 = xmm::load_u<xmm::vu8x16>(src + 0);
            auto x1 = xmm::load_u<xmm::vu8x16>(src + 4);
            auto x2 = xmm::load_u<xmm::vu8x16>(src + 8);
            auto x3 = xmm::load_u<xmm::vu8x16>(src + 12);

            auto t0 = xmm::byte_shuffle_128(xmm::reinterpret<xmm::vu8x16>(x0), xmm::i8x16(1, 2, 3, 5, 1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15));
            auto t1 = xmm::byte_shuffle_128(xmm::reinterpret<xmm::vu8x16>(x1), xmm::i8x16(1, 2, 3, 5, 1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15));
            auto t2 = xmm::byte_shuffle_128(xmm::reinterpret<xmm::vu8x16>(x2), xmm::i8x16(1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15, 11, 13, 14, 15));
            auto t3 = xmm::byte_shuffle_128(xmm::reinterpret<xmm::vu8x16>(x3), xmm::i8x16(1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15, 11, 13, 14, 15));

            auto y0 = xmm::byte_align_r_128<4>(t0, t1);
            auto y1 = xmm::byte_align_r_128<8>(t1, t2);
            auto y2 = xmm::byte_align_r_128<12>(t2, t3);

            xmm::store_u<xmm::vu8x16>(reinterpret_cast<std::byte*>(dst) + 0, y0);
            xmm::store_u<xmm::vu8x16>(reinterpret_cast<std::byte*>(dst) + 16, y1);
            xmm::store_u<xmm::vu8x16>(reinterpret_cast<std::byte*>(dst) + 32, y2);

            src += 16;
            dst += 16;
        }
        count %= 16;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] = S24(src[i] >> 8);
        }
    }

    void ConvertCopy(S24* __restrict dst, const F32* __restrict src, size_t count) noexcept
    {
        const float scale = 0x1p23f; // = 1 << 23
        const float maxi = std::nextafter(+scale, -1.0f);
        const float mini = std::nextafter(-scale, +1.0f);

#ifdef __AVX2__
        const auto scalev = xmm::f32x4(0x1p31f);
        const auto maxiv = xmm::f32x4(std::nextafter(+0x1p31f, -1.0f));
        const auto miniv = xmm::f32x4(std::nextafter(-0x1p31f, +1.0f));

        for (size_t i = 0; i < count / 16; i++)
        {
            auto x0 = xmm::load_u<xmm::vf32x4>(src + 0);
            auto x1 = xmm::load_u<xmm::vf32x4>(src + 4);
            auto x2 = xmm::load_u<xmm::vf32x4>(src + 8);
            auto x3 = xmm::load_u<xmm::vf32x4>(src + 12);

            auto t0 = xmm::clamp(x0 * scalev, miniv, maxiv);
            auto t1 = xmm::clamp(x1 * scalev, miniv, maxiv);
            auto t2 = xmm::clamp(x2 * scalev, miniv, maxiv);
            auto t3 = xmm::clamp(x3 * scalev, miniv, maxiv);

            auto u0 = xmm::convert_cast<xmm::vi32x4>(t0);
            auto u1 = xmm::convert_cast<xmm::vi32x4>(t1);
            auto u2 = xmm::convert_cast<xmm::vi32x4>(t2);
            auto u3 = xmm::convert_cast<xmm::vi32x4>(t3);

            auto w0 = xmm::byte_shuffle_128(xmm::reinterpret<xmm::vu8x16>(u0), xmm::i8x16(1, 2, 3, 5, 1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15));
            auto w1 = xmm::byte_shuffle_128(xmm::reinterpret<xmm::vu8x16>(u1), xmm::i8x16(1, 2, 3, 5, 1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15));
            auto w2 = xmm::byte_shuffle_128(xmm::reinterpret<xmm::vu8x16>(u2), xmm::i8x16(1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15, 11, 13, 14, 15));
            auto w3 = xmm::byte_shuffle_128(xmm::reinterpret<xmm::vu8x16>(u3), xmm::i8x16(1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15, 11, 13, 14, 15));

            auto y0 = xmm::byte_align_r_128<4>(w0, w1);
            auto y1 = xmm::byte_align_r_128<8>(w1, w2);
            auto y2 = xmm::byte_align_r_128<12>(w2, w3);

            xmm::store_u<xmm::vu8x16>(reinterpret_cast<std::byte*>(dst) + 0, y0);
            xmm::store_u<xmm::vu8x16>(reinterpret_cast<std::byte*>(dst) + 16, y1);
            xmm::store_u<xmm::vu8x16>(reinterpret_cast<std::byte*>(dst) + 32, y2);

            src += 16;
            dst += 16;
        }
        count %= 16;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] = S24(static_cast<S32>(std::clamp<float>(src[i] * scale, mini, maxi)));
        }
    }

    void ConvertCopy(S32* __restrict dst, const S16* __restrict src, size_t count) noexcept
    {
#ifdef __AVX2__
        const auto zerov = xmm::zero<xmm::vi16x8>();
        for (size_t i = 0; i < count / 8; i++)
        {
            auto x = xmm::load_u<xmm::vi16x8>(src);
            auto y0 = xmm::reinterpret<xmm::vi32x4>(xmm::unpack_lo(zerov, x));
            auto y1 = xmm::reinterpret<xmm::vi32x4>(xmm::unpack_hi(zerov, x));
            xmm::store_u<xmm::vi32x4>(dst + 0, y0);
            xmm::store_u<xmm::vi32x4>(dst + 4, y1);
            src += 8;
            dst += 8;
        }
        count %= 8;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] = static_cast<S32>(src[i]) << 16;
        }
    }

    void ConvertCopy(S32* __restrict dst, const S24* __restrict src, size_t count) noexcept
    {
#ifdef __AVX2__
        for (size_t i = 0; i < count / 16; i++)
        {
            auto x0 = xmm::load_u<xmm::vu8x16>(reinterpret_cast<const std::byte*>(src) + 0);
            auto x1 = xmm::load_u<xmm::vu8x16>(reinterpret_cast<const std::byte*>(src) + 16);
            auto x2 = xmm::load_u<xmm::vu8x16>(reinterpret_cast<const std::byte*>(src) + 32);

            auto y0 = xmm::byte_shuffle_128(x0, xmm::i8x16(-1, 0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11));
            auto y1 = xmm::byte_shuffle_128(xmm::byte_align_r_128<12>(x0, x1), xmm::i8x16(-1, 0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11));
            auto y2 = xmm::byte_shuffle_128(xmm::byte_align_r_128<8>(x1, x2), xmm::i8x16(-1, 0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11));
            auto y3 = xmm::byte_shuffle_128(xmm::byte_align_r_128<4>(x2, x2), xmm::i8x16(-1, 0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11));

            xmm::store_u<xmm::vu8x16>(dst + 0, y0);
            xmm::store_u<xmm::vu8x16>(dst + 4, y1);
            xmm::store_u<xmm::vu8x16>(dst + 8, y2);
            xmm::store_u<xmm::vu8x16>(dst + 12, y3);

            src += 16;
            dst += 16;
        }
        count %= 16;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] = static_cast<S32>(src[i]) << 8;
        }
    }

    void ConvertCopy(S32* __restrict dst, const F32* __restrict src, size_t count) noexcept
    {
        const float scale = 0x1p31f; // = 1 << 31
        const float maxi = std::nextafter(+scale, -1.0f);
        const float mini = std::nextafter(-scale, +1.0f);

#ifdef __AVX2__
        const auto scalev = xmm::f32x4(scale);
        const auto maxiv = xmm::f32x4(maxi);
        const auto miniv = xmm::f32x4(mini);
        for (size_t i = 0; i < count / 4; i++)
        {
            auto x = xmm::load_u<xmm::vf32x4>(src);
            auto w = xmm::clamp(x * scalev, miniv, maxiv);
            auto y = xmm::convert_cast<xmm::vi32x4>(w);
            xmm::store_u<xmm::vi32x4>(dst, y);
            src += 4;
            dst += 4;
        }
        count %= 4;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] = static_cast<S32>(std::clamp<float>(src[i] * scale, mini, maxi));
        }
    }

    void ConvertCopy(F32* __restrict dst, const S16* __restrict src, size_t count) noexcept
    {
        const float scale = 0x1p-15f; // = 1 / (1 << 15)
#ifdef __AVX2__
        const auto scalev = xmm::f32x4(scale);
        for (size_t i = 0; i < count / 8; i++)
        {
            auto x = xmm::load_u<xmm::vi16x8>(src);
            auto w0 = xmm::convert_cast<xmm::vi32x4>(x);
            auto w1 = xmm::convert_cast<xmm::vi32x4>(xmm::byte_shift_r_128<8>(x));
            auto y0 = xmm::convert_cast<xmm::vf32x4>(w0);
            auto y1 = xmm::convert_cast<xmm::vf32x4>(w1);
            y0 *= scalev;
            y1 *= scalev;
            xmm::store_u<xmm::vf32x4>(dst + 0, y0);
            xmm::store_u<xmm::vf32x4>(dst + 4, y1);
            src += 8;
            dst += 8;
        }
        count %= 8;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] = static_cast<float>(src[i]) * scale;
        }
    }

    void ConvertCopy(F32* __restrict dst, const S24* __restrict src, size_t count) noexcept
    {
        const float scale = 0x1p-23f; // = 1 / (1 << 23)

#ifdef __AVX2__
        for (size_t i = 0; i < count / 16; i++)
        {
            const auto scalev = xmm::f32x4(0x1p-31f);
            auto x0 = xmm::load_u<xmm::vu8x16>(reinterpret_cast<const std::byte*>(src) + 0);
            auto x1 = xmm::load_u<xmm::vu8x16>(reinterpret_cast<const std::byte*>(src) + 16);
            auto x2 = xmm::load_u<xmm::vu8x16>(reinterpret_cast<const std::byte*>(src) + 32);

            auto t0 = xmm::byte_shuffle_128(x0, xmm::i8x16(-1, 0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11));
            auto t1 = xmm::byte_shuffle_128(xmm::byte_align_r_128<12>(x0, x1), xmm::i8x16(-1, 0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11));
            auto t2 = xmm::byte_shuffle_128(xmm::byte_align_r_128<8>(x1, x2), xmm::i8x16(-1, 0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11));
            auto t3 = xmm::byte_shuffle_128(xmm::byte_align_r_128<4>(x2, x2), xmm::i8x16(-1, 0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11));

            auto y0 = xmm::convert_cast<xmm::vf32x4>(xmm::reinterpret<xmm::vi32x4>(t0)) * scalev;
            auto y1 = xmm::convert_cast<xmm::vf32x4>(xmm::reinterpret<xmm::vi32x4>(t1)) * scalev;
            auto y2 = xmm::convert_cast<xmm::vf32x4>(xmm::reinterpret<xmm::vi32x4>(t2)) * scalev;
            auto y3 = xmm::convert_cast<xmm::vf32x4>(xmm::reinterpret<xmm::vi32x4>(t3)) * scalev;

            xmm::store_u<xmm::vf32x4>(dst + 0, y0);
            xmm::store_u<xmm::vf32x4>(dst + 4, y1);
            xmm::store_u<xmm::vf32x4>(dst + 8, y2);
            xmm::store_u<xmm::vf32x4>(dst + 12, y3);

            src += 16;
            dst += 16;
        }
        count %= 16;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] = static_cast<float>(static_cast<S32>(src[i])) * scale;
        }
    }

    void ConvertCopy(F32* __restrict dst, const S32* __restrict src, size_t count) noexcept
    {
        const float scale = 0x1p-31f; // = 1 / (1 << 31)

#ifdef __AVX2__
        const auto scalev = xmm::f32x4(scale);
        for (size_t i = 0; i < count / 4; i++)
        {
            auto x = xmm::load_u<xmm::vi32x4>(src);
            auto t = xmm::convert_cast<xmm::vf32x4>(x);
            auto y = t * scalev;
            xmm::store_u<xmm::vf32x4>(dst, y);
            src += 4;
            dst += 4;
        }
        count %= 4;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] = static_cast<float>(src[i]) * scale;
        }
    }

    void Mix(F32* __restrict dst, const F32* __restrict src, size_t count, float mix) noexcept
    {
#ifdef __AVX2__
        const auto scalev = xmm::f32x8(mix);
        for (size_t i = 0; i < count / 8; i++)
        {
            auto s = xmm::load_u<xmm::vf32x8>(src);
            auto d = xmm::load_u<xmm::vf32x8>(dst);
            d = d + s * scalev;
            xmm::store_u<xmm::vf32x8>(dst, d);
            src += 8;
            dst += 8;
        }
        count %= 8;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] += src[i] * mix;
        }
    }

    void MixStereo(F32Stereo* __restrict dst, const F32Stereo* __restrict src, size_t count, float lch_mix, float rch_mix) noexcept
    {
#ifdef __AVX2__
        const auto scalev = xmm::f32x8(lch_mix, rch_mix, lch_mix, rch_mix, lch_mix, rch_mix, lch_mix, rch_mix);
        for (size_t i = 0; i < count / 4; i++)
        {
            auto s = xmm::load_u<xmm::vf32x8>(src);
            auto d = xmm::load_u<xmm::vf32x8>(dst);
            d = d + s * scalev;
            xmm::store_u<xmm::vf32x8>(dst, d);
            src += 4;
            dst += 4;
        }
        count %= 4;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i].l += src[i].l * lch_mix;
            dst[i].r += src[i].r * rch_mix;
        }
    }

    void ProcessHardLimit(F32* dst, const F32* src, size_t count, float multiplier, float limit) noexcept
    {
#ifdef __AVX2__
        const auto scalev = xmm::f32x8(multiplier);
        const auto maxv = xmm::f32x8(+limit);
        const auto minv = xmm::f32x8(-limit);
        for (size_t i = 0; i < count / 8; i++)
        {
            xmm::store_u<xmm::vf32x8>(dst, xmm::clamp(xmm::load_u<xmm::vf32x8>(src) * scalev, minv, maxv));
            src += 8;
            dst += 8;
        }
        count %= 8;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i] = std::clamp<float>(src[i] * multiplier, -limit, limit);
        }
    }
}
