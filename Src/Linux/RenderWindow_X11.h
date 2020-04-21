//=============================================================================
// RenderWindow_X11.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

class RayTracerProgram;

class RenderWindow
{
public:
	RenderWindow();
	~RenderWindow();

	bool Create(int width, int height, bool fullscreen = false, int bpp = 32);
	void Destroy();

	void SetRenderBufferParameters(int BufferWidth, int BufferHeight, void* BufferData);

	void RunWindowLoop(RayTracerProgram* Program);

    void SetTitle(const char* Title);
private:
	struct X11WindowContext;
	X11WindowContext* Context;
};
