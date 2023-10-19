#include "Engine_PCH.h"
#include "Resource.h"

Hail::ResourceValidator::ResourceValidator()
{
	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		m_dirtyFrameData[i] = false;
	}
	m_resourceIsDirty = false;
}

void Hail::ResourceValidator::MarkResourceAsDirty(uint32 frameInFlight)
{
	if (m_resourceIsDirty)
	{
		return;
	}
	m_frameInFlightThatSetDirty = frameInFlight;
	m_resourceIsDirty = true;
	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		m_dirtyFrameData[i] = true;
	}
}

bool Hail::ResourceValidator::IsAllFrameResourcesDirty() const
{
	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		if (!m_dirtyFrameData[i])
		{
			return false;
		}
	}
	return true;
}

void Hail::ResourceValidator::ClearFrameData(uint32 frameInFlight)
{
	m_dirtyFrameData[frameInFlight] = false;
	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		if (m_dirtyFrameData[i])
		{
			return;
		}
	}
	m_resourceIsDirty = false;
}
