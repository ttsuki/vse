/// @file
/// @brief  winasio - TransferSingleChannelFromInterleavedSource
/// @author (C) 2022 ttsuki

#pragma once

#include <cstddef>
#include <cstdint>
#include <array>

#include "asio.h"

// sample transferring helper
namespace asio::util
{
    using std::int8_t;
    using std::int16_t;
    using std::int32_t;
    using std::int64_t;
    using std::uint8_t;
    using std::uint16_t;
    using std::uint32_t;
    using std::uint64_t;
    using float32_t = float;
    using float64_t = double;

    struct int24_t
    {
        std::uint8_t b0;
        std::uint8_t b1;
        std::int8_t b2;

        int24_t() = default;
        constexpr static int24_t from_int(int32_t s) noexcept { return int24_t{static_cast<std::uint8_t>(s), static_cast<std::uint8_t>(s >> 8), static_cast<std::int8_t>(s >> 16)}; }
        constexpr operator int32_t() const noexcept { return b2 << 16 | b1 << 8 | b0; }
        constexpr operator int64_t() const noexcept { return b2 << 16 | b1 << 8 | b0; }
    };

    struct uint24_t
    {
        std::uint8_t b0;
        std::uint8_t b1;
        std::uint8_t b2;

        uint24_t() = default;
        constexpr static uint24_t from_uint(uint32_t s) noexcept { return uint24_t{static_cast<std::uint8_t>(s), static_cast<std::uint8_t>(s >> 8), static_cast<std::uint8_t>(s >> 16)}; }
        constexpr operator uint32_t() const noexcept { return b2 << 16 | b1 << 8 | b0; }
        constexpr operator uint64_t() const noexcept { return b2 << 16 | b1 << 8 | b0; }
    };

    static_assert(sizeof(int24_t) == 3);                 // checks alignment
    static_assert(sizeof(uint24_t) == 3);                // checks alignment
    static_assert(sizeof(std::array<int24_t, 2>) == 6);  // checks alignment
    static_assert(sizeof(std::array<uint24_t, 2>) == 6); // checks alignment

    template <class To, class From>
    static inline __forceinline To bit_cast(From&& from)
    {
        To to;
        std::memcpy(&to, &from, sizeof(To));
        return to;
    }

    static inline __forceinline uint16_t byteswap16(uint16_t v) { return _byteswap_ushort(v); }
    static inline __forceinline uint24_t byteswap24(uint24_t v) { return uint24_t{v.b2, v.b1, v.b0}; }
    static inline __forceinline uint32_t byteswap32(uint32_t v) { return _byteswap_ulong(v); }
    static inline __forceinline uint64_t byteswap64(uint64_t v) { return _byteswap_uint64(v); }


    template <AsioSampleType Type> static auto ConvertSample(int32_t sample);
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int16MSB>(int32_t sample) { return uint16_t{byteswap16(static_cast<uint16_t>(sample >> 16))}; }                         // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int24MSB>(int32_t sample) { return uint24_t{byteswap24(uint24_t::from_uint(sample >> 8))}; }                            // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int32MSB>(int32_t sample) { return uint32_t{byteswap32(static_cast<uint32_t>(sample))}; }                               // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Float32MSB>(int32_t sample) { return uint32_t{byteswap32(bit_cast<uint32_t>(static_cast<float>(sample) * 0x1p-31f))}; } // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Float64MSB>(int32_t sample) { return uint64_t{byteswap64(bit_cast<uint64_t>(static_cast<double>(sample) * 0x1p-31))}; } // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int32MSB16>(int32_t sample) { return uint32_t{byteswap32(static_cast<uint32_t>(sample >> 16))}; }                       // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int32MSB18>(int32_t sample) { return uint32_t{byteswap32(static_cast<uint32_t>(sample >> 14))}; }                       // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int32MSB20>(int32_t sample) { return uint32_t{byteswap32(static_cast<uint32_t>(sample >> 12))}; }                       // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int32MSB24>(int32_t sample) { return uint32_t{byteswap32(static_cast<uint32_t>(sample >> 8))}; }                        // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int16LSB>(int32_t sample) { return int16_t{static_cast<int16_t>(sample >> 16)}; }                                       // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int24LSB>(int32_t sample) { return int24_t::from_int(sample >> 8); }                                                    // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int32LSB>(int32_t sample) { return int32_t{sample}; }                                                                   // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Float32LSB>(int32_t sample) { return float32_t{static_cast<float32_t>(sample) * 0x1p-31f}; }                            // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Float64LSB>(int32_t sample) { return float64_t{static_cast<float64_t>(sample) * 0x1p-31}; }                             // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int32LSB16>(int32_t sample) { return int32_t{sample >> 16}; }                                                           // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int32LSB18>(int32_t sample) { return int32_t{sample >> 14}; }                                                           // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int32LSB20>(int32_t sample) { return int32_t{sample >> 12}; }                                                           // NOT TESTED
    template <> inline __forceinline auto ConvertSample<AsioSampleType::Int32LSB24>(int32_t sample) { return int32_t{sample >> 8}; }                                                            // NOT TESTED

