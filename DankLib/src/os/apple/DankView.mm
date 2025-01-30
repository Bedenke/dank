//
//  DankView.mm
//  DankApple
//
//  Created by Lucas Ribeiro Goraieb on 1/30/25.
//
#include "DankView.h"
#include "AppleOS.hpp"
#include "Metal.hpp"
#include "modules/input/InputEvent.hpp"
#include "modules/os/OS.hpp"
#include "os/apple/renderer/AppleRenderer.hpp"
#include "os/apple/support/Events.h"
#include <cstddef>
#include <dlfcn.h>

using namespace dank;

class AppleOS : public OS {
public:
  void getDataFromURI(URI &uri, ResourceData &output) override {
    NSString *protocol = [NSString stringWithCString:uri.protocol.c_str()
                                            encoding:NSUTF8StringEncoding];
    assert([protocol isEqualToString:@"file"]);

    NSString *host = [NSString stringWithCString:uri.host.c_str()
                                        encoding:NSUTF8StringEncoding];
    NSString *path = [NSString stringWithCString:uri.path.c_str()
                                        encoding:NSUTF8StringEncoding];
    NSString *filePath = [NSString stringWithFormat:@"%@/%@", host, path];
    if ([[NSFileManager defaultManager] fileExistsAtPath:filePath]) {
      NSData *data = [[NSFileManager defaultManager] contentsAtPath:filePath];
      output.size = [data length];
      output.data = malloc(output.size);
      memcpy(output.data, [data bytes], output.size);
    } else {
      NSLog(@"File not exits: %@", filePath);
      output.size = 0;
      output.data = nullptr;
    }
  }
};

AppleOS *appleOS = new AppleOS();

@implementation DankView {
  NSString *currentDylib;
  void *_Nullable dankLibHandle;
  NSString *currentMetalLib;
  NSDate *lastUpdateTime;
  float hotReloadTimer;
  bool resized;

  id<MTLLibrary> shaderLibrary;

  NSMutableCharacterSet *characterSet;
  InputKey MAC_NATIVE_TO_KEY[128];

  apple::OnStartFunctionType onStart;
  apple::OnStopFunctionType onStop;
  apple::OnHotReloadFunctionType onHotReload;
  apple::OnDrawFunctionType onDraw;
  apple::OnResizeFunctionType onResize;
  apple::OnInputEventFunctionType onInputEvent;
}

- (id)initWithCoder:(NSCoder *)coder {
  self = [super initWithCoder:coder];
  if (self) {
    [self initDeviceAndLibraries];
  }

  return self;
}

- (NSString *)findDynamicFile:(nonnull NSString *)sourcePath
                withExtension:(nonnull NSString *)extension {
  NSError *error = nil;

  NSFileManager *fm = [NSFileManager defaultManager];

  NSArray *dirs = [fm contentsOfDirectoryAtPath:sourcePath error:&error];
  if (!dirs) {
    NSLog(@"Error accessing directory: %@", error.localizedDescription);
    return nil;
  }

  __block NSDate *youngest = [[NSDate alloc] initWithTimeIntervalSince1970:0];
  NSMutableString *resultFile = [NSMutableString stringWithString:@""];
  [dirs enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
    NSString *filename = (NSString *)obj;
    NSString *ext = [[filename pathExtension] lowercaseString];
    if ([ext isEqualToString:extension]) {
      NSString *file = [sourcePath stringByAppendingPathComponent:filename];
      NSDate *lastModified =
          [[fm attributesOfItemAtPath:file error:NULL] fileModificationDate];
      if ([lastModified isGreaterThan:youngest]) {
        youngest = lastModified;
        [resultFile setString:file];
      }
    }
  }];

  if (!resultFile || [resultFile length] == 0) {
    NSLog(@"No file with extension %@ found: %@\n", extension, sourcePath);
    return nil;
  }

  return [NSString stringWithString:resultFile];
}

