#pragma once

#include "modules/os/Capture.hpp"
#include <Foundation/Foundation.h>

@interface CaptureEngine : NSObject
+ (instancetype)sharedInstance;
-(void)updateConfig: (dank::CaptureConfig*) config;
-(void)updateSharableContent: (dank::CaptureSharableContent*) output;
@end