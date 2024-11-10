#pragma once
#include "Shared_PCH.h"
#include "StringMemoryAllocator.h"
#include "Threading.h"

using namespace Hail;

StringMemoryAllocator* StringMemoryAllocator::m_pInstance = nullptr;

void Hail::StringMemoryAllocator::Initialize()
{
	H_ASSERT(GetIsMainThread(), "Only main thread should create the allocator.");
	H_ASSERT(!m_pInstance, "Can not create the main instance more than once.");
	m_pInstance = new StringMemoryAllocator();

	m_pInstance->m_charBlock.m_pBuffer = new char[m_pInstance->m_charBlock.GetMemoryBufferLength()];
	m_pInstance->m_wCharBlock.m_pBuffer = new wchar_t[m_pInstance->m_wCharBlock.GetMemoryBufferLength()];
	constexpr uint32 sizeofChar = sizeof(wchar_t);

}

void Hail::StringMemoryAllocator::Deinitialize()
{
	H_ASSERT(GetIsMainThread(), "Only main thread should destroy the allocator.");
	H_ASSERT(!m_pInstance, "Programming error, deleting a non valid instance.");
	SAFEDELETE_ARRAY(m_pInstance->m_charBlock.m_pBuffer);
	SAFEDELETE_ARRAY(m_pInstance->m_wCharBlock.m_pBuffer);
	SAFEDELETE(m_pInstance);
}

void Hail::StringMemoryAllocator::AllocateString(const char* const pString, uint32 length, char** pOwningPointer)
{
	m_charBlock.AllocateString(length, pOwningPointer);
	if (pString)
	{
		memcpy(*pOwningPointer, pString, length * sizeof(char));
		(*pOwningPointer)[length] = 0;
	}

}

void Hail::StringMemoryAllocator::AllocateString(const wchar_t* const pString, uint32 length, wchar_t** pOwningPointer)
{
	m_wCharBlock.AllocateString(length, pOwningPointer);
	if (pString)
	{
		memcpy(*pOwningPointer, pString, length * sizeof(wchar_t));
		(*pOwningPointer)[length] = 0;
	}
}

void Hail::StringMemoryAllocator::MoveStringAllocator(char** pFrom, char** pToo)
{
	m_charBlock.MoveStringAllocator(pFrom, pToo);
}

void Hail::StringMemoryAllocator::MoveStringAllocator(wchar_t** pFrom, wchar_t** pToo)
{
	m_wCharBlock.MoveStringAllocator(pFrom, pToo);
}

void Hail::StringMemoryAllocator::DeallocateString(char** pToDeAllocate)
{
	m_charBlock.DeallocateString(pToDeAllocate);
}

void Hail::StringMemoryAllocator::DeallocateString(wchar_t** pToDeAllocate)
{
	m_wCharBlock.DeallocateString(pToDeAllocate);
}

