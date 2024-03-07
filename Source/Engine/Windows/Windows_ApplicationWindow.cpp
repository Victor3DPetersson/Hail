#include "Engine_PCH.h"

#include "Windows_ApplicationWindow.h"




#include "Windows_InputHandler.h"
#include "Input/InputHandler.h"
#include "HailEngine.h"

#include "imgui.h"

using namespace Hail;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Hail::Windows_ApplicationWindow::WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
	static Hail::Windows_ApplicationWindow* windowHandler = nullptr;
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;
	if (uMsg == WM_DESTROY || uMsg == WM_CLOSE)
	{
		Hail::ShutDownEngine();
		return 0;
	}
	else if (uMsg == WM_CREATE)
	{
		CREATESTRUCT* createstruct = reinterpret_cast<CREATESTRUCT*>(lParam);
		windowHandler = reinterpret_cast<Hail::Windows_ApplicationWindow*>(createstruct->lpCreateParams);
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

	Hail::ApplicationMessage message;

	switch (uMsg)
	{

	 case WM_ENTERSIZEMOVE:
	 	break;

	 case WM_EXITSIZEMOVE:
	 	message.command |= Hail::MOVE_WINDOW;
	 	break;
	
	 case WM_GETMINMAXINFO:
	 	((MINMAXINFO*)lParam)->ptMinTrackSize.x = 720;
	 	((MINMAXINFO*)lParam)->ptMinTrackSize.y = 425;
	 	break;

	 case WM_ACTIVATE:
	 	if (LOWORD(wParam) == WA_INACTIVE)
	 	{
	 		message.command |= Hail::DROP_FOCUS;
	 	}
	 	else
	 	{
			message.command |= Hail::RESTORE_FOCUS;
	 	}
	 	break;

	 case WM_SYSCOMMAND:
	 	if (wParam == SC_MINIMIZE)
	 	{
			message.command |= Hail::MINIMIZE;
	 	}
	 	if (wParam == SC_MAXIMIZE)
	 	{
	 		message.command |= Hail::MAXIMIZE;
	 	}
	 	if (wParam == SC_RESTORE)
	 	{
			message.command |= Hail::RESTORE;
	 	}
	 	break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:

	case WM_SYSCHAR:
		 //if (wParam == VK_RETURN)
		 //{
			// message.command |= Hail::DROP_FOCUS;
		 //}
		break;
	default:
		break;
	}	  
	Hail::HandleApplicationMessage(message);
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


bool Hail::Windows_ApplicationWindow::Init(StartupAttributes startupData, Hail::InputHandler* inputHandler)
{
	m_defaultWindowPosition = { startupData.startPositionX, startupData.startPositionY };

	m_windowModule = (HINSTANCE)GetModuleHandle(NULL);
	glm::uvec2 resolution = ResolutionFromEnum(startupData.startupWindowResolution);
	m_previousSize = resolution;

	WNDCLASS windowClass = {};
	windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = Windows_ApplicationWindow::WinProc;
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.lpszClassName = L"Hail"; // TODO, replace with actual name from startup message
	//windowClass.hIcon = ::LoadIcon(nullptr, MAKEINTRESOURCE(IDI_ICON)); // for when we create an icon for the window
	RegisterClass(&windowClass);
	DWORD windowStyle = 0;
	//windowStyle = WS_POPUP | WS_VISIBLE;
	windowStyle = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE;
	m_windowHandle = CreateWindowW(L"Hail",
		_T("Hail"), //title of the window
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
	SetWindowPos(m_windowHandle, HWND_NOTOPMOST, m_defaultWindowPosition.x, m_defaultWindowPosition.y, m_previousSize.x, m_previousSize.y + (int)m_borderSize.y - (int)m_borderSize.x, SWP_SHOWWINDOW);

	m_frameBufferSize = m_windowSize - m_borderSize;
	m_previousFrameBufferSize = m_frameBufferSize;
	//if (startupData.acceptDragNDrop)
	{
		//DragAcceptFiles(m_windowHandle, TRUE);
	}

	m_inputHandler = inputHandler;
	m_windowsInputHandler = reinterpret_cast<Hail::Windows_InputHandler*>(m_inputHandler);
	m_windowsInputHandler->SetWindowHandle(m_windowHandle);

	return true;
}			

void Hail::Windows_ApplicationWindow::ApplicationUpdateLoop()
{

	//Recieve window messages
	MSG windowMsg;
	while (PeekMessage(&windowMsg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&windowMsg);
		DispatchMessage(&windowMsg);

		if (m_hasFocus)
		{
			m_windowsInputHandler->ReadInputEvents(windowMsg.message, windowMsg.wParam, windowMsg.lParam);
		}
	}
}

void Hail::Windows_ApplicationWindow::SetApplicationSettings(Hail::ApplicationMessage message)
{
	uint32_t messageCommand = static_cast<uint32_t>(message.command);
	if (messageCommand & static_cast<uint32_t>(Hail::APPLICATION_COMMAND::TOGGLE_FULLSCREEN))
	{
		if (m_isFullScreen)
		{
			m_windowSize = m_previousSize;
			m_hasBorder = m_previousHasBorder;
		}
		else
		{
			m_hasBorder = m_previousHasBorder;
			m_previousSize = m_windowSize;
			RECT desktop;
			// Get a handle to the desktop window
			const HWND hDesktop = GetDesktopWindow();
			// Get the size of screen to the variable desktop
			GetWindowRect(hDesktop, &desktop);
			m_windowSize.x = desktop.right;
			m_windowSize.y = desktop.bottom;
		}
		m_isFullScreen = !m_isFullScreen;
		InternalSetWindowPos();
	}
	if (messageCommand & static_cast<uint32_t>(Hail::APPLICATION_COMMAND::SET_RESOLUTION))
	{

	}
	if (messageCommand & static_cast<uint32_t>(Hail::APPLICATION_COMMAND::TOGGLE_BORDER))
	{

	}
	if (messageCommand & static_cast<uint32_t>(Hail::APPLICATION_COMMAND::MINIMIZE))
	{
		m_previousSize = m_windowSize;
		m_windowSize = { 0, 0 };
		m_previousHasBorder = m_hasBorder;
		m_previousFrameBufferSize = m_frameBufferSize;
		m_frameBufferSize = { 0, 0 };

	}
	if (messageCommand & static_cast<uint32_t>(Hail::APPLICATION_COMMAND::MAXIMIZE))
	{
		m_previousSize = m_windowSize;
		m_previousFrameBufferSize = m_frameBufferSize;
		const HWND hDesktop = GetDesktopWindow();
		RECT desktop;
		// Get the size of screen to the variable desktop
		GetWindowRect(hDesktop, &desktop);
		m_windowSize = { desktop.right, desktop.bottom };
		m_frameBufferSize = m_windowSize - m_borderSize;
	}
	if (messageCommand & static_cast<uint32_t>(Hail::APPLICATION_COMMAND::RESTORE))
	{
		m_windowSize = m_previousSize;
		m_hasBorder = m_previousHasBorder;
		m_frameBufferSize = m_previousFrameBufferSize;
		InternalSetWindowPos();
	}
	if (messageCommand & static_cast<uint32_t>(Hail::APPLICATION_COMMAND::MOVE_WINDOW))
	{
		RECT window;
		GetWindowRect(m_windowHandle, &window);
		if (m_windowSize.x != window.right - window.left || m_windowSize.y != window.bottom - window.top)
		{
			m_previousSize = m_windowSize;
			m_windowSize.x = window.right - window.left;
			m_windowSize.y = window.bottom - window.top;
			m_previousFrameBufferSize = m_frameBufferSize;
			m_frameBufferSize = m_windowSize - m_borderSize;
		}

		if (m_defaultWindowPosition.x != window.left || m_defaultWindowPosition.y != window.top)
		{
			m_defaultWindowPosition.x = window.left;
			m_defaultWindowPosition.y = window.top;
		}

		InternalSetWindowPos();

	}
	if (messageCommand & static_cast<uint32_t>(Hail::APPLICATION_COMMAND::DROP_FOCUS))
	{

	}
	if (messageCommand & static_cast<uint32_t>(Hail::APPLICATION_COMMAND::RESTORE_FOCUS))
	{
		if (m_inputHandler)
			m_inputHandler->ResetKeyStates();
	}
}


glm::uvec2 Hail::Windows_ApplicationWindow::GetWindowResolution()
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

glm::uvec2 Hail::Windows_ApplicationWindow::GetWindowPosition()
{
	return glm::uvec2();
}

glm::uvec2 Hail::Windows_ApplicationWindow::GetMonitorResolution()
{
	return glm::uvec2();
}

void Hail::Windows_ApplicationWindow::InternalSetWindowPos()
{
	if (m_isFullScreen)
	{
		DWORD windowStyle = WS_POPUP | WS_VISIBLE;
		SetWindowLongPtr(m_windowHandle, GWL_STYLE, windowStyle);
		SetWindowPos(m_windowHandle, HWND_TOPMOST, 0, 0, m_windowSize.x, m_windowSize.y, SWP_SHOWWINDOW);
	}
	else
	{
		if (m_hasBorder)
		{
			DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE;
			SetWindowLongPtr(m_windowHandle, GWL_STYLE, windowStyle);
			m_frameBufferSize = m_windowSize - m_borderSize;
		}
		else
		{
			DWORD windowStyle = WS_POPUP | WS_VISIBLE;
			SetWindowLongPtr(m_windowHandle, GWL_STYLE, windowStyle);
		}
		SetWindowPos(m_windowHandle, HWND_NOTOPMOST, m_defaultWindowPosition.x, m_defaultWindowPosition.y, m_windowSize.x, m_windowSize.y, SWP_SHOWWINDOW);
	}

}