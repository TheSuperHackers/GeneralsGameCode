#import "../../Include/MacOSGameWindowManager.h"
#include "GameClient/Display.h"
#include "GameClient/DisplayString.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "PreRTS.h"

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
  if (ds && ds->getTextLength() > 0) {
    ICoord2D origin, size;
    window->winGetScreenPosition(&origin.x, &origin.y);
    window->winGetSize(&size.x, &size.y);
    Int tw, th;
    ds->getSize(&tw, &th);
    // Center text
    Int tx = origin.x + (size.x - tw) / 2;
    Int ty = origin.y + (size.y - th) / 2;
    ds->draw(tx, ty, 0xFFFFFFFF, 0xFF000000);
  }
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

  const Image *img = nullptr;
  if (window->winGetStatus() & WIN_STATUS_ENABLED) {
    img = window->winGetEnabledImage(0);
  } else {
    img = window->winGetDisabledImage(0);
  }

  if (img) {
    TheWindowManager->winDrawImage(img, origin.x, origin.y, origin.x + size.x,
                                   origin.y + size.y, 0xFFFFFFFF);
  } else {
    DrawBeveledRect(origin.x, origin.y, size.x, size.y, 0xFF777777);
  }
  DrawGadgetText(window, instData);
}

void MacOSGadgetPushButtonImageDraw(GameWindow *window,
                                    WinInstanceData *instData) {
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
