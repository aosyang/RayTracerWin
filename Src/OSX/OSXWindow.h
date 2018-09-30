//=============================================================================
// OSXWindow.h by Shiyang Ao, 2017 All Rights Reserved.
//
//
//=============================================================================

#pragma once

class RenderWindow
{
public:
    bool Create(int width, int height, bool fullscreen = false, int bpp = 32);
    void Destroy();
    
    void SetRenderBufferParameters(int BufferWidth, int BufferHeight, void* BufferData);
    
    void RunWindowLoop();
    
    void SetTitle(const char* Title);
};