    template <AsioSampleType destination_sample_type, size_t src_channel_count>
    static void TransferSingleChannelFromInterleavedSource(
        void* dst,
        const int32_t* src, int src_channel_index,
        size_t sample_count_to_copy)
    {
        auto src_base = src + src_channel_index;
        auto dst_base = static_cast<decltype(ConvertSample<destination_sample_type>(int32_t{}))*>(dst);
        for (size_t i = 0; i < sample_count_to_copy; ++i)
        {
            dst_base[i] = ConvertSample<destination_sample_type>(src_base[i * src_channel_count]);
        }
    }

    template <AsioSampleType destination_sample_type>
    static void TransferSingleChannelFromInterleavedSource(
        void* dst,
        const int32_t* src, int src_channel_count, int src_channel_index,
        size_t sample_count_to_copy)
    {
        if (src_channel_count == 1) return TransferSingleChannelFromInterleavedSource<destination_sample_type, 1>(dst, src, src_channel_index, sample_count_to_copy);
        if (src_channel_count == 2) return TransferSingleChannelFromInterleavedSource<destination_sample_type, 2>(dst, src, src_channel_index, sample_count_to_copy);
        if (src_channel_count == 3) return TransferSingleChannelFromInterleavedSource<destination_sample_type, 3>(dst, src, src_channel_index, sample_count_to_copy);
        if (src_channel_count == 4) return TransferSingleChannelFromInterleavedSource<destination_sample_type, 4>(dst, src, src_channel_index, sample_count_to_copy);
        if (src_channel_count == 5) return TransferSingleChannelFromInterleavedSource<destination_sample_type, 5>(dst, src, src_channel_index, sample_count_to_copy);
        if (src_channel_count == 6) return TransferSingleChannelFromInterleavedSource<destination_sample_type, 6>(dst, src, src_channel_index, sample_count_to_copy);
        if (src_channel_count == 7) return TransferSingleChannelFromInterleavedSource<destination_sample_type, 7>(dst, src, src_channel_index, sample_count_to_copy);
        if (src_channel_count == 8) return TransferSingleChannelFromInterleavedSource<destination_sample_type, 8>(dst, src, src_channel_index, sample_count_to_copy);

        auto src_base = src + src_channel_index;
        auto dst_base = static_cast<decltype(ConvertSample<destination_sample_type>(int32_t{}))*>(dst);
        for (size_t d = 0, s = 0; d < sample_count_to_copy; ++d, s += src_channel_count)
        {
            dst_base[d] = ConvertSample<destination_sample_type>(src_base[s]);
        }
    }

    static void TransferSingleChannelFromInterleavedSource(
        void* dst, AsioSampleType dst_sample_type,
        const int32_t* src, int src_channel_count, int src_channel_index,
        size_t sample_count_to_copy)
    {
        // dispatch dynamic type to template function
        switch (dst_sample_type)
        {
        case AsioSampleType::Int16LSB: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int16LSB>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Int24LSB: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int24LSB>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Int32LSB: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int32LSB>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Float32LSB: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Float32LSB>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Float64LSB: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Float64LSB>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Int32LSB16: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int32LSB16>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Int32LSB18: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int32LSB18>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Int32LSB20: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int32LSB20>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Int32LSB24: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int32LSB24>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Int16MSB: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int16MSB>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Int24MSB: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int24MSB>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Int32MSB: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int32MSB>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Float32MSB: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Float32MSB>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Float64MSB: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Float64MSB>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Int32MSB16: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int32MSB16>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Int32MSB18: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int32MSB18>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Int32MSB20: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int32MSB20>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        case AsioSampleType::Int32MSB24: return TransferSingleChannelFromInterleavedSource<AsioSampleType::Int32MSB24>(dst, src, src_channel_count, src_channel_index, sample_count_to_copy);
        }
#ifndef NDEBUG
        __debugbreak(); // not supported
#endif
    }
}
