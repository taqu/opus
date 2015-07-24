#ifndef INC_LSOUND_SLENGINE_H__
#define INC_LSOUND_SLENGINE_H__
/**
@file SLEngine.h
@author t-sakai
@date 2015/07/16 create
*/
#include "../../lsound.h"

namespace lsound
{
    class SLEngine
    {
    public:
        typedef SLEngineItf interface_type;

        SLEngine()
            :engine_(NULL)
        {}

        explicit SLEngine(SLEngineItf engine)
            :engine_(engine)
        {}

        ~SLEngine()
        {}

        void swap(SLEngine& rhs)
        {
            lcore::swap(engine_, rhs.engine_);
        }

        operator SLEngineItf()
        {
            return engine_;
        }

        bool valid() const
        {
            return NULL != engine_;
        }

        SLresult CreateLEDDevice(
            SLObjectItf * pDevice,
            SLuint32 deviceID,
            SLuint32 numInterfaces,
            const SLInterfaceID * pInterfaceIds,
            const SLboolean * pInterfaceRequired)
        {
            return (*engine_)->CreateLEDDevice(engine_, pDevice, deviceID, numInterfaces, pInterfaceIds, pInterfaceRequired);
        }
        
        SLresult CreateVibraDevice(
            SLObjectItf * pDevice,
            SLuint32 deviceID,
            SLuint32 numInterfaces,
            const SLInterfaceID * pInterfaceIds,
            const SLboolean * pInterfaceRequired)
        {
            return (*engine_)->CreateVibraDevice(engine_, pDevice, deviceID, numInterfaces, pInterfaceIds, pInterfaceRequired);
        }

        SLresult CreateAudioPlayer(
            SLObjectItf * pPlayer,
            SLDataSource *pAudioSrc,
            SLDataSink *pAudioSnk,
            SLuint32 numInterfaces,
            const SLInterfaceID * pInterfaceIds,
            const SLboolean * pInterfaceRequired)
        {
            return (*engine_)->CreateAudioPlayer(engine_, pPlayer, pAudioSrc, pAudioSnk, numInterfaces, pInterfaceIds, pInterfaceRequired);
        }

        SLresult CreateAudioRecorder(
            SLObjectItf * pRecorder,
            SLDataSource *pAudioSrc,
            SLDataSink *pAudioSnk,
            SLuint32 numInterfaces,
            const SLInterfaceID * pInterfaceIds,
            const SLboolean * pInterfaceRequired)
        {
            return (*engine_)->CreateAudioRecorder(engine_, pRecorder, pAudioSrc, pAudioSnk, numInterfaces, pInterfaceIds, pInterfaceRequired);
        }

        SLresult CreateMidiPlayer(
            SLObjectItf * pPlayer,
            SLDataSource *pMIDISrc,
            SLDataSource *pBankSrc,
            SLDataSink *pAudioOutput,
            SLDataSink *pVibra,
            SLDataSink *pLEDArray,
            SLuint32 numInterfaces,
            const SLInterfaceID * pInterfaceIds,
            const SLboolean * pInterfaceRequired)
        {
            return (*engine_)->CreateMidiPlayer(engine_, pPlayer, pMIDISrc, pBankSrc, pAudioOutput, pVibra, pLEDArray, numInterfaces, pInterfaceIds, pInterfaceRequired);
        }

        SLresult CreateListener(
            SLObjectItf * pListener,
            SLuint32 numInterfaces,
            const SLInterfaceID * pInterfaceIds,
            const SLboolean * pInterfaceRequired)
        {
            return (*engine_)->CreateListener(engine_, pListener, numInterfaces, pInterfaceIds, pInterfaceRequired);
        }

        SLresult Create3DGroup(
            SLObjectItf * pGroup,
            SLuint32 numInterfaces,
            const SLInterfaceID * pInterfaceIds,
            const SLboolean * pInterfaceRequired)
        {
            return (*engine_)->Create3DGroup(engine_, pGroup, numInterfaces, pInterfaceIds, pInterfaceRequired);
        }

        SLresult CreateOutputMix(
            SLObjectItf * pMix,
            SLuint32 numInterfaces,
            const SLInterfaceID * pInterfaceIds,
            const SLboolean * pInterfaceRequired)
        {
            return (*engine_)->CreateOutputMix(engine_, pMix, numInterfaces, pInterfaceIds, pInterfaceRequired);
        }

        SLresult CreateMetadataExtractor(
            SLObjectItf * pMetadataExtractor,
            SLDataSource * pDataSource,
            SLuint32 numInterfaces,
            const SLInterfaceID * pInterfaceIds,
            const SLboolean * pInterfaceRequired)
        {
            return (*engine_)->CreateMetadataExtractor(engine_, pMetadataExtractor, pDataSource, numInterfaces, pInterfaceIds, pInterfaceRequired);
        }

        SLresult CreateExtensionObject(
            SLObjectItf * pObject,
            void * pParameters,
            SLuint32 objectID,
            SLuint32 numInterfaces,
            const SLInterfaceID * pInterfaceIds,
            const SLboolean * pInterfaceRequired)
        {
            return (*engine_)->CreateExtensionObject(engine_, pObject, pParameters, objectID, numInterfaces, pInterfaceIds, pInterfaceRequired);
        }

        SLresult QueryNumSupportedInterfaces(
            SLuint32 objectID,
            SLuint32 * pNumSupportedInterfaces)
        {
            return (*engine_)->QueryNumSupportedInterfaces(engine_, objectID, pNumSupportedInterfaces);
        }

        SLresult QuerySupportedInterfaces(
            SLuint32 objectID,
            SLuint32 index,
            SLInterfaceID * pInterfaceId)
        {
            return (*engine_)->QuerySupportedInterfaces(engine_, objectID, index, pInterfaceId);
        }

        SLresult QueryNumSupportedExtensions(
            SLuint32 * pNumExtensions)
        {
            return (*engine_)->QueryNumSupportedExtensions(engine_, pNumExtensions);
        }

        SLresult QuerySupportedExtension(
            SLuint32 index,
            SLchar * pExtensionName,
            SLint16 * pNameLength)
        {
            return (*engine_)->QuerySupportedExtension(engine_, index, pExtensionName, pNameLength);
        }

        SLresult IsExtensionSupported(
            const SLchar * pExtensionName,
            SLboolean * pSupported)
        {
            return (*engine_)->IsExtensionSupported(engine_, pExtensionName, pSupported);
        }
    private:
        SLEngine(const SLEngine&);
        SLEngine& operator=(const SLEngine&);

        SLEngineItf engine_;
    };
}
#endif //INC_LSOUND_SLENGINE_H__
