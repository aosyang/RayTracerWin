//=============================================================================
// RenderWindow.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include <windows.h>

class RenderWindow
{
public:
	bool Create(int width, int height, bool fullscreen = false, int bpp = 32);
	void Destroy();

	void SetRenderBufferParameters(int BufferWidth, int BufferHeight, void* BufferData);

	void RunWindowLoop();

	HWND GetHwnd() const { return m_Window; }
	LPCTSTR GetTitle() const;
	void SetTitle(const char* Title);

protected:
	void PresentRenderBuffer();

private:
	HINSTANCE		m_Instance;			// Win32 application instance
	HWND			m_Window;			// Win32 window handle
	HDC				m_WindowDC;
	bool			m_bFullScreen;

	int				m_BufferWidth;
	int				m_BufferHeight;
	void*			m_BufferData;
};
