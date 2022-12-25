/// @file
/// @brief  Vse - Waveform processing functions (Implementation helper)
/// @author (C) 2022 ttsuki

#pragma once

#include "../base/CommonTypes.h"

namespace vse::processing
{
    template <class T>
    static inline void Copy(void* __restrict dst, const void* __restrict src, const size_t count) noexcept
    {
        const auto d = static_cast<T*>(dst);
        const auto s = static_cast<const T*>(src);
        for (size_t i = 0; i < count; i++) d[i] = s[i];
    }

    template <class T>
    static inline void GatherCopy(void* __restrict dst, const void* __restrict src, const int src_stride, const int src_index, const size_t count) noexcept
    {
        const auto d = static_cast<T*>(dst);
        const auto s = static_cast<const T*>(src) + src_index;
        for (size_t i = 0; i < count; i++) d[i] = s[i * src_stride];
    }

    template <class T>
    static inline void ScatterCopy(void* __restrict dst, const int dst_stride, const int dst_index, const void* __restrict src, const size_t count) noexcept
    {
        const auto d = static_cast<T*>(dst) + dst_index;
        const auto s = static_cast<const T*>(src);
        for (size_t i = 0; i < count; i++) d[i * dst_stride] = s[i];
    }

    static inline void ConvertCopy(S16* __restrict dst, const S16* __restrict src, size_t count) noexcept { return Copy<S16>(dst, src, count); }
    static inline void ConvertCopy(S32* __restrict dst, const S32* __restrict src, size_t count) noexcept { return Copy<S32>(dst, src, count); }
    static inline void ConvertCopy(F32* __restrict dst, const F32* __restrict src, size_t count) noexcept { return Copy<F32>(dst, src, count); }
    void ConvertCopy(S16* __restrict dst, const S32* __restrict src, size_t count) noexcept;
    void ConvertCopy(S16* __restrict dst, const F32* __restrict src, size_t count) noexcept;
    void ConvertCopy(S32* __restrict dst, const S16* __restrict src, size_t count) noexcept;
    void ConvertCopy(S32* __restrict dst, const F32* __restrict src, size_t count) noexcept;
    void ConvertCopy(F32* __restrict dst, const S16* __restrict src, size_t count) noexcept;
    void ConvertCopy(F32* __restrict dst, const S32* __restrict src, size_t count) noexcept;

    void ConvertCopy32bitTo24bit(void* __restrict dst_24bit_pcm, const S32* __restrict src, size_t count) noexcept;
}
