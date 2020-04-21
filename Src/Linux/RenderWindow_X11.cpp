//=============================================================================
// RenderWindow_X11.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RenderWindow_X11.h"

#include <X11/Xlib.h>
#include <assert.h>
#include <iostream>

struct RenderWindow::X11WindowContext
{
    X11WindowContext()
     : display(nullptr)
    {
    }

    Display* display;
    Window window;
};

RenderWindow::RenderWindow()
{
    Context = new X11WindowContext();
}

RenderWindow::~RenderWindow()
{
    delete Context;
}

bool RenderWindow::Create(int width, int height, bool fullscreen, int bpp)
{
    Context->display = XOpenDisplay(NULL);
    assert(Context->display);

    int BlackColor = BlackPixel(Context->display, DefaultScreen(Context->display));
    int WhiteColor = WhitePixel(Context->display, DefaultScreen(Context->display));

    Context->window = XCreateSimpleWindow(Context->display, DefaultRootWindow(Context->display),
        0, 0, width, height, 0, BlackColor, BlackColor);

    XMapWindow(Context->display, Context->window);

    return true;
}

void RenderWindow::Destroy()
{

}

void RenderWindow::SetRenderBufferParameters(int BufferWidth, int BufferHeight, void* BufferData)
{

}

void RenderWindow::RunWindowLoop(RayTracerProgram* Program)
{
    std::cout << "Running X11 window loop" << std::endl;

    bool bQuit = false;
    while (!bQuit)
    {
        XEvent event;
        while (XPending(Context->display))
        {
            XNextEvent(Context->display, &event);
            if (XFilterEvent(&event, None))
            {
                continue;
            }
            
            switch (event.type)
            {
                case DestroyNotify:
                    bQuit = true;
                    break;
            }
        }
    }
}

void RenderWindow::SetTitle(const char* Title)
{
    XStoreName(Context->display, Context->window, Title);
}
