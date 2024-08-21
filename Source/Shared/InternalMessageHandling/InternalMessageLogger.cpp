#include "Shared_PCH.h"
#include "InternalMessageLogger.h"
#include "InternalMessageHandling.h"
#include "Hashing\xxh64_en.hpp"
using namespace Hail;

InternalMessageLogger* InternalMessageLogger::m_pInstance = nullptr;

void Hail::InternalMessageLogger::Initialize()
{
	H_ASSERT(GetIsMainThread(), "Only main thread should create the logger.");
	H_ASSERT(!m_pInstance, "Can not create the main instance more than once.");
	m_pInstance = new InternalMessageLogger();
}

void Hail::InternalMessageLogger::Deinitialize()
{
	H_ASSERT(GetIsMainThread(), "Only main thread should destroy the logger.");
	SAFEDELETE(m_pInstance);
}

void Hail::InternalMessageLogger::InsertMessage(InternalMessage message)
{
	m_signal.Signal();
	GrowingArray<InternalMessage>& m_insertMessageList = m_incomingMessages[m_currentIncomingMessageBuffer];
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

void Hail::InternalMessageLogger::Update()
{
	H_ASSERT(GetIsMainThread(), "Only main thread should update the logger.");
	AssertLock::Guard assertLock = m_assertLock.AssertLockFunction();
	m_bHasUpdatedMessageList = false;

	// Fetch and swap the read buffer index.
	m_signal.Signal();
	uint32 readBufferIndex = m_currentIncomingMessageBuffer;
	m_currentIncomingMessageBuffer = (m_currentIncomingMessageBuffer + 1) % 2;
	m_signal.Wait();

	const GrowingArray<InternalMessage>& m_readMessageList = m_incomingMessages[readBufferIndex];

	m_bHasUpdatedMessageList = m_readMessageList.Size() > 0;
	GrowingArray<const InternalMessage*> messagesToAdd;
	for (uint32 i = 0; i < m_readMessageList.Size(); i++)
	{
		bool bMessageExists = false;
		const InternalMessage& readMessage = m_readMessageList[i];

		m_allMessages.Add(readMessage);

		for (uint32 iExistingMessages = 0; iExistingMessages < m_messages.Size(); iExistingMessages++)
		{
			InternalMessage& existingMessage = m_messages[iExistingMessages];
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

	m_incomingMessages[readBufferIndex].RemoveAll();
}

const GrowingArray<InternalMessage>& Hail::InternalMessageLogger::GetUniqueueMessages() const
{
	return m_messages;
}

const GrowingArray<InternalMessage>& Hail::InternalMessageLogger::GetAllMessages() const
{
	return m_allMessages;
}

void Hail::InternalMessageLogger::ClearMessages()
{
	H_ASSERT(GetIsMainThread(), "Only main thread should clear the logger.");
	m_messages.RemoveAll();
	m_incomingMessages[(m_currentIncomingMessageBuffer + 1) % 2].RemoveAll();
}
