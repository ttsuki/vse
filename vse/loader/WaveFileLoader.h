/// @file
/// @brief  Vse - Wave File Loader
/// @author (C) 2022 ttsuki

#pragma once

#include "../base/IByteStream.h"
#include "../base/IWaveSource.h"
#include "../base/RandomAccessWaveBuffer.h"

#include <memory>
#include <utility>
#include <filesystem>

namespace vse
{
    [[nodiscard]] std::shared_ptr<ISeekableByteStream> OpenFile(const std::filesystem::path& path);
    [[nodiscard]] std::shared_ptr<ISeekableByteStream> OpenFile(std::shared_ptr<const void> file_image, size_t length);
    [[nodiscard]] std::shared_ptr<ISeekableByteStream> ReadOutToMemory(std::shared_ptr<ISeekableByteStream> stream);
    [[nodiscard]] inline std::shared_ptr<ISeekableByteStream> LoadFile(const std::filesystem::path& path) { return ReadOutToMemory(OpenFile(path)); }

    [[nodiscard]] std::shared_ptr<IWaveSource> CreateWaveSourceMediaFoundation(std::shared_ptr<ISeekableByteStream> file);
    [[nodiscard]] std::shared_ptr<IWaveSource> CreateWaveSourceOggVorbis(std::shared_ptr<ISeekableByteStream> file);
    [[nodiscard]] std::shared_ptr<IWaveSource> CreateWaveSourceForFile(std::shared_ptr<ISeekableByteStream> file);
    [[nodiscard]] std::shared_ptr<IWaveSource> ConvertWaveFormat(std::shared_ptr<IWaveSource> source, PcmWaveFormat desired_format);

    [[nodiscard]] inline std::shared_ptr<IWaveSource> OpenAudioFile(const std::filesystem::path& path) { return CreateWaveSourceForFile(OpenFile(path)); }
    [[nodiscard]] inline std::shared_ptr<IWaveSource> OpenAudioFile(const std::filesystem::path& path, PcmWaveFormat desired_format) { return ConvertWaveFormat(OpenAudioFile(path), desired_format); }
    [[nodiscard]] inline std::shared_ptr<IWaveSource> OpenAudioFile(std::shared_ptr<const void> file_image, size_t length) { return CreateWaveSourceForFile(OpenFile(std::move(file_image), length)); }
    [[nodiscard]] inline std::shared_ptr<IWaveSource> OpenAudioFile(std::shared_ptr<const void> file_image, size_t length, PcmWaveFormat desired_format) { return ConvertWaveFormat(OpenAudioFile(std::move(file_image), length), desired_format); }
    [[nodiscard]] inline std::shared_ptr<IWaveSource> OpenAudioFile(std::shared_ptr<ISeekableByteStream> file) { return CreateWaveSourceForFile(std::move(file)); }
    [[nodiscard]] inline std::shared_ptr<IWaveSource> OpenAudioFile(std::shared_ptr<ISeekableByteStream> file, PcmWaveFormat desired_format) { return ConvertWaveFormat(OpenAudioFile(std::move(file)), desired_format); }

    [[nodiscard]] inline std::shared_ptr<IRandomAccessWaveBuffer> LoadAudioFile(const std::filesystem::path& path) { return ReadOutToMemory(OpenAudioFile(path)); }
    [[nodiscard]] inline std::shared_ptr<IRandomAccessWaveBuffer> LoadAudioFile(const std::filesystem::path& path, PcmWaveFormat desired_format) { return ReadOutToMemory(OpenAudioFile(path, desired_format)); }
    [[nodiscard]] inline std::shared_ptr<IRandomAccessWaveBuffer> LoadAudioFile(std::shared_ptr<const void> file_image, size_t length) { return ReadOutToMemory(OpenAudioFile(std::move(file_image), length)); }
    [[nodiscard]] inline std::shared_ptr<IRandomAccessWaveBuffer> LoadAudioFile(std::shared_ptr<const void> file_image, size_t length, PcmWaveFormat desired_format) { return ReadOutToMemory(OpenAudioFile(std::move(file_image), length, desired_format)); }
    [[nodiscard]] inline std::shared_ptr<IRandomAccessWaveBuffer> LoadAudioFile(std::shared_ptr<ISeekableByteStream> file) { return ReadOutToMemory(OpenAudioFile(std::move(file))); }
    [[nodiscard]] inline std::shared_ptr<IRandomAccessWaveBuffer> LoadAudioFile(std::shared_ptr<ISeekableByteStream> file, PcmWaveFormat desired_format) { return ReadOutToMemory(OpenAudioFile(std::move(file), desired_format)); }

}
