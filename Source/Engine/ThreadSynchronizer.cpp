#include "Engine_PCH.h"
#include "ThreadSynchronizer.h"
#include "InputHandler.h"
#include "glm\common.hpp"

void Hail::ThreadSyncronizer::Init(float tickTimer)
{
	m_engineTickRate = tickTimer;
	m_currentActiveRenderPoolWrite = 0;
	m_currentActiveRenderPoolRead = 1;
	m_currentActiveRenderPoolLastRead = 2;
	m_appData.renderPool = &m_renderCommandPools[m_currentActiveRenderPoolWrite];
}

void Hail::ThreadSyncronizer::SynchronizeAppData(InputHandler& inputHandler, ImGuiCommandRecorder& imguiCommandRecorder)
{
	SwapBuffersInternal();
	ClearApplicationBuffers();
	m_appData.inputData = inputHandler.GetInputMap();
	m_appData.imguiCommandRecorder = &imguiCommandRecorder;
	m_currentRenderTimer = 0.0f;
}

void Hail::ThreadSyncronizer::SynchronizeRenderData(float frameDeltaTime)
{
	m_currentRenderTimer += frameDeltaTime;
	LerpRenderBuffers();
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
}

void Hail::ThreadSyncronizer::LerpRenderBuffers()
{
	TransferBufferSizes();
	float tValue = m_currentRenderTimer / m_engineTickRate;
	const RenderCommandPool& readPool = m_renderCommandPools[m_currentActiveRenderPoolRead];
	const RenderCommandPool& lastReadPool = m_renderCommandPools[m_currentActiveRenderPoolLastRead];
	m_rendererCommandPool.renderCamera = Camera::LerpCamera(readPool.renderCamera, lastReadPool.renderCamera, tValue);
	
	LerpSprites(tValue);
	Lerp3DModels(tValue);
}
namespace Hail
{
	void LerpSpriteCommand(const RenderCommand_Sprite& readSprite, const RenderCommand_Sprite& lastReadSprite, RenderCommand_Sprite& writeSprite, float t);
	void LerpMeshCommand(const RenderCommand_Mesh& readMesh, const RenderCommand_Mesh& lastReadMesh, RenderCommand_Mesh& writeMesh, float t);
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
		if (readSprite.lerpCommand)
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
		if (readMesh.lerpCommand)
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
					if (readMesh.index == lastReadPool.spriteCommands[missingMesh].index)
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


void Hail::LerpSpriteCommand(const RenderCommand_Sprite& readSprite, const RenderCommand_Sprite& lastReadSprite, RenderCommand_Sprite& writeSprite, float t)
{
	writeSprite.transform = Transform2D::LerpTransforms(readSprite.transform, lastReadSprite.transform, t);
	writeSprite.uvTR_BL = glm::mix(readSprite.uvTR_BL, lastReadSprite.uvTR_BL, t);
	writeSprite.color = glm::mix(readSprite.color, lastReadSprite.color, t);
	writeSprite.pivot = glm::mix(readSprite.pivot, lastReadSprite.pivot, t);
	writeSprite.index = readSprite.index;
	writeSprite.materialInstanceID = readSprite.materialInstanceID;
	writeSprite.lerpCommand = readSprite.lerpCommand;
}

void Hail::LerpMeshCommand(const RenderCommand_Mesh& readMesh, const RenderCommand_Mesh& lastReadMesh, RenderCommand_Mesh& writeMesh, float t)
{
	writeMesh.transform = Transform3D::LerpTransforms_t(readMesh.transform, lastReadMesh.transform, t);
	writeMesh.color = glm::mix(readMesh.color, lastReadMesh.color, t);
	writeMesh.meshID = readMesh.meshID;
	writeMesh.index = readMesh.index;
	writeMesh.materialInstanceID = readMesh.materialInstanceID;
	writeMesh.lerpCommand = readMesh.lerpCommand;
}