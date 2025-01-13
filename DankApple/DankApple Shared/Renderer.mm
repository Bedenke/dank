//
//  Renderer.m
//  DankApple Shared
//
//  Created by Lucas Ribeiro Goraieb on 1/9/25.
//

#import "Renderer.h"
#import "AppleOS.hpp"
#import "Metal.hpp"
#include <dlfcn.h>

@implementation Renderer
{
    NSString* currentDylib;
    void* _Nullable dankLibHandle;
    NSString* currentMetalLib;
    NSDate *lastUpdateTime;
    float hotReloadTimer;
    
    id<MTLLibrary> shaderLibrary;
    dank::apple::MetalView metalView;
    
    dank::apple::OnStartFunctionType onStart;
    dank::apple::OnStartFunctionType onHotReload;
    dank::apple::OnDrawFunctionType onDraw;
    dank::apple::OnResizeFunctionType onResize;
}

-(NSString*)findDynamicFile:(nonnull NSString *)sourcePath
              withExtension:(nonnull NSString *)extension
{
    NSError *error = nil;
    
    NSFileManager* fm = [NSFileManager defaultManager];
    
    
    NSArray* dirs = [fm contentsOfDirectoryAtPath:sourcePath
                                            error:&error];
    if (!dirs) {
        NSLog(@"Error accessing directory: %@", error.localizedDescription);
        return nil;
    }
    
    __block NSDate *youngest = [[NSDate alloc] initWithTimeIntervalSince1970:0];
    __block NSString* resultFile = NULL;
    [dirs enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSString *filename = (NSString *)obj;
        NSString *ext = [[filename pathExtension] lowercaseString];
        if ([ext isEqualToString:extension]) {
            NSString* file = [sourcePath stringByAppendingPathComponent:filename];
            NSDate *lastModified = [[fm attributesOfItemAtPath:file error:NULL] fileModificationDate];
            if ([lastModified isGreaterThan:youngest]) {
                youngest = lastModified;
                resultFile = file;
            }
        }
    }];
    
    if (!resultFile) {
        NSLog(@"No file with extension %@ found: %@\n", extension, sourcePath);
        return nil;
    }
    
    return resultFile;
}

-(bool)loadDynamicLibrary:(nonnull NSString *)dylib; {
    if (dankLibHandle) {
        if (dlclose(dankLibHandle) != 0) {
            printf("unload error: %s\n", dlerror());
        }
    }
    
    dankLibHandle = dlopen([dylib cStringUsingEncoding:kUnicodeUTF8Format], RTLD_LOCAL | RTLD_NOW);
    
    if(dankLibHandle) {
        onStart = (dank::apple::OnStartFunctionType)dlsym(dankLibHandle, "onStart");
        onHotReload = (dank::apple::OnHotReloadFunctionType)dlsym(dankLibHandle, "onHotReload");
        onDraw = (dank::apple::OnDrawFunctionType)dlsym(dankLibHandle, "onDraw");
        onResize = (dank::apple::OnResizeFunctionType)dlsym(dankLibHandle, "onResize");
        
        NSLog(@"dylib loaded: %@\n", dylib);
        return true;
    } else {
        NSLog(@"dlopen error: %s\n", dlerror());
        return false;
    }
}


-(bool) loadShaderLibrary:(id<MTLDevice>)device
               sourceFile: (nonnull NSString*) sourceFile
{
    
    NSURL *libraryURL = [NSURL URLWithString:sourceFile];
//    NSURL *libraryURL = [[NSBundle mainBundle] URLForResource:libraryName
//                                                withExtension:@"metallib"];


    if (libraryURL == nil) {
        NSLog(@"Couldn't find library file: %@", libraryURL);
        return false;
    }


    NSError *libraryError = nil;
    shaderLibrary = [device newLibraryWithURL:libraryURL
                                                  error:&libraryError];
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


-(bool)updateDylibSource {

    NSString *sourcePath = @"./DankLib/zig-out/lib";

    NSString* dylib = [self findDynamicFile:sourcePath withExtension:@"dylib"];
    if (dylib == nil) return false;
    
    if ([dylib isEqualTo:currentDylib]) return false;
    
    currentDylib = [NSString stringWithString:dylib];

    return true;
}

-(bool)updateMetalLibSource {

    NSString *sourcePath = @"./DankLib/zig-out/lib";

    NSString* metalLib = [self findDynamicFile:sourcePath withExtension:@"metallib"];
    if (metalLib == nil) return false;
    
    if ([metalLib isEqualTo:currentMetalLib]) return false;
    
    currentMetalLib = [NSString stringWithString:metalLib];
    
    return true;
}

-(bool)hotReload:(nonnull MTKView *)view {
    bool dylib = [self updateDylibSource];
    if (dylib) {
        if (![self loadDynamicLibrary:currentDylib]) return false;

        view.device = MTLCreateSystemDefaultDevice();

        [self updateMetalLibSource];
        if (![self loadShaderLibrary:view.device sourceFile:currentMetalLib]) return false;
        
        return true;
    }
    
    return false;
}

-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view;
{
    self = [super init];
    lastUpdateTime = [NSDate date];
    
    if ([self hotReload:view]) {
        [view setColorPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB];
        
        [self mtkView:view drawableSizeWillChange:view.drawableSize];

        onStart();
        
        return self;
    }
    
    return nil;
}

- (void)drawInMTKView:(nonnull MTKView *)view
{
    NSDate *currentTime = [NSDate date];
    NSTimeInterval deltaTime = [currentTime timeIntervalSinceDate:lastUpdateTime];
    lastUpdateTime = currentTime;
    hotReloadTimer -= deltaTime;

    bool reloaded = false;
    if (hotReloadTimer < 0) {
        hotReloadTimer = 1;
        reloaded = [self hotReload:view];
    }

    metalView.device =(__bridge MTL::Device *) view.device;
    metalView.shaderLibrary = (__bridge MTL::Library *) shaderLibrary;
    metalView.currentDrawable = (__bridge MTL::Drawable *) view.currentDrawable;
    metalView.currentRenderPassDescriptor = (__bridge MTL::RenderPassDescriptor *) view.currentRenderPassDescriptor;

    if (reloaded) onHotReload();

    onDraw(&metalView);
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    onResize(size.width, size.height);
}
@end