- (bool)loadDynamicLibrary:(nonnull NSString *)dylib;
{
  // dlclose causes a crash when closing the app
  dankLibHandle = dlopen([dylib cStringUsingEncoding:kUnicodeUTF8Format],
                         RTLD_LOCAL | RTLD_NOW);

  if (dankLibHandle) {
    onStart = (apple::OnStartFunctionType)dlsym(dankLibHandle, "onStart");
    onStop = (apple::OnStopFunctionType)dlsym(dankLibHandle, "onStop");
    onHotReload =
        (apple::OnHotReloadFunctionType)dlsym(dankLibHandle, "onHotReload");
    onDraw = (apple::OnDrawFunctionType)dlsym(dankLibHandle, "onDraw");
    onResize = (apple::OnResizeFunctionType)dlsym(dankLibHandle, "onResize");
    onInputEvent =
        (apple::OnInputEventFunctionType)dlsym(dankLibHandle, "onInputEvent");

    NSLog(@"dylib loaded: %@\n", dylib);
    return true;
  } else {
    NSLog(@"dlopen error: %s\n", dlerror());
    return false;
  }
}

- (bool)loadShaderLibrary:(id<MTLDevice>)device
               sourceFile:(nonnull NSString *)sourceFile {

  NSURL *libraryURL = [NSURL URLWithString:sourceFile];
  //    NSURL *libraryURL = [[NSBundle mainBundle] URLForResource:libraryName
  //                                                withExtension:@"metallib"];

  if (libraryURL == nil) {
    NSLog(@"Couldn't find library file: %@", libraryURL);
    return false;
  }

  NSError *libraryError = nil;
  shaderLibrary = [device newLibraryWithURL:libraryURL error:&libraryError];
  if (shaderLibrary == nil) {
    NSLog(@"Couldn't create library: %@", libraryURL);
    if (libraryError.localizedDescription != nil) {
      NSLog(@"Error description: %@", libraryError.localizedDescription);
    }
    return false;
  }

  NSLog(@"Shader library file: %@", libraryURL);

  return true;
}

- (bool)updateDylibSource {

  NSString *sourcePath = @"./DankLib/zig-out/lib";

  NSString *dylib = [self findDynamicFile:sourcePath withExtension:@"dylib"];
  if (dylib == nil)
    return false;

  if ([dylib isEqualTo:currentDylib])
    return false;

  currentDylib = [NSString stringWithString:dylib];

  return true;
}

- (bool)updateMetalLibSource {

  NSString *sourcePath = @"./DankLib/zig-out/lib";

  NSString *metalLib = [self findDynamicFile:sourcePath
                               withExtension:@"metallib"];
  if (metalLib == nil)
    return false;

  if ([metalLib isEqualTo:currentMetalLib])
    return false;

  currentMetalLib = [NSString stringWithString:metalLib];

  return true;
}

- (bool)hotReload {
  bool dylib = [self updateDylibSource];
  if (dylib) {
    if (onStop != nullptr)
      onStop();

    if (![self loadDynamicLibrary:currentDylib])
      return false;

    [self updateMetalLibSource];
    if (![self loadShaderLibrary:self.device sourceFile:currentMetalLib])
      return false;

    return true;
  }

  return false;
}

- (bool)initDeviceAndLibraries {
  lastUpdateTime = [NSDate date];

  characterSet = [NSMutableCharacterSet alphanumericCharacterSet];
  [characterSet formUnionWithCharacterSet:[NSCharacterSet symbolCharacterSet]];
  [characterSet
      formUnionWithCharacterSet:[NSCharacterSet punctuationCharacterSet]];
  [characterSet
      formUnionWithCharacterSet:[NSCharacterSet whitespaceCharacterSet]];

  [self mapKeys];

  self.device = MTLCreateSystemDefaultDevice();

  if ([self hotReload]) {
    [self setColorPixelFormat:MTLPixelFormatRGBA8Unorm];

    resized = true;
    onStart(appleOS);

    NSWindow *mainWindow =
        [[[NSApplication sharedApplication] windows] objectAtIndex:0];
    mainWindow.delegate = self;

    return true;
  }

  return false;
}

