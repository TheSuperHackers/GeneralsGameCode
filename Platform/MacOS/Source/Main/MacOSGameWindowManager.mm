#import "../../Include/MacOSGameWindowManager.h"
#include "GameClient/Display.h"
#include "GameClient/DisplayString.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "PreRTS.h"
#include <set>
#include <string>

// Simple MacOS GameWindow implementation
class MacOSGameWindow : public GameWindow {
  MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(MacOSGameWindow, "MacOSGameWindow", 256,
                                        32)
public:
  virtual void winDrawBorder(void) override {
    // Basic border drawing could go here if needed
  }
};

MacOSGameWindow::~MacOSGameWindow() {}

MacOSGameWindowManager::MacOSGameWindowManager(void) {}
MacOSGameWindowManager::~MacOSGameWindowManager(void) {}

GameWindow *MacOSGameWindowManager::allocateNewWindow(void) {
  return newInstance(MacOSGameWindow);
}

// Gadget draw functions
GameWinDrawFunc MacOSGameWindowManager::getPushButtonImageDrawFunc(void) {
  return MacOSGadgetPushButtonImageDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getPushButtonDrawFunc(void) {
  return MacOSGadgetPushButtonDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getCheckBoxImageDrawFunc(void) {
  return MacOSGadgetCheckBoxDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getCheckBoxDrawFunc(void) {
  return MacOSGadgetCheckBoxDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getRadioButtonImageDrawFunc(void) {
  return MacOSGadgetRadioButtonDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getRadioButtonDrawFunc(void) {
  return MacOSGadgetRadioButtonDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getTabControlImageDrawFunc(void) {
  return MacOSGadgetTabControlDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getTabControlDrawFunc(void) {
  return MacOSGadgetTabControlDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getListBoxImageDrawFunc(void) {
  return MacOSGadgetListBoxDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getListBoxDrawFunc(void) {
  return MacOSGadgetListBoxDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getComboBoxImageDrawFunc(void) {
  return MacOSGadgetComboBoxDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getComboBoxDrawFunc(void) {
  return MacOSGadgetComboBoxDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getHorizontalSliderImageDrawFunc(void) {
  return MacOSGadgetSliderDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getHorizontalSliderDrawFunc(void) {
  return MacOSGadgetSliderDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getVerticalSliderImageDrawFunc(void) {
  return MacOSGadgetSliderDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getVerticalSliderDrawFunc(void) {
  return MacOSGadgetSliderDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getProgressBarImageDrawFunc(void) {
  return MacOSGadgetProgressBarDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getProgressBarDrawFunc(void) {
  return MacOSGadgetProgressBarDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getStaticTextImageDrawFunc(void) {
  return MacOSGadgetStaticTextDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getStaticTextDrawFunc(void) {
  return MacOSGadgetStaticTextDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getTextEntryImageDrawFunc(void) {
  return MacOSGadgetTextEntryDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getTextEntryDrawFunc(void) {
  return MacOSGadgetTextEntryDraw;
}

void MacOSGameWindowManager::winDrawImage(const Image *image, Int startX,
                                          Int startY, Int endX, Int endY,
                                          Color color) {
  if (!image)
    return;
  // Route through TheDisplay->drawImage which uses the DX8/Metal pipeline
  // (Render2DClass -> DX8Wrapper -> MetalDevice8) instead of the separate
  // MacOSRenderDevice, ensuring all rendering uses the same encoder.
  TheDisplay->drawImage(image, startX, startY, endX, endY, color);
}

void MacOSGameWindowManager::winFillRect(Color color, Real width, Int startX,
                                         Int startY, Int endX, Int endY) {
  TheDisplay->drawFillRect(startX, startY, endX - startX, endY - startY, color);
}

void MacOSGameWindowManager::winOpenRect(Color color, Real width, Int startX,
                                         Int startY, Int endX, Int endY) {
  TheDisplay->drawOpenRect(startX, startY, endX - startX, endY - startY, width,
                           color);
}

void MacOSGameWindowManager::winDrawLine(Color color, Real width, Int startX,
                                         Int startY, Int endX, Int endY) {
  TheDisplay->drawLine(startX, startY, endX, endY, width, color);
}

void MacOSGameWindowManager::winFormatText(GameFont *font, UnicodeString text,
                                           Color color, Int x, Int y, Int width,
                                           Int height) {
  AsciiString ascii;
  ascii.translate(text);

  DisplayString *ds = TheDisplayStringManager->newDisplayString();
  if (ds) {
    ds->setText(text);
    ds->setFont(font);
    ds->draw(x, y, color, 0);
    TheDisplayStringManager->freeDisplayString(ds);
  }
}

void MacOSGameWindowManager::winGetTextSize(GameFont *font, UnicodeString text,
                                            Int *width, Int *height,
                                            Int maxWidth) {
  DisplayString *ds = TheDisplayStringManager->newDisplayString();
  if (ds) {
    ds->setText(text);
    ds->setFont(font);
    ds->getSize(width, height);
    TheDisplayStringManager->freeDisplayString(ds);
  }
}

void MacOSGameWindowManager::assignDefaultGadgetLook(GameWindow *gadget,
                                                     GameFont *defaultFont,
                                                     Bool assignVisual) {
  // Call base class first to set up colors and fonts
  GameWindowManager::assignDefaultGadgetLook(gadget, defaultFont, assignVisual);

  // Any MacOS specific defaults can go here
}
