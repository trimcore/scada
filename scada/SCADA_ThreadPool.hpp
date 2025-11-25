#ifndef SCADA_THREADPOOL_HPP
#define SCADA_THREADPOOL_HPP

#include "TRIMCORE.h"

namespace Scada {
    
    class ThreadPoolEnviroment { // ?
        TP_CALLBACK_ENVIRON data;

    public:

    };

    // ThreadPool
    //  - the class itself provides customizable variant of the thread pool
    //  - mainly facilitates namespace for system default thread pool implementation
    //
    class ThreadPool
        : private TRIMCORE::Log::Provider {

    public:
        class Io;
        class Work;
        class Wait;
        class Timer;
        class Callback;
        class CleanupGroup;

    private:
        PTP_POOL            handle;
        TP_CALLBACK_ENVIRON environment;

    public:
        ThreadPool ();
        ThreadPool (ThreadPool &&) noexcept;
        ThreadPool & operator = (ThreadPool &&) noexcept;
        ~ThreadPool ();

        // TODO: min, max threads
        // TODO: thread stack settings, etc.
        
    private:
        ThreadPool (const ThreadPool &) = delete;
        ThreadPool & operator = (const ThreadPool &) = delete;
    };
}

#endif

