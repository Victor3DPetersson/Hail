#include "Game_PCH.h"

#include "Game.h"

#include <iostream>
#include "ThreadSynchronizer.h"

#include "Camera.h"
namespace
{
	Hail::Camera g_camera;
	float g_movementSpeed = 10.0f;
}

void GameApplication::Init(void* initData)
{
	g_camera.GetTransform() = glm::lookAt(glm::vec3(300.0f, 300.0f, 300.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_inputMapping = *reinterpret_cast<Hail::InputMapping*>(initData);
}

void GameApplication::Update(float deltaTime, void* recievedFrameData)
{
	Hail::ApplicationFrameData& frameData = *reinterpret_cast<Hail::ApplicationFrameData*>(recievedFrameData);
	
	if (frameData.inputData.keyMap[m_inputMapping.B] == Hail::KEY_RELEASED)
	{
		Debug_PrintConsoleString64(String64::Format("%u : Game Released", m_inputMapping.B));
	}
	if (frameData.inputData.keyMap[m_inputMapping.B] == Hail::KEY_PRESSED)
	{
		Debug_PrintConsoleString64(String64::Format("%u : Game Pressed", m_inputMapping.B));
	}

	if (frameData.inputData.keyMap[m_inputMapping.W] == Hail::KEY_PRESSED)
	{
		g_camera.GetTransform().AddToPosition(glm::vec3{ 0.0, 0.0, 1.0 } * g_movementSpeed);
	}
	if (frameData.inputData.keyMap[m_inputMapping.S] == Hail::KEY_PRESSED)
	{
		g_camera.GetTransform().AddToPosition(glm::vec3{ 0.0, 0.0, -1.0 } *g_movementSpeed);
	}
	//Make Gawme ^^
	FillFrameData(*frameData.renderPool);
}

void GameApplication::Shutdown()
{
	
}


void GameApplication::FillFrameData(Hail::RenderCommandPool& renderCommandPoolToFill)
{
	renderCommandPoolToFill.renderCamera = g_camera;
}

