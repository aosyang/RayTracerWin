//=============================================================================
// OSXWindow.mm by Shiyang Ao, 2017 All Rights Reserved.
//
//
//=============================================================================

#include "OSXWindow.h"

#import "Cocoa/Cocoa.h"
#import "OSXRenderView.h"
#import "OSXAppDelegate.h"

namespace
{
    NSWindow* Window;
    OSXRenderView* RenderView;
    id AppDelegate;
}

bool RenderWindow::Create(int width, int height, bool fullscreen, int bpp)
{
    [NSApplication sharedApplication];
    
    NSUInteger windowStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;
    
    NSScreen* MainScreen = [NSScreen mainScreen];
    NSRect windowRect = [MainScreen convertRectFromBacking:NSMakeRect(0, 0, width, height)];
    
    // Create the render window
    Window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO];
    
    // Center the window to screen
    [Window center];
    
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

void RenderWindow::SetTitle(const char* Title)
{
    // TODO: Fix title bar setting issues on OSX
    
    //NSString* windowTitle = [NSString stringWithCString:Title encoding:NSASCIIStringEncoding];
    //[Window setTitle:windowTitle];
}
