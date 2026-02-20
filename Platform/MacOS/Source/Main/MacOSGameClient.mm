#import "MacOSGameClient.h"
#import "Common/GameEngine.h"
#import "GameClient/Display.h"
#import "GameClient/GameClient.h"
#import "GameClient/GameWindowManager.h"
#import "MacOSGameWindowManager.h"
#import "MacOSWindowManager.h"
#include "PreRTS.h"
#import "StdKeyboard.h"
#import "StdMouse.h"
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include "GameClient/DisplayString.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/GameFont.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Snow.h"
#include "GameClient/TerrainVisual.h"
#include "GameClient/VideoPlayer.h"
#include "GameClient/View.h"
#include "W3DDevice/GameClient/W3DInGameUI.h"
#include "W3DDevice/GameClient/W3DView.h"

extern "C" {
Display *MacOS_CreateDisplay(void);
DisplayStringManager *MacOS_CreateDisplayStringManager(void);
}

class MacOSFontLibrary : public FontLibrary {
public:
  virtual Bool loadFontData(GameFont *font) override {
    if (!font) return FALSE;

    @autoreleasepool {
      // Map font name — game uses "Arial", "Times New Roman", "Generals" etc.
      NSString *fontName = [NSString stringWithUTF8String:font->nameString.str()];
      CGFloat pointSize = (CGFloat)font->pointSize;

      // Try the requested font, fall back to system font
      NSFont *nsFont = [NSFont fontWithName:fontName size:pointSize];
      if (!nsFont && [fontName isEqualToString:@"Generals"]) {
        nsFont = [NSFont fontWithName:@"Arial-BoldMT" size:pointSize];
      }
      if (!nsFont) {
        nsFont = font->bold
          ? [NSFont boldSystemFontOfSize:pointSize]
          : [NSFont systemFontOfSize:pointSize];
      }

      // Calculate pixel height using the same formula as W3D GDI path:
      // font_height = -MulDiv(PointSize, 96, 72) → logical height
      // Then GetTextMetrics returns tmHeight as actual pixel height
      // On macOS: ascender + descender + leading ≈ pixel height
      int pixelHeight = (int)ceil([nsFont ascender] - [nsFont descender] + [nsFont leading]);
      if (pixelHeight < 1) pixelHeight = (int)ceil(pointSize * 96.0 / 72.0);

      font->height = pixelHeight;
      font->fontData = nullptr; // MacOSDisplayString doesn't use fontData

      printf("FONT: Loaded '%s' pt=%d bold=%d → height=%d px\n",
             font->nameString.str(), font->pointSize, (int)font->bold, font->height);
      fflush(stdout);
    }
    return TRUE;
  }
};

class MacOSSnowManager : public SnowManager {
public:
  virtual void init(void) override { SnowManager::init(); }
  virtual void reset(void) override {}
  virtual void update(void) override {}
};

class MacOSTerrainVisual : public TerrainVisual {
public:
  virtual void init(void) override { TerrainVisual::init(); }
  virtual void setRawMapHeight(const ICoord2D *gridPos, Int height) override {}
  virtual Int getRawMapHeight(const ICoord2D *gridPos) override { return 0; }
  virtual void replaceSkyboxTextures(
      const AsciiString *oldTexName[NumSkyboxTextures],
      const AsciiString *newTexName[NumSkyboxTextures]) override {}
  virtual void getTerrainColorAt(Real x, Real y, RGBColor *pColor) override {}
  virtual TerrainType *getTerrainTile(Real x, Real y) override {
    return nullptr;
  }
  virtual void enableWaterGrid(Bool enable) override {}
  virtual void setWaterGridHeightClamps(const WaterHandle *waterTable,
                                        Real minZ, Real maxZ) override {}
  virtual void setWaterAttenuationFactors(const WaterHandle *waterTable, Real a,
                                          Real b, Real c, Real range) override {
  }
  virtual void setWaterTransform(const WaterHandle *waterTable, Real angle,
                                 Real x, Real y, Real z) override {}
  virtual void setWaterTransform(const Matrix3D *transform) override {}
  virtual void getWaterTransform(const WaterHandle *waterTable,
                                 Matrix3D *transform) override {}
  virtual void setWaterGridResolution(const WaterHandle *waterTable,
                                      Real gridCellsX, Real gridCellsY,
                                      Real cellSize) override {}
  virtual void getWaterGridResolution(const WaterHandle *waterTable,
                                      Real *gridCellsX, Real *gridCellsY,
                                      Real *cellSize) override {}
  virtual void changeWaterHeight(Real x, Real y, Real delta) override {}
  virtual void addWaterVelocity(Real worldX, Real worldY, Real velocity,
                                Real preferredHeight) override {}
  virtual Bool getWaterGridHeight(Real worldX, Real worldY,
                                  Real *height) override {
    return FALSE;
  }
  virtual void setTerrainTracksDetail(void) override {}
  virtual void setShoreLineDetail(void) override {}
  virtual void addFactionBib(Object *factionBuilding, Bool highlight,
                             Real extra = 0) override {}
  virtual void removeFactionBib(Object *factionBuilding) override {}
  virtual void addFactionBibDrawable(Drawable *factionBuilding, Bool highlight,
                                     Real extra = 0) override {}
  virtual void removeFactionBibDrawable(Drawable *factionBuilding) override {}
  virtual void removeAllBibs(void) override {}
  virtual void removeBibHighlighting(void) override {}
  virtual void removeTreesAndPropsForConstruction(const Coord3D *pos,
                                                  const GeometryInfo &geom,
                                                  Real angle) override {}
  virtual void addProp(const ThingTemplate *tt, const Coord3D *pos,
                       Real angle) override {}
};

