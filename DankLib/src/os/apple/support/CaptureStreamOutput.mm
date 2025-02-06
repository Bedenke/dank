#import "CaptureStreamOutput.h"
#include "modules/Foundation.hpp"
#include "modules/os/Capture.hpp"
#include <CoreFoundation/CoreFoundation.h>
#include <CoreMedia/CoreMedia.h>
#include <ScreenCaptureKit/SCStream.h>
#include <ScreenCaptureKit/ScreenCaptureKit.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <vector>

@implementation CaptureStreamOutput {
  dank::CaptureConfig *captureConfig;
}

- (void)setCaptureConfig:(dank::CaptureConfig *)config {
  captureConfig = config;
}

- (void)stream:(SCStream *)stream
    didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
                   ofType:(SCStreamOutputType)type {

  switch (type) {
  case SCStreamOutputTypeScreen: {

    CFArrayRef attachments =
        CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, false);
    if (attachments == NULL)
      break;

    NSDictionary *first =
        (NSDictionary *)CFArrayGetValueAtIndex(attachments, 0);

    SCFrameStatus status = (SCFrameStatus)[
        [first valueForKey:SCStreamFrameInfoStatus] integerValue];

    if (status != SCFrameStatusComplete)
      break;

    CVImageBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    if (!pixelBuffer)
      break;

    [self handleScreenInput:pixelBuffer captureConfig:captureConfig];

    break;
  }
  case SCStreamOutputTypeAudio:
    [self handleAudioInput:sampleBuffer
               audioOutput:&captureConfig->audioOutput];
    break;
  case SCStreamOutputTypeMicrophone:
    [self handleAudioInput:sampleBuffer audioOutput:&captureConfig->micOutput];
    break;
  }
}

- (void)handleAudioInput:(CMSampleBufferRef)sampleBuffer
             audioOutput:(dank::CaptureAudioOutput *)audioOutput {
  for (auto it = audioOutput->buffers.begin();
       it != audioOutput->buffers.end();) {
    if (it->consumed) {
      CFRelease(it->reference);
      it = audioOutput->buffers.erase(it);
    } else {
      ++it;
    }
  }
  CMBlockBufferRef buffer = CMSampleBufferGetDataBuffer(sampleBuffer);

  CMItemCount numSamplesInBuffer = CMSampleBufferGetNumSamples(sampleBuffer);
  AudioBufferList audioBufferList;

  dank::AudioBlockBuffer audioBlockBuffer;
  audioBlockBuffer.consumed = false;
  audioBlockBuffer.reference = buffer;

  CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(
      sampleBuffer, NULL, &audioBufferList, sizeof(audioBufferList), NULL, NULL,
      kCMSampleBufferFlag_AudioBufferList_Assure16ByteAlignment, &buffer);

  for (int bufferCount = 0; bufferCount < audioBufferList.mNumberBuffers;
       bufferCount++) {
    dank::AudioBuffer audioBuffer{};
    audioBuffer.bitDepth = dank::AudioBuffer::BitDepth::BitDepth32;
    audioBuffer.numChannels =
        audioBufferList.mBuffers[bufferCount].mNumberChannels;
    audioBuffer.dataByteSize =
        audioBufferList.mBuffers[bufferCount].mDataByteSize;
    audioBuffer.numSamples = audioBuffer.dataByteSize / sizeof(float);
    audioBuffer.data = (float *)audioBufferList.mBuffers[bufferCount].mData;

    audioBlockBuffer.audioBuffers.push_back(audioBuffer);
  }

  audioOutput->buffers.push_back(audioBlockBuffer);
  audioOutput->frame++;
}

- (void)handleScreenInput:(CVImageBufferRef)imageBuffer
            captureConfig:(dank::CaptureConfig *)config {

  // Check for supported pixel format
  OSType pixelFormat = CVPixelBufferGetPixelFormatType(imageBuffer);
  if (pixelFormat == kCVPixelFormatType_32BGRA) {
    config->screenOutput.format = dank::PixelFormat::BGRA32;
  } else {
    // Unsupported
    return;
  }

  // Lock the base address of the image buffer
  CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);

  config->screenOutput.width = CVPixelBufferGetWidth(imageBuffer);
  config->screenOutput.height = CVPixelBufferGetHeight(imageBuffer);
  config->screenOutput.channels = 4;

  size_t outputDataSize = config->screenOutput.width *
                          config->screenOutput.height *
                          config->screenOutput.channels;

  if (config->screenOutput.size != outputDataSize) {
    config->screenOutput.size = outputDataSize;
    if (config->screenOutput.data != nullptr) {
      free(config->screenOutput.data);
    }
    config->screenOutput.data = (uint8_t *)malloc(outputDataSize);
  }

  memcpy(config->screenOutput.data,
         (uint8_t *)CVPixelBufferGetBaseAddress(imageBuffer), outputDataSize);

  config->screenOutput.frame++;

  CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
}

- (void)stream:(SCStream *)stream didStopWithError:(NSError *)error {
  NSLog(@"[CaptureStreamOutput] didStopWithError error=%@", error);
}

- (void)streamDidBecomeActive:(SCStream *)stream {
  NSLog(@"[CaptureStreamOutput] streamDidBecomeActive");
}

- (void)streamDidBecomeInactive:(SCStream *)stream {
  NSLog(@"[CaptureStreamOutput] streamDidBecomeInactive");
}

@end