#ifndef MACOS_SCREENSHOT_H
#define MACOS_SCREENSHOT_H

// MacOS Screenshot Module
// Easily enable/disable via ENABLE_SCREENSHOTS define.
// When enabled, call MacOS_SaveScreenshot() after each frame present.
// Screenshots are saved to GameResources/Screenshots/

#ifndef ENABLE_SCREENSHOTS
#define ENABLE_SCREENSHOTS 1 // Set to 0 to disable
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Call once during initialization to set up the screenshot directory
void MacOS_InitScreenshots(void);

// Save a screenshot of the current drawable to disk.
// Returns true if saved successfully.
// intervalFrames: only save every N frames (0 = save every call)
// forceNow: if true, ignore interval and save immediately
bool MacOS_SaveScreenshot(void *mtlTexture, int intervalFrames, bool forceNow);

// Get the path to the last saved screenshot
const char *MacOS_GetLastScreenshotPath(void);

#ifdef __cplusplus
}
#endif

#endif // MACOS_SCREENSHOT_H
