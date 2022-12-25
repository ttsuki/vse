/// @file
/// @brief  Vse - Byte Stream Interface
/// @author (C) 2022 ttsuki

#pragma once

#include <cstddef>
#include <ios>

#include "Interface.h"

namespace vse
{
    /// Represents a readonly byte stream.
    class IByteStream : protected virtual Interface
    {
    public:
        /// Reads bytes from the source to buffer.
        /// @param buffer Destination buffer.
        /// @param buffer_length Destination buffer length in bytes.
        /// @returns The number of bytes read to the buffer.
        [[nodiscard]] virtual size_t Read(void* buffer, size_t buffer_length) = 0;
    };

    /// Represents a readonly byte stream.
    class ISeekableByteStream : public IByteStream
    {
    public:
        /// Reads bytes from the source to buffer.
        /// @param buffer Destination buffer.
        /// @param buffer_length Destination buffer length in bytes.
        /// @returns The number of bytes read to the buffer.
        [[nodiscard]] virtual size_t Read(void* buffer, size_t buffer_length) override = 0;

        /// Gets the total sample count in bytes.
        [[nodiscard]] virtual size_t Size() const = 0;

        /// Gets the current cursor position in bytes.
        [[nodiscard]] virtual size_t Tell() const = 0;

        /// Sets the current cursor position in bytes.
        [[nodiscard]] virtual size_t Seek(size_t new_position) { return Seek(new_position, std::ios::beg); }

        /// Sets the current cursor position in bytes.
        [[nodiscard]] virtual size_t Seek(ptrdiff_t new_position, int whence) = 0;
    };
}
