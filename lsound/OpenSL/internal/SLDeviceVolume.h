#ifndef INC_LSOUND_SLDEVICEVOLUME_H__
#define INC_LSOUND_SLDEVICEVOLUME_H__
/**
@file SLDeviceVolume.h
@author t-sakai
@date 2015/07/16 create
*/
#include "../../lsound.h"

namespace lsound
{
    class SLDeviceVolume
    {
    public:
        typedef SLDeviceVolumeItf interface_type;

        SLDeviceVolume()
            :volume_(NULL)
        {}

        explicit SLDeviceVolume(SLDeviceVolumeItf volume)
            :volume_(volume)
        {}

        ~SLDeviceVolume()
        {}

        void swap(SLDeviceVolume& rhs)
        {
            lcore::swap(volume_, rhs.volume_);
        }

        operator SLDeviceVolumeItf()
        {
            return volume_;
        }

        bool valid() const
        {
            return NULL != volume_;
        }

        SLresult GetVolumeScale(
            SLuint32 deviceID,
            SLint32 *pMinValue,
            SLint32 *pMaxValue,
            SLboolean *pIsMillibelScale)
        {
            return (*volume_)->GetVolumeScale(volume_, deviceID, pMinValue, pMaxValue, pIsMillibelScale);
        }

        SLresult SetVolume(
            SLuint32 deviceID,
            SLint32 volume)
        {
            return (*volume_)->SetVolume(volume_, deviceID, volume);
        }

        SLresult GetVolume(
            SLuint32 deviceID,
            SLint32 *pVolume)
        {
            return (*volume_)->GetVolume(volume_, deviceID, pVolume);
        }

    private:
        SLDeviceVolume(const SLDeviceVolume&);
        SLDeviceVolume& operator=(const SLDeviceVolume&);

        SLDeviceVolumeItf volume_;
    };
}
#endif //INC_LSOUND_SLDEVICEVOLUME_H__
