#import "../../Include/MacOSWindowManager.h"
#import "always.h"
#import <Cocoa/Cocoa.h>

@interface MacOSWindow : NSWindow
@end

@implementation MacOSWindow
- (BOOL)canBecomeKeyWindow {
  return YES;
}
- (BOOL)canBecomeMainWindow {
  return YES;
}
- (BOOL)acceptsFirstResponder {
  return YES;
}
// Override to prevent beep on keyDown
- (void)keyDown:(NSEvent *)event {
  // Don't call super — prevents NSBeep for unhandled key events
}
@end

// Custom content view that accepts first responder for keyboard input
@interface GameContentView : NSView
@end

@implementation GameContentView
- (BOOL)acceptsFirstResponder {
  return YES;
}
- (BOOL)canBecomeKeyView {
  return YES;
}
// Override to prevent beep on unhandled keys
- (void)keyDown:(NSEvent *)event {
  // Intentionally empty — events are handled in MacOS_PumpEvents
}
- (void)keyUp:(NSEvent *)event {
  // Intentionally empty
}
@end


@interface MacOSWindowDelegate
    : NSObject <NSWindowDelegate, NSApplicationDelegate>
@end

static BOOL g_ShouldQuit = NO;

@implementation MacOSWindowDelegate
- (void)windowWillClose:(NSNotification *)notification {
  g_ShouldQuit = YES;
  [NSApp terminate:nil];
}
@end
static id g_WindowDelegate = nil;
static id g_AppDelegate = nil;

#import "Common/AsciiString.h"
#import "Common/GlobalData.h"
#import "Common/UnicodeString.h"
#import "Common/version.h"

extern int GameMain();
extern "C" void MacOS_InitRenderer(void *windowHandle);

#include "Common/CommandLine.h"

int MacOS_Main(int argc, char *argv[]) {
  @autoreleasepool {
    printf("MacOS_Main: Initializing TheVersion...\n");
    fflush(stdout);
    TheVersion = new Version();
    printf("MacOS_Main: TheVersion = %p\n", (void *)TheVersion);
    fflush(stdout);
    printf("MacOS_Main: Initializing NSApp...\n");
    fflush(stdout);
    [NSApplication sharedApplication];
    printf("MacOS_Main: Setting Activation Policy...\n");
    fflush(stdout);
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    printf("MacOS_Main: Entering GameMain()...\n");
    fflush(stdout);

    // Give Cocoa time to settle
    [NSThread sleepForTimeInterval:0.1];

    // Force bring to front
    [NSApp activateIgnoringOtherApps:YES];

    // Finish launching to ensure NSApp is ready
    [NSApp finishLaunching];

    // Parse command line and init global data
    CommandLine::parseCommandLineForStartup();

    // Create window
    int w = 800;
    int h = 600;
    if (TheGlobalData && TheGlobalData->m_xResolution > 0) {
      w = TheGlobalData->m_xResolution;
      h = TheGlobalData->m_yResolution;
    }
    extern void *ApplicationHWnd;
    void *window =
        MacOS_CreateWindow(w, h, "Command & Conquer Generals (macOS)");
    ApplicationHWnd = window; // Must set before GameMain → WW3D::Init
    fprintf(stderr, "DEBUG: ApplicationHWnd SET to %p (window=%p)\n",
            ApplicationHWnd, window);

    // Pump events for a bit to ensure window shows up
    for (int i = 0; i < 30; ++i) {
      MacOS_PumpEvents();
      [NSThread sleepForTimeInterval:0.01];
    }

    // Initialize renderer
    MacOS_InitRenderer(window);

    printf("MacOS_Main: About to call GameMain()...\n");
    fflush(stdout);
    int result = GameMain();
    printf("MacOS_Main: GameMain() returned %d\n", result);
    fflush(stdout);
    return result;
  }
}

