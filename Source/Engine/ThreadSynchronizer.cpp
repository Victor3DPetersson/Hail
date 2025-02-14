#include "Engine_PCH.h"
#include "ThreadSynchronizer.h"
#include "Input\InputHandler.h"
#include "glm\common.hpp"
#include "Resources\ResourceManager.h"
#include "Rendering\SwapChain.h"
#include "Input\InputActionMap.h"

using namespace Hail;

void Hail::ThreadSyncronizer::Init(float tickTimer)
{
	m_engineTickRate = tickTimer;
	m_currentActiveRenderPoolWrite = 0;
	m_currentActiveRenderPoolRead = 1;
	m_currentActiveRenderPoolLastRead = 2;
	m_appData.renderPool = &m_renderCommandPools[m_currentActiveRenderPoolWrite];
}

void Hail::ThreadSyncronizer::SynchronizeAppData(InputActionMap& inputActionMap, ImGuiCommandRecorder& imguiCommandRecorder, ResourceManager& resourceManager)
{
	m_currentResolution = ResolutionFromEnum(resourceManager.GetTargetResolution());
	SwapBuffersInternal();
	ClearApplicationBuffers();
	m_appData.rawInputData = inputActionMap.GetRawInputMap();
	m_appData.inputActionMap = &inputActionMap;
	m_appData.imguiCommandRecorder = &imguiCommandRecorder;
	m_currentRenderTimer = 0.0f;
	m_appData.renderPool->horizontalAspectRatio = resourceManager.GetSwapChain()->GetHorizontalAspectRatio();
	m_appData.renderPool->inverseHorizontalAspectRatio = 1.0 / m_appData.renderPool->horizontalAspectRatio;
	m_appData.renderPool->camera2D.SetResolution(m_currentResolution);
}

void Hail::ThreadSyncronizer::SynchronizeRenderData(float frameDeltaTime)
{
	m_currentRenderTimer += frameDeltaTime;
	LerpRenderBuffers();
}

void Transform2DLineFromPixelSpaceToNormalizedSpace(glm::vec3& start, glm::vec3& end, const glm::uvec2& screenResoltuion)
{
	glm::vec2 transformedPosition1 = glm::vec2(start) / glm::vec2(screenResoltuion);
	glm::vec2 transformedPosition2 = glm::vec2(end) / glm::vec2(screenResoltuion);
	transformedPosition1 += 0.5f;
	transformedPosition2 += 0.5f;
	start.x = transformedPosition1.x;
	start.y = transformedPosition1.y;
	end.x = transformedPosition2.x;
	end.y = transformedPosition2.y;
}

void Hail::ThreadSyncronizer::PrepareApplicationData()
{
	RenderCommandPool* pPool = m_appData.renderPool;
	Camera2D& camera = pPool->camera2D;
	camera.SetResolution(m_currentResolution);
	for (size_t i = 0; i < pPool->spriteCommands.Size(); i++)
	{
		RenderCommand_Sprite& sprite = pPool->spriteCommands[i];
		if (sprite.bIsAffectedBy2DCamera)
		{
			camera.TransformToCameraSpace(sprite.transform);
		}
	}
	for (size_t i = 0; i < pPool->textCommands.Size(); i++)
	{
		RenderCommand_Text& textC = pPool->textCommands[i];
		if (!textC.bNormalizedPosition)
			camera.TransformToCameraSpace(textC.transform);
	}
	for (size_t i = 0; i < pPool->debugLineCommands.Size(); i++)
	{
		RenderCommand_DebugLine& line = pPool->debugLineCommands[i];
		if (line.bIs2D)
		{
			if (line.bIsAffectedBy2DCamera)
			{
				if (line.bIsNormalized)
					camera.TransformLineToCameraSpaceFromNormalizedSpace(line.pos1, line.pos2);
				else
					camera.TransformLineToCameraSpaceFromPixelSpace(line.pos1, line.pos2);
			}
			else if (!line.bIsNormalized)
				Transform2DLineFromPixelSpaceToNormalizedSpace(line.pos1, line.pos2, camera.GetResolution());
		}
	}

}

void Hail::ThreadSyncronizer::SwapBuffersInternal()
{
	const uint32_t lastRead = m_currentActiveRenderPoolLastRead;
	m_currentActiveRenderPoolLastRead = m_currentActiveRenderPoolRead;
	m_currentActiveRenderPoolRead = m_currentActiveRenderPoolWrite;
	m_currentActiveRenderPoolWrite = lastRead;
	m_appData.renderPool = &m_renderCommandPools[m_currentActiveRenderPoolWrite];
}

void Hail::ThreadSyncronizer::ClearApplicationBuffers()
{
	m_appData.renderPool->debugLineCommands.Clear();
	m_appData.renderPool->meshCommands.Clear();
	m_appData.renderPool->spriteCommands.Clear();
	m_appData.renderPool->textCommands.Clear();
}


