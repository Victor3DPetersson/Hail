#include "Game_PCH.h"

#include "Game.h"

#include <iostream>
#include "../Engine/ThreadSynchronizer.h"
#include "../Engine/RenderCommands.h"

#include "Camera.h"

#include "../Engine/ImGui/ImGuiCommands.h"
namespace
{
	Hail::Camera g_camera;
	float g_movementSpeed = 10.0f;
	float g_spriteMovementSpeed = 0.005f;
	Hail::RenderCommand_Sprite player;
	Hail::RenderCommand_Sprite sprites[5];
	bool renderPlayer = false;
}

void GameApplication::Init(void* initData)
{
	g_camera.GetTransform() = glm::lookAt(glm::vec3(300.0f, 300.0f, 300.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_inputMapping = *reinterpret_cast<Hail::InputMapping*>(initData);
	player.transform.SetPosition({ 0.5f, 0.5f });
	player.materialInstanceID = 1;
	for (uint32_t i = 0; i < 5; i++)
	{
		sprites[i].transform.SetPosition({ 0.1f + 0.15f * i, 0.5f });
		sprites[i].transform.SetRotation({ i * Math::PIf * 0.25f });
		sprites[i].index = i + 1;
	}
}

bool g_bTest = false;
float g_fTest = 0.0f;
String256 g_sTest = "";

void GameApplication::Update(double totalTime, float deltaTime, Hail::ApplicationFrameData& recievedFrameData)
{
	Hail::ApplicationFrameData& frameData = recievedFrameData;
	//Make Gawme ^^

	g_fTest = recievedFrameData.imguiCommandRecorder->GetResponseValue<float>(3);
	g_sTest = recievedFrameData.imguiCommandRecorder->GetResponseValue<String256>(4);
	if(recievedFrameData.imguiCommandRecorder->AddBeginCommand("Will this work?", 0))
	{
		if (recievedFrameData.imguiCommandRecorder->AddButton("Button 1", 1))
		{
			g_fTest += 100.0f;
		}
		g_bTest = recievedFrameData.imguiCommandRecorder->GetResponseValue<bool>(2);
		recievedFrameData.imguiCommandRecorder->SameLine();
		recievedFrameData.imguiCommandRecorder->AddCheckbox("Check this out", 2, g_bTest);
		recievedFrameData.imguiCommandRecorder->Seperator();
		if (recievedFrameData.imguiCommandRecorder->AddTextInput("Text test", 4, g_sTest))
		{
			g_fTest -= 50.0f;
		}
		recievedFrameData.imguiCommandRecorder->AddFloatSlider("FloatyMcFloatFace", 3, g_fTest);
	}
	recievedFrameData.imguiCommandRecorder->AddCloseCommand();



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

	for (uint32_t i = 0; i < 5; i++)
	{
		sprites[i].transform.AddToRotation({  0.025f * (i + 1) });
	}

	FillFrameData(*frameData.renderPool);
}

void GameApplication::Shutdown()
{
	
}


void GameApplication::FillFrameData(Hail::RenderCommandPool& renderCommandPoolToFill)
{
	renderCommandPoolToFill.renderCamera = g_camera;
	for (uint32_t i = 0; i < 5; i++)
	{
		renderCommandPoolToFill.spriteCommands.Add(sprites[i]);
	}
	if (renderPlayer)
	{
		renderCommandPoolToFill.spriteCommands.Add(player);
	}

	renderCommandPoolToFill.meshCommands.Add(Hail::RenderCommand_Mesh());
}

