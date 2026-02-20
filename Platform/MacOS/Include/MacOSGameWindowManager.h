#pragma once

#include "W3DDevice/GameClient/W3DGameWindowManager.h"

// MacOSGameWindowManager inherits from W3DGameWindowManager to get all the
// original W3D gadget draw functions (push buttons, combo boxes, list boxes,
// sliders, progress bars, etc.) that properly render UI images, borders,
// and decorations through the DX8→Metal pipeline.
//
// We override allocateNewWindow() to return a MacOSGameWindow that safely
// handles the case where font->fontData is nullptr (since macOS uses
// MacOSDisplayString for text instead of W3D's Render2DSentenceClass).

class MacOSGameWindow; // forward decl

class MacOSGameWindowManager : public W3DGameWindowManager {
public:
  MacOSGameWindowManager(void);
  virtual ~MacOSGameWindowManager(void);

  // Override to create MacOSGameWindow (safe with nullptr fontData)
  virtual GameWindow *allocateNewWindow(void) override;

  // Override winFormatText/winGetTextSize to use MacOS DisplayString
  virtual void winFormatText(GameFont *font, UnicodeString text, Color color,
                             Int x, Int y, Int width, Int height) override;
  virtual void winGetTextSize(GameFont *font, UnicodeString text, Int *width,
                              Int *height, Int maxWidth) override;
};
