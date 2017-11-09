//=============================================================================
// OSXAppDelegate.m by Shiyang Ao, 2017 All Rights Reserved.
//
//
//=============================================================================

#import <Foundation/Foundation.h>
#import "OSXAppDelegate.h"

@implementation OSXAppDelegate

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication
{
    return YES;
}

@end
