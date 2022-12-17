/// @file
/// @brief  Vse - Output Device
/// @author (C) 2022 ttsuki

#pragma once

#include "../base/Interface.h"
#include "../base/WaveFormat.h"

namespace vse
{
    /// Represents a Output Device.
    class IOutputDevice : protected virtual Interface
    {
    public:
        /// Open the output device with default format.
        /// @returns true if device is opened successfully.
        [[nodiscard]] virtual bool Open() = 0;

        /// Gets the current output buffer format.
        /// @return the current output buffer format.
        [[nodiscard]] virtual WAVEFORMATEXTENSIBLE GetFormat() const = 0;

        /// Gets the next output buffer size
        /// @returns the next output buffer size.
        [[nodiscard]] virtual int GetBufferSize() const = 0;

        /// Starts the device.
        /// @returns true if the device is started successfully.
        [[nodiscard]] virtual bool Start() = 0;

        /// Locks next output buffer.
        /// @param buffer_Size pass the return value of GetBufferSize
        /// @returns buffer pointer
        [[nodiscard]] virtual void* LockBuffer(int buffer_Size) = 0;

        /// Unlocks the buffer and send data to device.
        /// @param wrote_bytes wrote bytes
        virtual void UnlockBuffer(int wrote_bytes) = 0;

        /// Reset the device.
        virtual void Reset() = 0;

        /// Close the device.
        virtual void Close() = 0;
    };
}
