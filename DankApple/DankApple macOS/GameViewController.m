//
//  GameViewController.m
//  DankApple macOS
//
//  Created by Lucas Ribeiro Goraieb on 1/9/25.
//

#import "GameViewController.h"
#import "Renderer.h"

@implementation GameViewController
{
    MTKView *_view;

    Renderer *_renderer;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    _view = (MTKView *)self.view;

    _renderer = [[Renderer alloc] initWithMetalKitView:_view];

    if(!_renderer)
    {
        NSLog(@"Metal is not supported on this device");
        self.view = [[NSView alloc] initWithFrame:self.view.frame];
        return;
    }

    _view.delegate = _renderer;
}

@end
