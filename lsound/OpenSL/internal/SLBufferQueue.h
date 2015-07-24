#ifndef INC_LSOUND_SLBUFFERQUEUE_H__
#define INC_LSOUND_SLBUFFERQUEUE_H__
/**
@file SLBufferQueue.h
@author t-sakai
@date 2015/07/16 create
*/
#include "../../lsound.h"

namespace lsound
{
    class SLBufferQueue
    {
    public:
        typedef SLAndroidSimpleBufferQueueItf interface_type;

        SLBufferQueue()
            :bufferQueue_(NULL)
        {}

        explicit SLBufferQueue(SLAndroidSimpleBufferQueueItf bufferQueue)
            :bufferQueue_(bufferQueue)
        {}

        ~SLBufferQueue()
        {}

        void swap(SLBufferQueue& rhs)
        {
            lcore::swap(bufferQueue_, rhs.bufferQueue_);
        }

        operator SLAndroidSimpleBufferQueueItf()
        {
            return bufferQueue_;
        }

        bool valid() const
        {
            return NULL != bufferQueue_;
        }

        SLresult Enqueue(
            const void *pBuffer,
            SLuint32 size)
        {
            return (*bufferQueue_)->Enqueue(bufferQueue_, pBuffer, size);
        }

        SLresult Clear()
        {
            return (*bufferQueue_)->Clear(bufferQueue_);
        }

        SLresult GetState(SLAndroidSimpleBufferQueueState *pState)
        {
            return (*bufferQueue_)->GetState(bufferQueue_, pState);
        }

        SLresult RegisterCallback(
            slAndroidSimpleBufferQueueCallback callback,
            void* pContext)
        {
            return (*bufferQueue_)->RegisterCallback(bufferQueue_, callback, pContext);
        }
    private:
        SLBufferQueue(const SLBufferQueue&);
        SLBufferQueue& operator=(const SLBufferQueue&);

        SLAndroidSimpleBufferQueueItf bufferQueue_;
    };
}
#endif //INC_LSOUND_SLBUFFERQUEUE_H__
