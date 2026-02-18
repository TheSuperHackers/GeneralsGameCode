#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include "Common/AsciiString.h"
#include "Common/GameMemory.h"
#include "Common/UnicodeString.h"
#include "GameClient/Display.h"
#include "GameClient/DisplayString.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/GameFont.h"
#include "Lib/BaseType.h"

// DX8 includes for rendering through the Metal pipeline
#include "WW3D2/dx8wrapper.h"
#include "d3d8.h"

class MacOSDisplayString : public DisplayString {
public:
  void *operator new(size_t size, const char *msg) {
    return getClassMemoryPool()->allocateBlockImplementation(msg);
  }
  void operator delete(void *p) { getClassMemoryPool()->freeBlock(p); }
  virtual MemoryPool *getObjectMemoryPool() override {
    return getClassMemoryPool();
  }
  static MemoryPool *getClassMemoryPool() {
    static MemoryPool *pool =
        TheMemoryPoolFactory->findMemoryPool("DisplayString");
    return pool;
  }

  MacOSDisplayString();
  virtual ~MacOSDisplayString();

  void invalidate() { m_Stale = true; }

  virtual void setText(UnicodeString text) override {
    m_textString = text;
    invalidate();
  }
  virtual void setFont(GameFont *font) override {
    m_font = font;
    invalidate();
  }
  virtual void setWordWrap(Int wordWrap) override { invalidate(); }
  virtual void setWordWrapCentered(Bool isCentered) override { invalidate(); }
  virtual void notifyTextChanged(void) override { invalidate(); }

  void updateTexture() {
    if (!m_Stale && m_D3DTexture)
      return;
    if (m_textString.getLength() == 0)
      return;

    AsciiString ascii;
    ascii.translate(m_textString);
    NSString *nsText = [NSString stringWithUTF8String:ascii.str()];
    if (!nsText)
      return;

    float fontSize = 16.0f;
    if (m_font && m_font->pointSize > 0)
      fontSize = (float)m_font->pointSize;

    NSFont *font = [NSFont fontWithName:@"Arial-BoldMT" size:fontSize];
    if (!font)
      font = [NSFont boldSystemFontOfSize:fontSize];

    NSDictionary *attrs = @{
      NSFontAttributeName : font,
      NSForegroundColorAttributeName : [NSColor whiteColor]
    };

    NSSize size = [nsText sizeWithAttributes:attrs];
    if (size.width <= 0 || size.height <= 0)
      return;

    m_Width = (int)ceil(size.width) + 4;
    m_Height = (int)ceil(size.height) + 4;

    // Release old texture if size changed
    if (m_D3DTexture) {
      D3DSURFACE_DESC desc;
      m_D3DTexture->GetLevelDesc(0, &desc);
      if (desc.Width != (unsigned int)m_Width ||
          desc.Height != (unsigned int)m_Height) {
        m_D3DTexture->Release();
        m_D3DTexture = nullptr;
      }
    }

    // Create new texture via DX8 pipeline (MetalDevice8)
    if (!m_D3DTexture) {
      IDirect3DDevice8 *dev = DX8Wrapper::_Get_D3D_Device8();
      if (!dev)
        return;
      HRESULT hr = dev->CreateTexture(m_Width, m_Height, 1, 0, D3DFMT_A8R8G8B8,
                                      D3DPOOL_MANAGED, &m_D3DTexture);
      if (FAILED(hr) || !m_D3DTexture)
        return;
    }

    // Render text to bitmap and upload to texture
    D3DLOCKED_RECT lr;
    if (m_D3DTexture->LockRect(0, &lr, nullptr, 0) == D3D_OK) {
      NSBitmapImageRep *rep = [[NSBitmapImageRep alloc]
          initWithBitmapDataPlanes:NULL
                        pixelsWide:m_Width
                        pixelsHigh:m_Height
                     bitsPerSample:8
                   samplesPerPixel:4
                          hasAlpha:YES
                          isPlanar:NO
                    colorSpaceName:NSDeviceRGBColorSpace
                      bitmapFormat:0
                       bytesPerRow:m_Width * 4
                      bitsPerPixel:32];

      unsigned char *src = [rep bitmapData];
      memset(src, 0, m_Width * m_Height * 4);

      [NSGraphicsContext saveGraphicsState];
      NSGraphicsContext *context =
          [NSGraphicsContext graphicsContextWithBitmapImageRep:rep];
      [NSGraphicsContext setCurrentContext:context];

      [nsText drawInRect:NSMakeRect(2, 0, m_Width - 4, m_Height)
          withAttributes:attrs];
      [NSGraphicsContext restoreGraphicsState];

      unsigned char *dst = (unsigned char *)lr.pBits;
      for (int i = 0; i < m_Width * m_Height; i++) {
        unsigned char r = src[i * 4 + 0];
        unsigned char g = src[i * 4 + 1];
        unsigned char b = src[i * 4 + 2];
        unsigned char a = src[i * 4 + 3];

        // Unpremultiply and convert to BGRA
        if (a > 0 && a < 255) {
          dst[i * 4 + 0] = (unsigned char)((int)b * 255 / a);
          dst[i * 4 + 1] = (unsigned char)((int)g * 255 / a);
          dst[i * 4 + 2] = (unsigned char)((int)r * 255 / a);
        } else {
          dst[i * 4 + 0] = b;
          dst[i * 4 + 1] = g;
          dst[i * 4 + 2] = r;
        }
        dst[i * 4 + 3] = a;
      }
      m_D3DTexture->UnlockRect(0);
    }
    m_Stale = false;
  }

