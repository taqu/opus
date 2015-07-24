/**
@file Device.cpp
@author t-sakai
@date 2015/07/06 create
*/
#include "Device.h"
#include <Audioclient.h>
#include <Audiopolicy.h>
#include "../opus/Stream.h"
#include "Player.h"

namespace lsound
{
    Device::Device()
        :device_(NULL)
        ,audioSessionManager_(NULL)
        //,audioSessionControl_(NULL)
        ,simpleVolume_(NULL)
    {
    }

    Device::~Device()
    {
        destroy();
    }

    void Device::destroy()
    {
        LSOUND_RELEASE(simpleVolume_);
        //LSOUND_RELEASE(audioSessionControl_);
        LSOUND_RELEASE(audioSessionManager_);
        LSOUND_RELEASE(device_);
    }
}