- (void)drawRect:(NSRect)dirtyRect {
  NSDate *currentTime = [NSDate date];
  NSTimeInterval deltaTime = [currentTime timeIntervalSinceDate:lastUpdateTime];
  lastUpdateTime = currentTime;
  hotReloadTimer -= deltaTime;

  bool reloaded = false;
  if (hotReloadTimer < 0) {
    hotReloadTimer = 1;
    reloaded = [self hotReload];
  }

  if (reloaded) {
    onStart(appleOS);
    onHotReload();
    resized = true;
  }

  apple::MetalView metalView;
  metalView.device = (__bridge MTL::Device *)self.device;
  metalView.shaderLibrary = (__bridge MTL::Library *)shaderLibrary;
  metalView.currentDrawable = (__bridge MTL::Drawable *)self.currentDrawable;
  metalView.currentRenderPassDescriptor =
      (__bridge MTL::RenderPassDescriptor *)self.currentRenderPassDescriptor;
  metalView.viewWidth = [self drawableSize].width;
  metalView.viewHeight = [self drawableSize].height;

  onDraw(&metalView);

  if (resized) {
    resized = false;
    onResize(metalView.viewWidth, metalView.viewHeight);
  }
}

- (void)windowDidResize:(NSNotification *)notification {
  resized = true;
}

- (void)windowDidResignMain:(NSNotification *)notification {
  // TODO: pause
}

- (void)windowDidBecomeMain:(NSNotification *)notification {
  // TODO: resume
}

- (void)viewDidMoveToWindow {
  [[self window] setAcceptsMouseMovedEvents:YES];
  [[self window] makeFirstResponder:self];
}

- (void)mouseMoved:(NSEvent *)theEvent {
  InputEvent event{};
  event.type = InputEventType::MouseMove;
  event.x = theEvent.locationInWindow.x;
  event.y = theEvent.locationInWindow.y;
  event.button = 0;
  onInputEvent(event);
}

- (void)mouseDown:(NSEvent *)theEvent {
  InputEvent event{};
  event.type = InputEventType::MouseDown;
  event.x = theEvent.locationInWindow.x;
  event.y = theEvent.locationInWindow.y;
  event.button = 0;
  onInputEvent(event);
}

- (void)mouseDragged:(NSEvent *)theEvent {
  InputEvent event{};
  event.type = InputEventType::MouseDrag;
  event.x = theEvent.locationInWindow.x;
  event.y = theEvent.locationInWindow.y;
  event.button = 0;
  onInputEvent(event);
}

- (void)mouseUp:(NSEvent *)theEvent {
  InputEvent event{};
  event.type = InputEventType::MouseUp;
  event.x = theEvent.locationInWindow.x;
  event.y = theEvent.locationInWindow.y;
  event.button = 0;
  onInputEvent(event);
}

- (void)rightMouseDown:(NSEvent *)theEvent {
  InputEvent event{};
  event.type = InputEventType::MouseDown;
  event.x = theEvent.locationInWindow.x;
  event.y = theEvent.locationInWindow.y;
  event.button = 1;
  onInputEvent(event);
}

- (void)rightMouseDragged:(NSEvent *)theEvent {
  InputEvent event{};
  event.type = InputEventType::MouseDrag;
  event.x = theEvent.locationInWindow.x;
  event.y = theEvent.locationInWindow.y;
  event.button = 1;
  onInputEvent(event);
}

- (void)rightMouseUp:(NSEvent *)theEvent {
  InputEvent event{};
  event.type = InputEventType::MouseUp;
  event.x = theEvent.locationInWindow.x;
  event.y = theEvent.locationInWindow.y;
  event.button = 1;
  onInputEvent(event);
}

- (void)otherMouseDown:(NSEvent *)theEvent {
  InputEvent event{};
  event.type = InputEventType::MouseDown;
  event.x = theEvent.locationInWindow.x;
  event.y = theEvent.locationInWindow.y;
  event.button = (int)theEvent.buttonNumber;
  onInputEvent(event);
}

- (void)otherMouseDragged:(NSEvent *)theEvent {
  InputEvent event{};
  event.type = InputEventType::MouseDrag;
  event.x = theEvent.locationInWindow.x;
  event.y = theEvent.locationInWindow.y;
  event.button = (int)theEvent.buttonNumber;
  onInputEvent(event);
}

