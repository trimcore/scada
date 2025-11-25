#ifndef SCADA_THREADPOOL_WAIT_HPP
#define SCADA_THREADPOOL_WAIT_HPP

#include "SCADA_ThreadPool.hpp"

namespace Scada {
    
    // ThreadPool::Wait
    //  - 
    //
    class ThreadPool::Wait
        : virtual TRIMCORE::Log::Provider {

        PTP_WAIT handle;

    public:
        Wait ();
        // explicit Wait (ThreadPool::Environment &);
        virtual ~Wait ();

        // TODO: movable, but log error if pending and cancel pending source

        // API

        void WaitInThreadPool (HANDLE object) {// TODO: timeout or SYSTEMTIME
            SetThreadpoolWait (this->handle, object, NULL);
        }
        void WaitCancel () {
            SetThreadpoolWait (this->handle, NULL, NULL);
        }
        void WaitForCompletion (bool cancel = false) {
            WaitForThreadpoolWaitCallbacks (this->handle, cancel);
        }
        
    protected:
        virtual void ThreadPoolWaitCompletion (bool timeouted) noexcept = 0;

    private:
        Wait (const Wait &) = delete;
        Wait & operator = (const Wait &) = delete;

        static void CALLBACK ThreadPoolWaitCallback (PTP_CALLBACK_INSTANCE, PVOID, PTP_WAIT, TP_WAIT_RESULT) noexcept;
    };
}

#endif