void MacOS_GetScreenSize(int &w, int &h) {
  NSScreen *screen = [NSScreen mainScreen];
  if (screen) {
    NSRect frame = [screen visibleFrame];
    w = (int)frame.size.width;
    h = (int)frame.size.height;
  } else {
    w = 0;
    h = 0;
  }
}

void *MacOS_CreateWindow(int width, int height, const char *title) {
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  NSRect frame = NSMakeRect(0, 0, width, height);
  MacOSWindow *window = [[MacOSWindow alloc]
      initWithContentRect:frame
                styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                          NSWindowStyleMaskMiniaturizable |
                          NSWindowStyleMaskResizable
                  backing:NSBackingStoreBuffered
                    defer:NO];

  [window setTitle:[NSString stringWithUTF8String:title]];
  printf("MacOS_CreateWindow: Window created at %p, title: %s, thread=%p\n",
         (__bridge void *)window, title,
         (__bridge void *)[NSThread currentThread]);
  
  // Create custom content view that accepts keyboard input
  GameContentView *contentView = [[GameContentView alloc] initWithFrame:frame];
  [window setContentView:contentView];
  
  [window setBackgroundColor:[NSColor redColor]];
  fflush(stdout);
  [window makeKeyAndOrderFront:nil];
  [window orderFrontRegardless];
  [window center];

  g_WindowDelegate = [[MacOSWindowDelegate alloc] init];
  [window setDelegate:g_WindowDelegate];

  [NSApp activateIgnoringOtherApps:YES];
  [window makeKeyWindow];
  [window makeMainWindow];
  
  // Make the content view first responder for keyboard input
  [window makeFirstResponder:contentView];

  printf("MacOS_CreateWindow: Window visibility: %s, frame=(%f,%f,%f,%f)\n",
         [window isVisible] ? "YES" : "NO", frame.origin.x, frame.origin.y,
         frame.size.width, frame.size.height);
  printf("MacOS_CreateWindow: firstResponder=%p (contentView=%p)\n",
         (__bridge void *)[window firstResponder],
         (__bridge void *)contentView);
  fflush(stdout);

  return (__bridge void *)window;
}

#import "StdKeyboard.h"
#import "StdMouse.h"

