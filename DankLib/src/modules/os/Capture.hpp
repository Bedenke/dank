#pragma once
#include "modules/Foundation.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace dank {
struct CaptureSharableContent {
  uint8_t displayCount;
};

struct CaptureScreenOutput {
  uint32_t frame = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t channels = 0;
  PixelFormat format;
  size_t size = 0;
  uint8_t *data = nullptr;
};

struct AudioBuffer {
  enum class BitDepth { BitDepth16, BitDepth32 };
  BitDepth bitDepth;
  uint32_t numChannels;
  uint32_t dataByteSize;
  uint32_t numSamples;
  void *data;
  float get(size_t sampleIndex) {
    return bitDepth == BitDepth::BitDepth32
               ? ((float *)data)[sampleIndex]
               : ((int16_t *)data)[sampleIndex] / 32767.0f;
  }
};

struct AudioBlockBuffer {
  std::vector<AudioBuffer> audioBuffers;
  bool consumed;
  void *reference;
};

struct CaptureAudioOutput {
  uint32_t frame;
  std::vector<AudioBlockBuffer> buffers;
};

struct CaptureAudioInput {};

struct CaptureConfig {

  bool captureScreen;
  bool captureMicrophone;
  bool captureAudio;

  uint8_t selectedDisplay;

  CaptureScreenOutput screenOutput;
  CaptureAudioOutput audioOutput;
  CaptureAudioOutput micOutput;
};

} // namespace dank