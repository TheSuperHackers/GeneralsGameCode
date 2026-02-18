/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
*/

#pragma once

#include "GameClient/Mouse.h"

class StdMouse : public Mouse {
public:
  StdMouse(void);
  virtual ~StdMouse(void);

  virtual void init(void) override;
  virtual void reset(void) override;
  virtual void update(void) override;
  virtual void initCursorResources(void) override;

  virtual void setCursor(MouseCursor cursor) override;
  virtual void setVisibility(Bool visible) override;
  virtual void draw(void) override;

  virtual void loseFocus() override;
  virtual void regainFocus() override;

protected:
  virtual void capture(void) override;
  virtual void releaseCapture(void) override;

  virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush) override;

  struct MacOSMouseEvent {
    int type;
    int x, y;
    int button;
    int wheelDelta;
    unsigned int time;
  };

  enum { MAX_EVENTS = 256 };
  MacOSMouseEvent m_eventBuffer[MAX_EVENTS];
  unsigned int m_nextFreeIndex;
  unsigned int m_nextGetIndex;

public:
  // This will be called from the macOS event loop bridge
  void addEvent(int type, int x, int y, int button, int wheelDelta,
                unsigned int time);
};

// Types for bridge
enum MacOSMouseEventType {
  MACOS_MOUSE_MOVE,
  MACOS_MOUSE_LBUTTON_DOWN,
  MACOS_MOUSE_LBUTTON_UP,
  MACOS_MOUSE_RBUTTON_DOWN,
  MACOS_MOUSE_RBUTTON_UP,
  MACOS_MOUSE_MBUTTON_DOWN,
  MACOS_MOUSE_MBUTTON_UP,
  MACOS_MOUSE_WHEEL,
};
