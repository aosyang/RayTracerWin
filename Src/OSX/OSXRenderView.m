//=============================================================================
// OSXRenderView.m by Shiyang Ao, 2017 All Rights Reserved.
//
//
//=============================================================================

#import <Foundation/Foundation.h>
#import "OSXRenderView.h"

#define BYTES_PER_PIXEL 4
#define BITS_PER_COMPONENT 8
#define BITS_PER_PIXEL 32

@implementation OSXRenderView

-(void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    CGContextRef Context = (CGContextRef)[[NSGraphicsContext currentContext] CGContext];
    
    CGColorSpaceRef ColorSpace = CGColorSpaceCreateDeviceRGB();
    
    // Create data provider from pixel data
    CGDataProviderRef DataProvider = CGDataProviderCreateWithData(nil, self.BufferData, self.ViewWidth * self.ViewHeight, nil);
    
    CGImageRef Image = CGImageCreate(self.ViewWidth,
                                     self.ViewHeight,
                                     BITS_PER_COMPONENT,
                                     BITS_PER_PIXEL,
                                     BYTES_PER_PIXEL * self.ViewWidth,
                                     ColorSpace,
                                     kCGBitmapByteOrder32Little,
                                     DataProvider, nil, NO,
                                     kCGRenderingIntentDefault);
    
    CGRect DrawRect = CGRectMake(0, 0, self.ViewWidth, self.ViewHeight);
    CGContextDrawImage(Context, DrawRect, Image);
    
    CGImageRelease(Image);
    CGColorSpaceRelease(ColorSpace);
    CGDataProviderRelease(DataProvider);
}

-(void)startUpdate
{
    [NSTimer scheduledTimerWithTimeInterval: 0.02
                                     target: self
                                   selector: @selector(OnTimer)
                                   userInfo: nil
                                    repeats: YES];
}

-(void)setRenderBufferData:(Byte*) data bufferWidth:(int) width bufferHeight:(int) height
{
    self.ViewWidth = width;
    self.ViewHeight = height;
    self.BufferData = data;
}

-(void)OnTimer
{
    [self setNeedsDisplay:YES];
}

@end

