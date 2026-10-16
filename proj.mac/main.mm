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

// The game lays its screens out in a 480x320 design resolution (AppMacros.h),
// but its art is drawn much larger and AppDelegate picks an asset scale from the
// *frame* size: at 480x320 it chooses the 1x bucket, which leaves sprites sized
// in raw pixels four times bigger than on a phone, and renders a 480x320 image
// that the display then has to stretch. So give the engine a frame as large as
// the window's real pixel count instead -- same asset bucket a phone gets, and
// one rendered pixel per physical pixel.
static const CGFloat kAspect = 480.0 / 320.0;
static const CGFloat kMinWindowHeight = 480;

static const unsigned short kEscapeKeyCode = 53;

@interface GameAppController : NSObject <NSApplicationDelegate, NSWindowDelegate>
@property (nonatomic, retain) NSWindow *window;
@property (nonatomic, retain) EAGLView *glView;
@property (nonatomic, retain) id backKeyMonitor;
@property (nonatomic, retain) id backClickMonitor;
@end

@implementation GameAppController

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    NSRect frame = NSMakeRect(0, 0, [self preferredWindowSize].width,
                                    [self preferredWindowSize].height);

    self.window = [[NSWindow alloc] initWithContentRect:frame
                                             styleMask:(NSWindowStyleMaskTitled |
                                                        NSWindowStyleMaskClosable |
                                                        NSWindowStyleMaskMiniaturizable |
                                                        NSWindowStyleMaskResizable)
                                               backing:NSBackingStoreBuffered
                                                 defer:NO];

    self.glView = [[EAGLView alloc] initWithFrame:frame];
    // Render at the display's real pixel density rather than upscaling later.
    [self.glView setWantsBestResolutionOpenGLSurface:YES];

    [self.window setContentView:self.glView];
    [self.window setTitle:@"Panda vs Zombies"];
    [self.window setAcceptsMouseMovedEvents:NO];
    [self.window setDelegate:self];
    [self.window setContentMinSize:NSMakeSize(kMinWindowHeight * kAspect, kMinWindowHeight)];
    [self.window setContentAspectRatio:NSMakeSize(kAspect, 1.0)];
    [self.window makeKeyAndOrderFront:nil];
    [self.window center];

    // EAGLView only makes its GL context current inside -prepareOpenGL, which
    // AppKit defers until the view first draws -- i.e. after this method
    // returns. Without a current context the very first thing the engine does
    // (CCConfiguration::gatherGPUInfo querying glGetString) reads a NULL string
    // and segfaults, so bring the context up before handing over to cocos2d.
    [[self.glView openGLContext] makeCurrentContext];
    [self applyFrameSize];

    [self installMenuBar];
    [self installBackGestures];

    cocos2d::CCApplication::sharedApplication()->run();
}

// A window that is tall enough in *pixels* lands in the same asset bucket as a
// phone or tablet, which is what the layouts were tuned against.
- (NSSize)preferredWindowSize
{
    NSRect usable = [[NSScreen mainScreen] visibleFrame];
    CGFloat backing = [[NSScreen mainScreen] backingScaleFactor];

    CGFloat height = 640;
    if (backing < 2.0) {
        height = 800;  // no Retina doubling, so ask for the pixels directly
    }

    CGFloat maxHeight = usable.size.height * 0.85;
    CGFloat maxWidth = usable.size.width * 0.9;

    if (height > maxHeight) { height = maxHeight; }
    if (height * kAspect > maxWidth) { height = maxWidth / kAspect; }
    if (height < kMinWindowHeight) { height = kMinWindowHeight; }

    return NSMakeSize(round(height * kAspect), round(height));
}

// Hands the engine the drawable's pixel size and re-applies the letterboxed
// design resolution, so every screen keeps its 480x320 layout centred no matter
// the window or device aspect ratio.
- (void)applyFrameSize
{
    NSRect backing = [self.glView convertRectToBacking:[self.glView bounds]];
    if (backing.size.width <= 0 || backing.size.height <= 0) {
        return;
    }

    cocos2d::CCEGLView *view = cocos2d::CCEGLView::sharedOpenGLView();
    view->setFrameSize(backing.size.width, backing.size.height);
    view->setDesignResolutionSize(480, 320, kResolutionShowAll);
}

- (void)windowDidResize:(NSNotification *)notification
{
    [self applyFrameSize];
}

- (void)windowDidChangeBackingProperties:(NSNotification *)notification
{
    [self applyFrameSize];
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
