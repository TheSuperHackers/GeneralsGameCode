#include "SDL3Device/GameClient/SDL3AniReader.h"

#include <SDL3/SDL_endian.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_surface.h>

// TheSuperHackers @feature denysmitin 05/12/2025 Implemented SDL3 ANI loader with SDL cursor generation.

namespace
{
	static const Uint32 _FOURCC_LIST = SDL_FOURCC('L', 'I', 'S', 'T');
	static const Uint32 _FOURCC_ANIH = SDL_FOURCC('a', 'n', 'i', 'h');
	static const Uint32 _FOURCC_FRAM = SDL_FOURCC('f', 'r', 'a', 'm');
	static const Uint32 _FOURCC_ICON = SDL_FOURCC('i', 'c', 'o', 'n');
	static const Uint32 _FOURCC_RATE = SDL_FOURCC('r', 'a', 't', 'e');
	static const Uint32 _FOURCC_SEQ = SDL_FOURCC('s', 'e', 'q', ' ');

	static Uint16 readLE16(const Byte *ptr)
	{
		return static_cast<Uint16>((Uint16(ptr[0]) & 0xff) | (Uint16(ptr[1] << 8) & 0xff00));
	}

	static Uint32 readLE32(const Byte *ptr)
	{
		return static_cast<Uint32>((Uint32(ptr[0]) & 0xff) | ((Uint32(ptr[1]) << 8) & 0xff00) | ((Uint32(ptr[2]) << 16) & 0xff0000) | ((Uint32(ptr[3]) << 24) & 0xff000000));
	}

	static size_t paddedSize(size_t value)
	{
		return (value + 1) & ~static_cast<size_t>(1);
	}

	struct IconCandidate
	{
		const Byte *payload;
		size_t payloadSize;
		int hotspotX;
		int hotspotY;
	};

	struct CursorImage
	{
		SDL_Surface *surface;
		int hotspotX;
		int hotspotY;

		CursorImage() : surface(NULL), hotspotX(0), hotspotY(0) {}
	};

