#include "SDL3Device/GameClient/SDL3AniReader.h"

#include "Common/Debug.h"

#include <SDL3/SDL_endian.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_surface.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

// TheSuperHackers @refactor denysmitin 04/12/2025 Reimplemented SDL3 ANI loader with SDL cursor generation.

namespace
{
	static const Uint32 _FOURCC_LIST = SDL_FOURCC('L', 'I', 'S', 'T');
	static const Uint32 _FOURCC_ANIH = SDL_FOURCC('a', 'n', 'i', 'h');
	static const Uint32 _FOURCC_FRAM = SDL_FOURCC('f', 'r', 'a', 'm');
	static const Uint32 _FOURCC_ICON = SDL_FOURCC('i', 'c', 'o', 'n');
	static const Uint32 _FOURCC_RATE = SDL_FOURCC('r', 'a', 't', 'e');
	static const Uint32 _FOURCC_SEQ  = SDL_FOURCC('s', 'e', 'q', ' ');

	static Uint16 readLE16(const Byte *ptr)
	{
		return static_cast<Uint16>(ptr[0] | (ptr[1] << 8));
	}

	static Uint32 readLE32(const Byte *ptr)
	{
		return static_cast<Uint32>(ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24));
	}

	static size_t paddedSize(size_t value)
	{
		return (value + 1) & ~static_cast<size_t>(1);
	}

	struct IconCandidate
	{
		const Byte *payload;
		size_t payloadSize;
		Int hotspotX;
		Int hotspotY;
		Int score;
	};

	struct CursorImage
	{
		SDL_Surface *surface;
		Int hotspotX;
		Int hotspotY;

		CursorImage() : surface(NULL), hotspotX(0), hotspotY(0) {}
	};

	static Bool decodeCursorBitmap(const Byte *payload, size_t payloadSize, Int hotspotX, Int hotspotY, CursorImage &outImage)
	{
		if (payloadSize < 40)
		{
			DEBUG_CRASH(("SDL3AniReader: Icon image header truncated"));
			return FALSE;
		}

		Uint32 biSize = readLE32(payload + 0);
		if (biSize < 40 || biSize > payloadSize)
		{
			DEBUG_CRASH(("SDL3AniReader: Unsupported icon header size"));
			return FALSE;
		}

		Int width = static_cast<Int>(readLE32(payload + 4));
		Int heightField = static_cast<Int>(readLE32(payload + 8));
		Uint16 planes = readLE16(payload + 12);
		Uint16 bitCount = readLE16(payload + 14);
		Uint32 compression = readLE32(payload + 16);
		Uint32 clrUsed = readLE32(payload + 32);

		if (planes != 1 || compression != 0)
		{
			DEBUG_CRASH(("SDL3AniReader: Unsupported cursor planes/compression"));
			return FALSE;
		}

		if (width <= 0)
		{
			DEBUG_CRASH(("SDL3AniReader: Invalid cursor width"));
			return FALSE;
		}

		Int reportedHeight = (heightField < 0) ? -heightField : heightField;
		Int imageHeight = (heightField < 0) ? reportedHeight : (reportedHeight / 2);
		if (imageHeight <= 0)
		{
			DEBUG_CRASH(("SDL3AniReader: Invalid cursor height"));
			return FALSE;
		}

		const Byte *palette = payload + biSize;
		size_t paletteEntries = 0;
		if (bitCount <= 8)
		{
			paletteEntries = (clrUsed != 0) ? clrUsed : (1U << bitCount);
		}
		size_t paletteBytes = paletteEntries * 4;
		if (biSize + paletteBytes > payloadSize)
		{
			DEBUG_CRASH(("SDL3AniReader: Palette exceeds icon payload"));
			return FALSE;
		}

		const Byte *xorData = palette + paletteBytes;
		size_t xorStrideBits = static_cast<size_t>(width) * static_cast<size_t>(bitCount);
		size_t xorStride = ((xorStrideBits + 31) / 32) * 4;
		size_t xorBytes = xorStride * static_cast<size_t>(imageHeight);
		if (xorBytes == 0 || xorData + xorBytes > payload + payloadSize)
		{
			DEBUG_CRASH(("SDL3AniReader: XOR bitmap truncated"));
			return FALSE;
		}

		const Byte *maskData = xorData + xorBytes;
		size_t maskStride = ((static_cast<size_t>(width) + 31) / 32) * 4;
		size_t maskBytes = maskStride * static_cast<size_t>(imageHeight);
		Bool hasMask = TRUE;
		if (maskBytes == 0 || maskData + maskBytes > payload + payloadSize)
		{
			hasMask = FALSE;
			maskData = NULL;
		}

		SDL_Surface *surface = SDL_CreateSurface(width, imageHeight, SDL_PIXELFORMAT_ARGB8888);
		if (!surface)
		{
			DEBUG_CRASH(("SDL3AniReader: Failed to allocate surface: %s", SDL_GetError()));
			return FALSE;
		}

		if (SDL_LockSurface(surface) != 0)
		{
			DEBUG_CRASH(("SDL3AniReader: Failed to lock surface: %s", SDL_GetError()));
			SDL_DestroySurface(surface);
			return FALSE;
		}

		Uint8 *dstPixels = static_cast<Uint8 *>(surface->pixels);
		Int dstPitch = surface->pitch;

		for (Int y = 0; y < imageHeight; ++y)
		{
			Int srcY = imageHeight - 1 - y;
			const Byte *xorRow = xorData + static_cast<size_t>(srcY) * xorStride;
			const Byte *maskRow = hasMask ? (maskData + static_cast<size_t>(srcY) * maskStride) : NULL;
			Uint32 *dstRow = reinterpret_cast<Uint32 *>(dstPixels + static_cast<size_t>(y) * dstPitch);

			for (Int x = 0; x < width; ++x)
			{
				Uint8 r = 0;
				Uint8 g = 0;
				Uint8 b = 0;
				Uint8 a = 255;

				if (bitCount == 32)
				{
					const Byte *pixel = xorRow + x * 4;
					b = pixel[0];
					g = pixel[1];
					r = pixel[2];
					a = pixel[3];
				}
				else if (bitCount == 24)
				{
					const Byte *pixel = xorRow + x * 3;
					b = pixel[0];
					g = pixel[1];
					r = pixel[2];
				}
				else if (bitCount <= 8)
				{
					Int paletteIndex = 0;
					if (bitCount == 8)
					{
						paletteIndex = xorRow[x];
					}
					else if (bitCount == 4)
					{
						Uint8 byteVal = xorRow[x / 2];
						paletteIndex = (x & 1) ? (byteVal & 0x0F) : (byteVal >> 4);
					}
					else if (bitCount == 1)
					{
						Uint8 byteVal = xorRow[x / 8];
						paletteIndex = (byteVal >> (7 - (x & 7))) & 0x01;
					}
					paletteIndex = std::min<Int>(paletteIndex, static_cast<Int>(paletteEntries) - 1);
					const Byte *entry = palette + static_cast<size_t>(paletteIndex) * 4;
					b = entry[0];
					g = entry[1];
					r = entry[2];
				}
				else
				{
					SDL_UnlockSurface(surface);
					SDL_DestroySurface(surface);
					DEBUG_CRASH(("SDL3AniReader: Unsupported cursor bit depth %u", bitCount));
					return FALSE;
				}

				if (hasMask && maskRow)
				{
					Uint8 maskByte = maskRow[x / 8];
					Bool masked = ((maskByte >> (7 - (x & 7))) & 0x01) != 0;
					if (masked)
					{
						a = 0;
					}
				}

				Uint32 pixel = (static_cast<Uint32>(a) << 24) |
											 (static_cast<Uint32>(r) << 16) |
											 (static_cast<Uint32>(g) << 8) |
											 static_cast<Uint32>(b);
				dstRow[x] = pixel;
			}
		}

		SDL_UnlockSurface(surface);

		outImage.surface = surface;
		outImage.hotspotX = hotspotX;
		outImage.hotspotY = hotspotY;
		return TRUE;
	}

	static Bool createCursorFromIcon(const Byte *chunkData, size_t chunkSize, SDL3AniReader::Frame &outFrame)
	{
		if (chunkSize < 6)
		{
			DEBUG_CRASH(("SDL3AniReader: Icon chunk too small"));
			return FALSE;
		}

		const Byte *data = chunkData;
		Uint16 idReserved = readLE16(data + 0);
		Uint16 idType = readLE16(data + 2);
		Uint16 idCount = readLE16(data + 4);

		if (idReserved != 0 || idCount == 0)
		{
			DEBUG_CRASH(("SDL3AniReader: Invalid icon header"));
			return FALSE;
		}

		const size_t entrySize = 16;
		size_t directorySize = 6 + entrySize * static_cast<size_t>(idCount);
		if (chunkSize < directorySize)
		{
			DEBUG_CRASH(("SDL3AniReader: Icon directory truncated"));
			return FALSE;
		}

		const Byte *entryData = data + 6;
		IconCandidate best = {NULL, 0, 0, 0, -1};

		for (Uint16 i = 0; i < idCount; ++i)
		{
			const Byte *entry = entryData + entrySize * i;
			Int width = entry[0] ? entry[0] : 256;
			Int height = entry[1] ? entry[1] : 256;
			Uint16 bitCount = readLE16(entry + 6);
			Int score = width + height + static_cast<Int>(bitCount);

			Uint32 bytesInRes = readLE32(entry + 8);
			Uint32 imageOffset = readLE32(entry + 12);

			if (imageOffset + bytesInRes > chunkSize)
			{
				continue;
			}

			Int hotspotX = 0;
			Int hotspotY = 0;
			if (idType == 2)
			{
				hotspotX = static_cast<Int>(readLE16(entry + 4));
				hotspotY = static_cast<Int>(readLE16(entry + 6));
			}

			if (score > best.score)
			{
				best.payload = data + imageOffset;
				best.payloadSize = bytesInRes;
				best.hotspotX = hotspotX;
				best.hotspotY = hotspotY;
				best.score = score;
			}
		}

		if (!best.payload)
		{
			DEBUG_CRASH(("SDL3AniReader: No valid icon payload"));
			return FALSE;
		}

		CursorImage cursorImage;
		if (!decodeCursorBitmap(best.payload, best.payloadSize, best.hotspotX, best.hotspotY, cursorImage))
		{
			return FALSE;
		}

		SDL_Cursor *cursor = SDL_CreateColorCursor(cursorImage.surface, cursorImage.hotspotX, cursorImage.hotspotY);
		SDL_DestroySurface(cursorImage.surface);

		if (!cursor)
		{
			DEBUG_CRASH(("SDL3AniReader: SDL_CreateColorCursor failed: %s", SDL_GetError()));
			return FALSE;
		}

		outFrame.cursor = cursor;
		outFrame.hotspotX = cursorImage.hotspotX;
		outFrame.hotspotY = cursorImage.hotspotY;
		outFrame.durationMs = 0;
		return TRUE;
	}

	static void destroyFrames(std::vector<SDL3AniReader::Frame> &frames)
	{
		for (size_t i = 0; i < frames.size(); ++i)
		{
			if (frames[i].cursor)
			{
				SDL_DestroyCursor(frames[i].cursor);
				frames[i].cursor = NULL;
			}
		}
		frames.clear();
	}
}

