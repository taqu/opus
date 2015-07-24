#ifndef INC_LSOUND_SLENGINECAPABILITIES_H__
#define INC_LSOUND_SLENGINECAPABILITIES_H__
/**
@file SLEngineCapabilities.h
@author t-sakai
@date 2015/07/16 create
*/
#include "../../lsound.h"

namespace lsound
{
    class SLEngineCapabilities
    {
    public:
        typedef SLEngineCapabilitiesItf interface_type;

        SLEngineCapabilities()
            :capabilities_(NULL)
        {}

        explicit SLEngineCapabilities(SLEngineCapabilitiesItf capabilities)
            :capabilities_(capabilities)
        {}

        ~SLEngineCapabilities()
        {}

        void swap(SLEngineCapabilities& rhs)
        {
            lcore::swap(capabilities_, rhs.capabilities_);
        }

        operator SLEngineCapabilitiesItf()
        {
            return capabilities_;
        }

        bool valid() const
        {
            return NULL != capabilities_;
        }

        SLresult QuerySupportedProfiles(SLuint16 *pProfilesSupported)
        {
            return (*capabilities_)->QuerySupportedProfiles(capabilities_, pProfilesSupported);
        }

        SLresult QueryAvailableVoices(
            SLuint16 voiceType,
            SLint16 *pNumMaxVoices,
            SLboolean *pIsAbsoluteMax,
            SLint16 *pNumFreeVoices)
        {
            return (*capabilities_)->QueryAvailableVoices(capabilities_, voiceType, pNumMaxVoices, pIsAbsoluteMax, pNumFreeVoices);
        }

        SLresult QueryNumberOfMIDISynthesizers(SLint16 *pNumMIDIsynthesizers)
        {
            return (*capabilities_)->QueryNumberOfMIDISynthesizers(capabilities_, pNumMIDIsynthesizers);
        }

        SLresult QueryAPIVersion(
            SLint16 *pMajor,
            SLint16 *pMinor,
            SLint16 *pStep)
        {
            return (*capabilities_)->QueryAPIVersion(capabilities_, pMajor, pMinor, pStep);
        }

        SLresult QueryLEDCapabilities(
            SLuint32 *pIndex,
            SLuint32 *pLEDDeviceID,
            SLLEDDescriptor *pDescriptor)
        {
            return (*capabilities_)->QueryLEDCapabilities(capabilities_, pIndex, pLEDDeviceID, pDescriptor);
        }

        SLresult QueryVibraCapabilities(
            SLuint32 *pIndex,
            SLuint32 *pVibraDeviceID,
            SLVibraDescriptor *pDescriptor)
        {
            return (*capabilities_)->QueryVibraCapabilities(capabilities_, pIndex, pVibraDeviceID, pDescriptor);
        }

        SLresult IsThreadSafe(SLboolean *pIsThreadSafe)
        {
            return (*capabilities_)->IsThreadSafe(capabilities_, pIsThreadSafe);
        }
    private:
        SLEngineCapabilities(const SLEngineCapabilities&);
        SLEngineCapabilities& operator=(const SLEngineCapabilities&);

        SLEngineCapabilitiesItf capabilities_;
    };
}
#endif //INC_LSOUND_SLENGINECAPABILITIES_H__
