/**
@file Thread.cpp
@author t-sakai
@date 2011/08/06
*/
#include "Thread.h"

#if defined(_WIN32) || defined(_WIN64)
#include <process.h>
#else
#include <errno.h>
#endif

namespace lcore
{
#if defined(_WIN32) || defined(_WIN64)
    //----------------------------------------------------
    //---
    //--- Thread
    //---
    //----------------------------------------------------
    ThreadRaw::ThreadRaw()
        :handle_(NULL)
        ,id_(0)
        ,proc_(NULL)
        ,data_(NULL)
    {
    }

    ThreadRaw::~ThreadRaw()
    {
        release();
    }

    bool ThreadRaw::create(Proc proc, void* data, bool suspend)
    {
        LASSERT(NULL == handle_);
        proc_ = proc;
        data_ = data;
        handle_ = reinterpret_cast<HANDLE>( _beginthreadex(
            NULL,
            0,
            ThreadRaw::proc,
            this,
            (suspend)? CREATE_SUSPENDED : 0,
            &id_));

        return (handle_ != NULL);
    }

    void ThreadRaw::start()
    {
        resume();
    }

    void ThreadRaw::resume()
    {
        ResumeThread(handle_);
    }

    void ThreadRaw::suspend()
    {
        SuspendThread(handle_);
    }

    void ThreadRaw::terminate()
    {
        TerminateThread(handle_, 0);
    }

    thread::WaitStatus ThreadRaw::join(u32 timeout)
    {
        return static_cast<thread::WaitStatus>(WaitForSingleObject(handle_, timeout));
    }


    void ThreadRaw::release()
    {
        if(NULL != handle_){
            CloseHandle(handle_);
            handle_ = NULL;
            id_ = 0;
        }
    }


    u32 __stdcall ThreadRaw::proc(void* args)
    {
        ThreadRaw* thread = reinterpret_cast<ThreadRaw*>(args);
        LASSERT(NULL != thread);
        if(NULL != thread->proc_){
            thread->proc_(thread->data_);
        }
        _endthreadex(0);
        return 0;
    }

    //----------------------------------------------------
    //---
    //--- Thread
    //---
    //----------------------------------------------------
    Thread::Thread()
        :handle_(NULL)
        ,id_(0)
        ,canRun_(true)
        ,suspend_(true)
    {
    }

    Thread::~Thread()
    {
        release();
    }

    bool Thread::create()
    {
        LASSERT(NULL == handle_);
        canRun_ = true;
        suspend_ = true;
        handle_ = reinterpret_cast<HANDLE>( _beginthreadex(
            NULL,
            0,
            proc,
            this,
            CREATE_SUSPENDED,
            &id_));

        return (handle_ != NULL);
    }

    void Thread::start()
    {
        resume();
    }

    void Thread::resume()
    {
        CSLock lock(cs_);
        if(suspend_){
            ResumeThread(handle_);
            suspend_ = false;
        }
    }

    void Thread::suspend()
    {
        CSLock lock(cs_);
        if(!suspend_){
            SuspendThread(handle_);
            suspend_ = true;
        }
    }

    void Thread::stop()
    {
        {
            CSLock lock(cs_);
            canRun_ = false;
        }
        resume();
    }

    void Thread::terminate()
    {
        TerminateThread(handle_, 0);
    }

    thread::WaitStatus Thread::join(u32 timeout)
    {
        return static_cast<thread::WaitStatus>(WaitForSingleObject(handle_, timeout));
    }

    // スレッド終了すべきか
    bool Thread::canRun()
    {
        CSLock lock(cs_);
        return canRun_;
    }

    void Thread::release()
    {
        if(NULL != handle_){
            CloseHandle(handle_);
            handle_ = NULL;
            id_ = 0;
        }
    }


    u32 __stdcall Thread::proc(void* args)
    {
        Thread* thread = reinterpret_cast<Thread*>(args);
        if(thread){
            thread->run();
        }

        _endthreadex(0);
        return 0;
    }

