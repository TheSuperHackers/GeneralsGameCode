#pragma once

#include "GameClient/GameWindowManager.h"

class MacOSGameWindowManager : public GameWindowManager {
public:
  MacOSGameWindowManager(void);
  virtual ~MacOSGameWindowManager(void);

  virtual GameWindow *allocateNewWindow(void) override;

  virtual GameWinDrawFunc getPushButtonImageDrawFunc(void) override;
  virtual GameWinDrawFunc getPushButtonDrawFunc(void) override;
  virtual GameWinDrawFunc getCheckBoxImageDrawFunc(void) override;
  virtual GameWinDrawFunc getCheckBoxDrawFunc(void) override;
  virtual GameWinDrawFunc getRadioButtonImageDrawFunc(void) override;
  virtual GameWinDrawFunc getRadioButtonDrawFunc(void) override;
  virtual GameWinDrawFunc getTabControlImageDrawFunc(void) override;
  virtual GameWinDrawFunc getTabControlDrawFunc(void) override;
  virtual GameWinDrawFunc getListBoxImageDrawFunc(void) override;
  virtual GameWinDrawFunc getListBoxDrawFunc(void) override;
  virtual GameWinDrawFunc getComboBoxImageDrawFunc(void) override;
  virtual GameWinDrawFunc getComboBoxDrawFunc(void) override;
  virtual GameWinDrawFunc getHorizontalSliderImageDrawFunc(void) override;
  virtual GameWinDrawFunc getHorizontalSliderDrawFunc(void) override;
  virtual GameWinDrawFunc getVerticalSliderImageDrawFunc(void) override;
  virtual GameWinDrawFunc getVerticalSliderDrawFunc(void) override;
  virtual GameWinDrawFunc getProgressBarImageDrawFunc(void) override;
  virtual GameWinDrawFunc getProgressBarDrawFunc(void) override;
  virtual GameWinDrawFunc getStaticTextImageDrawFunc(void) override;
  virtual GameWinDrawFunc getStaticTextDrawFunc(void) override;
  virtual GameWinDrawFunc getTextEntryImageDrawFunc(void) override;
  virtual GameWinDrawFunc getTextEntryDrawFunc(void) override;
  virtual void winDrawImage(const Image *image, Int startX, Int startY,
                            Int endX, Int endY,
                            Color color = 0xFFFFFFFF) override;
  virtual void winFillRect(Color color, Real width, Int startX, Int startY,
                           Int endX, Int endY) override;
  virtual void winOpenRect(Color color, Real width, Int startX, Int startY,
                           Int endX, Int endY) override;
  virtual void winDrawLine(Color color, Real width, Int startX, Int startY,
                           Int endX, Int endY) override;
  virtual void winFormatText(GameFont *font, UnicodeString text, Color color,
                             Int x, Int y, Int width, Int height) override;
  virtual void winGetTextSize(GameFont *font, UnicodeString text, Int *width,
                              Int *height, Int maxWidth) override;

  virtual void assignDefaultGadgetLook(GameWindow *gadget,
                                       GameFont *defaultFont,
                                       Bool assignVisual) override;
};

// Gadget draw functions
void MacOSGadgetPushButtonDraw(GameWindow *window, WinInstanceData *instData);
void MacOSGadgetPushButtonImageDraw(GameWindow *window,
                                    WinInstanceData *instData);
void MacOSGadgetStaticTextDraw(GameWindow *window, WinInstanceData *instData);
void MacOSGadgetCheckBoxDraw(GameWindow *window, WinInstanceData *instData);
void MacOSGadgetRadioButtonDraw(GameWindow *window, WinInstanceData *instData);
void MacOSGadgetTabControlDraw(GameWindow *window, WinInstanceData *instData);
void MacOSGadgetListBoxDraw(GameWindow *window, WinInstanceData *instData);
void MacOSGadgetComboBoxDraw(GameWindow *window, WinInstanceData *instData);
void MacOSGadgetSliderDraw(GameWindow *window, WinInstanceData *instData);
void MacOSGadgetProgressBarDraw(GameWindow *window, WinInstanceData *instData);
void MacOSGadgetTextEntryDraw(GameWindow *window, WinInstanceData *instData);
