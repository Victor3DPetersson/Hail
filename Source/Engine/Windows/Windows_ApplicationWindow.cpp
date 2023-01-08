#include "Engine_PCH.h"

#include "Windows_ApplicationWindow.h"




#include "Windows_InputHandler.h"
#include "InputHandler.h"
#include "CrabEngine.h"

#include "imgui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Windows_ApplicationWindow::WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
	static Windows_ApplicationWindow* windowHandler = nullptr;
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;
	if (uMsg == WM_DESTROY || uMsg == WM_CLOSE)
	{
		Crab::ShutDownEngine();
		return 0;
	}
	else if (uMsg == WM_CREATE)
	{
		CREATESTRUCT* createstruct = reinterpret_cast<CREATESTRUCT*>(lParam);
		windowHandler = reinterpret_cast<Windows_ApplicationWindow*>(createstruct->lpCreateParams);
	}
	// For drag and drop this window message is needed
	// else if (uMsg == WM_DROPFILES) { 
	// 	HDROP drop = (HDROP)wParam;
	// 	UINT num_paths = DragQueryFileW(drop, 0xFFFFFFFF, 0, 512);

	// 	wchar_t* filename = nullptr;
	// 	UINT max_filename_len = 0;

	// 	for (UINT i = 0; i < num_paths; ++i) {
	// 		UINT filename_len = DragQueryFileW(drop, i, nullptr, 512) + 1;
	// 		if (filename_len > max_filename_len) {
	// 			max_filename_len = filename_len;
	// 			wchar_t* tmp = (wchar_t*)realloc(filename, max_filename_len * sizeof(*filename));
	// 			if (tmp != nullptr) {
	// 				filename = tmp;
	// 			}
	// 		}
	// 		DragQueryFileW(drop, i, filename, filename_len);
	// 		EngineInterface::GetDragNDropHandler()->Callback(filename);
	// 	}
	// 	free(filename);
	// 	DragFinish(drop);
	// }

	//ApplicationMessage message;

	switch (uMsg)
	{

	// case WM_ENTERSIZEMOVE:
	// 	message.command = EIsResizing;
	// 	SetApplicationSettings(message);
	// 	break;

	// case WM_EXITSIZEMOVE:
	// 	message.myMessage = EIsDoneResizing | EMove;
	// 	SetApplicationSettings(message);
	// 	break;
	
	// case WM_GETMINMAXINFO:
	// 	((MINMAXINFO*)lParam)->ptMinTrackSize.x = 720;
	// 	((MINMAXINFO*)lParam)->ptMinTrackSize.y = 425;
	// 	break;

	// case WM_ACTIVATE:
	// 	if (LOWORD(wParam) == WA_INACTIVE)
	// 	{
	// 		message.myMessage = EIsTabbedOut;
	// 		SetApplicationSettings(message);

	// 		globalEngineData.windowHandler.TabToggle(EIsTabbedOut);
	// 	}
	// 	else
	// 	{
	// 		message.myMessage = EIsTabbedIn;
	// 		EngineInterface::SetWindowSettings(message);

	// 		globalEngineData.windowHandler.TabToggle(EIsTabbedIn);
	// 	}
	// 	break;

	// case WM_SYSCOMMAND:
	// 	if (wParam == SC_MINIMIZE)
	// 	{
	// 		message.myMessage = EIsTabbedOut;
	// 		EngineInterface::SetWindowSettings(message);
	// 	}
	// 	if (wParam == SC_MAXIMIZE)
	// 	{
	// 		message.myMessage = EUpdateResolution_FromEvent;
	// 		EngineInterface::SetWindowSettings(message);
	// 	}
	// 	if (wParam == SC_RESTORE)
	// 	{
	// 		message.myMessage = EUpdateResolution_FromEvent;
	// 		EngineInterface::SetWindowSettings(message);
	// 	}
	// 	break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:

	case WM_SYSCHAR:
		// if (wParam == VK_RETURN)
		// {
		// 	message.myMessage = EAltTab;
		// 	EngineInterface::SetWindowSettings(message);
		// }
		break;
	default:
		break;
	}	  

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


