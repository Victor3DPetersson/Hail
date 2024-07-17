#pragma once
#include "ErrorHandling.h"
#include "Threading.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "Containers\StaticArray\StaticArray.h"

namespace Hail
{
	struct ErrorMessage
	{
		uint64 m_systemTimeLastHappened;
		uint32 m_numberOfOccurences;
		uint16 m_codeLine;
		eMessageType m_type;
		String256 m_message;
		String256 m_fileName;
		uint64 m_stringHash;
	};

	class ErrorLogger
	{
	public:
		// Operations that should only happen on the main thread, will assert if not followed.
		static void Initialize();
		static void Deinitialize();
		static ErrorLogger& GetInstance() { return *m_instance; }

		// Flips the doublebuffers
		void Update();
		const GrowingArray<ErrorMessage>& GetCurrentMessages() const;
		void ClearMessages();

		// Thread safe operation.
		void InsertMessage(ErrorMessage message);

	private:

		static ErrorLogger* m_instance;

		//Double buffering insertions of messages to make it threadsafe
		uint32 m_currentIncomingMessageBuffer{};
		StaticArray<GrowingArray<ErrorMessage>, 2> m_incomingMessages;
		GrowingArray<ErrorMessage> m_messages;

		BinarySemaphore m_signal;
		AssertLock m_assertLock;
	};
}
 