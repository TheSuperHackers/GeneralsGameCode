#ifndef MACOSGAMECLIENT_H
#define MACOSGAMECLIENT_H

#import "GameClient/GameClient.h"

class SnowManager;

class MacOSGameClient : public GameClient {
public:
  MacOSGameClient();
  virtual ~MacOSGameClient();

  virtual void init(void) override;
  virtual void update(void) override;

  virtual Drawable *
  friend_createDrawable(const ThingTemplate *thing,
                        DrawableStatusBits statusBits) override;

  virtual void addScorch(const Coord3D *pos, Real radius,
                         Scorches type) override;
  virtual void createRayEffectByTemplate(const Coord3D *start,
                                         const Coord3D *end,
                                         const ThingTemplate *tmpl) override;
  virtual void setTeamColor(Int red, Int green, Int blue) override;
  virtual void setTextureLOD(Int level) override;
  virtual void releaseShadows(void) override;
  virtual void allocateShadows(void) override;

  // These pure virtuals only exist in GeneralsMD (Zero Hour) GameClient
#if RTS_ZEROHOUR
  virtual void notifyTerrainObjectMoved(Object *obj) override;
  virtual SnowManager *createSnowManager(void) override;
#endif

protected:
  virtual Display *createGameDisplay(void) override;
  virtual InGameUI *createInGameUI(void) override;
  virtual GameWindowManager *createWindowManager(void) override;
  virtual FontLibrary *createFontLibrary(void) override;
  virtual DisplayStringManager *createDisplayStringManager(void) override;
  virtual VideoPlayerInterface *createVideoPlayer(void) override;
  virtual TerrainVisual *createTerrainVisual(void) override;
  virtual Keyboard *createKeyboard(void) override;
  virtual Mouse *createMouse(void) override;
  virtual void setFrameRate(Real msecsPerFrame) override;
};

#endif // MACOSGAMECLIENT_H