    //----------------------------------------------------
    //---
    //--- MultipleWait
    //---
    //----------------------------------------------------
    MultipleWait::MultipleWait()
    {
        for(u32 i=0; i<NumMaxThreads; ++i){
            handles_[i] = NULL;
        }
    }

    MultipleWait::~MultipleWait()
    {
    }

    void MultipleWait::set(u32 index, ThreadRaw* thread)
    {
        LASSERT(index<=NumMaxThreads);
        handles_[index] = thread->handle_;
    }

    void MultipleWait::set(u32 index, Thread* thread)
    {
        LASSERT(index<=NumMaxThreads);
        handles_[index] = thread->handle_;
    }

    u32 MultipleWait::join(u32 numThreads, u32 timeout)
    {
        return WaitForMultipleObjects(numThreads, handles_, TRUE, timeout);
    }

#else
    //----------------------------------------------------
    //---
    //--- Thread
    //---
    //----------------------------------------------------
#ifndef LCORE_THREAD_HAS_PTHREAD_TIMEDJOIN
    s32 ThreadRaw::getState()
    {
        CSLock lock(cs_);
        return state_;
    }
    void ThreadRaw::setState(s32 state)
    {
        CSLock lock(cs_);
        state_ = state;
    }
#endif

    ThreadRaw::ThreadRaw()
        :handle_(0)
        ,proc_(NULL)
        ,data_(NULL)
#ifndef LCORE_THREAD_HAS_PTHREAD_TIMEDJOIN
        ,state_(State_None)
#endif
    {
    }

    ThreadRaw::~ThreadRaw()
    {
        release();
    }

    bool ThreadRaw::create(Proc proc, void* data, bool suspend)
    {
        LASSERT(0 == handle_);

        proc_ = proc;
        data_ = data;
        s32 ret = pthread_create(&handle_, NULL, ThreadRaw::proc, this);
        return (0==ret);
    }

    void ThreadRaw::start()
    {
    }

    void ThreadRaw::resume()
    {
    }

    void ThreadRaw::suspend()
    {
    }

    void ThreadRaw::terminate()
    {
#ifdef LCORE_THREAD_HAS_PTHREAD_CANCEL
        pthread_cancel(handle_);
#endif
    }

    thread::WaitStatus ThreadRaw::join(u32 timeout)
    {
        s32 ret;
        if(Infinite == timeout){
            ret = pthread_join(handle_, NULL);
        } else{
#ifdef LCORE_THREAD_HAS_PTHREAD_TIMEDJOIN
            timespec ts;
            if(clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                return thread::Wait_Failed;
            }
            while(1000<timeout){
                ts.tv_sec += 1;
                timeout -= 1000;
            }
            ts.tv_nsec += 1000000L * timeout;

            ret = pthread_timedjoin_np(handle_, NULL, &ts);
#else
            s32 state = getState();
            if(State_Term != state){
                lcore::sleep(timeout);
                state = getState();
            }
            if(State_Term == state){
                ret = pthread_join(handle_, NULL);
            }else{
                ret = ETIMEDOUT;
            }
#endif
        }

        switch(ret)
        {
        case EINVAL:
        case ESRCH:
        case EDEADLK:
            return thread::Wait_Failed;

        case EBUSY:
        case ETIMEDOUT:
            return thread::Wait_Timeout;

        default:
            return thread::Wait_Success;
        }
    }


    void ThreadRaw::release()
    {
#ifdef LCORE_THREAD_HAS_PTHREAD_CANCEL
        if(0 != handle_){
            pthread_cancel(handle_);
            pthread_join(handle_, NULL);
            handle_ = 0;
        }
#else
        if(0 != handle_){
            //pthread_detach(handle_);
            pthread_join(handle_, NULL);
            handle_ = 0;
        }
#endif
    }


