#ifndef SCADA_MANIFOLD_HPP
#define SCADA_MANIFOLD_HPP

#include <WinSock2.h>
#include <mswsock.h>

// #include "TRIMCORE.h"
#include "SCADA_ThreadPool_Wait.hpp"

namespace Scada {

    // Manifold
    //  - 
    //  - TODO: pages
    //
    class Manifold
        : private TRIMCORE::Log::Provider {

        // CompletionQueue
        //  - abstracts Registered I/O Completion Queue waited on in Thread Pool
        //  - completed operations are ...
        //
        class CompletionQueue
            : virtual TRIMCORE::Log::Provider
            , private ThreadPool::Wait {

            friend class Manifold;

            HANDLE event;
            RIO_CQ handle;

        public:

            // CompletionQueue
            //  - 
            //  - 'n' is number of completions the queue must initially accomodate
            //
            explicit CompletionQueue (std::size_t n);
            virtual ~CompletionQueue ();

            // resize
            //  - attempts to resize the queue to 'n' completions
            //  - returns if the operation succeeded
            //
            bool resize (std::size_t n);

            // activate
            //  - 
            //
            bool activate ();

            // cancel/complete
            //  - 

            using ThreadPool::Wait::WaitCancel;
            using ThreadPool::Wait::WaitForCompletion;

        protected:

            // dequeue
            //  - 
            //
            std::size_t dequeue (RIORESULT * target, std::size_t size);

            // ContinueRioThreadPoolWait
            //  - 
            //
            bool ContinueRioThreadPoolWait ();

        private:
            CompletionQueue (const CompletionQueue &) = delete;
            CompletionQueue & operator = (const CompletionQueue &) = delete;
        };

        // TODO: consider merging to object above

        class RcvCompletionQueue : virtual TRIMCORE::Log::Provider, public CompletionQueue {
        public:
            RcvCompletionQueue () : TRIMCORE::Log::Provider ("Completion Queue", 0x2000, L"RCV"), CompletionQueue (1024 * 1024) {}
        protected:
            virtual void ThreadPoolWaitCompletion (bool timeouted) noexcept override;
        } receives;

        class TrmCompletionQueue : virtual TRIMCORE::Log::Provider, public CompletionQueue {
        public:
            TrmCompletionQueue () : TRIMCORE::Log::Provider ("Completion Queue", 0x2000, L"TRM"), CompletionQueue (1024) {}
        protected:
            virtual void ThreadPoolWaitCompletion (bool timeouted) noexcept override;
        } transmits;

    public:
        class Address;
        class Socket;
        class DatagramReceiver;
        class DatagramTransmitter;
        class DatagramEndPoint;

        class Listener;
        class Connection;
        class Buffer;

        // TODO: factory (?) will choose Rio for INET, Overlapped for other

        class Overlapped;
        class IpcHandle;
        class Mailslot;
        class Pipe;
        class PipeServer;
        class PipeListener;

        class Server;
        class RioReceiver;
        class RioTransmitter;
        class OverlappedOperation;
        class OverlappedReceiver;
        class OverlappedTransmitter;

    public:
        Manifold ();
        ~Manifold ();

        // attach
        //  - to completion queues
        //
        void attach (DatagramEndPoint &, std::size_t nrcv, std::size_t ntrm);
        

        // activate
        //  -
        //
        bool activate () {
            return this->receives.activate ()
                && this->transmits.activate ();
        }

        bool shutdown () {
            this->receives.WaitCancel ();
            this->transmits.WaitCancel ();
            this->receives.WaitForCompletion (true);
            this->transmits.WaitForCompletion (true);
            return true;
        }

    private:
        Manifold (const Manifold &) = delete;
        Manifold & operator = (const Manifold &) = delete;
    };
}

#endif

