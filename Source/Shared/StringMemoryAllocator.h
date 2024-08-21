#pragma once
#include <assert.h>
#include <stdarg.h>
#include "Containers\GrowingArray\GrowingArray.h"
#include "Threading.h"

namespace Hail
{
	struct StringAllocator
	{
		void* m_memorySpot;

	};

	class StringMemoryAllocator
	{
	public:
		static void Initialize();
		static void Deinitialize();
		static StringMemoryAllocator& GetInstance() { return *m_pInstance; }

		void AllocateString(const char* const pString, uint32 length, char** pOwningPointer);
		void AllocateString(const wchar_t* const pString, uint32 length, wchar_t** pOwningPointer);

		void MoveStringAllocator(char** pFrom, char** pToo);

		void DeallocateString(char** pToDeAllocate);
		void DeallocateString(wchar_t** pToDeAllocate);

	private:
		static StringMemoryAllocator* m_pInstance;

		template<typename MemoryType>
		class Block
		{
		public:

			void AllocateString(uint32 length, MemoryType** pOwningPointer);
			void MoveStringAllocator(MemoryType** pFrom, MemoryType** pToo);
			void DeallocateString(MemoryType** pToDeAllocate);
			uint64 GetMemoryBufferLength() { return 0xffffff * sizeof(MemoryType); }

			MemoryType* m_pBuffer;
			std::atomic_int m_offset;
			struct Node
			{
				MemoryType** m_pOwningPointer;
				uint32 m_length;
			};
			GrowingArray<Node> m_registeredStrings;
			BinarySemaphore m_signal;
		};

		Block<char> m_charBlock;
		Block<wchar_t> m_wCharBlock;
	};

	template<typename MemoryType>
	inline void StringMemoryAllocator::Block<MemoryType>::AllocateString(uint32 length, MemoryType** pOwningPointer)
	{
		m_signal.Signal();
		if (m_registeredStrings.Empty())
		{
			Node newNode;
			newNode.m_pOwningPointer = pOwningPointer;
			*newNode.m_pOwningPointer = m_pBuffer;

			std::atomic_fetch_add(&m_offset, (length + 1) * sizeof(MemoryType));
			newNode.m_length = (length + 1) * sizeof(MemoryType);
			m_pBuffer[m_offset] = 0;
			m_registeredStrings.Add(newNode);
			m_signal.Wait();
			return;
		}

		int findResult = -1;
		if (pOwningPointer)
		{
			for (size_t i = 0; i < m_registeredStrings.Size(); i++)
			{
				if (pOwningPointer == m_registeredStrings[i].m_pOwningPointer)
				{
					findResult = i;
					break;
				}
			}
		}
		if (findResult >= 0)
		{
			Node& resultNode = m_registeredStrings[findResult];
			const uint32 lengthOfString = resultNode.m_length;
			if (*resultNode.m_pOwningPointer && lengthOfString > length)
			{
				m_signal.Wait();
				return;
			}

			if (findResult == m_registeredStrings.Size() - 1)
			{
				const int howMuchToRemove = (int)lengthOfString * -1;
				m_offset.fetch_add(howMuchToRemove);
				m_registeredStrings.RemoveLast();
			}
			else
			{
				m_registeredStrings.RemoveAtIndex(findResult);
			}
		}

		H_ASSERT(m_offset.load() + length < GetMemoryBufferLength(), "Out of range allocation");
		{
			uint32 oldOffset = m_offset.fetch_add(length + 1);
			Node newNode;
			newNode.m_pOwningPointer = pOwningPointer;
			*newNode.m_pOwningPointer = m_pBuffer + oldOffset;
			newNode.m_length = length + 1;
			m_pBuffer[(oldOffset + length + 1)] = 0;
			m_registeredStrings.Add(newNode);
		}
		m_signal.Wait();
	}

	template<typename MemoryType>
	inline void StringMemoryAllocator::Block<MemoryType>::MoveStringAllocator(MemoryType** pFrom, MemoryType** pToo)
	{
		m_signal.Signal();
		int findResult = -1;
		for (size_t i = 0; i < m_registeredStrings.Size(); i++)
		{
			if (pFrom == m_registeredStrings[i].m_pOwningPointer)
			{
				findResult = i;
				break;
			}
		}
		H_ASSERT(findResult != -1, "Tried to move a non allocated string.");
		*pToo = *(m_registeredStrings[findResult].m_pOwningPointer);
		m_registeredStrings[findResult].m_pOwningPointer = pToo;
		*pFrom = nullptr;
		m_signal.Wait();
	}

	template<typename MemoryType>
	inline void StringMemoryAllocator::Block<MemoryType>::DeallocateString(MemoryType** pToDeAllocate)
	{
		m_signal.Signal();
		int findResult = -1;
		for (size_t i = 0; i < m_registeredStrings.Size(); i++)
		{
			if (pToDeAllocate == m_registeredStrings[i].m_pOwningPointer)
			{
				findResult = i;
				break;
			}
		}
		if (findResult >= 0)
		{
			Node& resultNode = m_registeredStrings[findResult];

			if (findResult == m_registeredStrings.Size() - 1)
			{
				const int howMuchToRemove = (int)resultNode.m_length * -1;
				m_offset.fetch_add(howMuchToRemove);
				m_registeredStrings.RemoveLast();
			}
			else
			{
				m_registeredStrings.RemoveAtIndex(findResult);
			}
		}
		else
		{
			H_ASSERT(false, "Deallocating non-existent string.");
		}
		*pToDeAllocate = nullptr;
		m_signal.Wait();
	}
}
