//=============================================================================
// OSXWindow.mm by Shiyang Ao, 2017 All Rights Reserved.
//
//
//=============================================================================

#include "OSXWindow.h"

#import "Cocoa/Cocoa.h"
#import "OSXRenderView.h"
#import "OSXAppDelegate.h"

#include <thread>
#include <chrono>

namespace
{
    NSWindow* Window;
    OSXRenderView* RenderView;
    id AppDelegate;
}

bool RenderWindow::Create(int width, int height, bool fullscreen, int bpp)
{
    [NSApplication sharedApplication];
    
    NSUInteger windowStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
    
    CGSize ScreenSize = [[NSScreen mainScreen] frame].size;
    NSRect windowRect = NSMakeRect((ScreenSize.width - width)/2, (ScreenSize.height - height)/2, width, height);
    Window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO];
    
    // Create custom view for rendering
    RenderView = [[OSXRenderView alloc] initWithFrame:windowRect];
    
    // Tell window to use custom view
    [Window setContentView:RenderView];
    
    // Use application delegate to handle program exit when user closes the window
    AppDelegate = [[OSXAppDelegate alloc] init];
    [NSApp setDelegate:AppDelegate];
    
    [Window orderFrontRegardless];
    
    [RenderView startUpdate];
    
    return true;
}

void RenderWindow::Destroy()
{
    [AppDelegate release];
    [RenderView release];
    [Window release];
}

void RenderWindow::SetRenderBufferParameters(int BufferWidth, int BufferHeight, void* BufferData)
{
    RenderView.ViewWidth = BufferWidth;
    RenderView.ViewHeight = BufferHeight;
    RenderView.BufferData = (Byte*)BufferData;
}

void RenderWindow::RunWindowLoop()
{
    [NSApp run];
}

