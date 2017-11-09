//=============================================================================
// OSXRenderView.h by Shiyang Ao, 2017 All Rights Reserved.
//
//
//=============================================================================

#pragma once

#import <Cocoa/Cocoa.h>

@interface OSXRenderView : NSView

-(void)startUpdate;
-(void)setRenderBufferData:(Byte*) data bufferWidth:(int) width bufferHeight:(int) height;

-(void)OnTimer;

@property int ViewWidth;
@property int ViewHeight;
@property Byte *BufferData;

@end

