#pragma once
#include <atomic>
#include <mutex>
#include "Types.h"

namespace Hail
{
	class BinarySemaphore
	{
	public:
		// Increment tha atomic counter.
		void Signal();
		// Decrement the atomic counter and waits for it to be free.
		void Wait();
	private:
		std::atomic_bool m_counter{true};
	};

    extern uint64 g_mainThreadID;
    void SetMainThread();
    uint64 GetCurrentThreadID();
    bool GetIsMainThread();

    // Lock that knows what thread locked it, will assert if locked from a different thread as it is used
    class AssertLock
    {
    public:
        AssertLock();
        class Guard
        {
        public:
            ~Guard();
            AssertLock* m_owner;
            friend class AssertLock;
        };
        const Guard AssertLockFunction();
    private:
        std::atomic_uint64_t m_holdingThread{};
    };

}


