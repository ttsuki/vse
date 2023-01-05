/// @file
/// @brief  Vse project-wide common types
/// @author (C) 2022 ttsuki

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace vse
{
    using std::size_t;
    using std::ptrdiff_t;
    using index_t = std::ptrdiff_t;

    template <int v> static constexpr std::integral_constant<int, v> const_int{};
    template <size_t v> static constexpr std::integral_constant<size_t, v> const_size{};
    template <index_t v> static constexpr std::integral_constant<index_t, v> const_index{};

    template <class T> using shared_ptr = std::shared_ptr<T>;

    using S16 = int16_t; ///< 16-bit signed integer sample
    using S32 = int32_t; ///< 32-bit signed integer sample
    using F32 = float;   ///< 32-bit floating-point sample

    struct S24
    {
        uint8_t b0;
        uint8_t b1;
        int8_t b2;

        S24() = default;
        constexpr explicit S24(S32 s) noexcept : b0{static_cast<uint8_t>(s)}, b1{static_cast<uint8_t>(s >> 8)}, b2{static_cast<int8_t>(s >> 16)} {}
        constexpr operator S32() const noexcept { return b2 << 16 | b1 << 8 | b0; }
    };

    template <class T>
    struct Stereo
    {
        T l, r;
    };

    using S16Stereo = Stereo<S16>; ///< 16-bit signed integer stereo sample
    using S24Stereo = Stereo<S24>; ///< 24-bit signed integer stereo sample
    using S32Stereo = Stereo<S32>; ///< 32-bit signed integer stereo sample
    using F32Stereo = Stereo<F32>; ///< 32-bit floating-point stereo sample

    static_assert(std::is_pod_v<S24>);         // type requirement check
    static_assert(sizeof(S24) == 3);           // type requirement check
    static_assert(sizeof(S24Stereo[2]) == 12); // type requirement check
    static_assert(alignof(S24Stereo[2]) == 1); // type requirement check
}
