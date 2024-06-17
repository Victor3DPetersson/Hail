#include "Shared_PCH.h"
#include "ErrorLogger.h"
#include "ErrorHandling.h"
#include "Hashing\xxh64_en.hpp"
using namespace Hail;

ErrorLogger* ErrorLogger::m_instance = nullptr;

void Hail::ErrorLogger::Initialize()
{
	H_ASSERT(GetIsMainThread(), "Only main thread should create the logger.");
	m_instance = new ErrorLogger();
}

void Hail::ErrorLogger::Deinitialize()
{
	H_ASSERT(GetIsMainThread(), "Only main thread should destroy the logger.");
	SAFEDELETE(m_instance);
}

void Hail::ErrorLogger::InsertMessage(ErrorMessage message)
{
	m_signal.Signal();
	GrowingArray<ErrorMessage>& m_insertMessageList = m_incomingMessages[m_currentIncomingMessageBuffer];
	message.m_stringHash = xxh64::hash(message.m_message.Data(), message.m_message.Length(), message.m_message.Length());
	bool bMessageExists = false;
	for (uint32 i = 0; i < m_insertMessageList.Size(); i++)
	{
		if (m_insertMessageList[i].m_stringHash == message.m_stringHash)
		{
			m_insertMessageList[i].m_numberOfOccurences++;
			m_insertMessageList[i].m_systemTimeLastHappened = message.m_systemTimeLastHappened;
			bMessageExists = true;
		}
	}
	if (bMessageExists)
	{
		m_signal.Wait();
		return;
	}

	m_insertMessageList.Add(message);
	m_signal.Wait();
}

void Hail::ErrorLogger::Update()
{
	AssertLock::Guard assertLock = m_assertLock.AssertLockFunction();

	// Fetch and swap the read buffer index.
	m_signal.Signal();
	uint32 readBufferIndex = m_currentIncomingMessageBuffer;
	m_currentIncomingMessageBuffer = (m_currentIncomingMessageBuffer + 1) % 2;
	m_signal.Wait();

	const GrowingArray<ErrorMessage>& m_readMessageList = m_incomingMessages[readBufferIndex];
	GrowingArray<const ErrorMessage*> messagesToAdd;
	for (uint32 i = 0; i < m_readMessageList.Size(); i++)
	{
		bool bMessageExists = false;
		const ErrorMessage& readMessage = m_readMessageList[i];

		for (uint32 iExistingMessages = 0; iExistingMessages < m_messages.Size(); iExistingMessages++)
		{
			ErrorMessage& existingMessage = m_messages[iExistingMessages];
			if (readMessage.m_stringHash == existingMessage.m_stringHash)
			{
				existingMessage.m_numberOfOccurences += readMessage.m_numberOfOccurences;
				existingMessage.m_systemTimeLastHappened = readMessage.m_systemTimeLastHappened;
				bMessageExists = true;
				break;
			}
		}
		if (!bMessageExists)
			messagesToAdd.Add(&readMessage);
	}
	for (uint32 i = 0; i < messagesToAdd.Size(); i++)
		m_messages.Add(*messagesToAdd[i]);

	m_incomingMessages[readBufferIndex].DeleteAll();
}

const GrowingArray<ErrorMessage>& Hail::ErrorLogger::GetCurrentMessages() const
{
	return m_messages;
}
