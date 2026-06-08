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
#include "Common/DataChunk.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/MapObject.h"
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
#include <errno.h>

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

// Source-over alpha blend of (r,g,bl) at coverage `alpha` (0..255) onto the
// existing pixel. Used for translucent overlays like the scale grid where
// fully opaque lines would fight the underlying terrain.
static inline void blendPixel(PixBuf *b, int x, int y,
                              unsigned char r, unsigned char g, unsigned char bl,
                              unsigned char alpha)
{
	if (x < 0 || y < 0 || x >= b->w || y >= b->h)
		return;
	unsigned char *p = b->rgb + (y * b->w + x) * 3;
	unsigned int a = alpha;
	unsigned int ia = 255 - a;
	p[0] = (unsigned char)((r  * a + p[0] * ia) / 255);
	p[1] = (unsigned char)((g  * a + p[1] * ia) / 255);
	p[2] = (unsigned char)((bl * a + p[2] * ia) / 255);
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

// Nearest-neighbor resample of an RGB PixBuf to arbitrary (dstW, dstH).
// Used to stretch the always-square 128x128 .tga preview into the world's
// actual aspect ratio so the rendered map isn't visually squished.
static bool scaleRGB_NN(const PixBuf *src, int dstW, int dstH, PixBuf *dst)
{
	if (dstW <= 0 || dstH <= 0 || !src || src->w <= 0 || src->h <= 0) return false;
	dst->w = dstW;
	dst->h = dstH;
	dst->rgb = (unsigned char *)malloc((size_t)dstW * dstH * 3);
	if (!dst->rgb)
	{
		dst->w = dst->h = 0;
		return false;
	}
	int y, x;
	for (y = 0; y < dstH; ++y)
	{
		int sy = y * src->h / dstH;
		if (sy >= src->h) sy = src->h - 1;
		const unsigned char *srow = src->rgb + sy * src->w * 3;
		unsigned char *drow = dst->rgb + y * dstW * 3;
		for (x = 0; x < dstW; ++x)
		{
			int sx = x * src->w / dstW;
			if (sx >= src->w) sx = src->w - 1;
			const unsigned char *sp = srow + sx * 3;
			unsigned char *dp = drow + x * 3;
			dp[0] = sp[0]; dp[1] = sp[1]; dp[2] = sp[2];
		}
	}
	return true;
}

// =========================================================================
// TGA loader (uncompressed TrueColor, 24/32 bpp). Reads via TheFileSystem
// so the file resolves equally from a .big archive or a loose file.
// Decodes into a top-down RGB888 buffer.
// =========================================================================
// Format a short diagnostic string into errOut on failure. Each failure
// branch sets a distinct message so the host log can tell "path missing"
// apart from "open failed (e.g. AV sharing violation)" or a corrupt TGA
// header without a separate trace. errOut may be nullptr; safe no-op then.
static void setTgaErr(char *errOut, int errCap, const char *fmt, ...)
{
	if (errOut == nullptr || errCap <= 0)
		return;
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(errOut, errCap - 1, fmt, ap);
	va_end(ap);
	errOut[errCap - 1] = '\0';
}

static bool loadTgaFromFileSystem(const AsciiString& path, PixBuf *out,
                                  char *errOut, int errCap)
{
	if (errOut != nullptr && errCap > 0) errOut[0] = '\0';
	out->w = out->h = 0;
	out->rgb = nullptr;
	if (path.isEmpty())
	{
		setTgaErr(errOut, errCap, "empty path");
		return false;
	}

	File *f = TheFileSystem->openFile(path.str(), File::READ | File::BINARY);
	if (!f)
	{
		// Capture errno + GetLastError immediately, before any other libc
		// or Win32 call below can clobber them. doesFileExist distinguishes
		// "path missing" (case/separator mismatch) from "path present but
		// unopenable" (AV scanner sharing violation, ACL, file in use by
		// the just-finished cncstats upload, etc.).
		int e = errno;
		DWORD le = GetLastError();
		const char *eStr = strerror(e);
		Bool exists = (TheFileSystem != nullptr)
			&& TheFileSystem->doesFileExist(path.str());
		setTgaErr(errOut, errCap,
			"openFile failed (errno=%d \"%s\", GetLastError=%lu, exists=%s)",
			e, eStr ? eStr : "?", (unsigned long)le, exists ? "yes" : "no");
		return false;
	}

	int total = f->seek(0, File::END);
	f->seek(0, File::START);
	if (total < 18)
	{
		setTgaErr(errOut, errCap, "file too short (%d bytes)", total);
		f->close();
		return false;
	}

	unsigned char *buf = (unsigned char *)malloc(total);
	if (!buf)
	{
		setTgaErr(errOut, errCap, "malloc(%d) failed", total);
		f->close();
		return false;
	}
	if (f->read(buf, total) != total)
	{
		setTgaErr(errOut, errCap, "short read");
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

	// imgType 2  = uncompressed TrueColor
	// imgType 10 = RLE-compressed TrueColor (some map editors emit these)
	bool ok = (imgType == 2 || imgType == 10)
	          && (cmapType == 0)
	          && (pixDepth == 24 || pixDepth == 32)
	          && width > 0 && height > 0
	          && width <= 4096 && height <= 4096;
	if (!ok)
	{
		setTgaErr(errOut, errCap,
			"bad header (imgType=%u cmapType=%u depth=%u %ux%u)",
			(unsigned)imgType, (unsigned)cmapType, (unsigned)pixDepth,
			(unsigned)width, (unsigned)height);
		free(buf);
		return false;
	}

	int bpp = pixDepth / 8;
	int dataOff = 18 + (int)idLen; // skip image-id field; no colormap for truecolor
	int needed = width * height * bpp;

	// For uncompressed (imgType==2) the pixel stream lives in `buf` starting
	// at dataOff. For RLE (imgType==10) we decode it once into rlePixels and
	// then point at that. Either way, `pixelData` is the contiguous
	// scanline-major BGR(A) buffer the row-loop below reads from.
	const unsigned char *pixelData = nullptr;
	unsigned char *rlePixels = nullptr;

	if (imgType == 2)
	{
		if (dataOff + needed > total)
		{
			setTgaErr(errOut, errCap,
				"pixel data truncated (need %d at offset %d, have %d)",
				needed, dataOff, total);
			free(buf);
			return false;
		}
		pixelData = buf + dataOff;
	}
	else
	{
		// RLE decode. Each packet starts with a 1-byte header:
		//   MSB set  -> run-length packet: (header & 0x7F)+1 copies of the
		//               following single bpp-byte pixel
		//   MSB clear-> raw packet: (header & 0x7F)+1 bpp-byte pixels follow
		// Packets may cross scanline boundaries per the TGA spec, so we just
		// fill the linear `needed`-byte buffer end-to-end without caring
		// where each scanline begins.
		rlePixels = (unsigned char *)malloc(needed);
		if (rlePixels == nullptr)
		{
			setTgaErr(errOut, errCap, "RLE buffer malloc(%d) failed", needed);
			free(buf);
			return false;
		}

		int dstPos = 0;
		int srcPos = dataOff;
		while (dstPos < needed)
		{
			if (srcPos >= total)
			{
				setTgaErr(errOut, errCap,
					"RLE truncated (need %d more bytes)", needed - dstPos);
				free(rlePixels);
				free(buf);
				return false;
			}
			unsigned char pkt = buf[srcPos++];
			int count = (int)(pkt & 0x7F) + 1;
			int copyBytes = count * bpp;
			if (dstPos + copyBytes > needed)
				copyBytes = needed - dstPos; // clamp final packet
			if (pkt & 0x80)
			{
				// Run-length: one pixel repeated.
				if (srcPos + bpp > total)
				{
					setTgaErr(errOut, errCap, "RLE run pixel truncated");
					free(rlePixels);
					free(buf);
					return false;
				}
				int i;
				for (i = 0; i < copyBytes; ++i)
					rlePixels[dstPos + i] = buf[srcPos + (i % bpp)];
				srcPos += bpp;
			}
			else
			{
				// Raw: count pixels copied verbatim.
				if (srcPos + copyBytes > total)
				{
					setTgaErr(errOut, errCap, "RLE raw run truncated");
					free(rlePixels);
					free(buf);
					return false;
				}
				memcpy(rlePixels + dstPos, buf + srcPos, copyBytes);
				srcPos += copyBytes;
			}
			dstPos += copyBytes;
		}
		pixelData = rlePixels;
	}

	out->w = width;
	out->h = height;
	out->rgb = (unsigned char *)malloc((size_t)width * height * 3);
	if (!out->rgb)
	{
		setTgaErr(errOut, errCap, "out rgb malloc(%d) failed", width * height * 3);
		if (rlePixels) free(rlePixels);
		free(buf);
		out->w = out->h = 0;
		return false;
	}

	bool topDown = (descByte & 0x20) != 0;
	int y, x;
	for (y = 0; y < height; ++y)
	{
		int srcRow = topDown ? y : (height - 1 - y);
		const unsigned char *src = pixelData + srcRow * width * bpp;
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

	if (rlePixels) free(rlePixels);
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

// One occupied (or empty) lobby slot, snapshotted at game-start so the
// helper can iterate it without re-reading mutable LANGameSlot state.
struct LobbySlotInfo
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

// Deep-copy an RGB PixBuf. Used to start each render variant (original
// and mirror) from the same upscaled+icons "base" image without rebuilding
// the expensive parts.
static bool copyPixBuf(const PixBuf *src, PixBuf *dst)
{
	if (!src || !src->rgb || src->w <= 0 || src->h <= 0) return false;
	dst->w = src->w;
	dst->h = src->h;
	dst->rgb = (unsigned char *)malloc((size_t)src->w * src->h * 3);
	if (!dst->rgb) { dst->w = dst->h = 0; return false; }
	memcpy(dst->rgb, src->rgb, (size_t)src->w * src->h * 3);
	return true;
}

// Compute a "matchup-preserving mirror": each player on team A is paired
// with a player on team B such that, in the mirror image, the two
// players swap *identity* (name + team + color) while the
// position+faction of each slot stays anchored. The intent is that the
// rematch puts every player at their previous opponent's start playing
// their previous opponent's faction — any per-player advantage in
// terrain or faction is cancelled across the two games.
//
// Pairing rule: reflect each team-A player's position across the
// perpendicular bisector of the two team centroids, then match each
// team-A player to the team-B player nearest to that reflected point
// (via min-cost bipartite matching on reflection-to-actual distances).
//
// Why not min-cost on raw player-to-player distance? The latter ties
// frequently on symmetric layouts — e.g. on Combat Encounter's
// left-vs-right split, "1↔3, 2↔4" (straight-across) and "1↔4, 2↔3"
// (corner-mirrored) can have identical total distance and the tie
// breaker silently picks the lex-first permutation rather than the
// geometric mirror the host actually wants. Reflecting through the
// team-split axis collapses the ambiguity: only the perm that respects
// the map's symmetry minimizes distance-to-reflected-point.
//
// Returns true on success and fills `playerOverride[S]` with the slot
// whose player identity should render at slot S in the mirror image.
// Slots that are unoccupied/observer/random-start stay self-mapped.
// Returns false (and the mirror post is skipped) when:
//   * there aren't exactly two distinct teams among occupied slots, OR
//   * the two teams have unequal player counts, OR
//   * either team has zero players, OR
//   * either team has more than MAX_PAIR (4) players (brute-force cap),
//     OR
//   * the two team centroids coincide (no meaningful split axis), OR
//   * any required waypoint is missing.
#define MAX_PAIR 4
static bool computeMirrorSwap(const LobbySlotInfo *info,
                              const MapMetaData *mmd,
                              int playerOverride[MAX_SLOTS])
{
	int s;
	for (s = 0; s < MAX_SLOTS; ++s) playerOverride[s] = s;
	if (!mmd) return false;

	// Partition occupied non-observer slots by team. We accept exactly
	// two distinct team numbers; any third team aborts. Random teams
	// (team < 0) also abort since "matchup" is undefined.
	int teamLabel[2] = { -1, -1 };
	int teamSlots[2][MAX_PAIR];
	int teamCount[2] = { 0, 0 };
	for (s = 0; s < MAX_SLOTS; ++s)
	{
		const LobbySlotInfo &si = info[s];
		if (!si.occupied || si.isObserver) continue;
		if (si.startPos < 0) return false;
		if (si.team < 0) return false;

		int bucket = -1;
		if (teamLabel[0] == -1) { teamLabel[0] = si.team; bucket = 0; }
		else if (si.team == teamLabel[0]) bucket = 0;
		else if (teamLabel[1] == -1) { teamLabel[1] = si.team; bucket = 1; }
		else if (si.team == teamLabel[1]) bucket = 1;
		else return false; // third team
		if (teamCount[bucket] >= MAX_PAIR) return false;
		teamSlots[bucket][teamCount[bucket]++] = s;
	}
	if (teamCount[0] == 0 || teamCount[1] == 0) return false;
	if (teamCount[0] != teamCount[1]) return false;

	// Resolve start positions to world coords for the geometry below.
	int n = teamCount[0];
	Coord3D posA[MAX_PAIR], posB[MAX_PAIR];
	int i;
	for (i = 0; i < n; ++i)
	{
		AsciiString wp;
		wp.format("Player_%d_Start", info[teamSlots[0][i]].startPos + 1);
		WaypointMap::const_iterator it = mmd->m_waypoints.find(wp);
		if (it == mmd->m_waypoints.end()) return false;
		posA[i] = it->second;

		wp.format("Player_%d_Start", info[teamSlots[1][i]].startPos + 1);
		it = mmd->m_waypoints.find(wp);
		if (it == mmd->m_waypoints.end()) return false;
		posB[i] = it->second;
	}

	// Team centroids define the team-split direction; their
	// perpendicular bisector is the mirror axis.
	Real cAx = 0.0f, cAy = 0.0f, cBx = 0.0f, cBy = 0.0f;
	for (i = 0; i < n; ++i)
	{
		cAx += posA[i].x; cAy += posA[i].y;
		cBx += posB[i].x; cBy += posB[i].y;
	}
	cAx /= n; cAy /= n;
	cBx /= n; cBy /= n;

	Real splitX = cAx - cBx;
	Real splitY = cAy - cBy;
	double splitLen = sqrt((double)(splitX * splitX + splitY * splitY));
	// Reject when the two teams overlap centroids (e.g. concentric or
	// pathological layouts) — there's no meaningful axis to mirror over.
	if (splitLen < 1.0)
		return false;

	Real nx = (Real)(splitX / splitLen);
	Real ny = (Real)(splitY / splitLen);
	Real mx = (cAx + cBx) * 0.5f;
	Real my = (cAy + cBy) * 0.5f;

	// Reflect each team-A position across the split axis. The axis
	// passes through (mx, my) with unit normal (nx, ny):
	//   reflect(P) = P - 2 * ((P - mid) · n) * n
	// On a symmetric map this lands each posA[i] exactly on top of the
	// corresponding team-B position; on imperfectly symmetric maps it
	// lands close, and the min-cost matching below resolves which team-B
	// position is actually nearest.
	Coord3D rA[MAX_PAIR];
	for (i = 0; i < n; ++i)
	{
		Real dx = posA[i].x - mx;
		Real dy = posA[i].y - my;
		Real dot = dx * nx + dy * ny;
		rA[i].x = posA[i].x - 2.0f * dot * nx;
		rA[i].y = posA[i].y - 2.0f * dot * ny;
		rA[i].z = 0.0f;
	}

	// Min-cost perfect matching of (reflected-A positions) against
	// (actual-B positions). Brute force over n! permutations; n <= 4.
	int perm[MAX_PAIR];
	int bestPerm[MAX_PAIR];
	for (i = 0; i < n; ++i) perm[i] = i;
	Real bestCost = 1e30f;
	bool first = true;
	for (;;)
	{
		Real cost = 0.0f;
		int j;
		for (j = 0; j < n; ++j)
		{
			Real dx = rA[j].x - posB[perm[j]].x;
			Real dy = rA[j].y - posB[perm[j]].y;
			cost += dx * dx + dy * dy;
		}
		if (first || cost < bestCost)
		{
			bestCost = cost;
			for (j = 0; j < n; ++j) bestPerm[j] = perm[j];
			first = false;
		}
		// Next lexicographic permutation of perm[0..n).
		int k = n - 2;
		while (k >= 0 && perm[k] >= perm[k + 1]) --k;
		if (k < 0) break;
		int l = n - 1;
		while (perm[l] <= perm[k]) --l;
		int t = perm[k]; perm[k] = perm[l]; perm[l] = t;
		// Reverse perm[k+1..n).
		int lo = k + 1, hi = n - 1;
		while (lo < hi) { t = perm[lo]; perm[lo] = perm[hi]; perm[hi] = t; ++lo; --hi; }
	}

	// Apply: slot teamSlots[0][i] shows the player of teamSlots[1][bestPerm[i]]
	// and vice versa.
	for (i = 0; i < n; ++i)
	{
		int slotA = teamSlots[0][i];
		int slotB = teamSlots[1][bestPerm[i]];
		playerOverride[slotA] = slotB;
		playerOverride[slotB] = slotA;
	}
	return true;
}
#undef MAX_PAIR

// Draw a paper-map-style scale onto the upscaled base preview:
//
//   * a faint white grid every 300 world units (alpha-blended so terrain
//     stays readable). 300 wu = 10 s of USA dozer travel at Speed=30 in
//     Locomotor.ini AmericaVehicleDozerLocomotor and also = CC vision
//     radius (Object AmericaCommandCenter, VisionRange=300 in
//     FactionBuilding.ini), so one grid cell = one ring radius;
//
//   * notches with seconds labels along the bottom and left edges (0 s
//     at the bottom-left corner, increasing rightward and upward); plus
//     a small "s = USA dozer travel" caption in the top-right corner;
//
//   * a CC vision reference ring centered on each occupied non-observer
//     slot's Player_N_Start waypoint.
//
// Drawn onto the base image so the original and mirror renders both
// inherit the legend.
static void drawScaleLegend(PixBuf *big, const MapMetaData *mmd,
                            const LobbySlotInfo *info)
{
	if (!big || !mmd) return;
	float worldW = mmd->m_extent.hi.x - mmd->m_extent.lo.x;
	float worldH = mmd->m_extent.hi.y - mmd->m_extent.lo.y;
	if (worldW <= 0.0f || worldH <= 0.0f) return;

	const float kSegmentWU = 300.0f; // CC vision radius == 10s dozer travel
	float pxPerWUx = (float)big->w / worldW;
	float pxPerWUy = (float)big->h / worldH;
	int segmentPxX = (int)(kSegmentWU * pxPerWUx + 0.5f);
	int segmentPxY = (int)(kSegmentWU * pxPerWUy + 0.5f);
	if (segmentPxX < 16 || segmentPxY < 16) return; // ticks too close to read

	const int textScale = 1;
	const int glyphPx = 8 * textScale;
	const int tickLen = 10;
	const unsigned char kGridAlpha = 60; // ~24%, low enough to not fight terrain

	int i, j;

	// ---- Faint grid every 300 wu ----
	// Stops short of the edge tick zone so the notches stay crisp.
	int gridStopY = big->h - tickLen - 1;
	int gridStartX = tickLen + 1;
	for (i = 1; i * segmentPxX < big->w; ++i)
	{
		int gx = i * segmentPxX;
		for (j = 0; j <= gridStopY; ++j)
			blendPixel(big, gx, j, 255, 255, 255, kGridAlpha);
	}
	for (i = 1; i * segmentPxY < big->h; ++i)
	{
		int gy = big->h - 1 - i * segmentPxY; // 0s at bottom edge
		for (j = gridStartX; j < big->w; ++j)
			blendPixel(big, j, gy, 255, 255, 255, kGridAlpha);
	}

	// ---- Bottom edge notches + seconds labels ----
	int numSegsX = (big->w - 1) / segmentPxX;
	if (numSegsX > 12) numSegsX = 12; // sanity cap; very wide previews
	for (i = 0; i <= numSegsX; ++i)
	{
		int tx = i * segmentPxX;
		// 1px-wide white tick over a 3px-wide black backing so the tick
		// reads against any terrain color.
		fillRect(big, tx - 1, big->h - tickLen - 1, 3, tickLen + 1, 0, 0, 0);
		fillRect(big, tx,     big->h - tickLen,     1, tickLen,     255, 255, 255);
		char lbl[8];
		sprintf(lbl, "%ds", i * 10);
		int lw = textWidthPx(lbl, textScale);
		int lx = tx - lw / 2;
		if (lx < 1) lx = 1;
		if (lx + lw > big->w - 1) lx = big->w - 1 - lw;
		int ly = big->h - tickLen - glyphPx - 2;
		drawTextStroked(big, lx, ly, lbl, textScale, 255, 255, 255);
	}

	// ---- Left edge notches + seconds labels ----
	// Skip i==0: the bottom edge's "0s" already labels the origin.
	int numSegsY = (big->h - 1) / segmentPxY;
	if (numSegsY > 12) numSegsY = 12;
	for (i = 1; i <= numSegsY; ++i)
	{
		int ty = big->h - 1 - i * segmentPxY;
		fillRect(big, 0, ty - 1, tickLen + 1, 3, 0, 0, 0);
		fillRect(big, 0, ty,     tickLen,     1, 255, 255, 255);
		char lbl[8];
		sprintf(lbl, "%ds", i * 10);
		int ly = ty - glyphPx / 2;
		if (ly < 1) ly = 1;
		if (ly + glyphPx > big->h - 1) ly = big->h - 1 - glyphPx;
		int lx = tickLen + 3;
		drawTextStroked(big, lx, ly, lbl, textScale, 255, 255, 255);
	}

	// ---- Top-right corner caption ----
	const char *cap = "s = USA dozer travel";
	int cw = textWidthPx(cap, textScale);
	int captionX = big->w - cw - 4;
	int captionY = 3;
	if (captionX < 1) captionX = 1;
	drawTextStroked(big, captionX, captionY, cap, textScale, 255, 255, 255);

	// ---- Per-start CC vision rings ----
	if (!info) return;
	// Pick the tighter axis so the ring never exaggerates the radius if
	// the map's pixel-per-world ratios differ.
	float pxPerWU = pxPerWUx < pxPerWUy ? pxPerWUx : pxPerWUy;
	int ringR = (int)(kSegmentWU * pxPerWU + 0.5f);
	if (ringR < 8) return; // too small to read as a circle
	int s;
	for (s = 0; s < MAX_SLOTS; ++s)
	{
		const LobbySlotInfo &si = info[s];
		if (!si.occupied || si.isObserver) continue;
		if (si.startPos < 0) continue;
		AsciiString wp;
		wp.format("Player_%d_Start", si.startPos + 1);
		WaypointMap::const_iterator wpIt = mmd->m_waypoints.find(wp);
		if (wpIt == mmd->m_waypoints.end()) continue;
		const Coord3D &pos = wpIt->second;
		int cx = (int)((pos.x - mmd->m_extent.lo.x) / worldW * big->w);
		int cy = (int)((1.0f - (pos.y - mmd->m_extent.lo.y) / worldH) * big->h);
		drawCircleOutline(big, cx, cy, ringR + 1, 1, 0, 0, 0);
		drawCircleOutline(big, cx, cy, ringR, 1, 255, 255, 255);
	}
}

// Draw player markers + name/faction/team labels on a pre-built PixBuf.
//
// `playerOverride`, if non-null, redirects the *player identity* drawn at
// each slot: at slot S the renderer reads name/team/color from
// info[playerOverride[S]] instead of info[S]. The slot's start position
// and faction stay anchored to S itself, since the mirror semantic is
// "faction stays at the location, player walks into it bringing their
// team and color with them." For the original (non-mirror) render the
// caller passes nullptr and slot self-mappings are used implicitly.
//
// Markers and labels run as two passes so a marker never lands on top of
// an adjacent slot's label text.
static void drawPlayerMarkersAndLabels(PixBuf *big,
                                       const LobbySlotInfo *info,
                                       const MapMetaData *mmd,
                                       int scale,
                                       const int *playerOverride)
{
	#define WORLD_TO_PX_X(wx) ((int)(((wx) - mmd->m_extent.lo.x) / (mmd->m_extent.hi.x - mmd->m_extent.lo.x) * big->w))
	#define WORLD_TO_PX_Y(wy) ((int)((1.0f - ((wy) - mmd->m_extent.lo.y) / (mmd->m_extent.hi.y - mmd->m_extent.lo.y)) * big->h))

	int slot;
	for (slot = 0; slot < MAX_SLOTS; ++slot)
	{
		const LobbySlotInfo &si = info[slot];        // location anchor (position, faction)
		if (!si.occupied || si.isObserver) continue;
		if (si.startPos < 0) continue;
		// Player whose name/team/color shows up at this location.
		int playerSlot = (playerOverride && playerOverride[slot] >= 0) ? playerOverride[slot] : slot;
		const LobbySlotInfo &pi = info[playerSlot];

		AsciiString wp;
		wp.format("Player_%d_Start", si.startPos + 1);
		WaypointMap::const_iterator wpIt = mmd->m_waypoints.find(wp);
		if (wpIt == mmd->m_waypoints.end()) continue;
		const Coord3D &pos = wpIt->second;
		int cx = WORLD_TO_PX_X(pos.x);
		int cy = WORLD_TO_PX_Y(pos.y);

		// Marker color follows the player.
		unsigned char mr = 255, mg = 255, mb = 255;
		if (TheMultiplayerSettings && pi.colorIdx >= 0)
		{
			MultiplayerColorDefinition *cd = TheMultiplayerSettings->getColor(pi.colorIdx);
			if (cd)
			{
				RGBColor rc = cd->getRGBValue();
				mr = (unsigned char)(rc.red   * 255.0f);
				mg = (unsigned char)(rc.green * 255.0f);
				mb = (unsigned char)(rc.blue  * 255.0f);
			}
		}

		int markerR = 9 + scale;
		drawCircleOutline(big, cx, cy, markerR + 2, 1, 0, 0, 0);
		drawFilledCircle(big, cx, cy, markerR, mr, mg, mb);
		drawCircleOutline(big, cx, cy, markerR, 2, 255, 255, 255);

		char num[4];
		sprintf(num, "%d", si.startPos + 1);
		int numScale = (markerR >= 14) ? 2 : 1;
		int numW = textWidthPx(num, numScale);
		int numH = 8 * numScale;
		drawTextStroked(big, cx - numW / 2, cy - numH / 2, num, numScale,
		                255, 255, 255);
	}

	// Labels second pass.
	for (slot = 0; slot < MAX_SLOTS; ++slot)
	{
		const LobbySlotInfo &si = info[slot];
		if (!si.occupied || si.isObserver) continue;
		if (si.startPos < 0) continue;
		int playerSlot = (playerOverride && playerOverride[slot] >= 0) ? playerOverride[slot] : slot;
		const LobbySlotInfo &pi = info[playerSlot];

		AsciiString wp;
		wp.format("Player_%d_Start", si.startPos + 1);
		WaypointMap::const_iterator wpIt = mmd->m_waypoints.find(wp);
		if (wpIt == mmd->m_waypoints.end()) continue;
		const Coord3D &pos = wpIt->second;
		int cx = WORLD_TO_PX_X(pos.x);
		int cy = WORLD_TO_PX_Y(pos.y);
		int markerR = 8 + scale;

		// Name and team follow the player; faction stays at the location.
		AsciiString line;
		line = pi.displayName;
		if (line.isEmpty()) line = "?";
		line.concat(" (");
		line.concat(si.factionAbbrev);
		line.concat(") [");
		if (pi.team >= 0)
		{
			char teamStr[8];
			sprintf(teamStr, "%d", pi.team + 1);
			line.concat(teamStr);
		}
		else
		{
			line.concat("FFA");
		}
		line.concat(']');

		const int tScale = 1;
		int tW = textWidthPx(line.str(), tScale);
		int tx = cx - tW / 2;
		int ty = cy + markerR + 4;
		drawTextStroked(big, tx, ty, line.str(), tScale, 255, 255, 255);
	}

	#undef WORLD_TO_PX_X
	#undef WORLD_TO_PX_Y
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
// Cliff/impassable parsing.
// -------------------------------------------------------------------------
// Open the .map file ourselves and pull just the HeightMapData chunk's raw
// height bytes (one byte per cell, multiplied by MAP_HEIGHT_SCALE for world
// units). We then derive cliff cells from per-cell slope. This mirrors the
// engine's initCliffFlagsFromHeights() fallback (WorldHeightMap.cpp):
// a cell is "cliff" when the max-min of its four corner heights exceeds
// PATHFIND_CLIFF_SLOPE_LIMIT_F (9.8 world units). Sidestepping the
// BlendTileData chunk avoids depending on its version-gated layout (cliff
// info ndxes, extra blend tiles, etc.) while still matching the engine's
// own slope-derived passability for typical maps.
// =========================================================================
struct CliffHeightData
{
	int width;       // cells across (raw, including border)
	int height;      // cells tall (raw, including border)
	int borderSize;  // non-playable border in cells; playable origin = (border, border)
	unsigned char *heights; // width*height bytes; owned by this struct
	CliffHeightData() : width(0), height(0), borderSize(0), heights(nullptr) {}
	~CliffHeightData() { free(heights); }
};

static Bool parseLobbyHeightMapChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	CliffHeightData *out = (CliffHeightData *)userData;
	out->width = file.readInt();
	out->height = file.readInt();
	if (info->version >= K_HEIGHT_MAP_VERSION_3)
		out->borderSize = file.readInt();
	else
		out->borderSize = 0;

	if (info->version >= K_HEIGHT_MAP_VERSION_4)
	{
		int numBorders = file.readInt();
		int i;
		for (i = 0; i < numBorders; ++i)
		{
			file.readInt(); // boundary.x (ignored; m_extent already covers playable region)
			file.readInt(); // boundary.y
		}
	}

	int dataSize = file.readInt();
	if (dataSize <= 0 || dataSize != out->width * out->height)
		return false;
	if (out->heights) { free(out->heights); out->heights = nullptr; }
	out->heights = (unsigned char *)malloc((size_t)dataSize);
	if (!out->heights) return false;
	file.readArrayOfBytes((char *)out->heights, dataSize);

	if (info->version == K_HEIGHT_MAP_VERSION_1)
	{
		// Decimate 2:1 to match engine behavior for very old maps.
		int newW = (out->width + 1) / 2;
		int newH = (out->height + 1) / 2;
		int i, j;
		for (i = 0; i < newH; ++i)
			for (j = 0; j < newW; ++j)
				out->heights[i * newW + j] = out->heights[2 * i * out->width + 2 * j];
		out->width = newW;
		out->height = newH;
	}
	return true;
}

static bool loadCliffHeightData(const AsciiString &mapPath, CliffHeightData *out)
{
	CachedFileInputStream fileStrm;
	if (!fileStrm.open(mapPath))
		return false;
	DataChunkInput file(&fileStrm);
	file.registerParser("HeightMapData", AsciiString::TheEmptyString, parseLobbyHeightMapChunk, out);
	try
	{
		if (!file.parse(out))
			return false;
	}
	catch (...)
	{
		return false;
	}
	return out->heights != nullptr && out->width > 0 && out->height > 0;
}

// Alpha-blend a soft red tint over each cell whose 4-corner height span
// exceeds PATHFIND_CLIFF_SLOPE_LIMIT_F. Threshold expressed in raw height
// bytes for speed: 9.8 / MAP_HEIGHT_SCALE = 9.8 / 0.625 = 15.68, so any
// span >= 16 bytes counts. Drawn after icons/scale so the tint goes on
// top of the upscaled terrain; player markers and labels paint on top
// of the tint afterwards.
static void drawImpassableOverlay(PixBuf *big,
                                  const MapMetaData *mmd,
                                  const CliffHeightData *hm)
{
	if (!big || !mmd || !hm || !hm->heights) return;
	if (hm->width < 2 || hm->height < 2) return;
	float worldW = mmd->m_extent.hi.x - mmd->m_extent.lo.x;
	float worldH = mmd->m_extent.hi.y - mmd->m_extent.lo.y;
	if (worldW <= 0.0f || worldH <= 0.0f) return;

	const int kThresholdBytes = 16; // ceil(9.8 / MAP_HEIGHT_SCALE)
	const unsigned char tintR = 255, tintG = 60, tintB = 60;
	const unsigned char tintA = 70;

	int w = hm->width;
	int h = hm->height;
	int border = hm->borderSize;

	int x, y;
	for (y = 0; y < h - 1; ++y)
	{
		for (x = 0; x < w - 1; ++x)
		{
			unsigned char h1 = hm->heights[y * w + x];
			unsigned char h2 = hm->heights[y * w + (x + 1)];
			unsigned char h3 = hm->heights[(y + 1) * w + x];
			unsigned char h4 = hm->heights[(y + 1) * w + (x + 1)];
			unsigned char mn = h1, mx = h1;
			if (h2 < mn) mn = h2; if (h2 > mx) mx = h2;
			if (h3 < mn) mn = h3; if (h3 > mx) mx = h3;
			if (h4 < mn) mn = h4; if (h4 > mx) mx = h4;
			if ((int)mx - (int)mn < kThresholdBytes) continue;

			// Cell (x, y) covers world rect [(x-border)*MAP_XY_FACTOR ..
			// (x+1-border)*MAP_XY_FACTOR] in x; similarly in y. The extent
			// already represents the playable region, so cells in the
			// border map to negative/out-of-extent world coords and clip
			// against big->w/big->h via blendPixel.
			float wx0 = ((float)x - (float)border) * MAP_XY_FACTOR;
			float wx1 = ((float)x + 1.0f - (float)border) * MAP_XY_FACTOR;
			float wy0 = ((float)y - (float)border) * MAP_XY_FACTOR;
			float wy1 = ((float)y + 1.0f - (float)border) * MAP_XY_FACTOR;
			int px0 = (int)((wx0 - mmd->m_extent.lo.x) / worldW * big->w);
			int px1 = (int)((wx1 - mmd->m_extent.lo.x) / worldW * big->w);
			// World Y grows up; image Y grows down — flip and swap.
			int py0 = (int)((1.0f - (wy1 - mmd->m_extent.lo.y) / worldH) * big->h);
			int py1 = (int)((1.0f - (wy0 - mmd->m_extent.lo.y) / worldH) * big->h);
			if (px0 > px1) { int t = px0; px0 = px1; px1 = t; }
			if (py0 > py1) { int t = py0; py0 = py1; py1 = t; }
			int ix, iy;
			for (iy = py0; iy < py1; ++iy)
				for (ix = px0; ix < px1; ++ix)
					blendPixel(big, ix, iy, tintR, tintG, tintB, tintA);
		}
	}
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
	LobbySlotInfo info[MAX_SLOTS];
	int slotIdx;
	int humanCount = 0;
	for (slotIdx = 0; slotIdx < MAX_SLOTS; ++slotIdx)
	{
		const GameSlot *slot = game->getConstSlot(slotIdx);
		LobbySlotInfo &si = info[slotIdx];
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
	char tgaErr[256];
	tgaErr[0] = '\0';
	if (!loadTgaFromFileSystem(tgaPath, &src, tgaErr, (int)sizeof(tgaErr)))
	{
		debugChat("preview tga not loadable: %s [%s]", tgaPath.str(), tgaErr);
		printf("[discord] preview tga not loadable: %s [%s]\n",
			tgaPath.str(), tgaErr);
		return;
	}
	debugChat("loaded tga %dx%d from %s", src.w, src.h, tgaPath.str());

	// Resample the TGA preview (always 128x128 — the WorldBuilder writes a
	// fixed-size square regardless of the actual map dimensions) into the
	// world's real aspect ratio so non-square maps stop looking squished.
	// Aim for ~1024 px on the long edge so labels and overlay icons have
	// room to breathe, and clamp so the PNG stays under Discord's
	// attachment cap.
	int outW, outH;
	{
		float worldW = mmd->m_extent.hi.x - mmd->m_extent.lo.x;
		float worldH = mmd->m_extent.hi.y - mmd->m_extent.lo.y;
		if (worldW <= 0.0f || worldH <= 0.0f)
		{
			// Degenerate extent; fall back to TGA pixel aspect.
			worldW = (float)src.w;
			worldH = (float)src.h;
		}
		const int kTargetLongEdge = 1024;
		const int kMaxLongEdge    = 1600;
		const int kMinLongEdge    = 512;
		if (worldW >= worldH)
		{
			outW = kTargetLongEdge;
			outH = (int)((float)kTargetLongEdge * worldH / worldW + 0.5f);
		}
		else
		{
			outH = kTargetLongEdge;
			outW = (int)((float)kTargetLongEdge * worldW / worldH + 0.5f);
		}
		if (outW < 1) outW = 1;
		if (outH < 1) outH = 1;
		// Enforce minimum long edge in case aspect-ratio math underflowed.
		int curLong = outW > outH ? outW : outH;
		if (curLong > kMaxLongEdge)
		{
			float k = (float)kMaxLongEdge / (float)curLong;
			outW = (int)(outW * k + 0.5f);
			outH = (int)(outH * k + 0.5f);
		}
		else if (curLong < kMinLongEdge)
		{
			float k = (float)kMinLongEdge / (float)curLong;
			outW = (int)(outW * k + 0.5f);
			outH = (int)(outH * k + 0.5f);
		}
	}

	PixBuf big;
	if (!scaleRGB_NN(&src, outW, outH, &big))
	{
		debugChat("upscale failed");
		return;
	}
	debugChat("resampled to %dx%d (world %.0fx%.0f)", big.w, big.h,
		mmd->m_extent.hi.x - mmd->m_extent.lo.x,
		mmd->m_extent.hi.y - mmd->m_extent.lo.y);

	// Effective integer "scale" used by overlays sized in tga-pixel terms
	// (player marker radius, etc.). Derived from the ratio of the bigger
	// output edge to the bigger source edge so behavior stays close to the
	// previous integer-scale code path.
	int srcLong = src.w > src.h ? src.w : src.h;
	int dstLong = big.w > big.h ? big.w : big.h;
	int scale = srcLong > 0 ? dstLong / srcLong : 8;
	if (scale < 4) scale = 4;
	if (scale > 12) scale = 12;

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
	// Garrisonable civilian buildings (KINDOF_GARRISONABLE_UNTIL_DESTROYED):
	// a small white dot with a 1px black outline so it reads against any
	// terrain. Sized smaller than the player markers so it doesn't get
	// confused for a start position.
	{
		int dotR = iconSize / 5;
		if (dotR < 3) dotR = 3;
		Coord3DList::const_iterator it;
		for (it = mmd->m_garrisonablePositions.begin(); it != mmd->m_garrisonablePositions.end(); ++it)
		{
			int gx = WORLD_TO_PX_X(it->x);
			int gy = WORLD_TO_PX_Y(it->y);
			drawFilledCircle(&big, gx, gy, dotR + 1, 0, 0, 0);
			drawFilledCircle(&big, gx, gy, dotR, 255, 255, 255);
		}
	}

	#undef DRAW_ICON_OR_GLYPH
	#undef WORLD_TO_PX_X
	#undef WORLD_TO_PX_Y

	// Impassable cliff tint: live-parse the .map's HeightMapData chunk
	// and blend a light red wash over every cell whose 4-corner height
	// span crosses the engine's cliff slope threshold. Drawn before the
	// scale legend so the grid lines render on top of the wash.
	{
		CliffHeightData hm;
		AsciiString mapPath = game->getMap();
		if (loadCliffHeightData(mapPath, &hm))
		{
			drawImpassableOverlay(&big, mmd, &hm);
			debugChat("cliff overlay: %dx%d cells, border=%d", hm.width, hm.height, hm.borderSize);
		}
		else
		{
			debugChat("cliff overlay: heightmap parse failed for %s", mapPath.str());
		}
	}

	// Paper-map scale legend (faint 300-wu grid + edge notches with
	// seconds labels + CC vision ring at each occupied start) drawn onto
	// the base so both render variants pick it up for free.
	drawScaleLegend(&big, mmd, info);

	// At this point `big` is the "base" image: upscaled preview + neutral
	// overlays. Both the original and the mirror render start from a
	// fresh copy of this base so markers from one don't leak into the
	// other. Sharing the expensive parts (upscale, icon load+scale, icon
	// composite) between the two renders keeps the second post cheap.

	// JSON-escape the map's display name once for both posts.
	AsciiString mapDisplayAscii;
	mapDisplayAscii.translate(mmd->m_displayName);
	AsciiString safeMap;
	{
		const char *p;
		for (p = mapDisplayAscii.str(); *p; ++p)
		{
			if (*p == '"' || *p == '\\') safeMap.concat('\\');
			safeMap.concat(*p);
		}
	}

	// Render + post variant. playerOverride==nullptr means "use the slot's
	// own player identity" (original assignment). Non-null arrays
	// redirect name/team/color at each slot to the slot whose player has
	// walked into that location for the mirror. Position and faction
	// always come from the slot itself. The variant's content string
	// lands in the Discord message above the attachment.
	struct RenderAndPost
	{
		static void run(const char *kWebhookUrl,
		                const PixBuf *baseImg,
		                const LobbySlotInfo *info,
		                const MapMetaData *mmd,
		                int scale,
		                const int *playerOverride,
		                const char *contentText,
		                const char *filename)
		{
			PixBuf canvas;
			if (!copyPixBuf(baseImg, &canvas))
			{
				debugChat("copy of base image failed");
				return;
			}
			drawPlayerMarkersAndLabels(&canvas, info, mmd, scale, playerOverride);

			unsigned char *png = nullptr;
			unsigned int pngLen = 0;
			if (!encodePng(&canvas, &png, &pngLen))
			{
				debugChat("png encode failed (%s)", filename);
				return;
			}
			debugChat("encoded %s %ux%u (%u bytes), posting...",
			          filename, (unsigned)canvas.w, (unsigned)canvas.h, pngLen);

			char json[768];
			sprintf(json, "{\"content\":\"%.500s\"}",
			        (contentText && *contentText) ? contentText : "");

			char err[128] = {0};
			unsigned long status = postPngToDiscord(kWebhookUrl, png, pngLen,
			                                       filename, json, err, sizeof(err));
			if (status >= 200 && status < 300)
				debugChat("posted ok (HTTP %lu, %s)", status, filename);
			else if (status != 0)
				debugChat("HTTP %lu (%s rejected)", status, filename);
			else
				debugChat("transport failed (%s): %s", filename, err[0] ? err : "(no detail)");

			free(png);
		}
	};

	// 1) Original assignment.
	{
		char content[256];
		sprintf(content, "Lobby starting on **%.200s** (%d humans)",
		        safeMap.str(), humanCount);
		RenderAndPost::run(kWebhookUrl, &big, info, mmd, scale,
		                   nullptr, content, "lobby.png");
	}

	// 2) Matchup-preserving mirror: pair each player with their geometric
	//    matchup opponent on the other team via min-cost bipartite
	//    matching, then swap the *players* (name + team + color) between
	//    each paired slot. Position and faction stay anchored at each
	//    location, so in the rematch every player ends up at their
	//    previous opponent's start playing their previous opponent's
	//    faction. Skipped on FFA / >2-team / unbalanced lobbies — see
	//    computeMirrorSwap for the precise constraints.
	int playerOverride[MAX_SLOTS];
	if (computeMirrorSwap(info, mmd, playerOverride))
	{
		bool anySwapped = false;
		int s;
		for (s = 0; s < MAX_SLOTS; ++s)
		{
			if (playerOverride[s] != s) { anySwapped = true; break; }
		}
		if (anySwapped)
		{
			char content[256];
			sprintf(content, "**Mirror** (players swap with their matched opponent; factions and start positions stay put)");
			RenderAndPost::run(kWebhookUrl, &big, info, mmd, scale,
			                   playerOverride, content, "lobby-mirror.png");
		}
		else
		{
			debugChat("mirror is a no-op (no players swapped)");
		}
	}
	else
	{
		debugChat("mirror unavailable (need exactly 2 teams of equal, non-zero size)");
	}
}
