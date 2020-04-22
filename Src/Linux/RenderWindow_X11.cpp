//=============================================================================
// RenderWindow_X11.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RenderWindow_X11.h"

#include <X11/Xlib.h>
#include <assert.h>
#include <iostream>
#include <unistd.h>

struct RenderWindow::X11WindowContext
{
    X11WindowContext()
     : display(nullptr)
     , Width(-1)
     , Height(-1)
     , Pixels(nullptr)
    {
    }

    Display* display;
    Window window;
    GC gc;

    int Width, Height;
    char* Pixels;

    Pixmap pixmap;
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

    Context->gc = XCreateGC(Context->display, Context->window, 0, 0);

    XMapWindow(Context->display, Context->window);

    // Handle window close event
    Atom wmDelete=XInternAtom(Context->display, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(Context->display, Context->window, &wmDelete, 1);

    return true;
}

void RenderWindow::Destroy()
{
    XDestroyWindow(Context->display, Context->window);
    XFreeGC(Context->display, Context->gc);
    XCloseDisplay(Context->display);
}

void RenderWindow::SetRenderBufferParameters(int BufferWidth, int BufferHeight, void* BufferData)
{
    Context->Width = BufferWidth;
    Context->Height = BufferHeight;
    Context->Pixels = (char*)BufferData;
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
                case ClientMessage:
                    bQuit = true;
                    break;
            }
        }

        //PresentRenderBuffer();
    }
}

void RenderWindow::SetTitle(const char* Title)
{
    XStoreName(Context->display, Context->window, Title);
}

void RenderWindow::PresentRenderBuffer()
{
    Context->pixmap = XCreatePixmapFromBitmapData(
        Context->display, Context->window,
        Context->Pixels, Context->Width, Context->Height,
        0, 0, 1);

    if (Context->pixmap == 0)
    {
        std::cout << "XCreatePixmapFromBitmapData encounted an error" << std::endl;
    }

    int RetCode = XCopyArea(
        Context->display, Context->pixmap, Context->window, Context->gc,
        0, 0, Context->Width, Context->Height,
        0, 0);

    if (RetCode != Success)
    {
        std::cout << "XCopyArea encounted an error with code " << RetCode << std::endl;
    }

    usleep(50);
}
