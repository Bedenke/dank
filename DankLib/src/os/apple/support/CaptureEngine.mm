#import "CaptureEngine.h"
#import "CaptureStreamOutput.h"
#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>
#include <ScreenCaptureKit/SCStream.h>
#include <ScreenCaptureKit/ScreenCaptureKit.h>

@implementation CaptureEngine {
  SCShareableContent *currentShareableContent;
  SCStream *stream;
  CaptureStreamOutput *streamOutput;
  dispatch_queue_t videoSampleBufferQueue;
  dispatch_queue_t audioSampleBufferQueue;
  dispatch_queue_t micSampleBufferQueue;
}

+ (instancetype)sharedInstance {
  static CaptureEngine *sharedInstance = nil;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    sharedInstance = [[self alloc] init];
  });
  return sharedInstance;
}

- (instancetype)init {
  self = [super init];
  streamOutput = [[CaptureStreamOutput alloc] init];
  videoSampleBufferQueue =
      dispatch_queue_create("dank.VideoSampleBufferQueue", NULL);
  micSampleBufferQueue =
      dispatch_queue_create("dank.MicSampleBufferQueue", NULL);
  audioSampleBufferQueue =
      dispatch_queue_create("dank.AudioSampleBufferQueue", NULL);
  NSLog(@"[ScreenRecorder] init %@", streamOutput);

  return self;
}

- (void)updateSharableContent:(dank::CaptureSharableContent *)output {
  [SCShareableContent
      getShareableContentExcludingDesktopWindows:false
                             onScreenWindowsOnly:true
                               completionHandler:^(
                                   SCShareableContent
                                       *_Nullable shareableContent,
                                   NSError *_Nullable error) {
                                 if (error != nil) {
                                   NSLog(@"[ScreenRecorder] recordFirstDisplay "
                                         @"error %@",
                                         error);
                                   return;
                                 }
                                 currentShareableContent = shareableContent;
                                 output->displayCount =
                                     shareableContent.displays.count;
                                 NSLog(@"[ScreenRecorder] displays %d",
                                       output->displayCount);
                               }];
}

- (void)updateConfig:(dank::CaptureConfig *)config {
  if (stream != nil) {
    [stream stopCaptureWithCompletionHandler:^(NSError *_Nullable error) {
      NSLog(@"[ScreenRecorder] stopCaptureWithCompletionHandler error=%@",
            error);
      stream = nil;
      [self updateConfig:config];
    }];
    return;
  }

  bool isRecording = config->captureScreen ||
                     config->captureAudio ||
                     config->captureMicrophone;

  if (!isRecording)
    return; // do nothing

  SCStreamConfiguration *streamConfig = [[SCStreamConfiguration alloc] init];
  SCContentFilter *filter;

  if (config->captureScreen) {
    SCDisplay *selectedDisplay =
        currentShareableContent.displays[config->selectedDisplay];

    NSLog(@"[ScreenRecorder] record display=%d %ldx%ld",
          selectedDisplay.displayID, selectedDisplay.width,
          selectedDisplay.height);

    [streamConfig setWidth:selectedDisplay.width];
    [streamConfig setHeight:selectedDisplay.height];
    [streamConfig setMinimumFrameInterval:CMTimeMake(1, 60)];
    [streamConfig setQueueDepth:5];
    [streamConfig setPixelFormat:kCVPixelFormatType_32BGRA];

    filter = [[SCContentFilter alloc] initWithDisplay:selectedDisplay
                                     excludingWindows:@[]];
  }

  if (config->captureMicrophone) {
    [streamConfig setCaptureMicrophone:true];
    // TODO: [streamConfig setMicrophoneCaptureDeviceID:]
  }

  if (config->captureAudio) {
    [streamConfig setCapturesAudio:true];
  }

  [streamOutput setCaptureConfig:config];

  stream = [[SCStream alloc] initWithFilter:filter
                              configuration:streamConfig
                                   delegate:streamOutput];

  if (config->captureScreen) {
    NSError *error;
    [stream addStreamOutput:streamOutput
                       type:SCStreamOutputTypeScreen
         sampleHandlerQueue:videoSampleBufferQueue
                      error:&error];
    NSLog(@"[ScreenRecorder] addStreamOutput SCStreamOutputTypeScreen error=%@",
          error);
  }

  if (config->captureMicrophone) {
    NSError *error;
    [stream addStreamOutput:streamOutput
                       type:SCStreamOutputTypeMicrophone
         sampleHandlerQueue:micSampleBufferQueue
                      error:&error];
    NSLog(@"[ScreenRecorder] addStreamOutput SCStreamOutputTypeMicrophone "
          @"error=%@",
          error);
  }

  if (config->captureAudio) {
    NSError *error;
    [stream addStreamOutput:streamOutput
                       type:SCStreamOutputTypeAudio
         sampleHandlerQueue:audioSampleBufferQueue
                      error:&error];
    NSLog(@"[ScreenRecorder] addStreamOutput SCStreamOutputTypeAudio error=%@",
          error);
  }

  [stream startCaptureWithCompletionHandler:^(NSError *_Nullable error) {
    NSLog(@"[ScreenRecorder] startCaptureWithCompletionHandler error=%@",
          error);
  }];
}


@end