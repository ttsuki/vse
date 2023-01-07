# 𝑉𝑆𝐸 - 𝑉𝑆𝐸 𝑆𝑜𝑢𝑛𝑑 𝐸𝑛𝑔𝑖𝑛𝑒 ⚡︎

𝑉𝑆𝐸 is a sound mixing/processing engine for Windows platform gaming.

## Features

- Wave FIle Loader [.h](vse/loader/WaveFileLoader.h)
  - Media foundation format (.wav, .mp3, .wma, .mp4, etc...) [.cpp](vse/loader/WaveSourceMediaFoundation.cpp)
  - Ogg vorbis (.ogg)  [.cpp](vse/loader/WaveSourceOggVorbis.cpp)
- OutputDevice
  - DirectSound [.h](vse/output/DirectSoundOutputDevice.h)
  - WASAPI shared/exclusive [.h](vse/output/WasapiOutputDevice.h)
  - ASIO (Audio Stream Input Output) [.h](vse/output/AsioOutputDevice.h)
- Wave Processing
  - Wave Format Converter [.h](vse/processing/WaveFormatConverter.h)
  - DirectSound Fx [.h](vse/processing/DirectSoundAudioEffectDsp.h)
  - Gain/HardLimit [.h](vse/processing/HardLimiter.h)
  - RBJ's Audio EQ Biqad filters [.h](vse/processing/RbjAudioEqProcessor.h)
- Voicing And Mixng
  - Simple Voice [.h](vse/pipeline/SimpleVoice.h)
  - Stereo Wave Mixer [.h](vse/pipeline/StereoWaveMixer.h)
  - Source Switcher [.h](vse/pipeline/SourceSwitcher.h)

## Sample Code
  - tests/SimpleAudioPlayback [.cpp](tests/SimpleAudioPlayback.cpp)
  - tests/SimpleAudioFilePlayback [.cpp](tests/SimpleAudioFilePlayback.cpp)
  - tests/SimpleAudioFilePlaybackAsio [.cpp](tests/SimpleAudioFilePlaybackAsio.cpp)
  - tests/VoiceMixingBeatBox [.cpp](tests/VoiceMixingBeatBox.cpp)
  - tests/SimpleBmsFilePlayback [.cpp](tests/SimpleBmsFilePlayback.cpp)

## Build environment
  - MSVC 2022
  - C++17
  - vcpkg (libogg, libvorbis)

## License

MIT License (C) 2022 ttsuki

