#ifndef INC_LSOUND_SLAUDIOIODEVICECAPABILITIES_H__
#define INC_LSOUND_SLAUDIOIODEVICECAPABILITIES_H__
/**
@file SLAudioIODeviceCapabilities.h
@author t-sakai
@date 2015/07/16 create
*/
#include "../../lsound.h"

namespace lsound
{
    class SLAudioIODeviceCapabilities
    {
    public:
        typedef SLAudioIODeviceCapabilitiesItf interface_type;

        SLAudioIODeviceCapabilities()
            :capabilities_(NULL)
        {}

        explicit SLAudioIODeviceCapabilities(SLAudioIODeviceCapabilitiesItf capabilities)
            :capabilities_(capabilities)
        {}

        ~SLAudioIODeviceCapabilities()
        {}

        void swap(SLAudioIODeviceCapabilities& rhs)
        {
            lcore::swap(capabilities_, rhs.capabilities_);
        }

        operator SLAudioIODeviceCapabilitiesItf()
        {
            return capabilities_;
        }

        bool valid() const
        {
            return NULL != capabilities_;
        }


        SLresult GetAvailableAudioInputs(
            SLint32 *pNumInputs,
            SLuint32 *pInputDeviceIDs)
        {
            return (*capabilities_)->GetAvailableAudioInputs(capabilities_, pNumInputs, pInputDeviceIDs);
        }

        SLresult QueryAudioInputCapabilities(
            SLuint32 deviceID,
            SLAudioInputDescriptor *pDescriptor)
        {
            return (*capabilities_)->QueryAudioInputCapabilities(capabilities_, deviceID, pDescriptor);
        }

        SLresult RegisterAvailableAudioInputsChangedCallback(
            slAvailableAudioInputsChangedCallback callback,
            void *pContext)
        {
            return (*capabilities_)->RegisterAvailableAudioInputsChangedCallback(capabilities_, callback, pContext);
        }

        SLresult GetAvailableAudioOutputs(
            SLint32 *pNumOutputs,
            SLuint32 *pOutputDeviceIDs)
        {
            return (*capabilities_)->GetAvailableAudioOutputs(capabilities_, pNumOutputs, pOutputDeviceIDs);
        }

        SLresult QueryAudioOutputCapabilities(
            SLuint32 deviceID,
            SLAudioOutputDescriptor *pDescriptor)
        {
            return (*capabilities_)->QueryAudioOutputCapabilities(capabilities_, deviceID, pDescriptor);
        }

        SLresult RegisterAvailableAudioOutputsChangedCallback(
            slAvailableAudioOutputsChangedCallback callback,
            void *pContext)
        {
            return (*capabilities_)->RegisterAvailableAudioOutputsChangedCallback(capabilities_, callback, pContext);
        }

        SLresult RegisterDefaultDeviceIDMapChangedCallback(
            slDefaultDeviceIDMapChangedCallback callback,
            void *pContext)
        {
            return (*capabilities_)->RegisterDefaultDeviceIDMapChangedCallback(capabilities_, callback, pContext);
        }

        SLresult GetAssociatedAudioInputs(
            SLuint32 deviceID,
            SLint32 *pNumAudioInputs,
            SLuint32 *pAudioInputDeviceIDs)
        {
            return (*capabilities_)->GetAssociatedAudioInputs(capabilities_, deviceID, pNumAudioInputs, pAudioInputDeviceIDs);
        }

        SLresult GetAssociatedAudioOutputs(
            SLuint32 deviceID,
            SLint32 *pNumAudioOutputs,
            SLuint32 *pAudioOutputDeviceIDs)
        {
            return (*capabilities_)->GetAssociatedAudioOutputs(capabilities_, deviceID, pNumAudioOutputs, pAudioOutputDeviceIDs);
        }

        SLresult GetDefaultAudioDevices(
            SLuint32 defaultDeviceID,
            SLint32 *pNumAudioDevices,
            SLuint32 *pAudioDeviceIDs)
        {
            return (*capabilities_)->GetDefaultAudioDevices(capabilities_, defaultDeviceID, pNumAudioDevices, pAudioDeviceIDs);
        }

        SLresult QuerySampleFormatsSupported(
            SLuint32 deviceID,
            SLmilliHertz samplingRate,
            SLint32 *pSampleFormats,
            SLint32 *pNumOfSampleFormats)
        {
            return (*capabilities_)->QuerySampleFormatsSupported(capabilities_, deviceID, samplingRate, pSampleFormats, pNumOfSampleFormats);
        }
    private:
        SLAudioIODeviceCapabilities(const SLAudioIODeviceCapabilities&);
        SLAudioIODeviceCapabilities& operator=(const SLAudioIODeviceCapabilities&);

        SLAudioIODeviceCapabilitiesItf capabilities_;
    };
}
#endif //INC_LSOUND_SLAUDIOIODEVICECAPABILITIES_H__