void MacOS_PumpEvents() {
  static int pumpCount = 0;
  BOOL isActive = [NSApp isActive];
  if (pumpCount++ % 50 == 0) {
    NSWindow *keyWin = [NSApp keyWindow];
    printf("DEBUG: Pump heartbeat #%d, isActive=%d, keyWin=%p\n", pumpCount,
           (int)isActive, (__bridge void *)keyWin);
    fflush(stdout);
  }

  if (pumpCount % 100 == 0 && !isActive) {
    [NSApp activateIgnoringOtherApps:YES];
    NSWindow *firstWin = [[NSApp windows] firstObject];
    if (firstWin) {
      [firstWin makeKeyAndOrderFront:nil];
      [firstWin makeKeyWindow];
    }
  }

  @autoreleasepool {
    NSEvent *event;
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:[NSDate distantPast]
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES])) {

      NSEventType type = [event type];

      // Debugging: Log EVERY event type to find the crash
      if (type != NSEventTypeMouseMoved) {
        // printf("COCOA EVENT: type=%lu\n", (unsigned long)type);
        // fflush(stdout);
      }

      unsigned int timestamp = (unsigned int)([event timestamp] * 1000.0);

      switch (type) {
      case NSEventTypeKeyDown:
        if (TheKeyboard) {
          unsigned char keyCode = [event keyCode];
          printf("INPUT: KeyDown raw=%d\n", keyCode);
          fflush(stdout);
          ((StdKeyboard *)TheKeyboard)->addEvent(keyCode, true, timestamp);
        } else {
          printf("INPUT: KeyDown %d IGNORED (TheKeyboard is null)\n",
                 [event keyCode]);
          fflush(stdout);
        }
        break;
      case NSEventTypeKeyUp:
        if (TheKeyboard) {
          unsigned char keyCode = [event keyCode];
          printf("INPUT: KeyUp raw=%d\n", keyCode);
          fflush(stdout);
          ((StdKeyboard *)TheKeyboard)->addEvent(keyCode, false, timestamp);
        }
        break;

      case NSEventTypeMouseMoved:
      case NSEventTypeLeftMouseDragged:
      case NSEventTypeRightMouseDragged:
      case NSEventTypeOtherMouseDragged:
      case NSEventTypeLeftMouseDown:
      case NSEventTypeLeftMouseUp: {
        if (TheMouse) {
          NSPoint loc = [event locationInWindow];
          NSWindow *win = [event window];
          if (!win)
            win = [NSApp keyWindow];
          if (!win)
            win = [[NSApp windows] firstObject];

          if (win) {
            NSView *cv = [win contentView];
            float h = [cv bounds].size.height;
            int x = (int)loc.x;
            int y = (int)(h - loc.y);

            NSEventType type = [event type];
            if (type == NSEventTypeLeftMouseDown) {
              printf("COCOA: DOWN x=%d y=%d (rawY=%.1f WinH=%.1f)\n", x, y,
                     loc.y, h);
              fflush(stdout);
              ((StdMouse *)TheMouse)
                  ->addEvent(MACOS_MOUSE_LBUTTON_DOWN, x, y, 0, 0, timestamp);
            } else if (type == NSEventTypeLeftMouseUp) {
              printf("COCOA: UP x=%d y=%d\n", x, y);
              fflush(stdout);
              ((StdMouse *)TheMouse)
                  ->addEvent(MACOS_MOUSE_LBUTTON_UP, x, y, 0, 0, timestamp);
            } else {
              ((StdMouse *)TheMouse)
                  ->addEvent(MACOS_MOUSE_MOVE, x, y, 0, 0, timestamp);
            }
          }
        }
        break;
      }

      case NSEventTypeRightMouseDown:
        if (TheMouse) {
          NSPoint loc = [event locationInWindow];
          NSWindow *win = [event window];
          if (win) {
            NSView *cv = [win contentView];
            float h = [cv bounds].size.height;
            int x = (int)loc.x;
            int y = (int)(h - loc.y);
            ((StdMouse *)TheMouse)
                ->addEvent(MACOS_MOUSE_RBUTTON_DOWN, x, y, 0, 0, timestamp);
          }
        }
        break;
      case NSEventTypeRightMouseUp:
        if (TheMouse) {
          NSPoint loc = [event locationInWindow];
          NSWindow *win = [event window];
          if (win) {
            NSView *cv = [win contentView];
            float h = [cv bounds].size.height;
            int x = (int)loc.x;
            int y = (int)(h - loc.y);
            ((StdMouse *)TheMouse)
                ->addEvent(MACOS_MOUSE_RBUTTON_UP, x, y, 0, 0, timestamp);
          }
        }
        break;

      case NSEventTypeScrollWheel:
        if (TheMouse) {
          int delta = (int)([event scrollingDeltaY]);
          ((StdMouse *)TheMouse)
              ->addEvent(MACOS_MOUSE_WHEEL, 0, 0, 0, delta, timestamp);
        }
        break;

      case NSEventTypeFlagsChanged:
        // Handle modifier keys (Shift, Ctrl, Alt)
        if (TheKeyboard) {
          unsigned long flags = [event modifierFlags];
          // We could translate flags to key events here if needed
        }
        break;

      default:
        break;
      }

      // Proactively handle activation if we get a click but aren't active
      if (type == NSEventTypeLeftMouseDown && !isActive) {
        [NSApp activateIgnoringOtherApps:YES];
        [[event window] makeKeyWindow];
      }

      [NSApp sendEvent:event];
    }
  }
}

bool MacOS_ShouldQuit() { return g_ShouldQuit; }
