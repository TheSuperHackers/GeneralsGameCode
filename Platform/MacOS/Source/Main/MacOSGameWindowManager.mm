#import "../../Include/MacOSGameWindowManager.h"
#include "GameClient/Display.h"
#include "GameClient/DisplayString.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "W3DDevice/GameClient/W3DGameWindow.h"
#include "PreRTS.h"

// MacOSGameWindow: a subclass of W3DGameWindow that safely handles
// nullptr fontData. On macOS, fonts use CoreText (via MacOSDisplayString)
// rather than Windows GDI (FontCharsClass), so font->fontData is always nullptr.
// W3DGameWindow::winSetFont would crash passing nullptr to Render2DSentenceClass::Set_Font.
// We override to skip that code path.
class MacOSGameWindow : public W3DGameWindow {
  MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(MacOSGameWindow, "MacOSGameWindow")
public:
  MacOSGameWindow(void) : W3DGameWindow() {}

  // Override winSetFont to skip the Render2DSentenceClass::Set_Font call
  // when fontData is nullptr (macOS uses MacOSDisplayString for text rendering)
  void winSetFont(GameFont *font) override {
    if (font == nullptr)
      return;
    // Only call the base GameWindow::winSetFont (which stores the font pointer)
    // but skip W3DGameWindow::winSetFont which calls m_textRenderer.Set_Font()
    GameWindow::winSetFont(font);
    // Don't call m_textRenderer.Set_Font since fontData is nullptr on macOS
  }

  // Override winSetText to skip Render2DSentenceClass::Build_Sentence
  Int winSetText(UnicodeString newText) override {
    // Only update the text through the base class
    return GameWindow::winSetText(newText);
    // Skip m_textRenderer.Build_Sentence which would crash with nullptr font
  }

  // Override drawText. Not virtual in base class, but MacOSGameWindow instances
  // won't use W3DGameWindow::drawText pathway since we skip Build_Sentence.
  // The Render2DSentenceClass will have empty SentenceData so Render() is safe.
  void drawText(Color color) {
    // No-op: text rendering is handled by MacOSDisplayString via
    // WinInstanceData::getTextDisplayString() -> draw()
  }
};

MacOSGameWindow::~MacOSGameWindow() {}

// MacOSGameWindowManager now inherits from W3DGameWindowManager which provides:
// - getDefaultDraw() → W3DGameWinDefaultDraw
// - getPushButtonDrawFunc() → W3DGadgetPushButtonDraw
// - getPushButtonImageDrawFunc() → W3DGadgetPushButtonImageDraw
// - getComboBoxDrawFunc() → W3DGadgetComboBoxDraw
// - getListBoxDrawFunc() → W3DGadgetListBoxDraw
// - getStaticTextDrawFunc() → W3DGadgetStaticTextDraw
// - (all other gadget draw functions)

MacOSGameWindowManager::MacOSGameWindowManager(void) {}
MacOSGameWindowManager::~MacOSGameWindowManager(void) {}

GameWindow *MacOSGameWindowManager::allocateNewWindow(void) {
  return newInstance(MacOSGameWindow);
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
