#include "StdMouse.h"
#include "Common/GlobalData.h"
#include "GameClient/Display.h"
#include "GameClient/GameWindow.h"
#include "GameClient/Image.h"
#include "always.h"
#import <AppKit/AppKit.h>

StdMouse::StdMouse(void) {
  m_nextFreeIndex = 0;
  m_nextGetIndex = 0;
}

StdMouse::~StdMouse(void) {}

void StdMouse::init(void) {
  m_inputMovesAbsolute = TRUE;
  setVisibility(TRUE);
}

void StdMouse::reset(void) {
  Mouse::reset();
  m_inputMovesAbsolute = TRUE;
  m_nextFreeIndex = 0;
  m_nextGetIndex = 0;
}

void StdMouse::update(void) { Mouse::update(); }

void StdMouse::initCursorResources(void) {
  // macOS system cursors are usually fine, but we could load custom ones here
}

void StdMouse::setCursor(MouseCursor cursor) {
  // Map engine cursors to macOS cursors
#if defined(__APPLE__) && defined(__OBJC__)
  @autoreleasepool {
    switch (cursor) {
    case ARROW:
    case NORMAL:
      [[NSCursor arrowCursor] set];
      break;
    case CROSS:
      [[NSCursor crosshairCursor] set];
      break;
    case SCROLL:
      [[NSCursor openHandCursor] set];
      break;
    default:
      [[NSCursor arrowCursor] set];
      break;
    }
  }
#endif
}

void StdMouse::setVisibility(Bool visible) {
  m_visible = visible;
  printf("DEBUG: StdMouse::setVisibility(%s) -> m_visible is now %d\n",
         visible ? "TRUE" : "FALSE", (int)m_visible);
  fflush(stdout);
#if defined(__APPLE__) && defined(__OBJC__)
  @autoreleasepool {
    if (visible) {
      [NSCursor unhide];
    } else {
      // Force unhide for now to help debugging
      [NSCursor unhide];
    }
  }
#endif
}

void StdMouse::draw(void) {
  if (!m_visible)
    return;

  CursorInfo *info = &m_cursorInfo[m_currentCursor];
  if (info->imageName.isNotEmpty()) {
    const Image *img =
        TheMappedImageCollection->findImageByName(info->imageName);
    if (!img) {
      static AsciiString lastMissingCursor;
      if (lastMissingCursor != info->imageName) {
        printf("DEBUG: StdMouse::draw: CURSOR IMAGE NOT FOUND: %s\n",
               info->imageName.str());
        fflush(stdout);
        lastMissingCursor = info->imageName;
      }
    }
    if (img) {
      Int w = img->getImageWidth();
      Int h = img->getImageHeight();
      Int x = m_currMouse.pos.x - info->hotSpotPosition.x;
      Int y = m_currMouse.pos.y - info->hotSpotPosition.y;

      TheDisplay->drawImage(img, x, y, x + w, y + h,
                            GameMakeColor(255, 255, 255, 255),
                            Display::DRAW_IMAGE_ALPHA);
    } else {
      // DEBUG FALLBACK: Big Green square (20x20) so it's impossible to miss
      TheDisplay->drawFillRect(m_currMouse.pos.x - 10, m_currMouse.pos.y - 10,
                               20, 20, GameMakeColor(0, 255, 0, 255));
    }
  }

  drawCursorText();
  drawTooltip();
}

void StdMouse::capture(void) {
  // macOS doesn't have a direct "capture" like Win32 SetCapture
  // but we can use CGAssociateMouseAndMouseCursorPosition or similar if needed
}

void StdMouse::releaseCapture(void) {
  // Reverse of capture
}

void StdMouse::regainFocus() {
  // macOS specific focus regain logic
}

void StdMouse::loseFocus() {
  // macOS specific focus loss logic
}

UnsignedByte StdMouse::getMouseEvent(MouseIO *result, Bool flush) {
  static int pollCount = 0;
  if (pollCount++ % 500 == 0) {
    // printf("DEBUG: StdMouse::getMouseEvent polled %d times, buffer
    // size=%d\n",
    //        pollCount, (m_nextFreeIndex - m_nextGetIndex + MAX_EVENTS) %
    //        MAX_EVENTS);
    // fflush(stdout);
  }

  if (m_nextGetIndex == m_nextFreeIndex) {
    if (result) {
      memset(result, 0, sizeof(MouseIO));
      result->pos = m_currMouse.pos;
      result->leftState = m_currMouse.leftState;
      result->rightState = m_currMouse.rightState;
      result->middleState = m_currMouse.middleState;
    }
    return 0;
  }

  MacOSMouseEvent &ev = m_eventBuffer[m_nextGetIndex];
  m_nextGetIndex = (m_nextGetIndex + 1) % MAX_EVENTS;

  memset(result, 0, sizeof(MouseIO));
  result->pos.x = ev.x;
  result->pos.y = ev.y;
  result->time = ev.time;
  result->wheelPos = ev.wheelDelta;

  switch (ev.type) {
  case MACOS_MOUSE_LBUTTON_DOWN:
    result->leftState = MBS_Down;
    break;
  case MACOS_MOUSE_LBUTTON_UP:
    result->leftState = MBS_Up;
    break;
  case MACOS_MOUSE_RBUTTON_DOWN:
    result->rightState = MBS_Down;
    break;
  case MACOS_MOUSE_RBUTTON_UP:
    result->rightState = MBS_Up;
    break;
  case MACOS_MOUSE_MBUTTON_DOWN:
    result->middleState = MBS_Down;
    break;
  case MACOS_MOUSE_MBUTTON_UP:
    result->middleState = MBS_Up;
    break;
  default:
    break;
  }

  if (ev.type != MACOS_MOUSE_MOVE) {
    printf("INPUT: Mouse Event type=%d pos=(%d,%d) L-State=%d\n", ev.type, ev.x,
           ev.y, (int)result->leftState);
    fflush(stdout);
  }

  return 1;
}

void StdMouse::addEvent(int type, int x, int y, int button, int wheelDelta,
                        unsigned int time) {
  // OPTIMIZATION: If same as last move event, skip to avoid flooding
  static int lastX = -1, lastY = -1;
  if (type == MACOS_MOUSE_MOVE && x == lastX && y == lastY) {
    return;
  }
  if (type == MACOS_MOUSE_MOVE) {
    lastX = x;
    lastY = y;
  }

  // Update position immediately so 'draw' stays in sync
  m_currMouse.pos.x = x;
  m_currMouse.pos.y = y;

  // LOG ALL BUTTON EVENTS
  if (type != MACOS_MOUSE_MOVE) {
    printf("QUEUE: addEvent type=%d at (%d, %d)\n", type, x, y);
    fflush(stdout);
  }

  unsigned int nextIndex = (m_nextFreeIndex + 1) % MAX_EVENTS;
  if (nextIndex == m_nextGetIndex) {
    // Buffer overflow, drop oldest event
    m_nextGetIndex = (m_nextGetIndex + 1) % MAX_EVENTS;
  }

  MacOSMouseEvent &ev = m_eventBuffer[m_nextFreeIndex];
  ev.type = type;
  ev.x = x;
  ev.y = y;
  ev.button = button;
  ev.wheelDelta = wheelDelta;
  ev.time = time;

  m_nextFreeIndex = nextIndex;
}
