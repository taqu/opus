#ifndef INC_LSOUND_SLPLAY_H__
#define INC_LSOUND_SLPLAY_H__
/**
@file SLPlay.h
@author t-sakai
@date 2015/07/16 create
*/
#include "../../lsound.h"

namespace lsound
{
    class SLPlay
    {
    public:
        typedef SLPlayItf interface_type;

        SLPlay()
            :play_(NULL)
        {}

        explicit SLPlay(SLPlayItf play)
            :play_(play)
        {}

        ~SLPlay()
        {}

        void swap(SLPlay& rhs)
        {
            lcore::swap(play_, rhs.play_);
        }

        operator SLPlayItf()
        {
            return play_;
        }

        bool valid() const
        {
            return NULL != play_;
        }

        SLresult SetPlayState(SLuint32 state)
        {
            return (*play_)->SetPlayState(play_, state);
        }

        SLresult GetPlayState(SLuint32 *pState)
        {
            return (*play_)->GetPlayState(play_, pState);
        }

        SLresult GetDuration(SLmillisecond *pMsec)
        {
            return (*play_)->GetDuration(play_, pMsec);
        }

        SLresult GetPosition(SLmillisecond *pMsec)
        {
            return (*play_)->GetPosition(play_, pMsec);
        }

        SLresult RegisterCallback(
            slPlayCallback callback,
            void *pContext)
        {
            return (*play_)->RegisterCallback(play_, callback, pContext);
        }

        SLresult SetCallbackEventsMask(SLuint32 eventFlags)
        {
            return (*play_)->SetCallbackEventsMask(play_, eventFlags);
        }

        SLresult GetCallbackEventsMask(SLuint32 *pEventFlags)
        {
            return (*play_)->GetCallbackEventsMask(play_, pEventFlags);
        }

        SLresult SetMarkerPosition(SLmillisecond mSec)
        {
            return (*play_)->SetMarkerPosition(play_, mSec);
        }

        SLresult ClearMarkerPosition()
        {
            return (*play_)->ClearMarkerPosition(play_);
        }

        SLresult GetMarkerPosition(SLmillisecond *pMsec)
        {
            return (*play_)->GetMarkerPosition(play_, pMsec);
        }

        SLresult SetPositionUpdatePeriod(SLmillisecond mSec)
        {
            return (*play_)->SetPositionUpdatePeriod(play_, mSec);
        }

        SLresult GetPositionUpdatePeriod(SLmillisecond *pMsec)
        {
            return (*play_)->GetPositionUpdatePeriod(play_, pMsec);
        }
    private:
        SLPlay(const SLPlay&);
        SLPlay& operator=(const SLPlay&);

        SLPlayItf play_;
    };
}
#endif //INC_LSOUND_SLPLAY_H__
