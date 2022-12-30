/// @file
/// @brief  Vse - Wave Source Interface
/// @author (C) 2022 ttsuki

#pragma once

#include "Interface.h"
#include "WaveFormat.h"

namespace vse
{
    /// Represents a readonly wave source.
    class IWaveSource : protected virtual Interface
    {
    public:
        /// Gets the source format.
        [[nodiscard]] virtual PcmWaveFormat GetFormat() const = 0;

        /// Reads samples from the source to buffer.
        /// @param buffer Destination buffer.
        /// @param buffer_length Destination buffer length in bytes.
        /// @returns The number of bytes read to the buffer.
        [[nodiscard]] virtual size_t Read(void* buffer, size_t buffer_length) = 0;
    };

    /// Represents a seekable readonly wave source
    class ISeekableWaveSource : public virtual IWaveSource
    {
    public:
        /// Gets the source format.
        [[nodiscard]] virtual PcmWaveFormat GetFormat() const override = 0;

        /// Reads samples from the source to buffer.
        /// @param buffer Destination buffer.
        /// @param buffer_length Destination buffer length in bytes.
        /// @returns The number of bytes read to the buffer.
        [[nodiscard]] virtual size_t Read(void* buffer, size_t buffer_length) override = 0;

        /// Gets the total sample count in samples.
        [[nodiscard]] virtual size_t GetTotalSampleCount() const = 0;

        /// Gets the current cursor position in samples.
        [[nodiscard]] virtual size_t GetSampleCursor() const = 0;

        /// Sets the current cursor position in samples.
        virtual size_t SetSampleCursor(size_t new_position) = 0;
    };

    /// Represents a random accessible wave buffer
    class IRandomAccessWaveBuffer : public virtual Interface
    {
    public:
        /// Gets the source format.
        [[nodiscard]] virtual PcmWaveFormat GetFormat() const = 0;

        /// Reads samples from the buffer.
        /// @param buffer Destination buffer.
        /// @param cursor Source position in bytes.
        /// @param length Length in bytes.
        /// @returns The number of bytes read to the buffer.
        [[nodiscard]] virtual size_t Read(void* buffer, size_t cursor, size_t length) const noexcept = 0;

        /// Writes samples to the buffer.
        /// @param buffer Source buffer.
        /// @param cursor Destination position in bytes.
        /// @param length Length in bytes.
        /// @returns The number of bytes read to the buffer.
        [[nodiscard]] virtual size_t Write(const void* buffer, size_t cursor, size_t length) noexcept = 0;

        /// Gets the total sample count in bytes.
        [[nodiscard]] virtual size_t Size() const = 0;

        /// Gets the total sample count in bytes.
        [[nodiscard]] virtual size_t Resize(size_t length) = 0;

        /// Gets the total sample count in samples.
        [[nodiscard]] virtual size_t GetTotalSampleCount() const { return Size() / GetFormat().BlockAlign(); }
    };
}
