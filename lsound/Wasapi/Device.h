#ifndef INC_LSOUND_WASAPI_DEVICE_H__
#define INC_LSOUND_WASAPI_DEVICE_H__
/**
@file Device.h
@author t-sakai
@date 2015/07/06 create
*/
#include "../lsound.h"

struct IAudioSessionManager;
struct IAudioSessionControl;
struct ISimpleAudioVolume;

namespace lsound
{
    class Player;

    class Device
    {
    public:
        Device();
        ~Device();

        inline bool valid() const;
        void destroy();
    private:
        friend class Context;

        IMMDevice* device_;
        IAudioSessionManager* audioSessionManager_;
        //IAudioSessionControl* audioSessionControl_;
        ISimpleAudioVolume* simpleVolume_;
    };

    inline bool Device::valid() const
    {
        return (NULL != device_);
    }
}
#endif //INC_LSOUND_WASAPI_DEVICE_H__
