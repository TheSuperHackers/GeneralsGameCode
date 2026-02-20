#include "StdKeyboard.h"
#include "GameClient/KeyDefs.h"
#include "always.h"

// Simple mapping from macOS virtual key codes to DIK codes
static unsigned char MacOSVirtualKeyToDIK(unsigned short keyCode) {
  switch (keyCode) {
  case 0x00:
    return DIK_A;
  case 0x01:
    return DIK_S;
  case 0x02:
    return DIK_D;
  case 0x03:
    return DIK_F;
  case 0x04:
    return DIK_H;
  case 0x05:
    return DIK_G;
  case 0x06:
    return DIK_Z;
  case 0x07:
    return DIK_X;
  case 0x08:
    return DIK_C;
  case 0x09:
    return DIK_V;
  case 0x0B:
    return DIK_B;
  case 0x0C:
    return DIK_Q;
  case 0x0D:
    return DIK_W;
  case 0x0E:
    return DIK_E;
  case 0x0F:
    return DIK_R;
  case 0x10:
    return DIK_Y;
  case 0x11:
    return DIK_T;
  case 0x12:
    return DIK_1;
  case 0x13:
    return DIK_2;
  case 0x14:
    return DIK_3;
  case 0x15:
    return DIK_4;
  case 0x16:
    return DIK_6;
  case 0x17:
    return DIK_5;
  case 0x18:
    return DIK_EQUALS;
  case 0x19:
    return DIK_9;
  case 0x1A:
    return DIK_7;
  case 0x1B:
    return DIK_MINUS;
  case 0x1C:
    return DIK_8;
  case 0x1D:
    return DIK_0;
  case 0x1E:
    return DIK_RBRACKET;
  case 0x1F:
    return DIK_O;
  case 0x20:
    return DIK_U;
  case 0x21:
    return DIK_LBRACKET;
  case 0x22:
    return DIK_I;
  case 0x23:
    return DIK_P;
  case 0x24:
    return DIK_RETURN;
  case 0x25:
    return DIK_L;
  case 0x26:
    return DIK_J;
  case 0x27:
    return DIK_APOSTROPHE;
  case 0x28:
    return DIK_K;
  case 0x29:
    return DIK_SEMICOLON;
  case 0x2A:
    return DIK_BACKSLASH;
  case 0x2B:
    return DIK_COMMA;
  case 0x2C:
    return DIK_SLASH;
  case 0x2D:
    return DIK_N;
  case 0x2E:
    return DIK_M;
  case 0x30:
    return DIK_TAB;
  case 0x31:
    return DIK_SPACE;
  case 0x32:
    return DIK_GRAVE;
  case 0x33:
    return DIK_BACK;
  case 0x35:
    return DIK_ESCAPE;
  case 0x38:
    return DIK_LSHIFT;
  case 0x39:
    return DIK_CAPITAL;
  case 0x3A:
    return DIK_LALT;
  case 0x3B:
    return DIK_LCONTROL;
  case 0x3C:
    return DIK_RSHIFT;
  case 0x3D:
    return DIK_RALT;
  case 0x3E:
    return DIK_RCONTROL;

  // F-keys
  case 0x7A:
    return DIK_F1;
  case 0x78:
    return DIK_F2;
  case 0x63:
    return DIK_F3;
  case 0x76:
    return DIK_F4;
  case 0x60:
    return DIK_F5;
  case 0x61:
    return DIK_F6;
  case 0x62:
    return DIK_F7;
  case 0x64:
    return DIK_F8;
  case 0x65:
    return DIK_F9;
  case 0x6D:
    return DIK_F10;
  case 0x67:
    return DIK_F11;
  case 0x6F:
    return DIK_F12;

  // Navigation keys
  case 0x7B:
    return DIK_LEFT;
  case 0x7C:
    return DIK_RIGHT;
  case 0x7D:
    return DIK_DOWN;
  case 0x7E:
    return DIK_UP;
  case 0x75:
    return DIK_DELETE;
  case 0x73:
    return DIK_HOME;
  case 0x77:
    return DIK_END;
  case 0x74:
    return DIK_PRIOR;  // Page Up
  case 0x79:
    return DIK_NEXT;   // Page Down

  // Period/dot
  case 0x2F:
    return DIK_PERIOD;
  // Numpad Enter
  case 0x4C:
    return DIK_NUMPADENTER;

  default:
    return 0;
  }
}

StdKeyboard::StdKeyboard(void) {
  m_nextFreeIndex = 0;
  m_nextGetIndex = 0;
}

StdKeyboard::~StdKeyboard(void) {}

void StdKeyboard::init(void) {
  Keyboard::init();
  reset();
}

void StdKeyboard::reset(void) {
  m_nextFreeIndex = 0;
  m_nextGetIndex = 0;
  for (int i = 0; i < KEY_COUNT; ++i) {
    m_keyStatus[i].key = (KeyDefType)i;
    m_keyStatus[i].state = KEY_STATE_UP;
    m_keyStatus[i].status = KeyboardIO::STATUS_UNUSED;
  }
}

void StdKeyboard::update(void) {
  // Call base class update() which calls updateKeys() → getKey()
  // This reads events from our ring buffer into m_keys array
  Keyboard::update();
}

Bool StdKeyboard::getCapsState(void) {
  return (m_keyStatus[DIK_CAPITAL].state & KEY_STATE_DOWN) != 0;
}

void StdKeyboard::getKey(KeyboardIO *result) {
  if (m_nextGetIndex == m_nextFreeIndex) {
    result->key = KEY_NONE;
    return;
  }

  MacOSKeyEvent &ev = m_eventBuffer[m_nextGetIndex];
  m_nextGetIndex = (m_nextGetIndex + 1) % MAX_EVENTS;

  result->key = ev.keyCode;
  result->state = ev.isDown ? KEY_STATE_DOWN : KEY_STATE_UP;
  result->status = KeyboardIO::STATUS_UNUSED;
  result->keyDownTimeMsec = ev.time;
}

void StdKeyboard::addEvent(unsigned char keyCode, bool isDown,
                           unsigned int time) {
  unsigned char dikCode = MacOSVirtualKeyToDIK(keyCode);
  if (dikCode == 0)
    return;

  // Update real-time key states in the base class array
  m_keyStatus[dikCode].state = isDown ? KEY_STATE_DOWN : KEY_STATE_UP;

  unsigned int nextIndex = (m_nextFreeIndex + 1) % MAX_EVENTS;
  if (nextIndex == m_nextGetIndex) {
    // Buffer overflow
    m_nextGetIndex = (m_nextGetIndex + 1) % MAX_EVENTS;
  }

  MacOSKeyEvent &ev = m_eventBuffer[m_nextFreeIndex];
  ev.keyCode = dikCode;
  ev.isDown = isDown;
  ev.time = time;

  m_nextFreeIndex = nextIndex;
}
