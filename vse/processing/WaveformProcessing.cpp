/// @file
/// @brief  Vse - Waveform processing functions (Implementation helper)
/// @author (C) 2022 ttsuki

#include "WaveformProcessing.h"

#include <cmath>
#include <algorithm>

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

    void ConvertCopy32bitTo24bit(void* __restrict dst_24bit, const S32* __restrict src, size_t count) noexcept
    {
        auto* dst = static_cast<uint8_t*>(dst_24bit);

#ifdef __AVX2__
        for (size_t i = 0; i < count / 16; i++)
        {
            auto x0 = xmm::load_u<xmm::vu8x16>(src + 0);
            auto x1 = xmm::load_u<xmm::vu8x16>(src + 4);
            auto x2 = xmm::load_u<xmm::vu8x16>(src + 8);
            auto x3 = xmm::load_u<xmm::vu8x16>(src + 12);

            x0 = xmm::byte_shuffle_128(xmm::reinterpret<xmm::vu8x16>(x0), xmm::i8x16(1, 2, 3, 5, 1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15));
            x1 = xmm::byte_shuffle_128(xmm::reinterpret<xmm::vu8x16>(x1), xmm::i8x16(1, 2, 3, 5, 1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15));
            x2 = xmm::byte_shuffle_128(xmm::reinterpret<xmm::vu8x16>(x2), xmm::i8x16(1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15, 11, 13, 14, 15));
            x3 = xmm::byte_shuffle_128(xmm::reinterpret<xmm::vu8x16>(x3), xmm::i8x16(1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15, 11, 13, 14, 15));

            auto y0 = xmm::byte_align_r_128<4>(x0, x1);
            auto y1 = xmm::byte_align_r_128<8>(x1, x2);
            auto y2 = xmm::byte_align_r_128<12>(x2, x3);

            xmm::store_u<xmm::vu8x16>(dst + 0, y0);
            xmm::store_u<xmm::vu8x16>(dst + 16, y1);
            xmm::store_u<xmm::vu8x16>(dst + 32, y2);

            src += 16;
            dst += 3 * 16;
        }
        count %= 16;
#endif

        for (size_t i = 0; i < count; i++)
        {
            dst[i * 3 + 0] = static_cast<uint8_t>(src[i] >> 8);
            dst[i * 3 + 1] = static_cast<uint8_t>(src[i] >> 16);
            dst[i * 3 + 2] = static_cast<uint8_t>(src[i] >> 24);
        }
    }
}
