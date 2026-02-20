#include "GameClient/Display.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Image.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Mouse.h"
#include "Lib/BaseType.h"
#include "PreRTS.h"
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include "MacOSDebugLog.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "WW3D2/render2d.h"
#include "WW3D2/ww3d.h"

// MacOSDisplay inherits from W3DDisplay to get all draw methods (drawImage,
// drawLine, drawFillRect, etc.) through Render2DClass → DX8Wrapper → Metal.
//
// Now that WW3D::Init() and DX8Wrapper::Set_Render_Device() are idempotent,
// W3DDisplay::init() can be called safely even when the device already exists.
//
// We override draw() because W3DDisplay::draw() requires many game subsystems
// (TheScriptEngine, TheTacticalView, etc.) that aren't available during shell.
class MacOSDisplay : public W3DDisplay {
public:
  MacOSDisplay() {
    TheDisplay = this;
    printf("DEBUG: MacOSDisplay created (W3DDisplay subclass) at %p\n", this);
    fflush(stdout);
  }
  virtual ~MacOSDisplay() {
    if (TheDisplay == this)
      TheDisplay = nullptr;
  }

  virtual void init(void) override {
    printf("DEBUG: MacOSDisplay::init calling W3DDisplay::init...\n");
    fflush(stdout);

    W3DDisplay::init();

    printf("DEBUG: MacOSDisplay::init finished, width=%d height=%d, "
           "m_2DRender=%p\n",
           (int)getWidth(), (int)getHeight(), (void *)m_2DRender);
    fflush(stdout);
  }

  virtual void reset(void) override { W3DDisplay::reset(); }

  // Override draw() — W3DDisplay::draw() requires TheScriptEngine,
  // TheFramePacer, TheTacticalView, TheParticleSystemManager, etc.
  // which are null during the shell/menu phase. Use simplified loop.
  virtual void draw(void) override {
    static int frameCount = 0;
    frameCount++;

    DLOG_RFLOW(1, "MacOSDisplay::draw frame=%d winMgr=%p inGameUI=%p mouse=%p",
      frameCount, (void*)TheWindowManager, (void*)TheInGameUI, (void*)TheMouse);

    // Simplified render loop — safe with any combination of subsystems
    WW3DErrorType result =
        WW3D::Begin_Render(true, true, Vector3(0.02f, 0.05f, 0.1f));
    if (result != WW3D_ERROR_OK) {
      DLOG_RFLOW(1, "MacOSDisplay::draw frame=%d Begin_Render FAILED result=%d",
        frameCount, (int)result);
      return;
    }

    // Draw all views of the world (may be empty during shell)
    Display::draw();

    // Draw the in-game UI (W3DInGameUI::draw() calls winRepaint() internally)
    if (TheInGameUI) {
      TheInGameUI->draw();
    }

    // Draw the mouse cursor
    if (TheMouse) {
      TheMouse->draw();
    }

    WW3D::End_Render();
  }

  // Inherited from W3DDisplay:
  // drawImage, drawLine, drawFillRect, drawOpenRect, drawRectClock,
  // drawRemainingRectClock, drawScaledVideoBuffer, drawVideoBuffer,
  // setClipRegion, isClippingEnabled, enableClipping, etc.
  // All go through m_2DRender (Render2DClass) → DX8Wrapper → Metal.

  virtual void takeScreenShot(void) override {}
  virtual void toggleMovieCapture(void) override {}
#if defined(RTS_DEBUG)
  virtual void dumpAssetUsage(const char *mapname) override {}
  virtual void dumpModelAssets(const char *path) override {}
#endif
  virtual void update(void) override {}
};

extern "C" Display *MacOS_CreateDisplay(void) {
  Display *d = new MacOSDisplay();
  printf("DEBUG: MacOS_CreateDisplay returning W3DDisplay-based %p\n", d);
  fflush(stdout);
  return d;
}
