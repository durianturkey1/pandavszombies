// macOS entry point for Panda vs Zombies.
//
// cocos2d-x 2.2.6's own Mac samples boot through NSApplicationMain and a
// MainMenu.xib, which needs Xcode's ibtool to compile. This sets the same thing
// up programmatically so the game builds with nothing but the command line
// tools: create the NSApplication, hand it a window backed by cocos2d's
// EAGLView, then let CCApplication::run() start the director's main loop.

#import <Cocoa/Cocoa.h>
#import "EAGLView.h"

#include "cocos2d.h"
#include "AppDelegate.h"

// AppDelegate's constructor registers it as the shared CCApplication, so it has
// to exist before CCApplication::sharedApplication() is called.
static AppDelegate s_sharedApplication;

// The game is designed for a 480x320 landscape phone screen (see AppMacros.h).
// Keeping the GL frame at that size makes AppDelegate pick the 1x asset scale;
// the zoom factor below just scales the window up so it is comfortable on a
// desktop display, and EAGLView divides mouse coordinates back down by it.
static const CGFloat kFrameWidth = 480;
static const CGFloat kFrameHeight = 320;
static const CGFloat kWindowZoom = 2.0;

static const unsigned short kEscapeKeyCode = 53;

@interface GameAppController : NSObject <NSApplicationDelegate>
@property (nonatomic, retain) NSWindow *window;
@property (nonatomic, retain) EAGLView *glView;
@property (nonatomic, retain) id backKeyMonitor;
@property (nonatomic, retain) id backClickMonitor;
@end

@implementation GameAppController

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    NSRect frame = NSMakeRect(0, 0, kFrameWidth, kFrameHeight);

    self.window = [[NSWindow alloc] initWithContentRect:frame
                                             styleMask:(NSWindowStyleMaskTitled |
                                                        NSWindowStyleMaskClosable |
                                                        NSWindowStyleMaskMiniaturizable)
                                               backing:NSBackingStoreBuffered
                                                 defer:NO];

    self.glView = [[EAGLView alloc] initWithFrame:frame];

    [self.window setContentView:self.glView];
    [self.window setTitle:@"Panda vs Zombies"];
    [self.window setAcceptsMouseMovedEvents:NO];
    [self.window makeKeyAndOrderFront:nil];

    [self.glView setFrameZoomFactor:kWindowZoom];
    [self.window center];

    // EAGLView only makes its GL context current inside -prepareOpenGL, which
    // AppKit defers until the view first draws -- i.e. after this method
    // returns. Without a current context the very first thing the engine does
    // (CCConfiguration::gatherGPUInfo querying glGetString) reads a NULL string
    // and segfaults, so bring the context up before handing over to cocos2d.
    [[self.glView openGLContext] makeCurrentContext];

    [self installMenuBar];
    [self installBackGestures];

    cocos2d::CCApplication::sharedApplication()->run();
}

// Every screen in this game leaves through Android's hardware Back button:
// AbstractMenuLayer::keyBackClicked pops the scene, and HelloWorld uses the same
// key to toggle pause. A desktop has no such key, which strands you in the store,
// the level picker, and mid-level with no way out. Map Escape and right-click
// onto the same engine message so all of that navigation works here too.
- (void)goBack
{
    cocos2d::CCDirector *director = cocos2d::CCDirector::sharedDirector();

    // Nothing underneath means popScene would end the director and leave a dead
    // window behind, so Back is a no-op at the root. Quit via Cmd+Q instead.
    if (director->getSceneStackCount() <= 1) {
        return;
    }

    director->getKeypadDispatcher()->dispatchKeypadMSG(cocos2d::kTypeBackClicked);
}

- (void)installBackGestures
{
    GameAppController * __block controller = self;

    self.backKeyMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyDown
                                                                handler:^NSEvent *(NSEvent *event) {
        if ([event keyCode] == kEscapeKeyCode) {
            [controller goBack];
            return nil;  // swallow it, or AppKit beeps
        }
        return event;
    }];

    self.backClickMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskRightMouseDown
                                                                  handler:^NSEvent *(NSEvent *event) {
        [controller goBack];
        return nil;
    }];
}

// Programmatic startup means there is no menu bar, and without one the standard
// Cmd+Q never reaches the app.
- (void)installMenuBar
{
    NSMenu *menuBar = [[[NSMenu alloc] init] autorelease];
    NSMenuItem *appMenuItem = [[[NSMenuItem alloc] init] autorelease];
    [menuBar addItem:appMenuItem];

    NSMenu *appMenu = [[[NSMenu alloc] init] autorelease];
    [appMenu addItemWithTitle:@"Hide Panda vs Zombies"
                       action:@selector(hide:)
                keyEquivalent:@"h"];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:@"Quit Panda vs Zombies"
                       action:@selector(terminate:)
                keyEquivalent:@"q"];
    [appMenuItem setSubmenu:appMenu];

    [NSApp setMainMenu:menuBar];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)application
{
    return YES;
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    cocos2d::CCDirector::sharedDirector()->end();
}

@end

int main(int argc, const char *argv[])
{
    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        GameAppController *controller = [[GameAppController alloc] init];
        [NSApp setDelegate:controller];
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
    }

    return 0;
}
