#ifndef INC_LSOUND_SLVOLUME_H__
#define INC_LSOUND_SLVOLUME_H__
/**
@file SLVolume.h
@author t-sakai
@date 2015/07/16 create
*/
#include "../../lsound.h"

namespace lsound
{
    class SLVolume
    {
    public:
        typedef SLVolumeItf interface_type;

        SLVolume()
            :levelExtent_(SL_MILLIBEL_MAX-SL_MILLIBEL_MIN)
            ,volume_(NULL)
        {}

        explicit SLVolume(SLVolumeItf volume)
            :volume_(volume)
        {}

        ~SLVolume()
        {}

        void swap(SLVolume& rhs)
        {
            lcore::swap(levelExtent_, rhs.levelExtent_);
            lcore::swap(volume_, rhs.volume_);
        }

        operator SLVolumeItf()
        {
            return volume_;
        }

        bool valid() const
        {
            return NULL != volume_;
        }

        SLresult SetVolumeLevel(SLmillibel level)
        {
            return (*volume_)->SetVolumeLevel(volume_, level);
        }

        SLresult GetVolumeLevel(SLmillibel *pLevel)
        {
            return (*volume_)->GetVolumeLevel(volume_, pLevel);
        }

        SLresult GetMaxVolumeLevel(SLmillibel *pMaxLevel)
        {
            return (*volume_)->GetMaxVolumeLevel(volume_, pMaxLevel);
        }

        SLresult SetMute(SLboolean mute)
        {
            return (*volume_)->SetMute(volume_, mute);
        }

        SLresult GetMute(SLboolean *pMute)
        {
            return (*volume_)->GetMute(volume_, pMute);
        }

        SLresult EnableStereoPosition(SLboolean enable)
        {
            return (*volume_)->EnableStereoPosition(volume_, enable);
        }

        SLresult IsEnabledStereoPosition(SLboolean *pEnable)
        {
            return (*volume_)->IsEnabledStereoPosition(volume_, pEnable);
        }

        SLresult SetStereoPosition(SLpermille stereoPosition)
        {
            return (*volume_)->SetStereoPosition(volume_, stereoPosition);
        }

        SLresult GetStereoPosition(SLpermille *pStereoPosition)
        {
            return (*volume_)->GetStereoPosition(volume_, pStereoPosition);
        }

        void setVolumeExtent()
        {
            SLmillibel maxLevel = 0;
            GetMaxVolumeLevel(&maxLevel);
            levelExtent_ = maxLevel - SL_MILLIBEL_MIN;
            lcore::Log("max volume level:%d", maxLevel);
        }

        SLmillibel getVolumeLevel(f32 level)
        {
            return static_cast<SLmillibel>(SL_MILLIBEL_MIN + levelExtent_*level);
        }

    private:
        SLVolume(const SLVolume&);
        SLVolume& operator=(const SLVolume&);

        SLmillibel levelExtent_;
        SLVolumeItf volume_;
    };
}
#endif //INC_LSOUND_SLVOLUME_H__
