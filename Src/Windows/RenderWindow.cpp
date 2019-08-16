//=============================================================================
// RenderWindow.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#include "RenderWindow.h"
#include <tchar.h>
#include "../RayTracerProgram.h"

static const TCHAR s_WindowClassName[] = _T("RenderWindow");
static const TCHAR s_WindowTitle[] = _T("Ray Tracer");

// Windows callback function
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		// share post quit message with WM_CLOSE
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		break;
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

bool RenderWindow::Create(int width, int height, bool fullscreen /*= false*/, int bpp /*= 32*/)
{
	SetProcessDPIAware();

	m_Instance = GetModuleHandle(NULL);
	m_bFullScreen = fullscreen;

	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_Instance;
	wcex.hIcon = LoadIcon(m_Instance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = s_WindowClassName;

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Hourglass Engine"),
			NULL);

		return false;
	}

	// Window style: WS_POPUP				- No caption, no maximize/minimize buttons
	//				 WS_OVERLAPPEDWINDOW	- Normal window
	DWORD dwStyle = m_bFullScreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;

	// Adjust window size according to window style.
	// This will make sure correct client area.
	RECT win_rect = { 0, 0, width, height };
	AdjustWindowRect(&win_rect, dwStyle, false);

	int pos_x, pos_y;

	if (m_bFullScreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)width;
		dmScreenSettings.dmPelsHeight = (unsigned long)height;
		dmScreenSettings.dmBitsPerPel = bpp;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		pos_x = pos_y = 0;
	}
	else
	{
		pos_x = (GetSystemMetrics(SM_CXSCREEN) - (win_rect.right - win_rect.left)) / 2;
		pos_y = (GetSystemMetrics(SM_CYSCREEN) - (win_rect.bottom - win_rect.top)) / 2;
	}

	// Create window and validate
	m_Window = CreateWindow(
		s_WindowClassName,
		s_WindowTitle,
		dwStyle,
		pos_x, pos_y,
		win_rect.right - win_rect.left,
		win_rect.bottom - win_rect.top,
		NULL,
		NULL,
		m_Instance,
		NULL);

	if (!m_Window)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Hourglass Engine"),
			NULL);

		return false;
	}

	m_WindowDC = GetDC(m_Window);

	ShowWindow(m_Window, SW_SHOW);
	SetForegroundWindow(m_Window);
	SetFocus(m_Window);

	return true;
}

void RenderWindow::Destroy()
{
	if (m_bFullScreen)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(m_Window);
	m_Window = NULL;
	UnregisterClass(s_WindowClassName, m_Instance);
	m_Instance = NULL;
}


void RenderWindow::SetRenderBufferParameters(int BufferWidth, int BufferHeight, void* BufferData)
{
	m_BufferWidth = BufferWidth;
	m_BufferHeight = BufferHeight;
	m_BufferData = BufferData;
}

void RenderWindow::PresentRenderBuffer()
{
	BITMAPINFO	info;
	ZeroMemory(&info, sizeof(BITMAPINFO));
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = m_BufferWidth;
	info.bmiHeader.biHeight = -int(m_BufferHeight); // flip
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	SetDIBitsToDevice(m_WindowDC, 0, 0, m_BufferWidth, m_BufferHeight, 0, 0, 0, m_BufferHeight, m_BufferData, &info, DIB_RGB_COLORS);
}

void RenderWindow::RunWindowLoop(RayTracerProgram* Program)
{
	MSG msg;
	bool quit = false;

	while (!quit)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				quit = true;
			}
			else
			{
				// Translate the message and dispatch it to WndProc()
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		Sleep(10);
		PresentRenderBuffer();
	}

	Program->ExecuteCleanup();
}

LPCTSTR RenderWindow::GetTitle() const
{
	return s_WindowTitle;
}

void RenderWindow::SetTitle(const char* Title)
{
	SetWindowTextA(m_Window, Title);
}


