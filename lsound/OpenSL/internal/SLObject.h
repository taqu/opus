#ifndef INC_LSOUND_SLOBJECT_H__
#define INC_LSOUND_SLOBJECT_H__
/**
@file SLObject.h
@author t-sakai
@date 2015/07/16 create
*/
#include "../../lsound.h"

namespace lsound
{
    class SLObject
    {
    public:
        SLObject()
            :object_(NULL)
        {}

        explicit SLObject(SLObjectItf object)
            :object_(object)
        {}

        ~SLObject()
        {
            Destroy();
        }

        void swap(SLObject& rhs)
        {
            lcore::swap(object_, rhs.object_);
        }

        operator SLObjectItf()
        {
            return object_;
        }

        bool valid() const
        {
            return NULL != object_;
        }

        SLresult Realize(SLboolean async)
        {
            return (*object_)->Realize(object_, async);
        }

        SLresult Resume(SLboolean async)
        {
            return (*object_)->Resume(object_, async);
        }

        SLresult GetState(SLuint32 * pState)
        {
            return (*object_)->GetState(object_, pState);
        }

        SLresult GetInterface(
            const SLInterfaceID iid,
            void * pInterface)
        {
            return (*object_)->GetInterface(object_, iid, pInterface);
        }

        template<class T>
        SLresult GetInterface(const SLInterfaceID iid, T& iface);

        SLresult RegisterCallback(
            slObjectCallback callback,
            void * pContext)
        {
            return (*object_)->RegisterCallback(object_, callback, pContext);
        }

        void AbortAsyncOperation()
        {
            (*object_)->AbortAsyncOperation(object_);
        }

        void Destroy()
        {
            if(NULL != object_){
                (*object_)->Destroy(object_);
                object_ = NULL;
            }
        }

        SLresult SetPriority(
            SLint32 priority,
            SLboolean preemptable)
        {
            return (*object_)->SetPriority(object_, priority, preemptable);
        }

        SLresult GetPriority(
            SLint32 *pPriority,
            SLboolean *pPreemptable)
        {
            return (*object_)->GetPriority(object_, pPriority, pPreemptable);
        }

        SLresult SetLossOfControlInterfaces(
            SLint16 numInterfaces,
            SLInterfaceID * pInterfaceIDs,
            SLboolean enabled)
        {
            return (*object_)->SetLossOfControlInterfaces(object_, numInterfaces, pInterfaceIDs, enabled);
        }
    private:
        SLObject(const SLObject&);
        SLObject& operator=(SLObject&);

        SLObjectItf object_;
    };

    template<class T>
    SLresult SLObject::GetInterface(const SLInterfaceID iid, T& iface)
    {
        typename T::interface_type ptr = NULL;
        SLresult result = (*object_)->GetInterface(object_, iid, &ptr);
        if(SL_RESULT_SUCCESS == result){
            T(ptr).swap(iface);
        }
        return result;
    }
}
#endif //INC_LSOUND_SLOBJECT_H__
