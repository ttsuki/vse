/// @file
/// @brief  Vse - Wave Format
/// @author (C) 2022 ttsuki

#include "WaveFormat.h"

#include <Windows.h>
#include <mmreg.h>

#include <cstddef>
#include <string>
#include <stdexcept>

namespace vse
{
    static constexpr GUID MEDIA_AUDIO_SUBTYPE(DWORD tag) { return {tag, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}; }
    static constexpr GUID MEDIASUBTYPE_PCM = MEDIA_AUDIO_SUBTYPE(WAVE_FORMAT_PCM);
    static constexpr GUID MEDIASUBTYPE_IEEE_FLOAT = MEDIA_AUDIO_SUBTYPE(WAVE_FORMAT_IEEE_FLOAT);

    PcmWaveFormat PcmWaveFormat::Parse(const WAVEFORMATEX* wf) noexcept
    {
        if (wf->wFormatTag == WAVE_FORMAT_EXTENSIBLE && wf->cbSize >= sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))
        {
            auto wfx = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(wf);
            if (wfx->SubFormat == MEDIASUBTYPE_PCM && wfx->Format.wBitsPerSample == 16) return {SampleType::S16, static_cast<SpeakerBit>(wfx->dwChannelMask), static_cast<int>(wfx->Format.nSamplesPerSec)};
            if (wfx->SubFormat == MEDIASUBTYPE_PCM && wfx->Format.wBitsPerSample == 24) return {SampleType::S24, static_cast<SpeakerBit>(wfx->dwChannelMask), static_cast<int>(wfx->Format.nSamplesPerSec)};
            if (wfx->SubFormat == MEDIASUBTYPE_PCM && wfx->Format.wBitsPerSample == 32) return {SampleType::S32, static_cast<SpeakerBit>(wfx->dwChannelMask), static_cast<int>(wfx->Format.nSamplesPerSec)};
            if (wfx->SubFormat == MEDIASUBTYPE_IEEE_FLOAT && wfx->Format.wBitsPerSample == 32) return {SampleType::F32, static_cast<SpeakerBit>(wfx->dwChannelMask), static_cast<int>(wfx->Format.nSamplesPerSec)};
            return {SampleType::Unknown, static_cast<SpeakerBit>(wfx->dwChannelMask), static_cast<int>(wfx->Format.nSamplesPerSec)}; // not supported.
        }

        if ((wf->wFormatTag == WAVE_FORMAT_PCM || wf->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) && DefaultChannelMask(wf->nChannels) != SpeakerBit::None)
        {
            if (wf->wFormatTag == WAVE_FORMAT_PCM && wf->wBitsPerSample == 16) return {SampleType::S16, DefaultChannelMask(wf->nChannels), static_cast<int>(wf->nSamplesPerSec)};
            if (wf->wFormatTag == WAVE_FORMAT_PCM && wf->wBitsPerSample == 24) return {SampleType::S24, DefaultChannelMask(wf->nChannels), static_cast<int>(wf->nSamplesPerSec)};
            if (wf->wFormatTag == WAVE_FORMAT_PCM && wf->wBitsPerSample == 32) return {SampleType::S32, DefaultChannelMask(wf->nChannels), static_cast<int>(wf->nSamplesPerSec)};
            if (wf->wFormatTag == WAVE_FORMAT_IEEE_FLOAT && wf->wBitsPerSample == 32) return {SampleType::F32, DefaultChannelMask(wf->nChannels), static_cast<int>(wf->nSamplesPerSec)};
        }

