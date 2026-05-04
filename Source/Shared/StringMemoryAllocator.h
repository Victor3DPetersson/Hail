#pragma once
#include <assert.h>
#include <stdarg.h>
#include "Containers\GrowingArray\GrowingArray.h"
#include "Threading.h"

namespace Hail
{
	class StringMemoryAllocator
	{
	public:
		static void Initialize();
		static void Deinitialize();
		static StringMemoryAllocator& GetInstance() { return *m_pInstance; }

		void AllocateString(const char* const pString, uint32 length, char** pOwningPointer);
		void AllocateString(const wchar_t* const pString, uint32 length, wchar_t** pOwningPointer);

		void DeallocateString(char** pToDeAllocate);
		void DeallocateString(wchar_t** pToDeAllocate);

	private:
		static StringMemoryAllocator* m_pInstance;

		template<typename MemoryType>
		class Block
		{
			// A block organizes string in to a uint32 (length) + sequence of characters * length 
		public:
			void Init();
			void AllocateString(uint32 length, MemoryType** pOwningPointer);
			void DeallocateString(MemoryType** pToDeAllocate);
			uint32 GetMemoryBufferLength() { return 0xffffff * sizeof(MemoryType); }

			MemoryType* m_pBuffer;
			std::atomic_int m_freeOffset;
			std::atomic_int m_head;

			struct TableKey
			{
				// If next offset == 0, the next TableKey is the head of the list
				int32 m_nextOffset;
				uint32 m_freeSize;
			};
			BinarySemaphore m_signal;
		};
		Block<char> m_charBlock;
		Block<wchar_t> m_wCharBlock;
	};

	template<typename MemoryType>
	inline void StringMemoryAllocator::Block<MemoryType>::Init()
	{
		m_freeOffset = -1;
		m_head = 0;
		m_pBuffer = new MemoryType[GetMemoryBufferLength()];
		memset(m_pBuffer, 0, GetMemoryBufferLength());
		TableKey firstTableKey;
		firstTableKey.m_nextOffset = -1;
		firstTableKey.m_freeSize = GetMemoryBufferLength();
		memcpy(m_pBuffer, &firstTableKey, sizeof(TableKey));
	}

	template<typename MemoryType>
	inline void StringMemoryAllocator::Block<MemoryType>::AllocateString(uint32 length, MemoryType** pOwningPointer)
	{
		m_signal.Signal();
		H_ASSERT(pOwningPointer);

		uint32 requestedSize = sizeof(uint32) + (length + sizeof(MemoryType));

		int32 prevOffset = -1;
		int32 currentOffset = m_freeOffset;

		TableKey* current = nullptr;

		// Find first-fit
		while (currentOffset != -1)
		{
			current = (TableKey*)(m_pBuffer + currentOffset);

			if (current->m_freeSize >= requestedSize)
				break;

			prevOffset = currentOffset;
			currentOffset = current->m_nextOffset;
		}

		// No suitable free block -> allocate at head
		if (currentOffset == -1)
		{
			currentOffset = m_head;
			m_head += requestedSize;

			memcpy(m_pBuffer + currentOffset, &requestedSize, sizeof(uint32));
			*pOwningPointer = (MemoryType*)(m_pBuffer + currentOffset + sizeof(uint32));
			m_signal.Wait();
			return;
		}

		// Decide if we split
		const uint32 remaining = current->m_freeSize - requestedSize;
		const int minAllocationSize = sizeof(void*) * 2;
		if (remaining > minAllocationSize)
		{
			// Split block
			int32 newFreeOffset = currentOffset + requestedSize;

			TableKey newFree;
			newFree.m_freeSize = remaining;
			newFree.m_nextOffset = current->m_nextOffset;

			memcpy(m_pBuffer + newFreeOffset, &newFree, sizeof(TableKey));

			// Update free list
			if (prevOffset == -1)
			{
				m_freeOffset = newFreeOffset;
			}
			else
			{
				((TableKey*)(m_pBuffer + prevOffset))->m_nextOffset = newFreeOffset;
			}
		}
		else
		{
			// Take entire block
			requestedSize = current->m_freeSize;

			if (prevOffset == -1)
			{
				m_freeOffset = current->m_nextOffset;
			}
			else
			{
				((TableKey*)(m_pBuffer + prevOffset))->m_nextOffset = current->m_nextOffset;
			}
		}

		// Store allocation size
		memcpy(m_pBuffer + currentOffset, &requestedSize, sizeof(uint32));

		*pOwningPointer = (m_pBuffer + currentOffset + sizeof(uint32));

		m_signal.Wait();
	}

	template<typename MemoryType>
	inline void StringMemoryAllocator::Block<MemoryType>::DeallocateString(MemoryType** pToDeAllocate)
	{
		H_ASSERT(pToDeAllocate && (*pToDeAllocate));
		m_signal.Signal();

		MemoryType* rawPtr = (MemoryType*)(*pToDeAllocate);
		int32 offset = (int32)((rawPtr - (MemoryType*)m_pBuffer)) - sizeof(uint32);

		uint32 blockSize;
		memcpy(&blockSize, m_pBuffer + offset, sizeof(uint32));

		// Create new free block
		TableKey newFree;
		newFree.m_freeSize = blockSize;
		newFree.m_nextOffset = -1;

		// Insert into free list (sorted by address)
		int32 prev = -1;
		int32 curr = m_freeOffset;

		while (curr != -1 && curr < offset)
		{
			prev = curr;
			curr = ((TableKey*)(m_pBuffer + curr))->m_nextOffset;
		}
		newFree.m_nextOffset = curr;
		memcpy(m_pBuffer + offset, &newFree, sizeof(TableKey));

		if (prev == -1)
		{
			m_freeOffset = offset;
		}
		else
		{
			((TableKey*)(m_pBuffer + prev))->m_nextOffset = offset;
		}

		// Merge with next
		if (curr != -1)
		{
			TableKey* next = (TableKey*)(m_pBuffer + curr);
			if (offset + newFree.m_freeSize == curr)
			{
				((TableKey*)(m_pBuffer + offset))->m_freeSize += next->m_freeSize;
				((TableKey*)(m_pBuffer + offset))->m_nextOffset = next->m_nextOffset;
			}
		}

		// Merge with previous
		if (prev != -1)
		{
			TableKey* prevBlock = (TableKey*)(m_pBuffer + prev);
			if (prev + prevBlock->m_freeSize == offset)
			{
				prevBlock->m_freeSize += ((TableKey*)(m_pBuffer + offset))->m_freeSize;
				prevBlock->m_nextOffset = ((TableKey*)(m_pBuffer + offset))->m_nextOffset;
			}
		}

		*pToDeAllocate = nullptr;
		m_signal.Wait();
		return;
	}
}
