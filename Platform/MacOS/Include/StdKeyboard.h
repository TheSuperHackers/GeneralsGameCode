/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
*/

#pragma once

#include "GameClient/Keyboard.h"

class StdKeyboard : public Keyboard {
public:
  StdKeyboard(void);
  virtual ~StdKeyboard(void);

  virtual void init(void);
  virtual void reset(void);
  virtual void update(void);
  virtual Bool getCapsState(void);

protected:
  virtual void getKey(KeyboardIO *key);

  struct MacOSKeyEvent {
    unsigned char keyCode;
    bool isDown;
    unsigned int time;
  };

  enum { MAX_EVENTS = 256 };
  MacOSKeyEvent m_eventBuffer[MAX_EVENTS];
  unsigned int m_nextFreeIndex;
  unsigned int m_nextGetIndex;
  unsigned long m_lastFlags;

public:
  void addEvent(unsigned char keyCode, bool isDown, unsigned int time);
};
