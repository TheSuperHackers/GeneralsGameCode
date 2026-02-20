/*
**  WideCharUtils.h
**
**  Utility functions for handling WideChar (wchar_t) data in binary file I/O.
**
**  Problem:
**    On Windows, wchar_t is 2 bytes (UTF-16LE native).
**    On macOS/Linux, wchar_t is 4 bytes (UTF-32).
**    Binary game data files (CSF, DataChunk, etc.) always store text as UTF-16LE.
**    Direct read/write with sizeof(WideChar) breaks on non-Windows platforms.
**
**  Solution:
**    These template utilities transparently handle the conversion for any 
**    stream class with a read(void*, int)/write(void*, int) interface
**    (File, ChunkInputStream, InputStream, etc.)
**
**  On Windows (wchar_t == 2), these compile down to direct read/write calls.
*/

#ifndef WIDECHAR_UTILS_H
#define WIDECHAR_UTILS_H

#include <cstdint>
#include <cstdio>

// The on-disk size of a "wide character" in all game binary formats (UTF-16LE)
enum { UTF16_CHAR_SIZE = 2 };

#if WCHAR_MAX > 0xFFFF
// ============================================================================
// Non-Windows path: wchar_t is 4 bytes, need conversion from/to UTF-16LE
// ============================================================================

/**
 * Read UTF-16LE characters from any stream with a read(void*,int) method.
 * Expands 2-byte UTF-16 data to 4-byte wchar_t.
 *
 * @param stream    Any object with read(void*, int) → int (File*, ChunkInputStream*, etc.)
 * @param dest      Destination WideChar buffer (must fit charCount elements)
 * @param charCount Number of UTF-16 characters to read
 * @return Number of bytes actually read from the stream (charCount * 2 on success)
 */
template<typename StreamT>
inline int readUTF16FromFile(StreamT* stream, WideChar* dest, int charCount)
{
    if (charCount <= 0) { return 0; }

    // Use stack for small reads, heap for large
    static const int STACK_LIMIT = 4096;
    uint16_t stackBuf[STACK_LIMIT];
    uint16_t* buf = (charCount <= STACK_LIMIT) ? stackBuf : new uint16_t[charCount];

    int bytesRead = stream->read(buf, charCount * UTF16_CHAR_SIZE);

    int charsRead = bytesRead / UTF16_CHAR_SIZE;
    for (int i = 0; i < charsRead; i++)
    {
        dest[i] = (WideChar)buf[i];
    }

    if (buf != stackBuf) { delete[] buf; }
    return bytesRead;
}

/**
 * Read a single UTF-16LE character from any stream.
 */
template<typename StreamT>
inline int readUTF16CharFromFile(StreamT* stream, WideChar* dest)
{
    uint16_t ch;
    int bytesRead = stream->read(&ch, UTF16_CHAR_SIZE);
    if (bytesRead == UTF16_CHAR_SIZE)
    {
        *dest = (WideChar)ch;
    }
    return bytesRead;
}

/**
 * Write WideChar buffer to a C stdio FILE* as UTF-16LE.
 * Compresses 4-byte wchar_t to 2-byte UTF-16.
 */
inline size_t writeUTF16ToStdioFile(FILE* fp, const WideChar* src, int charCount)
{
    if (charCount <= 0) { return 0; }

    static const int STACK_LIMIT = 4096;
    uint16_t stackBuf[STACK_LIMIT];
    uint16_t* buf = (charCount <= STACK_LIMIT) ? stackBuf : new uint16_t[charCount];

    for (int i = 0; i < charCount; i++)
    {
        buf[i] = (uint16_t)src[i];
    }

    size_t result = ::fwrite(buf, UTF16_CHAR_SIZE, charCount, fp);

    if (buf != stackBuf) { delete[] buf; }
    return result;
}

/**
 * Write WideChar buffer to any stream with a write(void*,int) method as UTF-16LE.
 */
template<typename StreamT>
inline int writeUTF16ToFile(StreamT* stream, const WideChar* src, int charCount)
{
    if (charCount <= 0) { return 0; }

    static const int STACK_LIMIT = 4096;
    uint16_t stackBuf[STACK_LIMIT];
    uint16_t* buf = (charCount <= STACK_LIMIT) ? stackBuf : new uint16_t[charCount];

    for (int i = 0; i < charCount; i++)
    {
        buf[i] = (uint16_t)src[i];
    }

    int result = stream->write(buf, charCount * UTF16_CHAR_SIZE);

    if (buf != stackBuf) { delete[] buf; }
    return result;
}

#else
// ============================================================================
// Windows path: wchar_t is 2 bytes = UTF-16LE native, no conversion needed
// ============================================================================

template<typename StreamT>
inline int readUTF16FromFile(StreamT* stream, WideChar* dest, int charCount)
{
    if (charCount <= 0) { return 0; }
    return stream->read(dest, charCount * sizeof(WideChar));
}

template<typename StreamT>
inline int readUTF16CharFromFile(StreamT* stream, WideChar* dest)
{
    return stream->read(dest, sizeof(WideChar));
}

inline size_t writeUTF16ToStdioFile(FILE* fp, const WideChar* src, int charCount)
{
    if (charCount <= 0) { return 0; }
    return ::fwrite(src, sizeof(WideChar), charCount, fp);
}

template<typename StreamT>
inline int writeUTF16ToFile(StreamT* stream, const WideChar* src, int charCount)
{
    if (charCount <= 0) { return 0; }
    return stream->write(src, charCount * sizeof(WideChar));
}

#endif // WCHAR_MAX > 0xFFFF

#endif // WIDECHAR_UTILS_H
