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

    template <class T>
    struct Stereo
    {
        T l, r;
    };

    using S16Stereo = Stereo<S16>; ///< 16-bit signed integer stereo sample
    using S32Stereo = Stereo<S32>; ///< 32-bit signed integer stereo sample
    using F32Stereo = Stereo<F32>; ///< 32-bit floating-point stereo sample
}
