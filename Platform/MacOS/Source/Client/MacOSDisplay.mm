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

// For subsystem availability checks
#include "Common/FramePacer.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/View.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/ScriptEngine.h"

// MacOSDisplay inherits from W3DDisplay to get all draw methods (drawImage,
// drawLine, drawFillRect, etc.) through Render2DClass → DX8Wrapper → Metal.
//
// Strategy:
// - When critical subsystems exist (TheGameLogic, TheScriptEngine,
//   TheFramePacer, TheTacticalView, TheParticleSystemManager), delegate
//   to W3DDisplay::draw() which renders the full 3D scene.
// - During shell/menu phase when those are null, use a simplified
//   render loop that only draws views + UI + mouse cursor.
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

  virtual void draw(void) override {
    // W3DDisplay::draw() now has null-safety guards for all subsystems
    // (TheGameLogic, TheScriptEngine, TheFramePacer, TheTacticalView,
    // TheParticleSystemManager, TheWaterTransparency), so it is safe
    // to call at any phase — shell/menu or gameplay.
    W3DDisplay::draw();
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
  virtual void update(void) override { W3DDisplay::update(); }
};

extern "C" Display *MacOS_CreateDisplay(void) {
  Display *d = new MacOSDisplay();
  printf("DEBUG: MacOS_CreateDisplay returning W3DDisplay-based %p\n", d);
  fflush(stdout);
  return d;
}