- (void)otherMouseUp:(NSEvent *)theEvent {
  InputEvent event{};
  event.type = InputEventType::MouseUp;
  event.x = theEvent.locationInWindow.x;
  event.y = theEvent.locationInWindow.y;
  event.button = (int)theEvent.buttonNumber;
  onInputEvent(event);
}

- (void)flagsChanged:(NSEvent *)theEvent {
  InputEvent event{};
  event.key = MAC_NATIVE_TO_KEY[theEvent.keyCode];
  if (event.key == InputKey::KEY_SHIFT) {
    if ([theEvent modifierFlags] & NSEventModifierFlagShift) {
      event.type = InputEventType::KeyDown;
    } else {
      event.type = InputEventType::KeyUp;
    }
    onInputEvent(event);
  } else if (event.key == InputKey::KEY_CONTROL) {
    if ([theEvent modifierFlags] & NSEventModifierFlagControl) {
      event.type = InputEventType::KeyDown;
    } else {
      event.type = InputEventType::KeyUp;
    }
    onInputEvent(event);
  } else if (event.key == InputKey::KEY_ALT) {
    if ([theEvent modifierFlags] & NSEventModifierFlagOption) {
      event.type = InputEventType::KeyDown;
    } else {
      event.type = InputEventType::KeyUp;
    }
    onInputEvent(event);
  } else if (event.key == InputKey::KEY_COMMAND) {
    if ([theEvent modifierFlags] & NSEventModifierFlagCommand) {
      event.type = InputEventType::KeyDown;
    } else {
      event.type = InputEventType::KeyUp;
    }
    onInputEvent(event);
  } else if (event.key == InputKey::KEY_CAPSLOCK) {
    if ([theEvent modifierFlags] & NSEventModifierFlagCapsLock) {
      event.type = InputEventType::KeyDown;
    } else {
      event.type = InputEventType::KeyUp;
    }
    onInputEvent(event);
  }
}

- (void)keyDown:(NSEvent *)theEvent {
  NSString *const character = [theEvent charactersIgnoringModifiers];
  if ([character length] > 0) {
    unichar c = [character characterAtIndex:0];
    if ([characterSet characterIsMember:c]) {
      InputEvent event{};
      event.type = InputEventType::KeyTyped;
      event.character = c;
      onInputEvent(event);
    }
  }

  InputEvent event{};
  event.type = InputEventType::KeyDown;
  event.key = MAC_NATIVE_TO_KEY[theEvent.keyCode];
  onInputEvent(event);
}

- (void)keyUp:(NSEvent *)theEvent {
  InputEvent event{};
  event.type = InputEventType::KeyUp;
  event.key = MAC_NATIVE_TO_KEY[theEvent.keyCode];
  onInputEvent(event);
}

