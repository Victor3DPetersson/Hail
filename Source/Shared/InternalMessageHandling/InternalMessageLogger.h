#pragma once
#include "InternalMessageHandling.h"
#include "Threading.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "Containers\StaticArray\StaticArray.h"

namespace Hail
{
	struct InternalMessage
	{
		uint64 m_systemTimeLastHappened;
		uint32 m_numberOfOccurences;
		uint16 m_codeLine;
		eMessageType m_type;
		StringL m_message;
		StringL m_fileName;
		uint64 m_stringHash;
	};

	class InternalMessageLogger
	{
	public:
		// Operations that should only happen on the main thread, will assert if not followed.
		static void Initialize();
		static void Deinitialize();
		static InternalMessageLogger& GetInstance() { return *m_pInstance; }

		// Flips the doublebuffers
		void Update();
		const GrowingArray<InternalMessage>& GetUniqueueMessages() const;
		const GrowingArray<InternalMessage>& GetAllMessages() const;
		void ClearMessages();

		// Thread safe operation.
		void InsertMessage(InternalMessage message);

		bool HasRecievedNewMessages() const { return m_bHasUpdatedMessageList; }

	private:

		static InternalMessageLogger* m_pInstance;

		//Double buffering insertions of messages to make it threadsafe
		uint32 m_currentIncomingMessageBuffer{};
		StaticArray<GrowingArray<InternalMessage>, 2> m_incomingMessages;
		GrowingArray<InternalMessage> m_messages;
		GrowingArray<InternalMessage> m_allMessages;

		BinarySemaphore m_signal;
		AssertLock m_assertLock;
		bool m_bHasUpdatedMessageList;
	};
}
 