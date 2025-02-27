/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

/////////////////////////////////////////////////////////////////////////EA-V1
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/internal_io.h $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Internal header: I/O classes
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>

/// \internal \brief con debug I/O class
class DebugIOCon: public DebugIOInterface
{
  /**
    true if we allocated the console, false if there has already
    been a console
  */
  bool m_allocatedConsole;

  /**
    internal input buffer
  */
  char m_input[256];

  /**
    number of bytes in input buffer
  */
  unsigned m_inputUsed;

  /**
    Current read position. This variable is 0 while data is
    composed into m_input and >0 while multiple Read calls are
    received.
  */
  unsigned m_inputRead;

public:
  explicit DebugIOCon(void);
  virtual ~DebugIOCon();
  virtual int Read(char *buf, int maxchar);
  virtual void Write(StringType type, const char *src, const char *str);
  virtual void EmergencyFlush(void) {}
  virtual void Execute(class Debug& dbg, const char *cmd, bool structuredCmd,
                       unsigned argn, const char * const * argv);
  static DebugIOInterface *Create(void);
  virtual void Delete(void);
};

/// \internal \brief con flat I/O class
class DebugIOFlat: public DebugIOInterface
{
  /// \brief single output stream
  class OutputStream
  {
    OutputStream(const OutputStream&);
    OutputStream& operator=(const OutputStream&);

    /// output file name (dynamically allocated)
    char *m_fileName;

    /// output file handle (only if unlimited output buffer size)
    HANDLE m_fileHandle;

    /// file size limited?
    bool m_limitedFileSize;

    /// output buffer
    char *m_buffer;

    /// number of bytes in buffer
    unsigned m_bufferUsed;

    /// size of buffer (if m_limitedFileSize then m_bufferSize==m_maxSize)
    unsigned m_bufferSize;

    /// position of next free char in ring buffer (used if m_limitedFileSize)
    unsigned m_nextChar;

    // these are private so that we are forced to use Create/Destroy
    OutputStream(const char *filename, unsigned maxSize);
    ~OutputStream();

    /**
      \brief Writes some data to output stream.

      \param src string to write
      \param len number of bytes to write
    */
    void InternalWrite(const char *src, unsigned len);

  public:

    /**
      \brief Creates a new output stream instance.

      \param filename name of output file (will be overwritten if it exists)
      \param maxSize maximum size of output file, unlimited if 0
    */
    static OutputStream *Create(const char *filename, unsigned maxSize);

    /**
      \brief Destroys this object.

      If a path is given the file is copied to that path as well. If there
      is already a file with that name in the destination directory the
      current file is copied under a new unique name.

      \param path optional path to a destination directory
    */
    void Delete(const char *path=NULL);

    /**
      \brief Determines name of output stream.

      \return name of output stream
    */
    const char *GetFilename(void) { return m_fileName; }

    /**
      \brief Writes data to the output stream.

      \param src NUL delimited string to write
    */
    void Write(const char *src);

    /**
      \brief Flushes all buffered data.
    */
    void Flush(void);
  };

  /// \brief a single split structure
  struct SplitListEntry
  {
    /// next split
    SplitListEntry *next;

    /// for which string types?
    unsigned stringTypes;

    /// for which items?
    char items[256];

    /// split name
    char name[256];

    /// into which stream? (note: pointer not owned by this!)
    OutputStream *stream;
  };

  /// \brief List of output streams
  struct StreamListEntry
  {
    /// next entry
    StreamListEntry *next;

    /// which stream?
    OutputStream *stream;
  };

  /// first split
  SplitListEntry *m_firstSplit;

  /// end of split list pointer
  SplitListEntry **m_lastSplitPtr;

  /// first output stream (first is always the default output stream)
  StreamListEntry *m_firstStream;

  /// end of stream list pointer
  StreamListEntry **m_lastStreamPtr;

  /// base filename
  char m_baseFilename[256];

  /// copy location
  char m_copyDir[256];

  /**
    \brief Expands a magic filename into a real filename.

    \param src magic filename or real filename
    \param splitName split name, NULL for default stream
    \param buf output buffer, must have a size of at least 256 char's
  */
  static void ExpandMagic(const char *src, const char *splitName, char *buf);

public:
  explicit DebugIOFlat(void);
  virtual ~DebugIOFlat();
  virtual int Read(char *buf, int maxchar) { return 0; }
  virtual void Write(StringType type, const char *src, const char *str);
  virtual void EmergencyFlush(void);
  virtual void Execute(class Debug& dbg, const char *cmd, bool structuredCmd,
                       unsigned argn, const char * const * argv);
  static DebugIOInterface *Create(void);
  virtual void Delete(void);
};

/// \internal \brief net debug I/O class
class DebugIONet: public DebugIOInterface
{
  /// our pipe handle
  HANDLE m_pipe;

public:
  explicit DebugIONet(void);
  virtual ~DebugIONet();
  virtual int Read(char *buf, int maxchar);
  virtual void Write(StringType type, const char *src, const char *str);
  virtual void EmergencyFlush(void);
  virtual void Execute(class Debug& dbg, const char *cmd, bool structuredCmd,
                       unsigned argn, const char * const * argv);
  static DebugIOInterface *Create(void);
  virtual void Delete(void);
};

/// \internal \brief ods debug I/O class
class DebugIOOds: public DebugIOInterface
{
public:
  explicit DebugIOOds(void) {}
  virtual int Read(char *buf, int maxchar) { return 0; }
  virtual void Write(StringType type, const char *src, const char *str);
  virtual void EmergencyFlush(void) {}
  virtual void Execute(class Debug& dbg, const char *cmd, bool structuredCmd,
                       unsigned argn, const char * const * argv) {}
  static DebugIOInterface *Create(void);
  virtual void Delete(void);
};
