#include "Engine_PCH.h"
#include "RenderCommands.h"

void Hail::LerpRenderCommand2DBase(RenderCommand2DBase& dst, const RenderCommand2DBase& readCommand, const RenderCommand2DBase& lastReadCommand, float t)
{
	dst.m_transform = Transform2D::LerpTransforms(readCommand.m_transform, lastReadCommand.m_transform, t);
	dst.m_color = Color::Lerp(readCommand.m_color, lastReadCommand.m_color, t);
	dst.m_dataIndex = readCommand.m_dataIndex;
	dst.m_index_materialIndex_flags = readCommand.m_index_materialIndex_flags;
}
