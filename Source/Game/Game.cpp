#include "Game_PCH.h"

#include "Game.h"

#include <iostream>
#include "../Engine/ThreadSynchronizer.h"
#include "../Engine/RenderCommands.h"

#include "Camera.h"
#include "../Engine/Input/InputActionMap.h"

#include "../Engine/ImGui/ImGuiCommands.h"
#include "../Engine/ImGui/ImGuiFileBrowser.h"

#include "../Engine/HailEngine.h"
#include "../Engine/ImGui/ImGuiFileBrowser.h"

#include "../Engine/Interface/ResourceInterface.h"

#include "Utility\DebugLineHelpers.h"
#include "Reflection\Reflection.h"

namespace
{
	Hail::Camera g_camera;
	Hail::Camera2D g_2DCamera;
	float g_movementSpeed = 10.0f;
	float g_spriteMovementSpeed = 5.0f;
	Hail::RenderCommand_Sprite player;
	Hail::RenderCommand_Sprite sprites[5];
	bool renderPlayer = false;
	Hail::ImGuiFileBrowserData g_fileBrowserData;

	Hail::GUID spaceShipGuid;
	Hail::GUID backgroundGuid;
	Hail::GUID debugGridGuid;


}

namespace Hail
{
	void GameApplication::PostInit()
	{
		sprites[0].materialInstanceID = Hail::ResourceInterface::GetMaterialInstanceResourceHandle(backgroundGuid);
		player.materialInstanceID = Hail::ResourceInterface::GetMaterialInstanceResourceHandle(spaceShipGuid);
		for (size_t i = 1; i < 5; i++)
		{
			sprites[i].materialInstanceID = Hail::ResourceInterface::GetMaterialInstanceResourceHandle(debugGridGuid);
		}
	}

