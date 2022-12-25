/// @file
/// @brief  Vse - DirectSoundFx DSP
/// @author (C) 2022 ttsuki

#pragma once

#include <Windows.h>
#include <mmreg.h>
#include <dsound.h>

#include "../base/WaveFormat.h"
#include "../base/IWaveProcessor.h"

namespace vse
{
    template <class TParameterType>
    struct IDsFxWaveProcessor : IWaveProcessor, IParameterStore<TParameterType> { };

    static const inline DSFXGargle DSFXGargleDefaultParameters = {20, DSFXGARGLE_WAVE_TRIANGLE};
    static const inline DSFXChorus DSFXChorusDefaultParameters = {50.0f, 10.0f, 25.0f, 1.1f,DSFXCHORUS_WAVE_SIN, 16.0f, DSFXCHORUS_PHASE_90};
    static const inline DSFXFlanger DSFXFlangerDefaultParameters = {50.0f, 100.0f, -50.0f, 0.25f, DSFXFLANGER_WAVE_SIN, 2.0f, DSFXFLANGER_PHASE_ZERO};
    static const inline DSFXEcho DSFXEchoDefaultParameters = {50.0f, 50.0f, 500.0f, 500.0f, 0};
    static const inline DSFXDistortion DSFXDistortionDefaultParameters = {-18.0f, 15.0f, 2400.0f, 2400.0f, 8000.0f};
    static const inline DSFXCompressor DSFXCompressorDefaultParameters = {0.0f, 10.0f, 200.0f, -20.0f, 3.0f, 4.0f};
    static const inline DSFXParamEq DSFXParamEqDefaultParameters = {8000.0f, 12.0f, 0.0f};
    static const inline DSFXI3DL2Reverb DSFXI3DL2ReverbDefaultParameters = {-1000, -0, 0.0f, 1.49f, 0.83f, -2602, 0.007f, 200, 0.011f, 100.0f, 100.0f, 5000.0f};
    static const inline DSFXWavesReverb DSFXWavesReverbDefaultParameters = {0.0f, 0.0f, 1000.0f, 0.001f};

    std::shared_ptr<IDsFxWaveProcessor<DSFXGargle>> CreateGargleEffector(PcmWaveFormat format, DSFXGargle parameters = DSFXGargleDefaultParameters);
    std::shared_ptr<IDsFxWaveProcessor<DSFXChorus>> CreateChorusEffector(PcmWaveFormat format, DSFXChorus parameters = DSFXChorusDefaultParameters);
    std::shared_ptr<IDsFxWaveProcessor<DSFXFlanger>> CreateFlangerEffector(PcmWaveFormat format, DSFXFlanger parameters = DSFXFlangerDefaultParameters);
    std::shared_ptr<IDsFxWaveProcessor<DSFXEcho>> CreateEchoEffector(PcmWaveFormat format, DSFXEcho parameters = DSFXEchoDefaultParameters);
    std::shared_ptr<IDsFxWaveProcessor<DSFXDistortion>> CreateDistortionEffector(PcmWaveFormat format, DSFXDistortion parameters = DSFXDistortionDefaultParameters);
    std::shared_ptr<IDsFxWaveProcessor<DSFXCompressor>> CreateCompressorEffector(PcmWaveFormat format, DSFXCompressor parameters = DSFXCompressorDefaultParameters);
    std::shared_ptr<IDsFxWaveProcessor<DSFXParamEq>> CreateParamEqEffector(PcmWaveFormat format, DSFXParamEq parameters = DSFXParamEqDefaultParameters);
    std::shared_ptr<IDsFxWaveProcessor<DSFXI3DL2Reverb>> CreateI3DL2ReverbEffector(PcmWaveFormat format, DSFXI3DL2Reverb parameters = DSFXI3DL2ReverbDefaultParameters);
    std::shared_ptr<IDsFxWaveProcessor<DSFXWavesReverb>> CreateWavesReverbEffector(PcmWaveFormat format, DSFXWavesReverb parameters = DSFXWavesReverbDefaultParameters);
}
