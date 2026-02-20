#import "../../Include/MacOSGameWindowManager.h"
#include "GameClient/Display.h"
#include "GameClient/DisplayString.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "PreRTS.h"

#include "MacOSDebugLog.h"

// Helper to draw a beveled rectangle
static void DrawBeveledRect(Int x, Int y, Int w, Int h, Color bodyColor) {
  // Draw main body
  TheDisplay->drawFillRect(x, y, w, h, bodyColor);
  // Draw light top/left border
  TheDisplay->drawFillRect(x, y, w, 2, 0xFFAAAAAA);
  TheDisplay->drawFillRect(x, y, 2, h, 0xFFAAAAAA);
  // Draw dark bottom/right border
  TheDisplay->drawFillRect(x, y + h - 2, w, 2, 0xFF444444);
  TheDisplay->drawFillRect(x + w - 2, y, 2, h, 0xFF444444);
}

// Draw text label for a gadget
static void DrawGadgetText(GameWindow *window, WinInstanceData *instData) {
  DisplayString *ds = instData->getTextDisplayString();
  if (ds == nullptr || ds->getTextLength() == 0)
    return;

  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);

  // Set word wrap to button width
  ds->setWordWrapCentered(BitIsSet(instData->getStatus(), WIN_STATUS_WRAP_CENTERED));
  ds->setWordWrap(size.x);

  // Match font to window's font
  if (ds->getFont() != window->winGetFont())
    ds->setFont(window->winGetFont());

  // Get the right text color based on state
  Color textColor, dropColor;
  if (BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED) == FALSE) {
    textColor = window->winGetDisabledTextColor();
    dropColor = window->winGetDisabledTextBorderColor();
  } else if (BitIsSet(instData->getState(), WIN_STATE_HILITED)) {
    textColor = window->winGetHiliteTextColor();
    dropColor = window->winGetHiliteTextBorderColor();
  } else {
    textColor = window->winGetEnabledTextColor();
    dropColor = window->winGetEnabledTextBorderColor();
  }

  // If text color is still undefined or invisible, use fallback
  if (textColor == 0 || textColor == 0x00000000)
    textColor = 0xFFFFFFFF; // white fallback
  if (dropColor == 0)
    dropColor = 0xFF000000; // black outline fallback

  Int tw, th;
  ds->getSize(&tw, &th);
  // Center text
  Int tx = origin.x + (size.x - tw) / 2;
  Int ty = origin.y + (size.y - th) / 2;
  ds->draw(tx, ty, textColor, dropColor);
}

// Fallback draw for generic windows (like backdrops)
void MacOSGadgetDefaultDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);

  const Image *img = window->winGetEnabledImage(0);
  if (img) {
    TheWindowManager->winDrawImage(img, origin.x, origin.y, origin.x + size.x,
                                   origin.y + size.y, 0xFFFFFFFF);
  }
}

void MacOSGadgetPushButtonDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);

  DisplayString *ds = instData->getTextDisplayString();
  Int textLen = ds ? ds->getTextLength() : -1;
  DLOG_RFLOW(4, "MacOSPushButtonDraw pos=(%d,%d) size=(%dx%d) textLen=%d status=0x%x",
    origin.x, origin.y, size.x, size.y, textLen, (unsigned)window->winGetStatus());

  // Try to draw the appropriate image for the button state
  const Image *img = nullptr;
  if (BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED) == FALSE) {
    img = window->winGetDisabledImage(0);
  } else if (BitIsSet(instData->getState(), WIN_STATE_HILITED)) {
    img = window->winGetHiliteImage(0);
  }
  if (!img) {
    img = window->winGetEnabledImage(0);
  }

  if (img) {
    TheWindowManager->winDrawImage(img, origin.x, origin.y, origin.x + size.x,
                                   origin.y + size.y, 0xFFFFFFFF);
  } else {
    // Fallback: dark background if no texture
    DrawBeveledRect(origin.x, origin.y, size.x, size.y, 0xFF333333);
  }

  DrawGadgetText(window, instData);
}

void MacOSGadgetPushButtonImageDraw(GameWindow *window,
                                    WinInstanceData *instData) {
  DLOG_RFLOW(5, "MacOSPushButtonImageDraw -> forwarding to MacOSPushButtonDraw");
  MacOSGadgetPushButtonDraw(window, instData);
}

void MacOSGadgetStaticTextDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);

  const Image *img = window->winGetEnabledImage(0);
  if (img) {
    TheWindowManager->winDrawImage(img, origin.x, origin.y, origin.x + size.x,
                                   origin.y + size.y, 0xFFFFFFFF);
  }
}

void MacOSGadgetCheckBoxDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  DrawBeveledRect(origin.x, origin.y, 16, 16, 0xFF333333);
}

void MacOSGadgetRadioButtonDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  DrawBeveledRect(origin.x, origin.y, 16, 16, 0xFF333333);
}

void MacOSGadgetTabControlDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  DrawBeveledRect(origin.x, origin.y, size.x, size.y, 0xFF555555);
}

void MacOSGadgetListBoxDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  DrawBeveledRect(origin.x, origin.y, size.x, size.y, 0xFF111111);
}

void MacOSGadgetComboBoxDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  DrawBeveledRect(origin.x, origin.y, size.x, size.y, 0xFF777777);
}

void MacOSGadgetSliderDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  TheDisplay->drawFillRect(origin.x, origin.y + size.y / 2 - 1, size.x, 2,
                           0xFFFFFFFF);
}

void MacOSGadgetProgressBarDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  DrawBeveledRect(origin.x, origin.y, size.x, size.y, 0xFF222222);
  TheDisplay->drawFillRect(origin.x + 2, origin.y + 2, (size.x - 4) / 2,
                           size.y - 4, 0xFF00FF00); // 50% dummy
}

void MacOSGadgetTextEntryDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  DrawBeveledRect(origin.x, origin.y, size.x, size.y, 0xFF000000);
}