void Hail::ThreadSyncronizer::TransferBufferSizes()
{
	const RenderCommandPool& readPool = m_renderCommandPools[m_currentActiveRenderPoolRead];
	m_rendererCommandPool.spriteCommands.TransferSize(readPool.spriteCommands);
	m_rendererCommandPool.meshCommands.TransferSize(readPool.meshCommands);
	m_rendererCommandPool.debugLineCommands.TransferSize(readPool.debugLineCommands);
	m_rendererCommandPool.textCommands.TransferSize(readPool.textCommands);
}

void Hail::ThreadSyncronizer::LerpRenderBuffers()
{
	TransferBufferSizes();
	float tValue = m_currentRenderTimer / m_engineTickRate;
	const RenderCommandPool& readPool = m_renderCommandPools[m_currentActiveRenderPoolRead];
	const RenderCommandPool& lastReadPool = m_renderCommandPools[m_currentActiveRenderPoolLastRead];
	m_rendererCommandPool.camera3D = Camera::LerpCamera(readPool.camera3D, lastReadPool.camera3D, tValue);
	
	LerpSprites(tValue);
	Lerp3DModels(tValue);
	LerpDebugLines(tValue);
	LerpTextCommands(tValue);
}

namespace 
{
	void LerpSpriteCommand(const RenderCommand_Sprite& readSprite, const RenderCommand_Sprite& lastReadSprite, RenderCommand_Sprite& writeSprite, float t)
	{
		writeSprite.transform = Transform2D::LerpTransforms(readSprite.transform, lastReadSprite.transform, t);
		writeSprite.uvTR_BL = glm::mix(readSprite.uvTR_BL, lastReadSprite.uvTR_BL, t);
		writeSprite.color = glm::mix(readSprite.color, lastReadSprite.color, t);
		writeSprite.pivot = glm::mix(readSprite.pivot, lastReadSprite.pivot, t);
		writeSprite.index = readSprite.index;
		writeSprite.materialInstanceID = readSprite.materialInstanceID;
		writeSprite.bLerpCommand = readSprite.bLerpCommand;
		writeSprite.bIsAffectedBy2DCamera = readSprite.bIsAffectedBy2DCamera;
		writeSprite.bSizeRelativeToRenderTarget = readSprite.bSizeRelativeToRenderTarget;
	}

	void LerpMeshCommand(const RenderCommand_Mesh& readMesh, const RenderCommand_Mesh& lastReadMesh, RenderCommand_Mesh& writeMesh, float t)
	{
		writeMesh.transform = Transform3D::LerpTransforms_t(readMesh.transform, lastReadMesh.transform, t);
		writeMesh.color = glm::mix(readMesh.color, lastReadMesh.color, t);
		writeMesh.meshID = readMesh.meshID;
		writeMesh.index = readMesh.index;
		writeMesh.materialInstanceID = readMesh.materialInstanceID;
		writeMesh.bLerpCommand = readMesh.bLerpCommand;
	}

	void LerpDebugLine(const RenderCommand_DebugLine& readLine, const RenderCommand_DebugLine& lastReadLine, RenderCommand_DebugLine& writeLine, float t)
	{
		writeLine.color1 = glm::mix(readLine.color1, lastReadLine.color1, t);
		writeLine.color2 = glm::mix(readLine.color2, lastReadLine.color2, t);
		writeLine.pos1 = glm::mix(readLine.pos1, lastReadLine.pos1, t);
		writeLine.pos2 = glm::mix(readLine.pos2, lastReadLine.pos2, t);
		writeLine.bIs2D = readLine.bIs2D;
		writeLine.bLerpCommand = readLine.bLerpCommand;
		writeLine.bIsAffectedBy2DCamera = readLine.bIsAffectedBy2DCamera;
	}

	void LerpTextCommand(const RenderCommand_Text& readTextC, const RenderCommand_Text& lastReadTextC, RenderCommand_Text& writeTextC, float t)
	{
		writeTextC.transform = Transform2D::LerpTransforms(readTextC.transform, lastReadTextC.transform, t);
		writeTextC.color = glm::mix(readTextC.color, lastReadTextC.color, t);
		writeTextC.text = readTextC.text;
		writeTextC.index = readTextC.index;
		writeTextC.bLerpCommand = readTextC.bLerpCommand;
	}
}

void Hail::ThreadSyncronizer::LerpSprites(float tValue)
{
	const RenderCommandPool& readPool = m_renderCommandPools[m_currentActiveRenderPoolRead];
	const RenderCommandPool& lastReadPool = m_renderCommandPools[m_currentActiveRenderPoolLastRead];
	//Lerp sprites
	const uint32_t numberOfSprites = readPool.spriteCommands.Size();
	const uint32_t lastReadNumberOfSprites = lastReadPool.spriteCommands.Size();
	for (uint16_t sprite = 0; sprite < numberOfSprites; sprite++)
	{
		const RenderCommand_Sprite& readSprite = readPool.spriteCommands[sprite];
		RenderCommand_Sprite& writeSprite = m_rendererCommandPool.spriteCommands[sprite];

		if (sprite >= lastReadNumberOfSprites)
		{
			writeSprite = readSprite;
			continue;
		}

		const RenderCommand_Sprite& lastReadSprite = lastReadPool.spriteCommands[sprite];
		if (readSprite.bLerpCommand)
		{
			if (readSprite.index == lastReadSprite.index)
			{
				LerpSpriteCommand(readSprite, lastReadSprite, writeSprite, tValue);
			}
			else
			{
				//search for correct index
				//Switch to use an ordered heap or something like that later to increase performance.
				bool foundSprite = false;
				for (uint16_t missingSprite = sprite; missingSprite < lastReadNumberOfSprites; missingSprite++)
				{
					if (readSprite.index == lastReadPool.spriteCommands[missingSprite].index)
					{
						foundSprite = true;
						LerpSpriteCommand(readSprite, lastReadPool.spriteCommands[missingSprite], writeSprite, tValue);
						break;
					}
				}
				if (foundSprite == false)
				{
					writeSprite = readSprite;
				}
			}
		}
		else
		{
			writeSprite = readSprite;
		}
	}
}