    void* ThreadRaw::proc(void* args)
    {
        ThreadRaw* thread = reinterpret_cast<ThreadRaw*>(args);
        LASSERT(NULL != thread);
        if(NULL != thread->proc_){
#ifndef LCORE_THREAD_HAS_PTHREAD_TIMEDJOIN
            thread->setState(State_Proc);
#endif
            thread->proc_(thread->data_);
        }

#ifndef LCORE_THREAD_HAS_PTHREAD_TIMEDJOIN
        thread->setState(State_Term);
#endif
        return NULL;
    }

    //----------------------------------------------------
    //---
    //--- Thread
    //---
    //----------------------------------------------------
#ifndef LCORE_THREAD_HAS_PTHREAD_TIMEDJOIN
    s32 Thread::getState()
    {
        CSLock lock(cs_);
        return state_;
    }
    void Thread::setState(s32 state)
    {
        CSLock lock(cs_);
        state_ = state;
    }
#endif

    Thread::Thread()
        :handle_(0)
        ,canRun_(true)
        ,suspend_(true)
#ifndef LCORE_THREAD_HAS_PTHREAD_TIMEDJOIN
        ,state_(State_None)
#endif
    {
    }

    Thread::~Thread()
    {
        release();
    }

    bool Thread::create()
    {
        LASSERT(0 == handle_);
        canRun_ = true;
        suspend_ = true;
        s32 ret = pthread_create(&handle_, NULL, Thread::proc, this);

        return (0==ret);
    }

    void Thread::start()
    {
        resume();
    }

    void Thread::resume()
    {
        CSLock lock(cs_);
        suspend_ = false;
    }

    void Thread::suspend()
    {
        CSLock lock(cs_);
        suspend_ = true;
    }

    void Thread::stop()
    {
        CSLock lock(cs_);
        canRun_ = false;
        suspend_ = false;
    }

    void Thread::terminate()
    {
#ifdef LCORE_THREAD_HAS_PTHREAD_CANCEL
        pthread_cancel(handle_);
#endif
    }

    thread::WaitStatus Thread::join(u32 timeout)
    {
        s32 ret;
        if(Infinite == timeout){
            ret = pthread_join(handle_, NULL);
        } else{
#ifdef LCORE_THREAD_HAS_PTHREAD_TIMEDJOIN
            timespec ts;
            if(clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                return thread::Wait_Failed;
            }
            while(1000<timeout){
                ts.tv_sec += 1;
                timeout -= 1000;
            }
            ts.tv_nsec += 1000000L * timeout;
            ret = pthread_timedjoin_np(handle_, NULL, &ts);
#else
            s32 state = getState();
            if(State_Term != state){
                lcore::sleep(timeout);
                state = getState();
            }
            if(State_Term == state){
                ret = pthread_join(handle_, NULL);
            }else{
                ret = ETIMEDOUT;
            }
#endif
        }

        switch(ret)
        {
        case EINVAL:
        case ESRCH:
        case EDEADLK:
            return thread::Wait_Failed;

        case EBUSY:
        case ETIMEDOUT:
            return thread::Wait_Timeout;

        default:
            return thread::Wait_Success;
        }
    }

    // スレッド終了すべきか
    bool Thread::canRun()
    {
        CSLock lock(cs_);
        return canRun_;
    }

    void Thread::release()
    {
#ifdef LCORE_THREAD_HAS_PTHREAD_CANCEL
        if(0 != handle_){
            pthread_cancel(handle_);
            pthread_join(handle_, NULL);
            handle_ = 0;
        }
#else
        if(0 != handle_){
            //pthread_detach(handle_);
            pthread_join(handle_, NULL);
            handle_ = 0;
        }
#endif
    }


    void* Thread::proc(void* args)
    {
        Thread* thread = reinterpret_cast<Thread*>(args);
        if(thread){
#ifndef LCORE_THREAD_HAS_PTHREAD_TIMEDJOIN
            thread->setState(State_Proc);
#endif
            thread->run();
        }

#ifndef LCORE_THREAD_HAS_PTHREAD_TIMEDJOIN
        thread->setState(State_Term);
#endif
        return NULL;
    }
#endif
}