	static bool decodeCursorBitmap(const Byte *payload, size_t payloadSize, int hotspotX, int hotspotY, CursorImage &outImage)
	{
		if (payloadSize < 40)
		{
			return SDL_SetError("Invalid bitmap payload size: %d", payloadSize);
		}

		Uint32 biSize = readLE32(payload + 0);
		if (biSize < 40 || biSize > payloadSize)
		{
			return SDL_SetError("Invalid bitmap size: %d", biSize);
		}

		int width = static_cast<int>(readLE32(payload + 4));
		int heightField = static_cast<int>(readLE32(payload + 8));
		Uint16 planes = readLE16(payload + 12);
		Uint16 bitCount = readLE16(payload + 14);
		Uint32 compression = readLE32(payload + 16);
		Uint32 clrUsed = readLE32(payload + 32);

		if (planes != 1 || compression != 0)
		{
			return SDL_SetError("Unsupported number of planes or compression");
		}

		if (width <= 0)
		{
			return SDL_SetError("Invalid image width");
		}

		int reportedHeight = (heightField < 0) ? -heightField : heightField;
		int imageHeight = (heightField < 0) ? reportedHeight : (reportedHeight / 2);
		if (imageHeight <= 0)
		{
			return SDL_SetError("Invalid image height");
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
			return SDL_SetError("Bitmap is missing palette");
		}

		const Byte *xorData = palette + paletteBytes;
		size_t xorStrideBits = static_cast<size_t>(width) * static_cast<size_t>(bitCount);
		size_t xorStride = ((xorStrideBits + 31) / 32) * 4;
		size_t xorBytes = xorStride * static_cast<size_t>(imageHeight);
		if (xorBytes == 0 || xorData + xorBytes > payload + payloadSize)
		{
			return SDL_SetError("Missing XOR map");
		}

		const Byte *maskData = xorData + xorBytes;
		size_t maskStride = ((static_cast<size_t>(width) + 31) / 32) * 4;
		size_t maskBytes = maskStride * static_cast<size_t>(imageHeight);
		bool hasMask = TRUE;
		if (maskBytes == 0 || maskData + maskBytes > payload + payloadSize)
		{
			hasMask = FALSE;
			maskData = NULL;
		}

		SDL_Surface *surface = SDL_CreateSurface(width, imageHeight, SDL_PIXELFORMAT_ARGB8888);
		if (!surface)
		{
			return SDL_SetError("Failed to crate SDL_Surface: %s", SDL_GetError());
		}

		if (!SDL_LockSurface(surface))
		{
			SDL_DestroySurface(surface);
			return SDL_SetError("Failed to lock SDL_Surface: %s", SDL_GetError());
		}

		Uint8 *dstPixels = static_cast<Uint8 *>(surface->pixels);
		int dstPitch = surface->pitch;

		for (int y = 0; y < imageHeight; ++y)
		{
			int srcY = imageHeight - 1 - y;
			const Byte *xorRow = xorData + static_cast<size_t>(srcY) * xorStride;
			const Byte *maskRow = hasMask ? (maskData + static_cast<size_t>(srcY) * maskStride) : NULL;
			Uint32 *dstRow = reinterpret_cast<Uint32 *>(dstPixels + static_cast<size_t>(y) * dstPitch);

			for (int x = 0; x < width; ++x)
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
					int paletteIndex = 0;
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
					paletteIndex = std::min<int>(paletteIndex, static_cast<int>(paletteEntries) - 1);
					const Byte *entry = palette + static_cast<size_t>(paletteIndex) * 4;
					b = entry[0];
					g = entry[1];
					r = entry[2];
				}
				else
				{
					SDL_UnlockSurface(surface);
					SDL_DestroySurface(surface);
					return SDL_SetError("Unsupported bit count: %d", bitCount);
					;
				}

				if (hasMask && maskRow)
				{
					Uint8 maskByte = maskRow[x / 8];
					bool masked = ((maskByte >> (7 - (x & 7))) & 0x01) != 0;
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

	static bool createCursorFromIcon(const Byte *chunkData, size_t chunkSize, SDL3AniReader::Frame &outFrame)
	{
		if (chunkSize < 6)
		{
			return SDL_SetError("Invalid icon chunk size: %d", chunkSize);
		}

		const Byte *data = chunkData;
		Uint16 idReserved = readLE16(data + 0); // must be = 0
		Uint16 idType = readLE16(data + 2);			// either 1 or 2
		Uint16 idCount = readLE16(data + 4);		// must be > 0

		if (idReserved != 0 || idCount == 0)
		{
			return SDL_SetError("Invalid icon chunk structure");
		}

		const size_t entrySize = idType == 1 ? 16 : 24; // 16 bytes for ICO, 24 bytes for CUR
		size_t directorySize = 6 + entrySize * static_cast<size_t>(idCount);
		if (chunkSize < directorySize)
		{
			return SDL_SetError("Invalid icon chunk structure");
		}

		const Byte *entryData = data + 6;
		IconCandidate best = {NULL, 0, 0, 0};

		for (Uint16 i = 0; i < idCount; ++i)
		{
			const Byte *entry = entryData + entrySize * i;
			// int width = entry[0] ? entry[0] : 32;
			// int height = entry[1] ? entry[1] : 32;
			entry += 2;
			// ColorCount is skipped
			entry++;
			// Reserved = 0
			entry++;
			best.hotspotX = readLE16(entry);
			best.hotspotY = readLE16(entry + 2);
			best.payloadSize = readLE32(entry + 4);
			// FileOffset is ignored
			uint32_t imageDataOffset = readLE32(entry + 8);
			best.payload = chunkData + imageDataOffset;

			// usually there's only one cursor in idCount
			break;
		}

		CursorImage cursorImage;
		if (!decodeCursorBitmap(best.payload, best.payloadSize, best.hotspotX, best.hotspotY, cursorImage))
		{
			return SDL_SetError("Failed to decode cursor bitmap: %s", SDL_GetError());
		}

		// Scale cursor 2 times for better size
		SDL_Surface *scaledCursorSurface = SDL_ScaleSurface(cursorImage.surface, cursorImage.surface->w << 1, cursorImage.surface->h << 1, SDL_SCALEMODE_NEAREST);
		if (scaledCursorSurface != NULL)
		{
			SDL_DestroySurface(cursorImage.surface);
			cursorImage.surface = scaledCursorSurface;
		}

		SDL_Cursor *cursor = SDL_CreateColorCursor(cursorImage.surface, cursorImage.hotspotX, cursorImage.hotspotY);
		SDL_DestroySurface(cursorImage.surface);

		if (!cursor)
		{
			return SDL_SetError("SDL_Cursor is NULL");
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

bool SDL3AniReader::load(const Byte *path, std::vector<Frame> &outFrames)
{
	outFrames.clear();

	size_t fileSize = 0;
	void *fileMemory = SDL_LoadFile(path, &fileSize);
	if (!fileMemory)
	{
		SDL_SetError("File '%s' doesn't exist", path);
		return FALSE;
	}

	const Byte *bytes = static_cast<const Byte *>(fileMemory);
	bool success = FALSE;

	if (fileSize < 12 || std::memcmp(bytes, "RIFF", 4) != 0 || std::memcmp(bytes + 8, "ACON", 4) != 0)
	{
		SDL_free(fileMemory);
		return SDL_SetError("Invalid RIFF signature");
	}

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

	bool parseOk = TRUE;
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

	SDL_free(fileMemory);
	return success;
}
