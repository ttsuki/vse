/// @file
/// @brief  Vse - Wave File Loader
/// @author (C) 2022 ttsuki

#pragma once

#include "../base/IByteStream.h"
#include "../base/IWaveSource.h"

#include <memory>
#include <filesystem>

namespace vse
{
    [[nodiscard]] std::shared_ptr<ISeekableByteStream> OpenFile(const std::filesystem::path& path);
    [[nodiscard]] std::shared_ptr<ISeekableByteStream> OpenFile(std::shared_ptr<const void> memory, size_t length);
    [[nodiscard]] std::shared_ptr<ISeekableByteStream> ReadOutToMemory(std::shared_ptr<ISeekableByteStream> stream);

    [[nodiscard]] std::shared_ptr<IWaveSource> CreateWaveSourceMediaFoundation(std::shared_ptr<ISeekableByteStream> file);
    [[nodiscard]] std::shared_ptr<IWaveSource> CreateWaveSourceOggVorbis(std::shared_ptr<ISeekableByteStream> file);
    [[nodiscard]] std::shared_ptr<IWaveSource> CreateWaveSourceForFile(std::shared_ptr<ISeekableByteStream> file);
    [[nodiscard]] std::shared_ptr<IWaveSource> CreateWaveSourceForFile(std::shared_ptr<ISeekableByteStream> file, PcmWaveFormat desired_format);

    [[nodiscard]] std::shared_ptr<IRandomAccessWaveBuffer> AllocateWaveBuffer(PcmWaveFormat format);
    [[nodiscard]] std::shared_ptr<IRandomAccessWaveBuffer> ReadOutToMemory(std::shared_ptr<IWaveSource> input);
    [[nodiscard]] std::shared_ptr<IRandomAccessWaveBuffer> DuplicateBuffer(std::shared_ptr<IRandomAccessWaveBuffer> source);
    [[nodiscard]] std::shared_ptr<ISeekableWaveSource> AllocateReadCursor(std::shared_ptr<IRandomAccessWaveBuffer> buffer);

    [[nodiscard]] inline std::shared_ptr<ISeekableByteStream> LoadFile(const std::filesystem::path& path)
    {
        return ReadOutToMemory(OpenFile(path));
    }

    [[nodiscard]] inline std::shared_ptr<IRandomAccessWaveBuffer> LoadAudioFile(const std::filesystem::path& path)
    {
        return ReadOutToMemory(CreateWaveSourceForFile(OpenFile(path)));
    }

    [[nodiscard]] inline std::shared_ptr<IRandomAccessWaveBuffer> LoadAudioFile(const std::filesystem::path& path, PcmWaveFormat desired_format)
    {
        return ReadOutToMemory(CreateWaveSourceForFile(OpenFile(path), desired_format));
    }

    [[nodiscard]] inline std::shared_ptr<ISeekableWaveSource> LoadAudioFileAsWaveSource(const std::filesystem::path& path)
    {
        return AllocateReadCursor(LoadAudioFile(path));
    }

    [[nodiscard]] inline std::shared_ptr<ISeekableWaveSource> LoadAudioFileAsWaveSource(const std::filesystem::path& path, PcmWaveFormat desired_format)
    {
        return AllocateReadCursor(LoadAudioFile(path, desired_format));
    }
}