- (void)mapKeys {
  MAC_NATIVE_TO_KEY[kVK_ANSI_A] = InputKey::KEY_A;
  MAC_NATIVE_TO_KEY[kVK_ANSI_B] = InputKey::KEY_B;
  MAC_NATIVE_TO_KEY[kVK_ANSI_C] = InputKey::KEY_C;
  MAC_NATIVE_TO_KEY[kVK_ANSI_D] = InputKey::KEY_D;
  MAC_NATIVE_TO_KEY[kVK_ANSI_E] = InputKey::KEY_E;
  MAC_NATIVE_TO_KEY[kVK_ANSI_F] = InputKey::KEY_F;
  MAC_NATIVE_TO_KEY[kVK_ANSI_G] = InputKey::KEY_G;
  MAC_NATIVE_TO_KEY[kVK_ANSI_H] = InputKey::KEY_H;
  MAC_NATIVE_TO_KEY[kVK_ANSI_I] = InputKey::KEY_I;
  MAC_NATIVE_TO_KEY[kVK_ANSI_J] = InputKey::KEY_J;
  MAC_NATIVE_TO_KEY[kVK_ANSI_K] = InputKey::KEY_K;
  MAC_NATIVE_TO_KEY[kVK_ANSI_L] = InputKey::KEY_L;
  MAC_NATIVE_TO_KEY[kVK_ANSI_M] = InputKey::KEY_M;
  MAC_NATIVE_TO_KEY[kVK_ANSI_N] = InputKey::KEY_N;
  MAC_NATIVE_TO_KEY[kVK_ANSI_O] = InputKey::KEY_O;
  MAC_NATIVE_TO_KEY[kVK_ANSI_P] = InputKey::KEY_P;
  MAC_NATIVE_TO_KEY[kVK_ANSI_Q] = InputKey::KEY_Q;
  MAC_NATIVE_TO_KEY[kVK_ANSI_R] = InputKey::KEY_R;
  MAC_NATIVE_TO_KEY[kVK_ANSI_S] = InputKey::KEY_S;
  MAC_NATIVE_TO_KEY[kVK_ANSI_T] = InputKey::KEY_T;
  MAC_NATIVE_TO_KEY[kVK_ANSI_U] = InputKey::KEY_U;
  MAC_NATIVE_TO_KEY[kVK_ANSI_V] = InputKey::KEY_V;
  MAC_NATIVE_TO_KEY[kVK_ANSI_X] = InputKey::KEY_X;
  MAC_NATIVE_TO_KEY[kVK_ANSI_Z] = InputKey::KEY_Z;
  MAC_NATIVE_TO_KEY[kVK_ANSI_W] = InputKey::KEY_W;
  MAC_NATIVE_TO_KEY[kVK_ANSI_Y] = InputKey::KEY_Y;
  MAC_NATIVE_TO_KEY[kVK_ANSI_0] = InputKey::KEY_0;
  MAC_NATIVE_TO_KEY[kVK_ANSI_1] = InputKey::KEY_1;
  MAC_NATIVE_TO_KEY[kVK_ANSI_2] = InputKey::KEY_2;
  MAC_NATIVE_TO_KEY[kVK_ANSI_3] = InputKey::KEY_3;
  MAC_NATIVE_TO_KEY[kVK_ANSI_4] = InputKey::KEY_4;
  MAC_NATIVE_TO_KEY[kVK_ANSI_5] = InputKey::KEY_5;
  MAC_NATIVE_TO_KEY[kVK_ANSI_6] = InputKey::KEY_6;
  MAC_NATIVE_TO_KEY[kVK_ANSI_7] = InputKey::KEY_7;
  MAC_NATIVE_TO_KEY[kVK_ANSI_8] = InputKey::KEY_8;
  MAC_NATIVE_TO_KEY[kVK_ANSI_9] = InputKey::KEY_9;
  MAC_NATIVE_TO_KEY[kVK_Space] = InputKey::KEY_SPACE;
  MAC_NATIVE_TO_KEY[kVK_UpArrow] = InputKey::KEY_UP;
  MAC_NATIVE_TO_KEY[kVK_DownArrow] = InputKey::KEY_DOWN;
  MAC_NATIVE_TO_KEY[kVK_LeftArrow] = InputKey::KEY_LEFT;
  MAC_NATIVE_TO_KEY[kVK_RightArrow] = InputKey::KEY_RIGHT;
  MAC_NATIVE_TO_KEY[kVK_Shift] = InputKey::KEY_SHIFT;
  MAC_NATIVE_TO_KEY[kVK_Control] = InputKey::KEY_CONTROL;
  MAC_NATIVE_TO_KEY[kVK_Option] = InputKey::KEY_ALT;
  MAC_NATIVE_TO_KEY[kVK_Command] = InputKey::KEY_COMMAND;
  MAC_NATIVE_TO_KEY[kVK_CapsLock] = InputKey::KEY_CAPSLOCK;
  MAC_NATIVE_TO_KEY[kVK_Escape] = InputKey::KEY_ESCAPE;
  MAC_NATIVE_TO_KEY[kVK_Return] = InputKey::KEY_RETURN;
  MAC_NATIVE_TO_KEY[kVK_Tab] = InputKey::KEY_TAB;
  MAC_NATIVE_TO_KEY[kVK_Delete] = InputKey::KEY_BACKSPACE;
}

@end