void Hail::ThreadSyncronizer::Lerp3DModels(float tValue)
{
	const RenderCommandPool& readPool = m_renderCommandPools[m_currentActiveRenderPoolRead];
	const RenderCommandPool& lastReadPool = m_renderCommandPools[m_currentActiveRenderPoolLastRead];
	//Lerp sprites
	const uint32_t numberOfModels = readPool.meshCommands.Size();
	const uint32_t lastReadNumberOfModels = lastReadPool.meshCommands.Size();
	for (uint16_t mesh = 0; mesh < numberOfModels; mesh++)
	{
		const RenderCommand_Mesh& readMesh = readPool.meshCommands[mesh];
		RenderCommand_Mesh& writeMesh = m_rendererCommandPool.meshCommands[mesh];

		if (mesh >= lastReadNumberOfModels)
		{
			writeMesh = readMesh;
			continue;
		}

		const RenderCommand_Mesh& lastReadMesh = lastReadPool.meshCommands[mesh];
		if (readMesh.bLerpCommand)
		{
			if (readMesh.index == lastReadMesh.index)
			{
				LerpMeshCommand(readMesh, lastReadMesh, writeMesh, tValue);
			}
			else
			{
				//search for correct index
				//Switch to use an ordered heap or something like that later to increase performance.
				bool foundMesh = false;
				for (uint16_t missingMesh = mesh; missingMesh < lastReadNumberOfModels; missingMesh++)
				{
					if (readMesh.index == lastReadPool.meshCommands[missingMesh].index)
					{
						foundMesh = true;
						LerpMeshCommand(readMesh, lastReadPool.meshCommands[missingMesh], writeMesh, tValue);
						break;
					}
				}
				if (foundMesh == false)
				{
					writeMesh = readMesh;
				}
			}
		}
		else
		{
			writeMesh = readMesh;
		}
	}
}

void Hail::ThreadSyncronizer::LerpDebugLines(float tValue)
{
	const RenderCommandPool& readPool = m_renderCommandPools[m_currentActiveRenderPoolRead];
	const RenderCommandPool& lastReadPool = m_renderCommandPools[m_currentActiveRenderPoolLastRead];
	//TODO: Add proper support for 3D lines and sort these lists
	const uint32_t numberOfLines = readPool.debugLineCommands.Size();
	const uint32_t lastReadNumberOfLines = lastReadPool.debugLineCommands.Size();
	for (uint16_t iLine = 0; iLine < numberOfLines; iLine++)
	{
		const RenderCommand_DebugLine& readLine = readPool.debugLineCommands[iLine];
		RenderCommand_DebugLine& writeLine = m_rendererCommandPool.debugLineCommands[iLine];

		if (iLine >= lastReadNumberOfLines)
		{
			writeLine = readLine;
			continue;
		}

		const RenderCommand_DebugLine& lastReadLine = lastReadPool.debugLineCommands[iLine];
		if (lastReadLine.bLerpCommand)
		{
			LerpDebugLine(readLine, lastReadLine, writeLine, tValue);
		}
		else
		{
			writeLine = readLine;
		}
	}
}

void Hail::ThreadSyncronizer::LerpTextCommands(float tValue)
{
	const RenderCommandPool& readPool = m_renderCommandPools[m_currentActiveRenderPoolRead];
	const RenderCommandPool& lastReadPool = m_renderCommandPools[m_currentActiveRenderPoolLastRead];
	const uint32_t numberOfTexts = readPool.textCommands.Size();
	const uint32_t lastReadNumberOfTexts = lastReadPool.textCommands.Size();
	for (uint16_t iTextC = 0; iTextC < numberOfTexts; iTextC++)
	{
		const RenderCommand_Text& readText = readPool.textCommands[iTextC];
		RenderCommand_Text& writeText = m_rendererCommandPool.textCommands[iTextC];

		if (iTextC >= lastReadNumberOfTexts)
		{
			writeText = readText;
			continue;
		}

		const RenderCommand_Text& lastReadText = lastReadPool.textCommands[iTextC];
		if (lastReadText.bLerpCommand)
		{
			LerpTextCommand(readText, lastReadText, writeText, tValue);
		}
		else
		{
			writeText = readText;
		}
	}
}