        return {SampleType::Unknown, DefaultChannelMask(wf->nChannels), static_cast<int>(wf->nSamplesPerSec)}; // not supported.
    }

    WAVEFORMATEX PcmWaveFormat::ToWaveFormatEx() const noexcept
    {
        WAVEFORMATEX wfx{};
        wfx.wFormatTag = SampleType() != SampleType::F32 ? WAVE_FORMAT_PCM : WAVE_FORMAT_IEEE_FLOAT;
        wfx.nChannels = static_cast<WORD>(ChannelCount());
        wfx.nSamplesPerSec = static_cast<DWORD>(SamplingFrequency());
        wfx.nAvgBytesPerSec = static_cast<DWORD>(AvgBytesPerSec());
        wfx.nBlockAlign = static_cast<WORD>(BlockAlign());
        wfx.wBitsPerSample = static_cast<WORD>(BitsPerSample());
        wfx.cbSize = 0;
        return wfx;
    }

    WAVEFORMATEXTENSIBLE PcmWaveFormat::ToWaveFormatExtensible(int valid_bits_per_sample) const
    {
        WAVEFORMATEXTENSIBLE wfx{};
        wfx.Format = ToWaveFormatEx();
        wfx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        wfx.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
        wfx.Samples.wValidBitsPerSample = static_cast<WORD>(valid_bits_per_sample);
        wfx.dwChannelMask = +ChannelMask();
        wfx.SubFormat = SampleType() != SampleType::F32 ? MEDIASUBTYPE_PCM : MEDIASUBTYPE_IEEE_FLOAT;

        if (wfx.Samples.wValidBitsPerSample > wfx.Format.wBitsPerSample)
            throw std::invalid_argument("wfx.Samples.wValidBitsPerSample > wfx.Format.wBitsPerSample");

        return wfx;
    }

    std::string PcmWaveFormat::ToString() const
    {
        return vse::ToString(static_cast<WAVEFORMATEXTENSIBLE>(*this));
    }

    bool Equals(const WAVEFORMATEX* lhs, const WAVEFORMATEX* rhs) noexcept
    {
        if (lhs->wFormatTag != rhs->wFormatTag) return false;
        if (lhs->nChannels != rhs->nChannels) return false;
        if (lhs->nSamplesPerSec != rhs->nSamplesPerSec) return false;
        if (lhs->nBlockAlign != rhs->nBlockAlign) return false;
        if (lhs->wBitsPerSample != rhs->wBitsPerSample) return false;

        if (lhs->wFormatTag != WAVE_FORMAT_PCM && lhs->wFormatTag != WAVE_FORMAT_IEEE_FLOAT)
        {
            if (lhs->cbSize != rhs->cbSize) return false;
            if (memcmp((&lhs) + 1, (&rhs) + 1, lhs->cbSize) != 0) return false;
        }

        return true;
    }

    std::string ToString(const WAVEFORMATEX* f)
    {
        std::string s;
        s.reserve(256);

        int validBits = f->wBitsPerSample;

        if (f->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
        {
            if (f->cbSize >= sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))
            {
                auto& tag = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(f)->SubFormat;
                if (tag == MEDIA_AUDIO_SUBTYPE(WAVE_FORMAT_PCM)) s += "PCM ";
                else if (tag == MEDIA_AUDIO_SUBTYPE(WAVE_FORMAT_IEEE_FLOAT)) s += "IEEE_FLOAT ";
                else s += "UNKNOWN FORMAT ";
                if (validBits) { validBits = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(f)->Samples.wValidBitsPerSample; }
            }
            else
            {
                s += "CORRUPTED WAVE_FORMAT_EXTENSIBLE ";
            }
        }
        else
        {
            auto& tag = f->wFormatTag;
            if (tag == WAVE_FORMAT_PCM) s += "PCM ";
            else if (tag == WAVE_FORMAT_IEEE_FLOAT) s += "IEEE_FLOAT ";
            else s += "UNKNOWN FORMAT ";
        }

        (s += std::to_string(f->nSamplesPerSec)) += "hz ";
        (s += std::to_string(validBits)) += "/";
        (s += std::to_string(f->wBitsPerSample)) += "bit ";
        (s += std::to_string(f->nChannels)) += "ch ";
        (s += std::to_string(f->nAvgBytesPerSec * 8 / 1000)) += "kbps";
        return s;
    }
}