class MacOSVideoPlayer : public VideoPlayer {
public:
  virtual void init(void) override { VideoPlayer::init(); }
  virtual void reset(void) override { VideoPlayer::reset(); }
  virtual void update(void) override {
    static bool first = true;
    if (first) {
      printf("DEBUG: MacOSVideoPlayer::update called!\n");
      fflush(stdout);
      first = false;
    }
    VideoPlayer::update();
  }
  virtual void deinit(void) override { VideoPlayer::deinit(); }
};

MacOSGameClient::MacOSGameClient() {
  printf("DEBUG: MacOSGameClient constructor called!\n");
}
MacOSGameClient::~MacOSGameClient() {}

void MacOSGameClient::init() {
  printf("DEBUG: MacOSGameClient::init entering.\n");
  fflush(stdout);

  GameClient::init();

  printf("DEBUG: MacOSGameClient::init (base class returned).\n");
  fflush(stdout);
}

void MacOSGameClient::update() {
  static int callCount = 0;
  if (callCount < 3) {
    printf("MENU_FLOW: MacOSGameClient::update() #%d\n", callCount++);
    fflush(stdout);
  }
  MacOS_PumpEvents();
  GameClient::update();
}

DisplayStringManager *MacOSGameClient::createDisplayStringManager(void) {
  return MacOS_CreateDisplayStringManager();
}

FontLibrary *MacOSGameClient::createFontLibrary(void) {
  return new MacOSFontLibrary();
}

Display *MacOSGameClient::createGameDisplay(void) {
  return MacOS_CreateDisplay();
}

InGameUI *MacOSGameClient::createInGameUI(void) { return new W3DInGameUI(); }

#include "W3DDevice/GameClient/W3DTerrainVisual.h"

TerrainVisual *MacOSGameClient::createTerrainVisual(void) {
  return new W3DTerrainVisual();
}

VideoPlayerInterface *MacOSGameClient::createVideoPlayer(void) {
  return new MacOSVideoPlayer();
}

GameWindowManager *MacOSGameClient::createWindowManager(void) {
  printf("DEBUG: MacOSGameClient::createWindowManager called!\n");
  return new MacOSGameWindowManager();
}

Keyboard *MacOSGameClient::createKeyboard(void) {
  fprintf(stderr, "DEBUG: MacOSGameClient::createKeyboard called!\n");
  fflush(stderr);
  return new StdKeyboard();
}
Mouse *MacOSGameClient::createMouse(void) {
  Mouse *m = new StdMouse();
  fprintf(stderr,
          "DEBUG: MacOSGameClient::createMouse called! Returning %p (storage "
          "at %p)\n",
          (void *)m, (void *)&TheMouse);
  fflush(stderr);
  return m;
}
void MacOSGameClient::setFrameRate(Real msecsPerFrame) {}

// Stubs for friend functions
#import "GameClient/Drawable.h"

Drawable *
MacOSGameClient::friend_createDrawable(const ThingTemplate *thing,
                                       DrawableStatusBits statusBits) {
  return newInstance(Drawable)(thing, statusBits);
}
void MacOSGameClient::addScorch(const Coord3D *pos, Real radius,
                                Scorches type) {}
void MacOSGameClient::createRayEffectByTemplate(const Coord3D *start,
                                                const Coord3D *end,
                                                const ThingTemplate *tmpl) {}
void MacOSGameClient::setTeamColor(Int red, Int green, Int blue) {}
void MacOSGameClient::setTextureLOD(Int level) {}
void MacOSGameClient::releaseShadows(void) {}
void MacOSGameClient::allocateShadows(void) {}
#if RTS_ZEROHOUR
void MacOSGameClient::notifyTerrainObjectMoved(Object *obj) {}
SnowManager *MacOSGameClient::createSnowManager(void) {
  return new MacOSSnowManager();
}
#endif
