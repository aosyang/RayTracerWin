//=============================================================================
// OSXWindow.h by Shiyang Ao, 2017 All Rights Reserved.
//
//
//=============================================================================

#pragma once

class RayTracerProgram;

class RenderWindow
{
public:
    bool Create(int width, int height, bool fullscreen = false, int bpp = 32);
    void Destroy();
    
    void SetRenderBufferParameters(int BufferWidth, int BufferHeight, void* BufferData);
    
    void RunWindowLoop(RayTracerProgram* Program);
    
    void SetTitle(const char* Title);
};
