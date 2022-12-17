/// @file
/// @brief  Interface
/// @author (C) 2022 ttsuki

#pragma once

namespace vse
{
    /// Interface.
    class Interface
    {
    public:
        Interface() = default;
        Interface(const Interface& other) = delete;
        Interface(Interface&& other) noexcept = delete;
        Interface& operator=(const Interface& other) = delete;
        Interface& operator=(Interface&& other) noexcept = delete;
        virtual ~Interface() = default;
    };
}
