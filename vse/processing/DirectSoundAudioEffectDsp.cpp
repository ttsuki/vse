/// @file
/// @brief  Vse - DirectSoundFx DSP
/// @author (C) 2022 ttsuki

#include "DirectSoundAudioEffectDsp.h"

#include <Windows.h>
#include <mediaobj.h>
#include <dsound.h>
#pragma comment(lib,"dsound.lib")
#pragma comment(lib,"dxguid.lib")

#include "DmoWaveProcessor.h"
#include "../base/win32/debug.h"
#include "../base/win32/com_ptr.h"
#include "../base/xtl/xtl_spin_lock_mutex.h"

interface DECLSPEC_UUID("d616f352-d622-11ce-aac5-0020af0b99a3") IDirectSoundFXGargle;
interface DECLSPEC_UUID("880842e3-145f-43e6-a934-a71806e50547") IDirectSoundFXChorus;
interface DECLSPEC_UUID("903e9878-2c92-4072-9b2c-ea68f5396783") IDirectSoundFXFlanger;
interface DECLSPEC_UUID("8bd28edf-50db-4e92-a2bd-445488d1ed42") IDirectSoundFXEcho;
interface DECLSPEC_UUID("8ecf4326-455f-4d8b-bda9-8d5d3e9e3e0b") IDirectSoundFXDistortion;
interface DECLSPEC_UUID("4bbd1154-62f6-4e2c-a15c-d3b6c417f7a0") IDirectSoundFXCompressor;
interface DECLSPEC_UUID("c03ca9fe-fe90-4204-8078-82334cd177da") IDirectSoundFXParamEq;
interface DECLSPEC_UUID("4b166a6a-0d66-43f3-80e3-ee6280dee1a4") IDirectSoundFXI3DL2Reverb;
interface DECLSPEC_UUID("46858c3a-0dc6-45e3-b760-d4eef16cb325") IDirectSoundFXWavesReverb;

namespace vse
{
    template <class IDirectSoundFx, class Parameters>
    static shared_ptr<IDsFxWaveProcessor<Parameters>> CreateFx(CLSID clsId, PcmWaveFormat format, Parameters parameters)
    {
        const WAVEFORMATEX wfx = format;
        win32::com_ptr<IMediaObject> mo{};
        if (HRESULT hr = VSE_EXPECT_SUCCESS CreateMediaObject(clsId, &wfx, &wfx, mo.put());
            FAILED(hr) || !mo)
            throw std::runtime_error("Failed to create dmo");

        win32::com_ptr<IDirectSoundFx> fx = mo;
        if (!fx) throw std::runtime_error("Failed to QueryInterface IDirectSoundFx");

        class Impl final
            : public DmoWaveProcessor
            , public IDsFxWaveProcessor<Parameters>
        {
            xtl::spin_lock_mutex mutex_;
            win32::com_ptr<IDirectSoundFx> fx_;

        public:
            Impl(IMediaObject* mo, IDirectSoundFx* fx, PcmWaveFormat format, Parameters initial_parameters)
                : DmoWaveProcessor(mo, format, format)
                , fx_(fx)
            {
                VSE_EXPECT_SUCCESS fx_->SetAllParameters(&initial_parameters);
            }

            [[nodiscard]] PcmWaveFormat GetInputFormat() const override { return DmoWaveProcessor::GetInputFormat(); }
            [[nodiscard]] PcmWaveFormat GetOutputFormat() const override { return DmoWaveProcessor::GetOutputFormat(); }

            [[nodiscard]] size_t Process(size_t (* read_source)(void* context, void* buffer, size_t buffer_length), void* context, void* destination_buffer, size_t destination_buffer_length) override
            {
                xtl::lock_guard lock(mutex_);
                return DmoWaveProcessor::Process(read_source, context, destination_buffer, destination_buffer_length);
            }

            [[nodiscard]] Parameters GetParameters() const override
            {
                Parameters parameters{};
                fx_->GetAllParameters(&parameters);
                return parameters;
            }

            void SetParameters(Parameters parameters) override
            {
                xtl::lock_guard lock(mutex_);
                VSE_EXPECT_SUCCESS fx_->SetAllParameters(&parameters);
            }
        };

        return std::make_shared<Impl>(mo.get(), fx.get(), format, parameters);
    }

    std::shared_ptr<IDsFxWaveProcessor<DSFXGargle>> CreateGargleEffector(PcmWaveFormat format, DSFXGargle parameters) { return CreateFx<IDirectSoundFXGargle, DSFXGargle>(GUID_DSFX_STANDARD_GARGLE, format, parameters); }
    std::shared_ptr<IDsFxWaveProcessor<DSFXChorus>> CreateChorusEffector(PcmWaveFormat format, DSFXChorus parameters) { return CreateFx<IDirectSoundFXChorus, DSFXChorus>(GUID_DSFX_STANDARD_CHORUS, format, parameters); }
    std::shared_ptr<IDsFxWaveProcessor<DSFXFlanger>> CreateFlangerEffector(PcmWaveFormat format, DSFXFlanger parameters) { return CreateFx<IDirectSoundFXFlanger, DSFXFlanger>(GUID_DSFX_STANDARD_FLANGER, format, parameters); }
    std::shared_ptr<IDsFxWaveProcessor<DSFXEcho>> CreateEchoEffector(PcmWaveFormat format, DSFXEcho parameters) { return CreateFx<IDirectSoundFXEcho, DSFXEcho>(GUID_DSFX_STANDARD_ECHO, format, parameters); }
    std::shared_ptr<IDsFxWaveProcessor<DSFXDistortion>> CreateDistortionEffector(PcmWaveFormat format, DSFXDistortion parameters) { return CreateFx<IDirectSoundFXDistortion, DSFXDistortion>(GUID_DSFX_STANDARD_DISTORTION, format, parameters); }
    std::shared_ptr<IDsFxWaveProcessor<DSFXCompressor>> CreateCompressorEffector(PcmWaveFormat format, DSFXCompressor parameters) { return CreateFx<IDirectSoundFXCompressor, DSFXCompressor>(GUID_DSFX_STANDARD_COMPRESSOR, format, parameters); }
    std::shared_ptr<IDsFxWaveProcessor<DSFXParamEq>> CreateParamEqEffector(PcmWaveFormat format, DSFXParamEq parameters) { return CreateFx<IDirectSoundFXParamEq, DSFXParamEq>(GUID_DSFX_STANDARD_PARAMEQ, format, parameters); }
    std::shared_ptr<IDsFxWaveProcessor<DSFXI3DL2Reverb>> CreateI3DL2ReverbEffector(PcmWaveFormat format, DSFXI3DL2Reverb parameters) { return CreateFx<IDirectSoundFXI3DL2Reverb, DSFXI3DL2Reverb>(GUID_DSFX_STANDARD_I3DL2REVERB, format, parameters); }
    std::shared_ptr<IDsFxWaveProcessor<DSFXWavesReverb>> CreateWavesReverbEffector(PcmWaveFormat format, DSFXWavesReverb parameters) { return CreateFx<IDirectSoundFXWavesReverb, DSFXWavesReverb>(GUID_DSFX_WAVES_REVERB, format, parameters); }
}
