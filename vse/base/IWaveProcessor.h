/// @file
/// @brief  Vse - Wave Processor Interface
/// @author (C) 2022 ttsuki

#pragma once

#include <cstddef>
#include <type_traits>

#include "Interface.h"
#include "WaveFormat.h"

namespace vse
{
    /// Represents a WaveProcessor
    class IWaveProcessor : protected virtual Interface
    {
    public:
        /// Gets Processor Input format
        [[nodiscard]] virtual PcmWaveFormat GetInputFormat() const = 0;

        /// Gets Processor Output format
        [[nodiscard]] virtual PcmWaveFormat GetOutputFormat() const = 0;

        /// Process wave data
        /// @param read_source Source reader callback
        /// @param context Source reader callback context
        /// @param destination_buffer Destination buffer
        /// @param destination_buffer_length Destination buffer length
        /// @returns Processed and written data length in bytes.
        [[nodiscard]] virtual size_t Process(
            size_t (*read_source)(void* context, void* buffer, size_t buffer_length), void* context,
            void* destination_buffer, size_t destination_buffer_length) = 0;

        /// Process wave data
        /// @param read_source_function Source reader callback
        /// @param destination_buffer Destination buffer
        /// @param destination_buffer_length Destination buffer length
        /// @returns Processed and written data length in bytes.
        template <class F, std::enable_if_t<std::is_invocable_r_v<size_t, F, void*, size_t>>* = nullptr>
        [[nodiscard]] inline size_t Process(F&& read_source_function, void* destination_buffer, size_t destination_buffer_length)
        {
            return this->Process(
                [](void* ctx, void* buf, size_t len) { return std::invoke(*static_cast<std::remove_reference_t<F*>>(ctx), buf, len); },
                std::addressof(read_source_function), destination_buffer, destination_buffer_length);
        }
    };

    /// Represents a Parameter store
    template <class ParameterType>
    class IParameterStore : protected virtual Interface
    {
    public:
        /// Get All parameters
        [[nodiscard]] virtual ParameterType GetParameters() const = 0;

        /// Set All parameters
        virtual void SetParameters(ParameterType parameters) = 0;
    };

    template <class ParameterType>
    class IParametricWaveProcessor
        : public IWaveProcessor
        , public IParameterStore<ParameterType>
    {
    public:
        using IWaveProcessor::GetInputFormat;
        using IWaveProcessor::GetOutputFormat;
        using IWaveProcessor::Process;
        using IParameterStore<ParameterType>::GetParameters;
        using IParameterStore<ParameterType>::SetParameters;
    };
}
