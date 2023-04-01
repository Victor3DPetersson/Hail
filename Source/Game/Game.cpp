#include "Game_PCH.h"

#include "Game.h"

#include <iostream>
#include "../Engine/ThreadSynchronizer.h"
#include "../Engine/RenderCommands.h"

#include "Camera.h"
namespace
{
	Hail::Camera g_camera;
	float g_movementSpeed = 10.0f;
	float g_spriteMovementSpeed = 0.005f;
	Hail::RenderCommand_Sprite player;
	bool renderPlayer = false;
}

void GameApplication::Init(void* initData)
{
	g_camera.GetTransform() = glm::lookAt(glm::vec3(300.0f, 300.0f, 300.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_inputMapping = *reinterpret_cast<Hail::InputMapping*>(initData);
	player.transform.SetPosition({ 0.5f, 0.5f });
}

void GameApplication::Update(double totalTime, float deltaTime, void* recievedFrameData)
{
	Hail::ApplicationFrameData& frameData = *reinterpret_cast<Hail::ApplicationFrameData*>(recievedFrameData);
	//Make Gawme ^^

	if (frameData.inputData.keyMap[m_inputMapping.Q] == Hail::KEY_PRESSED)
	{
		g_camera.GetTransform().AddToPosition(glm::vec3{ 0.0, 0.0, 1.0 } * g_movementSpeed);
	}
	if (frameData.inputData.keyMap[m_inputMapping.E] == Hail::KEY_PRESSED)
	{
		g_camera.GetTransform().AddToPosition(glm::vec3{ 0.0, 0.0, -1.0 } * g_movementSpeed);
	}
	glm::vec2 direction = { 0.0, 0.0 };
	bool moved = false;
	if (frameData.inputData.keyMap[m_inputMapping.A] == Hail::KEY_PRESSED)
	{
		direction += (glm::vec2{ -1.0, 0.0 } * g_spriteMovementSpeed);
		moved = true;
	}
	if (frameData.inputData.keyMap[m_inputMapping.D] == Hail::KEY_PRESSED)
	{
		direction += (glm::vec2{ 1.0, 0.0 } * g_spriteMovementSpeed);
		moved = true;
	}
	if (frameData.inputData.keyMap[m_inputMapping.W] == Hail::KEY_PRESSED)
	{
		direction += (glm::vec2{ 0.0, 1.0 } * g_spriteMovementSpeed);
		moved = true;
	}
	if (frameData.inputData.keyMap[m_inputMapping.S] == Hail::KEY_PRESSED)
	{
		direction += glm::vec2{ 0.0, -1.0 } * g_spriteMovementSpeed;
		moved = true;
	}
	player.transform.AddToPosition(direction);
	if (moved)
	{
		player.transform.LookAt(glm::normalize(direction));
	}
	if (frameData.inputData.keyMap[m_inputMapping.SPACE] == Hail::KEY_RELEASED)
	{
		renderPlayer = !renderPlayer;
	}

	player.color.r = sinf(totalTime) * 0.5 + 0.5f;

	FillFrameData(*frameData.renderPool);
}

void GameApplication::Shutdown()
{
	
}


void GameApplication::FillFrameData(Hail::RenderCommandPool& renderCommandPoolToFill)
{
	renderCommandPoolToFill.renderCamera = g_camera;
	if (renderPlayer)
	{
		renderCommandPoolToFill.spriteCommands.Add(player);
	}
}