bool Windows_ApplicationWindow::Init(StartupAttributes startupData, InputHandler* inputHandler)
{
	m_defaultWindowPosition = { startupData.startPositionX, startupData.startPositionY };

	m_windowModule = (HINSTANCE)GetModuleHandle(NULL);
	glm::uvec2 resolution = ResolutionFromEnum(startupData.startupResolution);
	m_previousSize = resolution;

	WNDCLASS windowClass = {};
	windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = Windows_ApplicationWindow::WinProc;
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.lpszClassName = L"Slask";
	//windowClass.hIcon = ::LoadIcon(nullptr, MAKEINTRESOURCE(IDI_ICON)); // for when we create an icon for the window
	RegisterClass(&windowClass);
	DWORD windowStyle = 0;
	//windowStyle = WS_POPUP | WS_VISIBLE;
	windowStyle = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE;
	m_windowHandle = CreateWindowW(L"Slask",
		_T("Slask"), //title of the window
		windowStyle, // window style
		m_defaultWindowPosition.x, m_defaultWindowPosition.y, //Position of Window
		resolution.x, resolution.y, //Size Of Window
		nullptr, // Parent Window (NULL) 
		nullptr, // extra menues (NULL) 
		nullptr, // application handle (NULL) 
		this); //Use with multiple Windows (NULL) 

	RECT rcClient, rcWind;
	POINT ptDiff;
	GetClientRect(m_windowHandle, &rcClient);
	GetWindowRect(m_windowHandle, &rcWind);
	m_windowSize.x = rcWind.right - rcWind.left;
	m_windowSize.y = rcWind.bottom - rcWind.top;

	m_borderSize.x = m_windowSize.x - rcClient.right;
	m_borderSize.y = m_windowSize.y - rcClient.bottom;

	m_frameBufferSize = m_windowSize - m_borderSize;

	SetWindowPos(m_windowHandle, HWND_NOTOPMOST, m_defaultWindowPosition.x, m_defaultWindowPosition.y, m_previousSize.x, m_previousSize.y + (int)m_borderSize.y - (int)m_borderSize.x, SWP_SHOWWINDOW);
	//if (startupData.acceptDragNDrop)
	{
		//DragAcceptFiles(m_windowHandle, TRUE);
	}

	m_inputHandler = inputHandler;
	m_windowsInputHandler = reinterpret_cast<Windows_InputHandler*>(m_inputHandler);
	m_windowsInputHandler->SetWindowHandle(m_windowHandle);

	return true;
}			

void Windows_ApplicationWindow::ApplicationUpdateLoop()
{
	if(m_runApplication)
	{
		//Recieve window messages
		m_inputHandler->Update();
		MSG windowMsg;

		while (PeekMessage(&windowMsg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&windowMsg);
			DispatchMessage(&windowMsg);

			//if (!globalEngineData.applicationPaused)
			{
				m_windowsInputHandler->ReadInputEvents(windowMsg.message, windowMsg.wParam, windowMsg.lParam);
			}
		}
	}
}

void Windows_ApplicationWindow::SetApplicationSettings(ApplicationMessage message)
{

}


glm::uvec2 Windows_ApplicationWindow::GetWindowResolution()
{
	if (m_hasBorder)
	{
		return m_frameBufferSize;
	}
	else
	{
		return m_windowSize;
	}
}

glm::uvec2 Windows_ApplicationWindow::GetWindowPosition()
{
	return glm::uvec2();
}

glm::uvec2 Windows_ApplicationWindow::GetMonitorResolution()
{
	return glm::uvec2();
}

void Windows_ApplicationWindow::InternalSetWindowPos()
{
	
}