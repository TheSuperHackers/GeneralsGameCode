/*
**	Command & Conquer Generals Zero Hour(tm)
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PreRTS.h"

#include "Common/LobbyDiscord.h"

#include "Common/AsciiString.h"
#include "Common/UnicodeString.h"
#include "Common/FileSystem.h"
#include "Common/File.h"
#include "Common/GlobalData.h"
#include "Common/MultiplayerSettings.h"
#include "Common/PlayerTemplate.h"
#include "GameClient/MapUtil.h"
#include "GameClient/Image.h"
#include "GameNetwork/GameInfo.h"
#include "GameNetwork/LANAPI.h"
#include "GameNetwork/LANAPICallbacks.h"
#include "DiscordWebhook.h"

#include <zlib.h>
#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#pragma comment(lib, "wininet.lib")

// =========================================================================
// 8x8 bitmap font (printable ASCII 32..126)
// -------------------------------------------------------------------------
// Public-domain font derived from the IBM PC BIOS / dhepper/font8x8 set
// (https://github.com/dhepper/font8x8 — author: Daniel Hepper, public
// domain). Each glyph is 8 rows of 8 bits packed into one byte per row,
// LSB = leftmost pixel. Characters outside the printable range fall back
// to a blank glyph.
// =========================================================================
static const unsigned char kFont8x8[95][8] = {
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // 0x20 ' '
	{0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, // 0x21 '!'
	{0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00}, // 0x22 '"'
	{0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0x00}, // 0x23 '#'
	{0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00}, // 0x24 '$'
	{0x00,0x63,0x33,0x18,0x0C,0x66,0x63,0x00}, // 0x25 '%'
	{0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00}, // 0x26 '&'
	{0x06,0x06,0x03,0x00,0x00,0x00,0x00,0x00}, // 0x27 '''
	{0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00}, // 0x28 '('
	{0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00}, // 0x29 ')'
	{0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, // 0x2A '*'
	{0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00}, // 0x2B '+'
	{0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x06}, // 0x2C ','
	{0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00}, // 0x2D '-'
	{0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00}, // 0x2E '.'
	{0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00}, // 0x2F '/'
	{0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00}, // 0x30 '0'
	{0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00}, // 0x31 '1'
	{0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00}, // 0x32 '2'
	{0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00}, // 0x33 '3'
	{0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00}, // 0x34 '4'
	{0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00}, // 0x35 '5'
	{0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00}, // 0x36 '6'
	{0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00}, // 0x37 '7'
	{0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00}, // 0x38 '8'
	{0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00}, // 0x39 '9'
	{0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x00}, // 0x3A ':'
	{0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x06}, // 0x3B ';'
	{0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00}, // 0x3C '<'
	{0x00,0x00,0x3F,0x00,0x00,0x3F,0x00,0x00}, // 0x3D '='
	{0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00}, // 0x3E '>'
	{0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00}, // 0x3F '?'
	{0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00}, // 0x40 '@'
	{0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0x00}, // 0x41 'A'
	{0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00}, // 0x42 'B'
	{0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00}, // 0x43 'C'
	{0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00}, // 0x44 'D'
	{0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00}, // 0x45 'E'
	{0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00}, // 0x46 'F'
	{0x3C,0x66,0x03,0x03,0x73,0x66,0x7C,0x00}, // 0x47 'G'
	{0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00}, // 0x48 'H'
	{0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, // 0x49 'I'
	{0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00}, // 0x4A 'J'
	{0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00}, // 0x4B 'K'
	{0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00}, // 0x4C 'L'
	{0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,0x00}, // 0x4D 'M'
	{0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00}, // 0x4E 'N'
	{0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00}, // 0x4F 'O'
	{0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00}, // 0x50 'P'
	{0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00}, // 0x51 'Q'
	{0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00}, // 0x52 'R'
	{0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00}, // 0x53 'S'
	{0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, // 0x54 'T'
	{0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00}, // 0x55 'U'
	{0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00}, // 0x56 'V'
	{0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, // 0x57 'W'
	{0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00}, // 0x58 'X'
	{0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00}, // 0x59 'Y'
	{0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00}, // 0x5A 'Z'
	{0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0x00}, // 0x5B '['
	{0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00}, // 0x5C '\'
	{0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0x00}, // 0x5D ']'
	{0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00}, // 0x5E '^'
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF}, // 0x5F '_'
	{0x0C,0x0C,0x18,0x00,0x00,0x00,0x00,0x00}, // 0x60 '`'
	{0x00,0x00,0x1E,0x30,0x3E,0x33,0x6E,0x00}, // 0x61 'a'
	{0x07,0x06,0x06,0x3E,0x66,0x66,0x3B,0x00}, // 0x62 'b'
	{0x00,0x00,0x1E,0x33,0x03,0x33,0x1E,0x00}, // 0x63 'c'
	{0x38,0x30,0x30,0x3E,0x33,0x33,0x6E,0x00}, // 0x64 'd'
	{0x00,0x00,0x1E,0x33,0x3F,0x03,0x1E,0x00}, // 0x65 'e'
	{0x1C,0x36,0x06,0x0F,0x06,0x06,0x0F,0x00}, // 0x66 'f'
	{0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x1F}, // 0x67 'g'
	{0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0x00}, // 0x68 'h'
	{0x0C,0x00,0x0E,0x0C,0x0C,0x0C,0x1E,0x00}, // 0x69 'i'
	{0x30,0x00,0x30,0x30,0x30,0x33,0x33,0x1E}, // 0x6A 'j'
	{0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0x00}, // 0x6B 'k'
	{0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, // 0x6C 'l'
	{0x00,0x00,0x33,0x7F,0x7F,0x6B,0x63,0x00}, // 0x6D 'm'
	{0x00,0x00,0x1F,0x33,0x33,0x33,0x33,0x00}, // 0x6E 'n'
	{0x00,0x00,0x1E,0x33,0x33,0x33,0x1E,0x00}, // 0x6F 'o'
	{0x00,0x00,0x3B,0x66,0x66,0x3E,0x06,0x0F}, // 0x70 'p'
	{0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x78}, // 0x71 'q'
	{0x00,0x00,0x3B,0x6E,0x66,0x06,0x0F,0x00}, // 0x72 'r'
	{0x00,0x00,0x3E,0x03,0x1E,0x30,0x1F,0x00}, // 0x73 's'
	{0x08,0x0C,0x3E,0x0C,0x0C,0x2C,0x18,0x00}, // 0x74 't'
	{0x00,0x00,0x33,0x33,0x33,0x33,0x6E,0x00}, // 0x75 'u'
	{0x00,0x00,0x33,0x33,0x33,0x1E,0x0C,0x00}, // 0x76 'v'
	{0x00,0x00,0x63,0x6B,0x7F,0x7F,0x36,0x00}, // 0x77 'w'
	{0x00,0x00,0x63,0x36,0x1C,0x36,0x63,0x00}, // 0x78 'x'
	{0x00,0x00,0x33,0x33,0x33,0x3E,0x30,0x1F}, // 0x79 'y'
	{0x00,0x00,0x3F,0x19,0x0C,0x26,0x3F,0x00}, // 0x7A 'z'
	{0x38,0x0C,0x0C,0x07,0x0C,0x0C,0x38,0x00}, // 0x7B '{'
	{0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00}, // 0x7C '|'
	{0x07,0x0C,0x0C,0x38,0x0C,0x0C,0x07,0x00}, // 0x7D '}'
	{0x6E,0x3B,0x00,0x00,0x00,0x00,0x00,0x00}, // 0x7E '~'
};

static const unsigned char *glyph(unsigned char c)
{
	if (c < 0x20 || c > 0x7E)
		return kFont8x8[0]; // blank
	return kFont8x8[c - 0x20];
}

// =========================================================================
// RGB pixel buffer + drawing primitives.
// =========================================================================
struct PixBuf
{
	int w, h;
	unsigned char *rgb; // tightly packed, w*h*3 bytes, top-down

	PixBuf() : w(0), h(0), rgb(nullptr) {}
	~PixBuf() { free(rgb); }
};

static inline void putPixel(PixBuf *b, int x, int y, unsigned char r, unsigned char g, unsigned char bl)
{
	if (x < 0 || y < 0 || x >= b->w || y >= b->h)
		return;
	unsigned char *p = b->rgb + (y * b->w + x) * 3;
	p[0] = r; p[1] = g; p[2] = bl;
}

static void fillRect(PixBuf *b, int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char bl)
{
	int x0 = x < 0 ? 0 : x;
	int y0 = y < 0 ? 0 : y;
	int x1 = x + w; if (x1 > b->w) x1 = b->w;
	int y1 = y + h; if (y1 > b->h) y1 = b->h;
	int yy, xx;
	for (yy = y0; yy < y1; ++yy)
	{
		unsigned char *p = b->rgb + (yy * b->w + x0) * 3;
		for (xx = x0; xx < x1; ++xx)
		{
			p[0] = r; p[1] = g; p[2] = bl;
			p += 3;
		}
	}
}

static void drawFilledCircle(PixBuf *b, int cx, int cy, int radius,
                             unsigned char r, unsigned char g, unsigned char bl)
{
	int dy, dx;
	int r2 = radius * radius;
	for (dy = -radius; dy <= radius; ++dy)
	{
		for (dx = -radius; dx <= radius; ++dx)
		{
			if (dx * dx + dy * dy <= r2)
				putPixel(b, cx + dx, cy + dy, r, g, bl);
		}
	}
}

static void drawCircleOutline(PixBuf *b, int cx, int cy, int radius, int thickness,
                              unsigned char r, unsigned char g, unsigned char bl)
{
	int dy, dx;
	int outer2 = radius * radius;
	int inner = radius - thickness; if (inner < 0) inner = 0;
	int inner2 = inner * inner;
	for (dy = -radius; dy <= radius; ++dy)
	{
		for (dx = -radius; dx <= radius; ++dx)
		{
			int d2 = dx * dx + dy * dy;
			if (d2 <= outer2 && d2 >= inner2)
				putPixel(b, cx + dx, cy + dy, r, g, bl);
		}
	}
}

// Render a single 8x8 glyph at (x, y) at integer scale `scale`, with `fg`
// for ink and (when bg_alpha != 0) `bg` filled behind for legibility.
static void drawGlyph(PixBuf *b, int x, int y, unsigned char c, int scale,
                      unsigned char fr, unsigned char fg, unsigned char fb,
                      bool drawBg, unsigned char br, unsigned char bg, unsigned char bb)
{
	const unsigned char *rows = glyph(c);
	int row, col;
	if (drawBg)
		fillRect(b, x, y, 8 * scale, 8 * scale, br, bg, bb);
	for (row = 0; row < 8; ++row)
	{
		unsigned char bits = rows[row];
		for (col = 0; col < 8; ++col)
		{
			if (bits & (1 << col))
				fillRect(b, x + col * scale, y + row * scale, scale, scale, fr, fg, fb);
		}
	}
}

// String width in pixels at the given scale (8 px per glyph at scale 1,
// no inter-glyph spacing — the font's own right column is empty so
// glyphs already read with one pixel of breathing room).
static int textWidthPx(const char *s, int scale)
{
	int n = 0;
	const char *p;
	for (p = s; *p; ++p) ++n;
	return n * 8 * scale;
}

static void drawText(PixBuf *b, int x, int y, const char *s, int scale,
                     unsigned char fr, unsigned char fg, unsigned char fb,
                     bool drawBg, unsigned char br, unsigned char bg, unsigned char bb)
{
	int dx = 0;
	const char *p;
	for (p = s; *p; ++p)
	{
		drawGlyph(b, x + dx, y, (unsigned char)*p, scale,
		          fr, fg, fb, drawBg, br, bg, bb);
		dx += 8 * scale;
	}
}

// Stroked text: draw the string in black at the eight 1-pixel offsets
// around (x, y), then in fg on top. Approximates radarvan's CSS
// `textShadow: 0 0 3px #000` look — soft-ish black outline that keeps
// white text readable over arbitrary terrain colors without an opaque
// backing rectangle.
static void drawTextStroked(PixBuf *b, int x, int y, const char *s, int scale,
                            unsigned char fr, unsigned char fg, unsigned char fb)
{
	int dy, dx;
	for (dy = -1; dy <= 1; ++dy)
	{
		for (dx = -1; dx <= 1; ++dx)
		{
			if (dx == 0 && dy == 0) continue;
			drawText(b, x + dx, y + dy, s, scale,
			         0, 0, 0, false, 0, 0, 0);
		}
	}
	drawText(b, x, y, s, scale, fr, fg, fb, false, 0, 0, 0);
}

// Stroked single-character symbol centered on (cx, cy). Used for the
// supply/tech/crate/derrick overlays so they read against any terrain.
static void drawSymbolCentered(PixBuf *b, int cx, int cy, char sym, int scale,
                               unsigned char fr, unsigned char fg, unsigned char fb)
{
	char tmp[2] = { sym, 0 };
	int w = 8 * scale;
	int h = 8 * scale;
	drawTextStroked(b, cx - w / 2, cy - h / 2, tmp, scale, fr, fg, fb);
}

// Nearest-neighbor upscale into a fresh buffer.
static bool upscaleNN(const PixBuf *src, int scale, PixBuf *dst)
{
	if (scale < 1) scale = 1;
	dst->w = src->w * scale;
	dst->h = src->h * scale;
	dst->rgb = (unsigned char *)malloc((size_t)dst->w * dst->h * 3);
	if (!dst->rgb)
	{
		dst->w = dst->h = 0;
		return false;
	}
	int sy, sx, ry, rx;
	for (sy = 0; sy < src->h; ++sy)
	{
		const unsigned char *srow = src->rgb + sy * src->w * 3;
		for (ry = 0; ry < scale; ++ry)
		{
			unsigned char *drow = dst->rgb + (sy * scale + ry) * dst->w * 3;
			unsigned char *dp = drow;
			for (sx = 0; sx < src->w; ++sx)
			{
				const unsigned char *sp = srow + sx * 3;
				for (rx = 0; rx < scale; ++rx)
				{
					dp[0] = sp[0]; dp[1] = sp[1]; dp[2] = sp[2];
					dp += 3;
				}
			}
		}
	}
	return true;
}

// =========================================================================
// TGA loader (uncompressed TrueColor, 24/32 bpp). Reads via TheFileSystem
// so the file resolves equally from a .big archive or a loose file.
// Decodes into a top-down RGB888 buffer.
// =========================================================================
static bool loadTgaFromFileSystem(const AsciiString& path, PixBuf *out)
{
	out->w = out->h = 0;
	out->rgb = nullptr;
	if (path.isEmpty())
		return false;

	File *f = TheFileSystem->openFile(path.str(), File::READ | File::BINARY);
	if (!f)
		return false;

	int total = f->seek(0, File::END);
	f->seek(0, File::START);
	if (total < 18)
	{
		f->close();
		return false;
	}

	unsigned char *buf = (unsigned char *)malloc(total);
	if (!buf)
	{
		f->close();
		return false;
	}
	if (f->read(buf, total) != total)
	{
		free(buf);
		f->close();
		return false;
	}
	f->close();

	unsigned char idLen     = buf[0];
	unsigned char cmapType  = buf[1];
	unsigned char imgType   = buf[2];
	unsigned short width    = (unsigned short)(buf[12] | (buf[13] << 8));
	unsigned short height   = (unsigned short)(buf[14] | (buf[15] << 8));
	unsigned char pixDepth  = buf[16];
	unsigned char descByte  = buf[17];

	bool ok = (imgType == 2) // truecolor uncompressed
	          && (cmapType == 0)
	          && (pixDepth == 24 || pixDepth == 32)
	          && width > 0 && height > 0
	          && width <= 4096 && height <= 4096;
	if (!ok)
	{
		free(buf);
		return false;
	}

	int bpp = pixDepth / 8;
	int dataOff = 18 + (int)idLen; // skip image-id field; no colormap for truecolor
	int needed = width * height * bpp;
	if (dataOff + needed > total)
	{
		free(buf);
		return false;
	}

	out->w = width;
	out->h = height;
	out->rgb = (unsigned char *)malloc((size_t)width * height * 3);
	if (!out->rgb)
	{
		free(buf);
		out->w = out->h = 0;
		return false;
	}

	bool topDown = (descByte & 0x20) != 0;
	int y, x;
	for (y = 0; y < height; ++y)
	{
		int srcRow = topDown ? y : (height - 1 - y);
		const unsigned char *src = buf + dataOff + srcRow * width * bpp;
		unsigned char *dst = out->rgb + y * width * 3;
		for (x = 0; x < width; ++x)
		{
			// TGA on disk is BGR(A); flip to RGB.
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst += 3;
			src += bpp;
		}
	}

	free(buf);
	return true;
}

// =========================================================================
// RGBA pixel buffer + helpers used to load icon textures from the game's
// MappedImage atlases (Cash, TecBuilding, ZuluCrateIcon, ZuluDerrickIcon)
// and alpha-composite them onto the upscaled map preview.
// =========================================================================
struct PixBufA
{
	int w, h;
	unsigned char *rgba; // tightly packed, w*h*4 bytes, top-down
	PixBufA() : w(0), h(0), rgba(nullptr) {}
	~PixBufA() { free(rgba); }
};

// Load an uncompressed truecolor TGA (24- or 32-bit) into RGBA. 24-bit
// sources get an opaque alpha (255). Same TheFileSystem-backed open path
// as loadTgaFromFileSystem so .big-archive resolution works.
static bool loadTgaRGBAFromFileSystem(const AsciiString& path, PixBufA *out)
{
	out->w = out->h = 0;
	out->rgba = nullptr;
	if (path.isEmpty()) return false;

	File *f = TheFileSystem->openFile(path.str(), File::READ | File::BINARY);
	if (!f) return false;

	int total = f->seek(0, File::END);
	f->seek(0, File::START);
	if (total < 18) { f->close(); return false; }

	unsigned char *buf = (unsigned char *)malloc(total);
	if (!buf) { f->close(); return false; }
	if (f->read(buf, total) != total) { free(buf); f->close(); return false; }
	f->close();

	unsigned char idLen     = buf[0];
	unsigned char cmapType  = buf[1];
	unsigned char imgType   = buf[2];
	unsigned short width    = (unsigned short)(buf[12] | (buf[13] << 8));
	unsigned short height   = (unsigned short)(buf[14] | (buf[15] << 8));
	unsigned char pixDepth  = buf[16];
	unsigned char descByte  = buf[17];

	bool ok = (imgType == 2)
	          && (cmapType == 0)
	          && (pixDepth == 24 || pixDepth == 32)
	          && width > 0 && height > 0
	          && width <= 4096 && height <= 4096;
	if (!ok) { free(buf); return false; }

	int bpp = pixDepth / 8;
	int dataOff = 18 + (int)idLen;
	int needed = width * height * bpp;
	if (dataOff + needed > total) { free(buf); return false; }

	out->w = width;
	out->h = height;
	out->rgba = (unsigned char *)malloc((size_t)width * height * 4);
	if (!out->rgba) { free(buf); out->w = out->h = 0; return false; }

	bool topDown = (descByte & 0x20) != 0;
	int y, x;
	for (y = 0; y < height; ++y)
	{
		int srcRow = topDown ? y : (height - 1 - y);
		const unsigned char *src = buf + dataOff + srcRow * width * bpp;
		unsigned char *dst = out->rgba + y * width * 4;
		for (x = 0; x < width; ++x)
		{
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst[3] = (bpp == 4) ? src[3] : 0xFF;
			dst += 4;
			src += bpp;
		}
	}

	free(buf);
	return true;
}

// Nearest-neighbor scale of an RGBA buffer into a new buffer. Used so the
// stock Cash/TecBuilding (30x30 inside a 512 atlas) and Zulu icons (64x64
// full TGAs) all render at the same target size on the composite.
static bool scaleRGBA_NN(const PixBufA *src, int dstW, int dstH, PixBufA *dst)
{
	if (dstW <= 0 || dstH <= 0 || src->w <= 0 || src->h <= 0) return false;
	dst->w = dstW;
	dst->h = dstH;
	dst->rgba = (unsigned char *)malloc((size_t)dstW * dstH * 4);
	if (!dst->rgba) { dst->w = dst->h = 0; return false; }
	int y, x;
	for (y = 0; y < dstH; ++y)
	{
		int sy = y * src->h / dstH;
		if (sy >= src->h) sy = src->h - 1;
		for (x = 0; x < dstW; ++x)
		{
			int sx = x * src->w / dstW;
			if (sx >= src->w) sx = src->w - 1;
			const unsigned char *sp = src->rgba + (sy * src->w + sx) * 4;
			unsigned char *dp = dst->rgba + (y * dstW + x) * 4;
			dp[0] = sp[0]; dp[1] = sp[1]; dp[2] = sp[2]; dp[3] = sp[3];
		}
	}
	return true;
}

// Alpha-composite an RGBA source onto an RGB destination at (dstX, dstY).
// Out-of-bounds pixels are clipped, fully-transparent pixels are skipped.
static void compositeRGBA(PixBuf *dst, int dstX, int dstY, const PixBufA *src)
{
	if (!dst || !src || !dst->rgb || !src->rgba) return;
	int y, x;
	for (y = 0; y < src->h; ++y)
	{
		int dy = dstY + y;
		if (dy < 0 || dy >= dst->h) continue;
		for (x = 0; x < src->w; ++x)
		{
			int dx = dstX + x;
			if (dx < 0 || dx >= dst->w) continue;
			const unsigned char *sp = src->rgba + (y * src->w + x) * 4;
			unsigned a = sp[3];
			if (a == 0) continue;
			unsigned char *dp = dst->rgb + (dy * dst->w + dx) * 3;
			if (a == 255)
			{
				dp[0] = sp[0]; dp[1] = sp[1]; dp[2] = sp[2];
			}
			else
			{
				// Standard "over" blend: dst = src*a + dst*(1-a)
				unsigned ia = 255 - a;
				dp[0] = (unsigned char)((sp[0] * a + dp[0] * ia) / 255);
				dp[1] = (unsigned char)((sp[1] * a + dp[1] * ia) / 255);
				dp[2] = (unsigned char)((sp[2] * a + dp[2] * ia) / 255);
			}
		}
	}
}

// Look up a MappedImage by name (e.g. "Cash", "TecBuilding",
// "ZuluCrateIcon", "ZuluDerrickIcon"), open its source TGA via
// TheFileSystem (trying the standard texture-search paths in order),
// crop to the MappedImage's UV sub-rect, and rescale to (targetW, targetH).
//
// The W3D texture pipeline normally resolves bare filenames through
// WW3DAssetManager::Get_Texture, which searches a hardcoded set of
// directories. We can't reach into that machinery for raw bytes, so the
// candidate list below mirrors the conventions used for shipped textures
// (Art\Textures\ for the original game's atlases, Data\English\Art\Textures\
// for Zulu mod assets, plus a bare-filename fallback for loose dev files).
static bool loadMappedImageScaled(const char *name, int targetW, int targetH, PixBufA *out)
{
	if (!name || !*name || !TheMappedImageCollection) return false;
	const Image *img = TheMappedImageCollection->findImageByName(name);
	if (!img) return false;
	AsciiString filename = img->getFilename();
	if (filename.isEmpty()) return false;

	PixBufA full;
	bool loaded = false;
	AsciiString tryPath;
	tryPath.format("Art\\Textures\\%s", filename.str());
	if (loadTgaRGBAFromFileSystem(tryPath, &full)) loaded = true;
	if (!loaded)
	{
		tryPath.format("Data\\English\\Art\\Textures\\%s", filename.str());
		if (loadTgaRGBAFromFileSystem(tryPath, &full)) loaded = true;
	}
	if (!loaded)
	{
		if (loadTgaRGBAFromFileSystem(filename, &full)) loaded = true;
	}
	if (!loaded) return false;

	// Crop to UV sub-rect. UV coords are normalized 0..1 against the
	// MappedImage's declared TextureWidth/Height (which match the on-disk
	// TGA's dimensions in practice).
	const Region2D *uv = img->getUV();
	if (!uv) return false;
	int x0 = (int)(uv->lo.x * full.w + 0.5f);
	int y0 = (int)(uv->lo.y * full.h + 0.5f);
	int x1 = (int)(uv->hi.x * full.w + 0.5f);
	int y1 = (int)(uv->hi.y * full.h + 0.5f);
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 > full.w) x1 = full.w;
	if (y1 > full.h) y1 = full.h;
	int sw = x1 - x0;
	int sh = y1 - y0;
	if (sw <= 0 || sh <= 0) return false;

	PixBufA cropped;
	cropped.w = sw;
	cropped.h = sh;
	cropped.rgba = (unsigned char *)malloc((size_t)sw * sh * 4);
	if (!cropped.rgba) { cropped.w = cropped.h = 0; return false; }
	int y;
	for (y = 0; y < sh; ++y)
	{
		memcpy(cropped.rgba + y * sw * 4,
		       full.rgba + ((y0 + y) * full.w + x0) * 4,
		       (size_t)sw * 4);
	}

	if (sw == targetW && sh == targetH)
	{
		out->w = sw;
		out->h = sh;
		out->rgba = cropped.rgba;
		cropped.rgba = nullptr; // ownership transferred
		return true;
	}
	return scaleRGBA_NN(&cropped, targetW, targetH, out);
}

// =========================================================================
// PNG encoder using zlib's compress2(). RGB only (color type 2), 8 bits
// per channel, no interlacing. Output buffer is malloc'd and ownership
// passes to the caller (free with free()).
// =========================================================================
static unsigned int beU32(unsigned int v) { return v; } // value, will be packed big-endian below
static void writeBE32(unsigned char *p, unsigned int v)
{
	p[0] = (unsigned char)((v >> 24) & 0xFF);
	p[1] = (unsigned char)((v >> 16) & 0xFF);
	p[2] = (unsigned char)((v >>  8) & 0xFF);
	p[3] = (unsigned char)( v        & 0xFF);
}

static bool writePngChunk(unsigned char **outPtr, unsigned int *outCap, unsigned int *outLen,
                          const char type[4], const unsigned char *data, unsigned int dataLen)
{
	unsigned int need = 12 + dataLen; // length(4) + type(4) + data + crc(4)
	if (*outLen + need > *outCap)
	{
		unsigned int newCap = *outCap * 2;
		if (newCap < *outLen + need) newCap = *outLen + need + 64;
		unsigned char *grown = (unsigned char *)realloc(*outPtr, newCap);
		if (!grown) return false;
		*outPtr = grown;
		*outCap = newCap;
	}
	unsigned char *p = *outPtr + *outLen;
	writeBE32(p, dataLen);
	p += 4;
	memcpy(p, type, 4);
	p += 4;
	if (dataLen) memcpy(p, data, dataLen);
	// CRC is over type + data
	unsigned long crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, (const Bytef *)type, 4);
	if (dataLen) crc = crc32(crc, (const Bytef *)data, dataLen);
	writeBE32(p + dataLen, (unsigned int)crc);
	*outLen += need;
	(void)beU32; // silence unused
	return true;
}

static bool encodePng(const PixBuf *src, unsigned char **outBuf, unsigned int *outLen)
{
	*outBuf = nullptr;
	*outLen = 0;
	if (!src || !src->rgb || src->w <= 0 || src->h <= 0)
		return false;

	// Build raw filtered scanlines: each row prefixed with a filter byte (0 = None).
	unsigned int rowLen = (unsigned int)src->w * 3 + 1;
	unsigned int rawLen = rowLen * (unsigned int)src->h;
	unsigned char *raw = (unsigned char *)malloc(rawLen);
	if (!raw) return false;
	int y;
	for (y = 0; y < src->h; ++y)
	{
		unsigned char *dst = raw + y * rowLen;
		dst[0] = 0;
		memcpy(dst + 1, src->rgb + y * src->w * 3, (size_t)src->w * 3);
	}

	// Compress with zlib (compress2 wraps a raw deflate stream in a zlib
	// container, which is exactly what the PNG IDAT chunk wants). The
	// bundled zlib is 1.1.4, which predates compressBound(), so we use
	// a safe manual upper bound: input + 0.1% + 12 (matching the
	// guidance in zlib 1.1.4's deflate.c).
	unsigned long compMax = (unsigned long)rawLen + (rawLen / 1000) + 32;
	unsigned char *comp = (unsigned char *)malloc(compMax);
	if (!comp) { free(raw); return false; }
	if (compress2(comp, &compMax, raw, rawLen, Z_BEST_COMPRESSION) != Z_OK)
	{
		free(comp); free(raw); return false;
	}
	free(raw);

	// Allocate output, write PNG signature.
	unsigned int cap = compMax + 64; // signature + IHDR + IDAT header/footer + IEND
	unsigned int len = 0;
	unsigned char *out = (unsigned char *)malloc(cap);
	if (!out) { free(comp); return false; }
	static const unsigned char kSig[8] = {0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A};
	memcpy(out, kSig, 8);
	len = 8;

	// IHDR
	unsigned char ihdr[13];
	writeBE32(ihdr + 0, (unsigned int)src->w);
	writeBE32(ihdr + 4, (unsigned int)src->h);
	ihdr[8]  = 8;  // bit depth
	ihdr[9]  = 2;  // color type = truecolor RGB
	ihdr[10] = 0;  // compression
	ihdr[11] = 0;  // filter
	ihdr[12] = 0;  // interlace
	if (!writePngChunk(&out, &cap, &len, "IHDR", ihdr, 13)) { free(out); free(comp); return false; }

	// IDAT
	if (!writePngChunk(&out, &cap, &len, "IDAT", comp, (unsigned int)compMax)) { free(out); free(comp); return false; }
	free(comp);

	// IEND
	if (!writePngChunk(&out, &cap, &len, "IEND", nullptr, 0)) { free(out); return false; }

	*outBuf = out;
	*outLen = len;
	return true;
}

// =========================================================================
// Discord webhook POST (multipart/form-data). Posts a single attachment
// as files[0]; Discord renders attached PNGs inline in the channel.
// =========================================================================
// Returns the HTTP status code (e.g. 204 on Discord success), or 0 on
// transport failure (failed to open/send/parse). Output `errOut` is
// populated with a short description on failure.
static unsigned long postPngToDiscord(const char *webhookUrl,
                                      const unsigned char *png, unsigned int pngLen,
                                      const char *filename,
                                      const char *contentJson,
                                      char *errOut, unsigned int errCap)
{
	if (errOut && errCap) errOut[0] = 0;
	if (!webhookUrl || !*webhookUrl || !png || pngLen == 0)
	{
		if (errOut && errCap) _snprintf(errOut, errCap - 1, "no url/png");
		return 0;
	}

	char hostBuf[256];
	char pathBuf[1024];
	URL_COMPONENTSA uc;
	memset(&uc, 0, sizeof(uc));
	uc.dwStructSize = sizeof(uc);
	uc.lpszHostName = hostBuf;
	uc.dwHostNameLength = sizeof(hostBuf);
	uc.lpszUrlPath = pathBuf;
	uc.dwUrlPathLength = sizeof(pathBuf);

	if (!InternetCrackUrlA(webhookUrl, 0, 0, &uc))
	{
		if (errOut && errCap) _snprintf(errOut, errCap - 1, "URL parse failed");
		return 0;
	}

	INTERNET_PORT port = uc.nPort;
	if (port == 0)
		port = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

	DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
	if (uc.nScheme == INTERNET_SCHEME_HTTPS)
		flags |= INTERNET_FLAG_SECURE;

	HINTERNET hInet = InternetOpenA("ZuluLobbyWebhook/1.0", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
	if (!hInet)
	{
		if (errOut && errCap) _snprintf(errOut, errCap - 1, "InternetOpen err=%lu", GetLastError());
		return 0;
	}
	HINTERNET hConn = InternetConnectA(hInet, hostBuf, port, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
	if (!hConn)
	{
		if (errOut && errCap) _snprintf(errOut, errCap - 1, "InternetConnect err=%lu", GetLastError());
		InternetCloseHandle(hInet);
		return 0;
	}
	HINTERNET hReq = HttpOpenRequestA(hConn, "POST", pathBuf, nullptr, nullptr, nullptr, flags, 0);
	if (!hReq)
	{
		if (errOut && errCap) _snprintf(errOut, errCap - 1, "HttpOpenRequest err=%lu", GetLastError());
		InternetCloseHandle(hConn);
		InternetCloseHandle(hInet);
		return 0;
	}

	static const char boundary[] = "----ZuluLobbyDiscordBoundaryK7nQv2pXr9TfH3";

	// Build the multipart body. Two parts: payload_json (text) + files[0] (the png).
	char part1[512];
	int part1Len = sprintf(part1,
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"payload_json\"\r\n"
		"Content-Type: application/json; charset=utf-8\r\n"
		"\r\n"
		"%s\r\n",
		boundary, (contentJson && *contentJson) ? contentJson : "{}");

	char part2[512];
	int part2Len = sprintf(part2,
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"files[0]\"; filename=\"%.255s\"\r\n"
		"Content-Type: image/png\r\n"
		"\r\n",
		boundary, (filename && *filename) ? filename : "lobby.png");

	char trailer[64];
	int trailerLen = sprintf(trailer, "\r\n--%s--\r\n", boundary);

	if (part1Len <= 0 || part2Len <= 0 || trailerLen <= 0)
	{
		if (errOut && errCap) _snprintf(errOut, errCap - 1, "sprintf trailer failed");
		InternetCloseHandle(hReq);
		InternetCloseHandle(hConn);
		InternetCloseHandle(hInet);
		return 0;
	}

	unsigned int bodyLen = (unsigned int)part1Len + (unsigned int)part2Len + pngLen + (unsigned int)trailerLen;
	unsigned char *body = (unsigned char *)malloc(bodyLen);
	if (!body)
	{
		if (errOut && errCap) _snprintf(errOut, errCap - 1, "malloc body %u failed", bodyLen);
		InternetCloseHandle(hReq);
		InternetCloseHandle(hConn);
		InternetCloseHandle(hInet);
		return 0;
	}
	unsigned char *bp = body;
	memcpy(bp, part1, part1Len);   bp += part1Len;
	memcpy(bp, part2, part2Len);   bp += part2Len;
	memcpy(bp, png, pngLen);       bp += pngLen;
	memcpy(bp, trailer, trailerLen);

	char headers[256];
	int hdrLen = sprintf(headers, "Content-Type: multipart/form-data; boundary=%s\r\n", boundary);

	unsigned long statusOut = 0;
	BOOL ok = HttpSendRequestA(hReq, headers, (DWORD)hdrLen, body, bodyLen);
	if (ok)
	{
		DWORD status = 0;
		DWORD szSize = sizeof(status);
		HttpQueryInfoA(hReq, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &szSize, nullptr);
		statusOut = status;
	}
	else
	{
		if (errOut && errCap) _snprintf(errOut, errCap - 1, "HttpSendRequest err=%lu", GetLastError());
	}

	free(body);
	InternetCloseHandle(hReq);
	InternetCloseHandle(hConn);
	InternetCloseHandle(hInet);
	return statusOut;
}

// =========================================================================
// Faction abbreviation: strip the leading base-side prefix and the trailing
// " General" suffix so "GLA Stealth General" reads as "Stealth" while plain
// "GLA" stays "GLA".
// =========================================================================
static AsciiString abbreviateFaction(const PlayerTemplate *pt)
{
	if (!pt) return AsciiString("Random");
	AsciiString display;
	display.translate(pt->getDisplayName());
	if (display.isEmpty())
		display = pt->getName();

	AsciiString baseSide = pt->getBaseSide();
	if (baseSide.isEmpty()) baseSide = pt->getSide();

	const char *s = display.str();
	int sLen = display.getLength();

	// Strip "<baseSide> " prefix if present.
	if (!baseSide.isEmpty())
	{
		int bLen = baseSide.getLength();
		if (sLen > bLen + 1
		    && strncmp(s, baseSide.str(), bLen) == 0
		    && s[bLen] == ' ')
		{
			AsciiString rest;
			int i;
			for (i = bLen + 1; i < sLen; ++i)
				rest.concat(s[i]);
			display = rest;
			s = display.str();
			sLen = display.getLength();
		}
	}

	// Strip trailing " General".
	static const char kSuffix[] = " General";
	int suffixLen = (int)(sizeof(kSuffix) - 1);
	if (sLen >= suffixLen
	    && strcmp(s + sLen - suffixLen, kSuffix) == 0)
	{
		AsciiString trimmed;
		int i;
		for (i = 0; i < sLen - suffixLen; ++i)
			trimmed.concat(s[i]);
		display = trimmed;
	}

	if (display.isEmpty())
		display = baseSide.isEmpty() ? AsciiString("?") : baseSide;

	return display;
}

// Sanitize a UnicodeString down to printable ASCII so the embedded 8x8
// font renders cleanly. Non-ASCII codepoints become '?'.
static AsciiString toRenderableAscii(const UnicodeString& u)
{
	AsciiString out;
	out.translate(u);
	const char *s = out.str();
	AsciiString clean;
	int i;
	int len = out.getLength();
	for (i = 0; i < len; ++i)
	{
		unsigned char c = (unsigned char)s[i];
		if (c >= 0x20 && c <= 0x7E)
			clean.concat((char)c);
		else
			clean.concat('?');
	}
	return clean;
}

// In -zulu_debug mode, mirror status to the lobby chat window so the host
// can watch the post pipeline progress without opening the debug log. No-op
// outside zulu_debug so production hosts don't see [discord] spam.
static void debugChat(const char *fmt, ...)
{
	if (!TheGlobalData || !TheGlobalData->m_zuluDebug) return;
	if (!TheLAN) return;
	char buf[512];
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
	buf[sizeof(buf) - 1] = 0;
	va_end(ap);
	UnicodeString u;
	u.translate(AsciiString(buf));
	TheLAN->OnChat(L"[discord]", TheLAN->GetLocalIP(), u, LANAPI::LANCHAT_SYSTEM);
	printf("[discord] %s\n", buf);
}

// =========================================================================
// Public entry point.
// =========================================================================
void PostLanLobbyMapToDiscord(LANGameInfo *game)
{
	static const char *kWebhookUrl = ZULU_DISCORD_WEBHOOK_URL;
	if (!kWebhookUrl || !*kWebhookUrl)
	{
		debugChat("disabled at build time (no webhook URL)");
		return;
	}

	if (!game) { debugChat("no game info"); return; }

	// Slot scan: count non-observer humans, build a fast-access roster.
	struct SlotInfo
	{
		bool occupied;
		bool isHuman;
		bool isObserver;
		int  startPos;
		int  templateIdx;
		int  colorIdx;
		int  team;
		AsciiString displayName;
		AsciiString factionAbbrev;
	};
	SlotInfo info[MAX_SLOTS];
	int slotIdx;
	int humanCount = 0;
	for (slotIdx = 0; slotIdx < MAX_SLOTS; ++slotIdx)
	{
		const GameSlot *slot = game->getConstSlot(slotIdx);
		SlotInfo &si = info[slotIdx];
		si.occupied = false;
		si.isHuman = false;
		si.isObserver = false;
		si.startPos = -1;
		si.templateIdx = -1;
		si.colorIdx = -1;
		si.team = -1;
		if (!slot || !slot->isOccupied())
			continue;
		si.occupied = true;
		si.isHuman = slot->isHuman();
		si.isObserver = (slot->getPlayerTemplate() == PLAYERTEMPLATE_OBSERVER);
		si.startPos = slot->getStartPos();
		si.templateIdx = slot->getPlayerTemplate();
		si.colorIdx = slot->getColor();
		si.team = slot->getTeamNumber();
		si.displayName = toRenderableAscii(slot->getName());
		const PlayerTemplate *pt = nullptr;
		if (si.templateIdx >= 0 && ThePlayerTemplateStore)
			pt = ThePlayerTemplateStore->getNthPlayerTemplate(si.templateIdx);
		si.factionAbbrev = (si.templateIdx == PLAYERTEMPLATE_RANDOM)
			? AsciiString("Random")
			: abbreviateFaction(pt);
		if (si.isHuman && !si.isObserver)
			++humanCount;
	}

	// Default gate: 2+ humans. -zulu_debug drops it to 1+ so a host can
	// iterate the rendering with just themselves in the lobby.
	int minHumans = (TheGlobalData && TheGlobalData->m_zuluDebug) ? 1 : 2;
	if (humanCount < minHumans)
	{
		debugChat("skipped: %d humans (need %d)", humanCount, minHumans);
		return;
	}
	debugChat("starting post: %d humans, map=%s", humanCount, game->getMap().str());

	// Resolve map metadata (extent + display name + waypoints).
	const MapMetaData *mmd = nullptr;
	if (TheMapCache)
		mmd = TheMapCache->findMap(game->getMap());
	if (!mmd)
	{
		debugChat("MapCache lookup failed for %s", game->getMap().str());
		return;
	}

	// Find and load the .tga preview file. The lobby caches a
	// sanitized copy in <userdata>/mapPreviews/, but for our purposes
	// loading the original next-to-the-.map TGA is simpler and equally
	// resolved by TheFileSystem (loose or .big-archive).
	AsciiString tgaPath = game->getMap();
	tgaPath.truncateBy(4); // ".map"
	tgaPath.concat(".tga");

	PixBuf src;
	if (!loadTgaFromFileSystem(tgaPath, &src))
	{
		debugChat("preview tga not loadable: %s", tgaPath.str());
		return;
	}
	debugChat("loaded tga %dx%d from %s", src.w, src.h, tgaPath.str());

	// Aim for ~1024 px on the long edge with a generous minimum (4x) so
	// labels and overlay icons have room to breathe. Cap so the PNG
	// stays comfortably under Discord's attachment limit.
	int longEdge = src.w > src.h ? src.w : src.h;
	int scale = 4;
	if (longEdge > 0)
	{
		scale = 1024 / longEdge;
		if (scale < 4) scale = 4;
		if (scale > 12) scale = 12;
		while (scale > 4 && longEdge * scale > 1600)
			scale--;
	}

	PixBuf big;
	if (!upscaleNN(&src, scale, &big))
	{
		debugChat("upscale failed");
		return;
	}
	debugChat("upscaled to %dx%d (scale x%d)", big.w, big.h, scale);

	// World -> upscaled-image pixel coordinate. Top-down image, world y
	// grows up, so flip Y. extent is the full playable region from
	// MapMetaData (same coordinates the lobby preview UI uses).
	#define WORLD_TO_PX_X(wx) ((int)(((wx) - mmd->m_extent.lo.x) / (mmd->m_extent.hi.x - mmd->m_extent.lo.x) * big.w))
	#define WORLD_TO_PX_Y(wy) ((int)((1.0f - ((wy) - mmd->m_extent.lo.y) / (mmd->m_extent.hi.y - mmd->m_extent.lo.y)) * big.h))

	// Drawing order: neutral overlays first, then player markers on top,
	// then text labels last so nothing covers the player names.
	//
	// Overlay icons reuse the same MappedImage assets the in-game lobby
	// preview uses (TecBuilding / Cash from the stock UI atlas, plus
	// ZuluCrateIcon / ZuluDerrickIcon shipped by the Zulu mod). Each
	// gets cropped to its UV sub-rect, scaled to a uniform target size,
	// and alpha-composited so transparent borders disappear cleanly.
	int iconSize = big.w / 26;
	if (iconSize < 18) iconSize = 18;
	if (iconSize > 48) iconSize = 48;

	PixBufA iconCash, iconTech, iconDerrick, iconCrate;
	bool haveCash    = loadMappedImageScaled("Cash",            iconSize, iconSize, &iconCash);
	bool haveTech    = loadMappedImageScaled("TecBuilding",     iconSize, iconSize, &iconTech);
	bool haveDerrick = loadMappedImageScaled("ZuluDerrickIcon", iconSize, iconSize, &iconDerrick);
	bool haveCrate   = loadMappedImageScaled("ZuluCrateIcon",   iconSize, iconSize, &iconCrate);
	debugChat("icons loaded: cash=%d tech=%d derrick=%d crate=%d (size=%dpx)",
	          (int)haveCash, (int)haveTech, (int)haveDerrick, (int)haveCrate, iconSize);

	// Helper: draw an icon centered at world (wx, wy). Falls back to a
	// stroked ASCII glyph if the icon failed to load (e.g. unmodded
	// install missing the source TGA).
	#define DRAW_ICON_OR_GLYPH(haveFlag, iconBuf, glyphCh, gr, gg, gb, wx, wy) do {           \
		int _cx = WORLD_TO_PX_X(wx);                                                          \
		int _cy = WORLD_TO_PX_Y(wy);                                                          \
		if (haveFlag)                                                                         \
			compositeRGBA(&big, _cx - iconSize / 2, _cy - iconSize / 2, &iconBuf);            \
		else                                                                                  \
			drawSymbolCentered(&big, _cx, _cy, (glyphCh), 2, (gr), (gg), (gb));               \
	} while (0)

	// Supply docks (Cash icon, green dollar in stock UI).
	{
		Coord3DList::const_iterator it;
		for (it = mmd->m_supplyPositions.begin(); it != mmd->m_supplyPositions.end(); ++it)
			DRAW_ICON_OR_GLYPH(haveCash, iconCash, '$', 50, 205, 50, it->x, it->y);
	}
	// Crate spawns (Zulu's pink crate icon).
	{
		Coord3DList::const_iterator it;
		for (it = mmd->m_cratePositions.begin(); it != mmd->m_cratePositions.end(); ++it)
			DRAW_ICON_OR_GLYPH(haveCrate, iconCrate, '?', 0, 220, 220, it->x, it->y);
	}
	// Tech buildings (TecBuilding star).
	{
		Coord3DList::const_iterator it;
		for (it = mmd->m_techPositions.begin(); it != mmd->m_techPositions.end(); ++it)
			DRAW_ICON_OR_GLYPH(haveTech, iconTech, 'T', 255, 221, 0, it->x, it->y);
	}
	// Tech derricks (Zulu derrick icon).
	{
		Coord3DList::const_iterator it;
		for (it = mmd->m_techDerrickPositions.begin(); it != mmd->m_techDerrickPositions.end(); ++it)
			DRAW_ICON_OR_GLYPH(haveDerrick, iconDerrick, 'D', 255, 221, 0, it->x, it->y);
	}

	#undef DRAW_ICON_OR_GLYPH

	// Player start markers: filled disc in the slot's lobby color with a
	// black halo + white border so it stays readable on any terrain. The
	// number inside is the start-position digit (1..N) so labels and
	// markers reference the same identity at a glance.
	int slot;
	for (slot = 0; slot < MAX_SLOTS; ++slot)
	{
		const SlotInfo &si = info[slot];
		if (!si.occupied || si.isObserver) continue;
		if (si.startPos < 0) continue; // random start: skip — not yet assigned
		AsciiString waypointName;
		waypointName.format("Player_%d_Start", si.startPos + 1);
		WaypointMap::const_iterator wpIt = mmd->m_waypoints.find(waypointName);
		if (wpIt == mmd->m_waypoints.end())
			continue;
		const Coord3D &pos = wpIt->second;
		int cx = WORLD_TO_PX_X(pos.x);
		int cy = WORLD_TO_PX_Y(pos.y);

		// Resolve the player's lobby color via MultiplayerSettings.
		// "Random" colors (-1) and any failed lookup fall back to white.
		unsigned char mr = 255, mg = 255, mb = 255;
		if (TheMultiplayerSettings && si.colorIdx >= 0)
		{
			MultiplayerColorDefinition *cd = TheMultiplayerSettings->getColor(si.colorIdx);
			if (cd)
			{
				RGBColor rc = cd->getRGBValue();
				mr = (unsigned char)(rc.red   * 255.0f);
				mg = (unsigned char)(rc.green * 255.0f);
				mb = (unsigned char)(rc.blue  * 255.0f);
			}
		}

		// Marker geometry: scale with the upscale factor so the marker
		// stays proportional to the map. Ring stack: outer black halo
		// (1px), filled colored disc (markerR), white border (2px).
		int markerR = 9 + scale;
		drawCircleOutline(&big, cx, cy, markerR + 2, 1, 0, 0, 0);
		drawFilledCircle(&big, cx, cy, markerR, mr, mg, mb);
		drawCircleOutline(&big, cx, cy, markerR, 2, 255, 255, 255);

		// Centered start-position digit. White-with-black-stroke so it
		// stays readable on light marker colors (yellow, white, etc).
		char num[4];
		sprintf(num, "%d", si.startPos + 1);
		int numScale = (markerR >= 14) ? 2 : 1;
		int numW = textWidthPx(num, numScale);
		int numH = 8 * numScale;
		drawTextStroked(&big, cx - numW / 2, cy - numH / 2, num, numScale,
		                255, 255, 255);
	}

	// Player labels (drawn after every marker so text is never hidden):
	// "<name> (<faction>) [<team>]" in stroked white. Matches radarvan's
	// fontSize 9 + textShadow look — small white text with black halo
	// rather than an opaque backing rectangle. Team uses 1-based output
	// to match radarvan's "Team 1/2/3/4" UI ("[FFA]" when teamNumber<0).
	for (slot = 0; slot < MAX_SLOTS; ++slot)
	{
		const SlotInfo &si = info[slot];
		if (!si.occupied || si.isObserver) continue;
		if (si.startPos < 0) continue;
		AsciiString waypointName;
		waypointName.format("Player_%d_Start", si.startPos + 1);
		WaypointMap::const_iterator wpIt = mmd->m_waypoints.find(waypointName);
		if (wpIt == mmd->m_waypoints.end())
			continue;
		const Coord3D &pos = wpIt->second;
		int cx = WORLD_TO_PX_X(pos.x);
		int cy = WORLD_TO_PX_Y(pos.y);
		int markerR = 8 + scale;

		AsciiString line;
		line = si.displayName;
		if (line.isEmpty()) line = "?";
		line.concat(" (");
		line.concat(si.factionAbbrev);
		line.concat(") [");
		if (si.team >= 0)
		{
			char teamStr[8];
			sprintf(teamStr, "%d", si.team + 1);
			line.concat(teamStr);
		}
		else
		{
			line.concat("FFA");
		}
		line.concat(']');

		const int tScale = 1; // ~8px tall, close to radarvan's fontSize 9
		int tW = textWidthPx(line.str(), tScale);
		int tx = cx - tW / 2;
		int ty = cy + markerR + 4;
		drawTextStroked(&big, tx, ty, line.str(), tScale, 255, 255, 255);
	}

	#undef WORLD_TO_PX_X
	#undef WORLD_TO_PX_Y

	// Encode PNG.
	unsigned char *png = nullptr;
	unsigned int pngLen = 0;
	if (!encodePng(&big, &png, &pngLen))
	{
		debugChat("png encode failed");
		return;
	}
	debugChat("encoded png %ux%u (%u bytes), posting...", (unsigned)big.w, (unsigned)big.h, pngLen);

	// Build a small JSON content blurb naming the map and player count.
	AsciiString mapDisplayAscii;
	mapDisplayAscii.translate(mmd->m_displayName);
	// Defensively escape JSON specials in the map name and player names
	// (limited set: backslash and double-quote). The label text itself
	// already went through toRenderableAscii so it's printable ASCII.
	AsciiString safeMap;
	{
		const char *p;
		for (p = mapDisplayAscii.str(); *p; ++p)
		{
			if (*p == '"' || *p == '\\') safeMap.concat('\\');
			safeMap.concat(*p);
		}
	}
	char json[512];
	sprintf(json, "{\"content\":\"Lobby starting on **%.200s** (%d humans)\"}",
	        safeMap.str(), humanCount);

	char err[128] = {0};
	unsigned long status = postPngToDiscord(kWebhookUrl, png, pngLen, "lobby.png", json, err, sizeof(err));
	if (status >= 200 && status < 300)
		debugChat("posted ok (HTTP %lu)", status);
	else if (status != 0)
		debugChat("HTTP %lu (Discord rejected)", status);
	else
		debugChat("transport failed: %s", err[0] ? err : "(no detail)");

	free(png);
}
