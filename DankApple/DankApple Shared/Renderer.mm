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
    NSString* previousDylib;
    void* _Nullable dankLibHandle;
    NSDate *lastUpdateTime;
    float hotReloadTimer;
    dank::apple::MetalView metalView;

    dank::apple::OnStartFunctionType onStart;
    dank::apple::OnDrawFunctionType onDraw;
    dank::apple::OnResizeFunctionType onResize;
}


-(NSString*)findDynamicLibrary:(nonnull NSString *)sourcePath; {
    NSError *error = nil;

    NSFileManager* fm = [NSFileManager defaultManager];
    
    
    NSArray* dirs = [fm contentsOfDirectoryAtPath:sourcePath
                                            error:&error];
    if (!dirs) {
        NSLog(@"Error accessing directory: %@", error.localizedDescription);
        return nil;
    }
    
    __block NSDate *youngest = [[NSDate alloc] initWithTimeIntervalSince1970:0];
    __block NSString* dylib = NULL;
    [dirs enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSString *filename = (NSString *)obj;
        NSString *extension = [[filename pathExtension] lowercaseString];
        if ([extension isEqualToString:@"dylib"]) {
            NSString* file = [sourcePath stringByAppendingPathComponent:filename];
            NSDate *lastModified = [[fm attributesOfItemAtPath:file error:NULL] fileModificationDate];
            if ([lastModified isGreaterThan:youngest]) {
                youngest = lastModified;
                dylib = file;
            }
        }
    }];
    
    if (!dylib) {
        NSLog(@"no dylib found: %@\n", sourcePath);
        return nil;
    }
    
    return dylib;
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
        onDraw = (dank::apple::OnDrawFunctionType)dlsym(dankLibHandle, "onDraw");
        onResize = (dank::apple::OnResizeFunctionType)dlsym(dankLibHandle, "onResize");
        
        NSLog(@"dylib loaded: %@\n", dylib);
        return true;
    } else {
        NSLog(@"dlopen error: %s\n", dlerror());
        return false;
    }
}


-(bool)hotReload {

    NSString *sourcePath = @"./DankLib/zig-out/lib";

    NSString* dylib = [self findDynamicLibrary:sourcePath];
    if (dylib == nil) return false;
    
    if ([dylib isEqualTo:previousDylib]) return false;
    
    previousDylib = [NSString stringWithString:dylib];

    return [self loadDynamicLibrary: dylib];
}

-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view;
{
    self = [super init];
    lastUpdateTime = [NSDate date];
    
    if ([self hotReload]) {
        
        metalView.device =(__bridge MTL::Device *) view.device;
        metalView.currentDrawable = (__bridge MTL::Drawable *) view.currentDrawable;
        metalView.currentRenderPassDescriptor = (__bridge MTL::RenderPassDescriptor *) view.currentRenderPassDescriptor;
        onStart();
    }
    
    return self;
}

- (void)drawInMTKView:(nonnull MTKView *)view
{
    NSDate *currentTime = [NSDate date];
    NSTimeInterval deltaTime = [currentTime timeIntervalSinceDate:lastUpdateTime];
    lastUpdateTime = currentTime;
    hotReloadTimer -= deltaTime;
    
    if (hotReloadTimer < 0) {
        hotReloadTimer = 1;
        [self hotReload];
    }
    
    metalView.device =(__bridge MTL::Device *) view.device;
    metalView.currentDrawable = (__bridge MTL::Drawable *) view.currentDrawable;
    metalView.currentRenderPassDescriptor = (__bridge MTL::RenderPassDescriptor *) view.currentRenderPassDescriptor;
    onDraw(&metalView);
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    onResize(size.width, size.height);
}
@end
