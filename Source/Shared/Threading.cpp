#include "Shared_PCH.h"
#include "Threading.h"
using namespace Hail;

uint64 Hail::g_mainThreadID = MAX_UINT64;

void Hail::BinarySemaphore::Signal()
{
	while (true) 
	{ 
		if (m_counter.load() == true)
		{
			bool expectedResult = true;
			const bool didWeAcquireTheLock = m_counter.compare_exchange_strong(expectedResult,false);
			if (didWeAcquireTheLock)
				break;
		}
	}
}

void Hail::BinarySemaphore::Wait()
{
	m_counter.exchange(true, std::memory_order::memory_order_release);
}

Hail::AssertLock::AssertLock()
{
	m_holdingThread.store(MAX_UINT64);
}

const AssertLock::Guard Hail::AssertLock::AssertLockFunction()
{
	const uint64 currentThreadId = GetCurrentThreadID();
	H_ASSERT(m_holdingThread.exchange(currentThreadId) == MAX_UINT64, "Function already in use by a different thread.");
	Guard guard;
	guard.m_owner = this;
	return guard;
}

Hail::AssertLock::Guard::~Guard()
{
	m_owner->m_holdingThread.store(MAX_UINT64);
}

uint64 Hail::GetCurrentThreadID()
{
	return std::hash<std::thread::id>{}(std::this_thread::get_id());
}

void Hail::SetMainThread()
{
	H_ASSERT(g_mainThreadID == MAX_UINT64, "Called SetMainThread() more than once.");
	g_mainThreadID = GetCurrentThreadID();
}

bool Hail::GetIsMainThread()
{
	return GetCurrentThreadID() == g_mainThreadID;
}