  virtual void draw(Int x, Int y, Color color, Color dropColor) override {
    if (m_textString.getLength() == 0)
      return;

    updateTexture();

    if (m_D3DTexture) {
      // Draw through the DX8/Metal pipeline using TheDisplay
      // For now, use DX8Wrapper directly to set texture and draw a quad
      float fx = (float)x - 2.0f;
      float fy = (float)y;

      // Use the W3D pipeline: DX8Wrapper::Set_Texture + Render2DClass
      DX8Wrapper::Set_Texture(0, nullptr);
      IDirect3DDevice8 *dev = DX8Wrapper::_Get_D3D_Device8();
      if (dev) {
        dev->SetTexture(0, m_D3DTexture);
      }

      // Extract color components
      float a = ((color >> 24) & 0xFF) / 255.0f;
      float r = ((color >> 16) & 0xFF) / 255.0f;
      float g = ((color >> 8) & 0xFF) / 255.0f;
      float b = (color & 0xFF) / 255.0f;

      // Draw a textured quad using DX8Wrapper
      struct TexturedVertex {
        float x, y, z, rhw;
        DWORD diffuse;
        float u, v;
      };

      DWORD d3dColor = color; // Already in D3DCOLOR format (ARGB)

      TexturedVertex verts[4] = {
          {fx, fy, 0.0f, 1.0f, d3dColor, 0.0f, 0.0f},
          {fx + m_Width, fy, 0.0f, 1.0f, d3dColor, 1.0f, 0.0f},
          {fx, fy + m_Height, 0.0f, 1.0f, d3dColor, 0.0f, 1.0f},
          {fx + m_Width, fy + m_Height, 0.0f, 1.0f, d3dColor, 1.0f, 1.0f},
      };

      dev->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
      dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts,
                           sizeof(TexturedVertex));
      dev->SetTexture(0, nullptr);
    }
  }

  virtual void draw(Int x, Int y, Color color, Color dropColor, Int xDrop,
                    Int yDrop) override {
    draw(x, y, color, dropColor);
  }

  virtual void getSize(Int *width, Int *height) override {
    updateTexture();
    if (width)
      *width = m_Width;
    if (height)
      *height = m_Height;
  }

  virtual Int getWidth(Int charPos = -1) override {
    updateTexture();
    return m_Width;
  }

  virtual void setUseHotkey(Bool useHotkey, Color hotKeyColor) override {}

private:
  IDirect3DTexture8 *m_D3DTexture;
  int m_Width, m_Height;
  bool m_Stale;
};

MacOSDisplayString::MacOSDisplayString()
    : m_D3DTexture(nullptr), m_Width(0), m_Height(0), m_Stale(true) {}

MacOSDisplayString::~MacOSDisplayString() {
  if (m_D3DTexture) {
    m_D3DTexture->Release();
    m_D3DTexture = nullptr;
  }
}

class MacOSDisplayStringManager : public DisplayStringManager {
public:
  MacOSDisplayStringManager() { TheDisplayStringManager = this; }
  virtual void init(void) override {
    TheMemoryPoolFactory->createMemoryPool(
        "DisplayString", sizeof(MacOSDisplayString), 100, 100);
    DisplayStringManager::init();
  }
  virtual DisplayString *newDisplayString(void) override {
    MacOSDisplayString *s = new ("DisplayString") MacOSDisplayString();
    link(s);
    return s;
  }
  virtual void freeDisplayString(DisplayString *string) override {
    unLink(string);
    deleteInstance(string);
  }
  virtual DisplayString *getGroupNumeralString(Int numeral) override {
    return nullptr;
  }
  virtual DisplayString *getFormationLetterString(void) override {
    return nullptr;
  }
};

extern "C" DisplayStringManager *MacOS_CreateDisplayStringManager(void) {
  return (DisplayStringManager *)new MacOSDisplayStringManager();
}
