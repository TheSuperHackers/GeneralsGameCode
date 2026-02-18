#include "MacOSScreenshot.h"
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <stdio.h>
#include <sys/stat.h>

#if ENABLE_SCREENSHOTS

static char s_screenshotDir[512] = {0};
static char s_lastPath[512] = {0};
static int s_frameCounter = 0;
static int s_screenshotIndex = 0;

void MacOS_InitScreenshots(void) {
  // Create Screenshots directory in working directory
  snprintf(s_screenshotDir, sizeof(s_screenshotDir), "Screenshots");
  mkdir(s_screenshotDir, 0755);
  printf("[Screenshot] Module initialized. Dir: %s\n", s_screenshotDir);
}

bool MacOS_SaveScreenshot(void *mtlTexture, int intervalFrames, bool forceNow) {
  s_frameCounter++;

  if (!forceNow && intervalFrames > 0 &&
      (s_frameCounter % intervalFrames) != 0) {
    return false;
  }

  @autoreleasepool {
    id<MTLTexture> tex = (__bridge id<MTLTexture>)mtlTexture;
    if (!tex) {
      printf("[Screenshot] Error: null texture\n");
      return false;
    }

    NSUInteger w = tex.width;
    NSUInteger h = tex.height;
    NSUInteger bytesPerRow = w * 4;
    NSUInteger totalBytes = bytesPerRow * h;

    uint8_t *pixels = (uint8_t *)malloc(totalBytes);
    if (!pixels)
      return false;

    [tex getBytes:pixels
        bytesPerRow:bytesPerRow
         fromRegion:MTLRegionMake2D(0, 0, w, h)
        mipmapLevel:0];

    // BGRA -> RGBA swap
    for (NSUInteger i = 0; i < totalBytes; i += 4) {
      uint8_t tmp = pixels[i];   // B
      pixels[i] = pixels[i + 2]; // R
      pixels[i + 2] = tmp;       // B
    }

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx =
        CGBitmapContextCreate(pixels, w, h, 8, bytesPerRow, colorSpace,
                              kCGImageAlphaPremultipliedLast);

    if (!ctx) {
      free(pixels);
      CGColorSpaceRelease(colorSpace);
      printf("[Screenshot] Error: failed to create bitmap context\n");
      return false;
    }

    CGImageRef image = CGBitmapContextCreateImage(ctx);

    snprintf(s_lastPath, sizeof(s_lastPath), "%s/frame_%05d.png",
             s_screenshotDir, s_screenshotIndex++);

    CFURLRef url = CFURLCreateFromFileSystemRepresentation(
        kCFAllocatorDefault, (const UInt8 *)s_lastPath, strlen(s_lastPath),
        false);
    CGImageDestinationRef dest =
        CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, NULL);

    if (dest) {
      CGImageDestinationAddImage(dest, image, NULL);
      CGImageDestinationFinalize(dest);
      CFRelease(dest);

      // Print every 10th screenshot to avoid log spam
      if (s_screenshotIndex % 10 == 1 || forceNow) {
        printf("[Screenshot] Saved: %s (%lux%lu)\n", s_lastPath,
               (unsigned long)w, (unsigned long)h);
      }
    }

    CFRelease(url);
    CGImageRelease(image);
    CGContextRelease(ctx);
    CGColorSpaceRelease(colorSpace);
    free(pixels);

    return true;
  }
}

const char *MacOS_GetLastScreenshotPath(void) { return s_lastPath; }

#else // !ENABLE_SCREENSHOTS

void MacOS_InitScreenshots(void) {}
bool MacOS_SaveScreenshot(void *mtlTexture, int intervalFrames, bool forceNow) {
  return false;
}
const char *MacOS_GetLastScreenshotPath(void) { return ""; }

#endif // ENABLE_SCREENSHOTS