SDL3AniReader::SDL3AniReader()
{
}

SDL3AniReader::~SDL3AniReader()
{
}

Bool SDL3AniReader::load(const char *path, std::vector<Frame> &outFrames)
{
	outFrames.clear();

	size_t fileSize = 0;
	void *fileMemory = SDL_LoadFile(path, &fileSize);
	if (!fileMemory)
	{
		DEBUG_CRASH(("SDL3AniReader: Failed to load %s: %s", path ? path : "<null>", SDL_GetError()));
		return FALSE;
	}

	const Byte *bytes = static_cast<const Byte *>(fileMemory);
	Bool success = FALSE;

	if (fileSize < 12 || std::memcmp(bytes, "RIFF", 4) != 0 || std::memcmp(bytes + 8, "ACON", 4) != 0)
	{
		DEBUG_CRASH(("SDL3AniReader: %s is not an animated cursor", path ? path : "<null>"));
	}
	else
	{
		std::vector<Frame> rawFrames;
		std::vector<Uint32> rates;
		std::vector<Uint32> sequence;

		struct AniHeader
		{
			Uint32 cbSizeOf;
			Uint32 cFrames;
			Uint32 cSteps;
			Uint32 cx;
			Uint32 cy;
			Uint32 cBitCount;
			Uint32 cPlanes;
			Uint32 jifRate;
			Uint32 flags;
		} header = {};

		Bool parseOk = TRUE;
		size_t offset = 12;
		while (offset + 8 <= fileSize)
		{
			Uint32 chunkId = readLE32(bytes + offset);
			Uint32 chunkSize = readLE32(bytes + offset + 4);
			size_t chunkDataOffset = offset + 8;
			size_t padded = paddedSize(chunkSize);
			if (chunkDataOffset + padded > fileSize)
			{
				break;
			}

			const Byte *chunkData = bytes + chunkDataOffset;

			if (chunkId == _FOURCC_ANIH)
			{
				if (chunkSize >= 36)
				{
					header.cbSizeOf = readLE32(chunkData + 0);
					header.cFrames = readLE32(chunkData + 4);
					header.cSteps = readLE32(chunkData + 8);
					header.cx = readLE32(chunkData + 12);
					header.cy = readLE32(chunkData + 16);
					header.cBitCount = readLE32(chunkData + 20);
					header.cPlanes = readLE32(chunkData + 24);
					header.jifRate = readLE32(chunkData + 28);
					header.flags = readLE32(chunkData + 32);
				}
			}
			else if (chunkId == _FOURCC_RATE)
			{
				Uint32 count = chunkSize / 4;
				rates.resize(count);
				for (Uint32 i = 0; i < count; ++i)
				{
					rates[i] = readLE32(chunkData + i * 4);
				}
			}
			else if (chunkId == _FOURCC_SEQ)
			{
				Uint32 count = chunkSize / 4;
				sequence.resize(count);
				for (Uint32 i = 0; i < count; ++i)
				{
					sequence[i] = readLE32(chunkData + i * 4);
				}
			}
			else if (chunkId == _FOURCC_LIST && chunkSize >= 4)
			{
				Uint32 listType = readLE32(chunkData);
				if (listType == _FOURCC_FRAM)
				{
					size_t listOffset = chunkDataOffset + 4;
					size_t listEnd = chunkDataOffset + chunkSize;
					while (listOffset + 8 <= listEnd)
					{
						Uint32 subId = readLE32(bytes + listOffset);
						Uint32 subSize = readLE32(bytes + listOffset + 4);
						size_t subDataOffset = listOffset + 8;
						size_t subPadded = paddedSize(subSize);
						if (subDataOffset + subPadded > listEnd)
						{
							break;
						}

						if (subId == _FOURCC_ICON)
						{
							Frame frame = {};
							if (!createCursorFromIcon(bytes + subDataOffset, subSize, frame))
							{
								destroyFrames(rawFrames);
								parseOk = FALSE;
								break;
							}
							rawFrames.push_back(frame);
						}

						listOffset = subDataOffset + subPadded;
					}
					if (!parseOk)
					{
						break;
					}
				}
			}

			offset = chunkDataOffset + padded;
			if (!parseOk)
			{
				break;
			}
		}

		if (parseOk && !rawFrames.empty())
		{
			std::vector<Uint32> frameOrder;
			if (!sequence.empty())
			{
				frameOrder = sequence;
			}
			else
			{
				Uint32 steps = header.cSteps ? header.cSteps : header.cFrames;
				if (steps == 0)
				{
					steps = static_cast<Uint32>(rawFrames.size());
				}
				for (Uint32 i = 0; i < steps && i < rawFrames.size(); ++i)
				{
					frameOrder.push_back(i);
				}
				if (frameOrder.empty())
				{
					for (Uint32 i = 0; i < rawFrames.size(); ++i)
					{
						frameOrder.push_back(i);
					}
				}
			}

			if (!frameOrder.empty())
			{
				Uint32 defaultRate = header.jifRate ? header.jifRate : 10;
				std::vector<Frame> finalFrames;
				finalFrames.reserve(frameOrder.size());

				for (size_t i = 0; i < frameOrder.size(); ++i)
				{
					Uint32 idx = frameOrder[i];
					if (idx >= rawFrames.size())
					{
						continue;
					}

					Uint32 rate = defaultRate;
					if (!rates.empty())
					{
						Uint32 rateIndex = (i < rates.size()) ? static_cast<Uint32>(i) : static_cast<Uint32>(rates.size() - 1);
						rate = rates[rateIndex];
					}
					if (rate == 0)
					{
						rate = defaultRate;
					}

					Uint32 durationMs = static_cast<Uint32>((1000ULL * rate) / 60ULL);
					if (durationMs == 0)
					{
						durationMs = 16;
					}

					Frame frame = rawFrames[idx];
					frame.durationMs = durationMs;
					finalFrames.push_back(frame);
				}

				if (!finalFrames.empty())
				{
					outFrames.swap(finalFrames);
					success = TRUE;
				}
			}

			if (!success)
			{
				destroyFrames(rawFrames);
			}
		}
		else if (!parseOk)
		{
			success = FALSE;
		}
	}

	SDL_free(fileMemory);
	return success;
}
