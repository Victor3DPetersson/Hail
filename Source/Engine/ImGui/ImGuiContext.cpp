#include "Engine_PCH.h"
#include "ImGuiContext.h"


using namespace Hail;

bool Hail::TextureContextAsset::operator==(const TextureContextAsset& other) const
{
	return m_texture == other.m_texture;
}

void Hail::ImGuiContext::SetCurrentContextObject(ImGuiContextsType contextType, void* contextObject)
{
	m_currentContextObject = contextObject;
	m_currentContextType = contextType;
}

void* Hail::ImGuiContext::GetCurrentContextObject()
{
	return m_currentContextObject;
}

ImGuiContextsType Hail::ImGuiContext::GetCurrentContextType() const
{
	return m_currentContextObject ? m_currentContextType : ImGuiContextsType::None;
}

void Hail::ImGuiContext::DeselectContext()
{
	m_currentContextObject = nullptr;
	m_currentContextType = ImGuiContextsType::None;
}

