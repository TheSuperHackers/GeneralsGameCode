# Input and UI Patterns
**Source:** `GeneralsX:old-multiplatform-attempt` (`docs/WORKDIR/phases/legacy/metal_port/PHASE34/SESSION_SUMMARY.md`)
**Time Context:** Oct 2025

## NSEvent Integration
The `MacOS_PumpEvents` function (in `MacOSWindowManager.mm`) is the primary entry point for macOS events into the engine.

### Routing Mechanism
Events follow this path:
1. `NSEvent` (Cocoa) -> `MacOS_PumpEvents()`
2. `MacOS_PumpEvents()` -> `StdMouse::addEvent()` or `StdKeyboard::addEvent()`
3. Game Loop (`GameEngine::update`) processes the input buffer.
4. `GameWindowManager::winProcessMouseEvent()` translates coordinates to UI hits.

### Coordinate Transformation
Generals expects (0,0) at Top-Left. Cocoa uses (0,0) at Bottom-Left.
**The fix implemented in `old-multiplatform-attempt`:**
```objc
NSPoint location = [event locationInWindow];
int x = (int)location.x;
int y = (int)(windowHeight - location.y); // Flip Y axis
```

### Known Input Traps
- **Modifier Keys**: `TheKeyboard` must handle `NSEventModifierFlagShift`, `Control`, `Option`, and `Command`.
- **Mouse Cursors**: The game uses native `.ani` and `.cur` files. The previous port had a full RIFF/ACON parser to convert these into `SDL_Cursor` or `NSCursor` instances.
- **Coordinates scaling**: For Retina displays, `convertRectToBacking:` was necessary to ensure the mouse coordinates matched the Metal viewport dimensions.
