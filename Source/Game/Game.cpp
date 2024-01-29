#include "Game_PCH.h"

#include "Game.h"

#include <iostream>
#include "../Engine/ThreadSynchronizer.h"
#include "../Engine/RenderCommands.h"

#include "Camera.h"

#include "../Engine/ImGui/ImGuiCommands.h"
#include "../Engine/ImGui/ImGuiFileBrowser.h"

#include "../Engine/HailEngine.h"
#include "../Engine/ImGui/ImGuiFileBrowser.h"

#include "ResourceInterface.h"

#include "Utility\DebugLineHelpers.h"
#include "Reflection\Reflection.h"

namespace
{
	Hail::Camera g_camera;
	float g_movementSpeed = 10.0f;
	float g_spriteMovementSpeed = 0.005f;
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
		spaceShipGuid.m_data1 = 217501042;
		spaceShipGuid.m_data2 = 62551;
		spaceShipGuid.m_data3 = 17719;
		spaceShipGuid.m_data4[0] = 158;
		spaceShipGuid.m_data4[1] = 133;
		spaceShipGuid.m_data4[2] = 106;
		spaceShipGuid.m_data4[3] = 192;
		spaceShipGuid.m_data4[4] = 197;
		spaceShipGuid.m_data4[5] = 0;
		spaceShipGuid.m_data4[6] = 17;
		spaceShipGuid.m_data4[7] = 61;
		Hail::ResourceInterface::LoadMaterialInstanceResource(spaceShipGuid);
		backgroundGuid.m_data1 = 2552708824;
		backgroundGuid.m_data2 = 38327;
		backgroundGuid.m_data3 = 17069;
		backgroundGuid.m_data4[0] = 165;
		backgroundGuid.m_data4[1] = 33;
		backgroundGuid.m_data4[2] = 142;
		backgroundGuid.m_data4[3] = 48;
		backgroundGuid.m_data4[4] = 122;
		backgroundGuid.m_data4[5] = 219;
		backgroundGuid.m_data4[6] = 156;
		backgroundGuid.m_data4[7] = 161;
		Hail::ResourceInterface::LoadMaterialInstanceResource(backgroundGuid);
		debugGridGuid.m_data1 = 2671171390;
		debugGridGuid.m_data2 = 34019;
		debugGridGuid.m_data3 = 18305;
		debugGridGuid.m_data4[0] = 133;
		debugGridGuid.m_data4[1] = 20;
		debugGridGuid.m_data4[2] = 248;
		debugGridGuid.m_data4[3] = 102;
		debugGridGuid.m_data4[4] = 102;
		debugGridGuid.m_data4[5] = 1;
		debugGridGuid.m_data4[6] = 161;
		debugGridGuid.m_data4[7] = 37;
		Hail::ResourceInterface::LoadMaterialInstanceResource(debugGridGuid);

		g_camera.GetTransform() = glm::lookAt(glm::vec3(300.0f, 300.0f, 300.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		m_inputMapping = *reinterpret_cast<Hail::InputMapping*>(initData);
		player.transform.SetPosition({ 0.5f, 0.5f });
		player.sizeRelativeToRenderTarget = true;
		player.transform.SetScale({ 0.085f, 0.085f });
		sprites[0].materialInstanceID = 2;
		sprites[0].transform.SetPosition({ 0.5f, 0.5f });

		sprites[1].transform.SetPosition({ 0.25f, 0.25f });
		sprites[2].transform.SetPosition({ 0.75f, 0.25f });
		sprites[3].transform.SetPosition({ 0.25f, 0.75f });
		sprites[4].transform.SetPosition({ 0.75f, 0.75f });
		for (uint32_t i = 1; i < 5; i++)
		{
			sprites[i].transform.SetRotation({ i * Math::PIf * 0.25f });
			sprites[i].index = i + 1;
			sprites[i].transform.SetScale({ 0.25, 0.25 });
		}
		g_fileBrowserData.objectsToSelect.Init(16);
		g_fileBrowserData.extensionsToSearchFor = { "tga", "frag", "vert" };
		g_fileBrowserData.pathToBeginSearchingIn = RESOURCE_DIR;

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

		}
		recievedFrameData.imguiCommandRecorder->AddCloseCommand();



		if (frameData.inputData.keyMap[m_inputMapping.Q] == Hail::KEY_PRESSED)
		{
			g_camera.GetTransform().AddToPosition(glm::vec3{ 0.0, 0.0, 1.0 } *g_movementSpeed);
		}
		if (frameData.inputData.keyMap[m_inputMapping.E] == Hail::KEY_PRESSED)
		{
			g_camera.GetTransform().AddToPosition(glm::vec3{ 0.0, 0.0, -1.0 } *g_movementSpeed);
		}
		glm::vec2 direction = { 0.0, 0.0 };
		bool moved = false;
		if (frameData.inputData.keyMap[m_inputMapping.A] == Hail::KEY_PRESSED)
		{
			direction += (glm::vec2{ -1.0, 0.0 });
			moved = true;
		}
		if (frameData.inputData.keyMap[m_inputMapping.D] == Hail::KEY_PRESSED)
		{
			direction += (glm::vec2{ 1.0, 0.0 });
			moved = true;
		}
		if (frameData.inputData.keyMap[m_inputMapping.W] == Hail::KEY_PRESSED)
		{
			direction += (glm::vec2{ 0.0, -1.0 });
			moved = true;
		}
		if (frameData.inputData.keyMap[m_inputMapping.S] == Hail::KEY_PRESSED)
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
		if (frameData.inputData.keyMap[m_inputMapping.SPACE] == Hail::KEY_RELEASED)

		{
			renderPlayer = !renderPlayer;
		}

		player.color.r = sinf(totalTime) * 0.5 + 0.5f;

		for (uint32_t i = 1; i < 5; i++)
		{
			sprites[i].transform.AddToRotation({ 0.15f * (i + 1) });
			DrawLine2D(*frameData.renderPool, sprites[i].transform.GetPosition(), sprites[i].transform.GetRotationRad(), 0.05f * (i + 1));
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

			DrawLine2D(renderCommandPoolToFill, player.transform.GetPosition(), player.transform.GetRotationRad(), g_spriteMovementSpeed * 20.0);
			DrawCircle2D(renderCommandPoolToFill, player.transform.GetPosition(), 0.05);
			DrawBox2D(renderCommandPoolToFill, player.transform.GetPosition(), glm::vec2(0.05, 0.05), glm::vec4(1.0, 0.0, 0.0, 1.0f));
			const glm::vec2 halfDimensions = glm::vec2(0.05, 0.05);
			DrawBox2DMinMax(renderCommandPoolToFill, player.transform.GetPosition() - halfDimensions, player.transform.GetPosition() + halfDimensions, glm::vec4(1.0, 0.6, 0.0, 1.0f));
		}

		renderCommandPoolToFill.meshCommands.Add(Hail::RenderCommand_Mesh());

	}

}