	void GameApplication::Init(void* initData)
	{
		spaceShipGuid.m_data1 = 3698324670;
		spaceShipGuid.m_data2 = 58616;
		spaceShipGuid.m_data3 = 18806;
		spaceShipGuid.m_data4[0] = 130;
		spaceShipGuid.m_data4[1] = 207;
		spaceShipGuid.m_data4[2] = 174;
		spaceShipGuid.m_data4[3] = 64;
		spaceShipGuid.m_data4[4] = 37;
		spaceShipGuid.m_data4[5] = 86;
		spaceShipGuid.m_data4[6] = 92;
		spaceShipGuid.m_data4[7] = 175;
		Hail::ResourceInterface::LoadMaterialInstanceResource(spaceShipGuid);
		backgroundGuid.m_data1 = 221196968;
		backgroundGuid.m_data2 = 33006;
		backgroundGuid.m_data3 = 17645;
		backgroundGuid.m_data4[0] = 180;
		backgroundGuid.m_data4[1] = 67;
		backgroundGuid.m_data4[2] = 112;
		backgroundGuid.m_data4[3] = 246;
		backgroundGuid.m_data4[4] = 44;
		backgroundGuid.m_data4[5] = 166;
		backgroundGuid.m_data4[6] = 126;
		backgroundGuid.m_data4[7] = 203;
		Hail::ResourceInterface::LoadMaterialInstanceResource(backgroundGuid);
		debugGridGuid.m_data1 = 107330914;
		debugGridGuid.m_data2 = 8412;
		debugGridGuid.m_data3 = 20452;
		debugGridGuid.m_data4[0] = 181;
		debugGridGuid.m_data4[1] = 33;
		debugGridGuid.m_data4[2] = 121;
		debugGridGuid.m_data4[3] = 115;
		debugGridGuid.m_data4[4] = 212;
		debugGridGuid.m_data4[5] = 6;
		debugGridGuid.m_data4[6] = 155;
		debugGridGuid.m_data4[7] = 118;
		Hail::ResourceInterface::LoadMaterialInstanceResource(debugGridGuid);

		g_camera.GetTransform() = glm::lookAt(glm::vec3(300.0f, 300.0f, 300.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		m_inputMapping = *reinterpret_cast<Hail::InputMapping*>(initData);
		player.transform.SetPosition({ 0.5f, 0.5f });
		player.bSizeRelativeToRenderTarget = true;
		player.transform.SetScale({ 0.285f, 0.285f });
		sprites[0].materialInstanceID = 2;
		sprites[0].transform.SetPosition({ 0.5f, 0.5f });

		sprites[1].transform.SetPosition({ -200, -200 });
		sprites[2].transform.SetPosition({ 200, 200 });
		sprites[3].transform.SetPosition({ -200, 200 });
		sprites[4].transform.SetPosition({ 200, -200 });
		for (uint32_t i = 1; i < 5; i++)
		{
			sprites[i].transform.SetRotation({ i * Math::PIf * 0.25f });
			sprites[i].index = i + 1;
			sprites[i].transform.SetScale({ 0.25, 0.25 });
		}
		g_fileBrowserData.extensionsToSearchFor = { "tga", "frag", "vert" };
		g_fileBrowserData.pathToBeginSearchingIn = RESOURCE_DIR;

	}

	bool g_bTest = false;
	float g_fTest = 0.0f;
	String64 g_sTest = "";


	void GameApplication::Update(double totalTime, float deltaTime, Hail::ApplicationFrameData& recievedFrameData)
	{
		Hail::ApplicationFrameData& frameData = recievedFrameData;
		g_2DCamera.SetResolution(frameData.renderPool->camera2D.GetResolution());
		//Make Gawme ^^
		g_fTest = recievedFrameData.imguiCommandRecorder->GetResponseValue<float>(3);
		g_sTest = recievedFrameData.imguiCommandRecorder->GetResponseValue<StringL>(4);
		if (recievedFrameData.imguiCommandRecorder->AddBeginCommand("ImGui From Game Thread", 0))
		{
			if (recievedFrameData.imguiCommandRecorder->AddButton("File Browser", 1))
			{
				g_fTest += 100.0f;
				recievedFrameData.imguiCommandRecorder->OpenFileBrowser(&g_fileBrowserData);
			}
			g_bTest = recievedFrameData.imguiCommandRecorder->GetResponseValue<bool>(2);
			recievedFrameData.imguiCommandRecorder->AddSameLine();
			recievedFrameData.imguiCommandRecorder->AddCheckbox("Checkbox", 2, g_bTest);
			recievedFrameData.imguiCommandRecorder->AddSeperator();
			if (recievedFrameData.imguiCommandRecorder->AddTreeNode("Tree", 5))
			{
				if (recievedFrameData.imguiCommandRecorder->AddTextInput("Text test", 4, g_sTest))
				{
					g_fTest -= 50.0f;
				}
				if (recievedFrameData.imguiCommandRecorder->AddTabBar("TabBar", 6))
				{
					bool anyTabOpened = false;
					if (recievedFrameData.imguiCommandRecorder->AddTabItem("TestTab1", 7))
					{
						anyTabOpened = true;
						recievedFrameData.imguiCommandRecorder->AddFloatSlider("Float slider", 3, g_fTest);
						recievedFrameData.imguiCommandRecorder->AddTabItemEnd();
					}
					if (recievedFrameData.imguiCommandRecorder->AddTabItem("TestTab2", 8))
					{
						recievedFrameData.imguiCommandRecorder->OpenMaterialEditor();
						anyTabOpened = true;
						recievedFrameData.imguiCommandRecorder->AddFloatSlider("Float slider", 3, g_fTest);
						recievedFrameData.imguiCommandRecorder->AddTabItemEnd();

						if (recievedFrameData.imguiCommandRecorder->AddTreeNode("Tree within a tree", 9))
						{
							recievedFrameData.imguiCommandRecorder->AddTreeNodeEnd();
						}
					}
					recievedFrameData.imguiCommandRecorder->AddTabBarEnd();
				}

				recievedFrameData.imguiCommandRecorder->AddTreeNodeEnd();
			}
			if (recievedFrameData.imguiCommandRecorder->AddTreeNode("Tree within a tree", 10))
			{
				recievedFrameData.imguiCommandRecorder->AddTreeNodeEnd();
			}
			const float cameraZoomResponse = recievedFrameData.imguiCommandRecorder->GetResponseValue<float>(11);
			g_2DCamera.SetZoom(cameraZoomResponse != 0.f ? cameraZoomResponse : g_2DCamera.GetZoom());
			recievedFrameData.imguiCommandRecorder->AddFloatSlider("2D Camera zoom", 11, g_2DCamera.GetZoom());

		}
		recievedFrameData.imguiCommandRecorder->AddCloseCommand();



		if (frameData.inputActionMap->GetButtonInput(eInputAction::PlayerAction1) == Hail::eInputState::Pressed)
		{
			g_camera.GetTransform().AddToPosition(glm::vec3{ 0.0, 0.0, 1.0 } *g_movementSpeed);
		}
		if (frameData.inputActionMap->GetButtonInput(eInputAction::PlayerAction2) == Hail::eInputState::Pressed)
		{
			g_camera.GetTransform().AddToPosition(glm::vec3{ 0.0, 0.0, -1.0 } *g_movementSpeed);
		}
		glm::vec2 direction = frameData.inputActionMap->GetDirectionInput(eInputAction::PlayerMoveJoystickL);
		direction.y *= (-1.0f);
		bool moved = false;


		if (direction.x != 0.0 || direction.y != 0.0f)
			moved = true;

		{
			const glm::vec2 oldCameraPos = g_2DCamera.GetPosition();
			const glm::vec2 directionRStick = frameData.inputActionMap->GetDirectionInput(eInputAction::PlayerMoveJoystickR);
			g_2DCamera.SetPosition(oldCameraPos + glm::vec2(directionRStick.x, directionRStick.y * -1.0f) * 10.f);
		}

		if (frameData.inputActionMap->GetButtonInput(eInputAction::PlayerMoveLeft) == Hail::eInputState::Down)
		{
			direction += (glm::vec2{ -1.0, 0.0 });
			moved = true;
		}
		if (frameData.inputActionMap->GetButtonInput(eInputAction::PlayerMoveRight) == Hail::eInputState::Down)
		{
			direction += (glm::vec2{ 1.0, 0.0 });
			moved = true;
		}
		if (frameData.inputActionMap->GetButtonInput(eInputAction::PlayerMoveUp) == Hail::eInputState::Down)
		{
			direction += (glm::vec2{ 0.0, -1.0 });
			moved = true;
		}
		if (frameData.inputActionMap->GetButtonInput(eInputAction::PlayerMoveDown) == Hail::eInputState::Down)
		{
			direction += glm::vec2{ 0.0, 1.0 };
			moved = true;
		}

		if (moved)
		{
			direction = glm::normalize(direction);
			player.transform.AddToPosition(direction * g_spriteMovementSpeed);
			player.transform.LookAt(direction);
		}
		if (frameData.inputActionMap->GetButtonInput(eInputAction::PlayerPause) == Hail::eInputState::Released)
		{
			renderPlayer = !renderPlayer;
		}

		if (frameData.inputActionMap->GetButtonInput(eInputAction::DebugAction1) == Hail::eInputState::Released)
		{
			H_ERROR("Error frome game");
		}
		if (frameData.inputActionMap->GetButtonInput(eInputAction::DebugAction2) == Hail::eInputState::Released)
		{
			H_WARNING("Warning frome game");
		}

		player.color.x = sinf(totalTime) * 0.5 + 0.5f;

		for (uint32_t i = 1; i < 5; i++)
		{
			sprites[i].transform.AddToRotation({ 0.15f * (i + 1) });
			DrawLine2DPixelSpace(*frameData.renderPool, sprites[i].transform.GetPosition(), sprites[i].transform.GetRotationRad(), 10.f * (i * 10.f), true);
		}
		FillFrameData(*frameData.renderPool);
	}

	void GameApplication::Shutdown()
	{

	}


	void GameApplication::FillFrameData(Hail::RenderCommandPool& renderCommandPoolToFill)
	{
		renderCommandPoolToFill.camera3D = g_camera;
		for (uint32_t i = 0; i < 5; i++)
		{
			renderCommandPoolToFill.spriteCommands.Add(sprites[i]);
		}
		if (renderPlayer)
		{
			renderCommandPoolToFill.spriteCommands.Add(player);

			DrawLine2DPixelSpace(renderCommandPoolToFill, player.transform.GetPosition(), player.transform.GetRotationRad(), g_spriteMovementSpeed * 20.0, true);
			DrawCircle2DPixelSpace(renderCommandPoolToFill, player.transform.GetPosition(), 10.0f, false);
			DrawBox2DPixelSpace(renderCommandPoolToFill, player.transform.GetPosition(), glm::vec2(40.0f, 40.0f), false, glm::vec4(1.0, 0.0, 0.0, 1.0f));
			const glm::vec2 halfDimensions = glm::vec2(10.0, 10.0);
			DrawBox2DMinMaxPixelSpace(renderCommandPoolToFill, player.transform.GetPosition() - halfDimensions, player.transform.GetPosition() + halfDimensions, true, glm::vec4(1.0, 0.6, 0.0, 1.0f));
		}
		renderCommandPoolToFill.camera2D = g_2DCamera;
		renderCommandPoolToFill.meshCommands.Add(Hail::RenderCommand_Mesh());

	}

}