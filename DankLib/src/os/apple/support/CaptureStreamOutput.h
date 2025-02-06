#include "modules/os/Capture.hpp"
#import <ScreenCaptureKit/ScreenCaptureKit.h>

@interface CaptureStreamOutput : NSObject<SCStreamOutput, SCStreamDelegate>
-(void)setCaptureConfig: (dank::CaptureConfig*) config;
@end