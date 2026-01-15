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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_debug.h $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// main Debug object (singleton)
//////////////////////////////////////////////////////////////////////////////

#pragma once

/**
  \class Debug debug.h <rts/debug.h>

  \brief Debug module main class (singleton).
*/
class Debug
{
  // necessary because all debug commands operate directly on this class
  friend class DebugCmdInterfaceDebug;

  // necessary because exception handler needs direct access
  friend class DebugExceptionhandler;

public:
  enum
  {
    /// maximum number of times a check can be hit before it is turned off
    MAX_CHECK_HITS  =   20
  };

  /**
    \brief HRESULT translator callback function type.

    See \ref Debug::AddHResultTranslator for more information.

    \param debug Debug instance where info can be written to
    \param hresult current HRESULT value
    \param user user value passed into AddHResultTranslator
    \return true if value translated and no more translators should be called,
            false if next translator should be tried
  */
  typedef bool (*HResultTranslator)(Debug &debug, long hresult, void *user);

  /**
    \class MemDump debug.h <rts/debug.h>

    \brief Helper class for performing a raw memory dump.

    Can be used directly in the output stream, e.g.
    \code
DLOG( "This is 16 bytes of memory:\n" << Debug::MemDump::Raw(&somePointer,16) );
    \endcode
  */
  class MemDump
  {
    // necessary because Debug needs access to the following private members
    friend Debug;

    const unsigned char *m_startPtr;  ///< start dumping with this address
    unsigned m_numItems;              ///< dump the given number of items
    unsigned m_bytePerItem;           ///< determines the number of bytes per item (1. 2 or 4)
    bool m_absAddr;                   ///< show absolute addresses (true) or relative addresses (false)
    bool m_withChars;                 ///< show printable characters on right side of dump (true) or not (false)

    // constructor is private on purpose so that nobody can
    // create instances of this class except the static functions
    // provided herein
    MemDump(const void *ptr, unsigned num, unsigned bpi, bool absAddr, bool withChars):
      m_startPtr((const unsigned char *)ptr), m_numItems(num),
      m_bytePerItem(bpi), m_absAddr(absAddr), m_withChars(withChars) {}
  public:

    /**
      Creates a memory dump descriptor.

      \param startPtr address to start dump from
      \param numItems number of items (usually bytes) to dump
      \param bytePerItem number of bytes per item (usually 1)
    */
    static MemDump Raw(const void *startPtr, unsigned numItems, unsigned bytePerItem=1)
    {
      return MemDump(startPtr,numItems,bytePerItem,true,false);
    }

    /**
      Creates a memory dump descriptor with relative addresses.

      \param startPtr address to start dump from
      \param numItems number of items (usually bytes) to dump
      \param bytePerItem number of bytes per item (usually 1)
    */
    static MemDump RawRel(const void *startPtr, unsigned numItems, unsigned bytePerItem=1)
    {
      return MemDump(startPtr,numItems,bytePerItem,false,false);
    }

    /**
      Creates a memory dump descriptor that dumps out ASCII chars as well.

      \param startPtr address to start dump from
      \param numItems number of items (usually bytes) to dump
      \param bytePerItem number of bytes per item (usually 1)
    */
    static MemDump Char(const void *startPtr, unsigned numItems, unsigned bytePerItem=1)
    {
      return MemDump(startPtr,numItems,bytePerItem,true,true);
    }

    /**
      Creates a memory dump descriptor with relative addresses that dumps out ASCII chars as well.

      \param startPtr address to start dump from
      \param numItems number of items (usually bytes) to dump
      \param bytePerItem number of bytes per item (usually 1)
    */
    static MemDump CharRel(const void *startPtr, unsigned numItems, unsigned bytePerItem=1)
    {
      return MemDump(startPtr,numItems,bytePerItem,false,true);
    }
  };

  /**
    \class HResult debug.h <rts/debug.h>

    \brief Helper class for writing HRESULTs to the debug stream.

    Can be used directly in the output stream, e.g.
    \code
DLOG( "My HResult is: " << Debug::HResult(SomeHRESULTValue) << "\n" );
    \endcode

    The most interesting thing about HRESULTs is that custom HRESULT translators
    can be added or removed during runtime, see \ref Debug::AddHResultTranslator for more
    information.
  */
  class HResult
  {
    // necessary because Debug needs access to the following private members
    friend Debug;

    long m_hresult;                   ///< HRESULT value

  public:

    /**
      Creates a HRESULT descriptor.

      \param hresult HRESULT value (checked, Windows declares HRESULT as typedef long)
    */
    explicit HResult(long hresult): m_hresult(hresult) {}
  };

  /** \internal

    \brief Helper class for adding log group descriptions.

  */
  class LogDescription
  {
    // sorry, no copies or assignments
    LogDescription(const LogDescription&);
    LogDescription& operator=(const LogDescription&);

  public:

    /** \internal

      Adds a description for a logging file/group.

      \param fileOrGroup filename or logging group (see \ref Debug::LogBegin)
      \param description descriptive text for logging file/group
    */
    LogDescription(const char *fileOrGroup, const char *description);
  };

  /**
    \brief Switches integer output to hexadecimal format.
  */
  class Hex {};

  /// \internal Performs actual switch to hexadecimal format.
  Debug& operator<<(const Hex)
  {
    SetPrefixAndRadix("0x",16);
    return *this;
  }

  /**
    \brief Switches integer output to decimal format.
  */
  class Dec {};

  /// \internal Performs actuals switch to decimal format
  Debug& operator<<(const Dec)
  {
    SetPrefixAndRadix("",10);
    return *this;
  }

  /**
    \brief Switches integer output to binary format.
  */
  class Bin {};

  /// \internal Performs actuals switch to binary format
  Debug& operator<<(const Bin)
  {
    SetPrefixAndRadix("%",2);
    return *this;
  }

  /**
    \brief Sets output width for the next insertion.
  */
  class Width
  {
    // necessary because Debug needs access to the following private members
    friend Debug;

    int m_width;  ///< output width

  public:
    /// \brief Sets new output width (next insertion only).
    explicit Width(int width): m_width(width) {}
  };

  /// \internal Performs actuals width switch
  Debug& operator<<(const Width w)
  {
    m_width=w.m_width;
    return *this;
  }

  /**
    \brief Sets new fill character.
  */
  class FillChar
  {
    // necessary because Debug needs access to the following private members
    friend Debug;

    char m_fill;  ///< fill character

  public:
    /// \brief Sets new fill character.
    explicit FillChar(char ch=' '): m_fill(ch) {}
  };

  /// \internal Performs actuals setting of fill char
  Debug& operator<<(const FillChar c)
  {
    m_fillChar=c.m_fill;
    return *this;
  }

  /**
    \brief Repeats a given character N times.
  */
  class RepeatChar
  {
    // necessary because Debug needs access to the following private members
    friend Debug;

    char m_char;  ///< character
    int m_count;  ///< repeat count

  public:
    /// \brief Repeats a given character N times
    explicit RepeatChar(char ch, int count): m_char(ch), m_count(count) {}
  };

  /// \internal Performs actuals repeating of char
  Debug& operator<<(RepeatChar c);

  /**
    \brief Old printf style formatting.

    \note Do not use this helper class for new code. It is mainly here
    to get the old code base adapted to the new debug module more quickly.
  */
  class Format
  {
    // necessary because Debug needs access to the following private members
    friend Debug;

    // no CC, AOp
    Format(const Format &);
    Format& operator=(const Format&);

    char m_buffer[512]; ///< this contains the string to write \note Fixed size buffer!

  public:
    /// \brief Old printf style formatting.
    explicit Format(const char *format, ...);
  };

  /// \internal Writes printf style formatted string to debug log.
  Debug& operator<<(const Format &f)
  {
    operator<<(f.m_buffer);
    return *this;
  }

  // this is necessary because LogDescription needs to call AddLogGroup
  friend class LogDescription;

  /** \internal

    \brief Performs logical cleanup.
  */
  ~Debug();

  /**
    \brief Installs exception handler for current thread.

    For the main thread this is already done, but for any additional
    threads being created this function must be called.
  */
  static void InstallExceptionHandler(void);

  /** \internal

    \brief Helper function for skipping over disabled asserts and logs.

    This function simply records the address it has been called from. If
    an assert or other function is disabled this function returns true,
    false otherwise.

    @todo_opt Change so that instead of returning true the call to this
              function is directly removed from the calling code

    \return true if next assert/log should be skipped, false otherwise
  */
  static bool SkipNext(void);

  /** \internal

    \brief Helper function which gets called if an assertion fails.

    Starts building the assert string which will then be send to the active
    output destinations. SkipNext must be called before calling this function
    since this function also associates the most recent SkipNext call with
    the current assertion.

    \param file file that contains DASSERT or DASSERT_MSG macro
    \param line line where assert macro can be found
    \param expr expression that triggered the assertion, nullptr for 'general failure' (\ref DFAIL)
    \return reference to Debug instance
  */
  static Debug &AssertBegin(const char *file, int line, const char *expr);

  /** \internal

    \brief Displays assertion window.

    Depending on the user feedback (abort, retry, ignore) the program then
    either aborts or continues. This function is to be used as the final
    call in a debug message stream.

    \return false (always)
  */
  bool AssertDone(void);

  /** \internal

    \brief Helper function which gets called if a check fails.

    Starts building the assert string which will then be send to the active
    output destinations. SkipNext must be called before calling this function
    since this function also associates the most recent SkipNext call with
    the current check.

    \param file file that contains DCHECK or DCHECK_MSG macro
    \param line line where check macro can be found
    \param expr expression that triggered the assertion
    \return reference to Debug instance
  */
  static Debug &CheckBegin(const char *file, int line, const char *expr);

  /** \internal

    \brief Flushes current 'check' message.

    This function is to be used as the final call in a debug message stream.

    \return false (always)
  */
  bool CheckDone(void);

  /** \internal

    \brief Helper function which gets when writing data to the output log.

    Starts building the log string which will then be send to the active
    output destinations. SkipNext must be called before calling this function
    since this function also associates the most recent SkipNext call with
    the current log group.

    \param fileOrGroup current file or group the following log data is for
    \return reference to Debug instance
  */
  static Debug &LogBegin(const char *fileOrGroup);

  /** \internal

    \brief Flushes current 'log' message.

    This function is to be used as the final call in a debug message stream.

    \return false (always)
  */
  bool LogDone(void);

  /** \internal

    \brief Helper function which gets called on crash.

    Starts building the crash string which will then be send to the active
    output destinations.

    \param file file that contains DCRASH or DCRASH_RELEASE macro, if nullptr
                then no file info is given (used by DCRASH_RELEASE in release
                builds)
    \param line line where crash macro can be found, 0 if no line info should
                be given
    \return reference to Debug instance
  */
  static Debug &CrashBegin(const char *file, int line);

  /** \internal

    \brief Exits program with a 'crash' message.

    This function is to be used as the final call in a debug message stream.

    \param die true if module should exit after displaying message, false if
               the user should have the choice
    \return false (always)
  */
  bool CrashDone(bool die);

  /** \internal

    \brief Write string to output log.

    \param str string to write
    \return *this
  */
  Debug& operator<<(const char *str);

  /** \internal

    \brief Define prefix and radix for integer output.

    \param prefix prefix to use (typically "" or "0x")
    \param radix radix to use (typically 10 or 16)
  */
  void SetPrefixAndRadix(const char *prefix, int radix);

  /** \internal

    \brief Write signed integer to output log.

    \param val signed integer
    \return *this
  */
  Debug& operator<<(int val);

  /** \internal

    \brief Write unsigned integer to output log.

    \param val unsigned integer
    \return *this
  */
  Debug& operator<<(unsigned val);

  /** \internal

    \brief Write signed long to output log.

    \param val signed long
    \return *this
  */
  Debug& operator<<(long val);

  /** \internal

    \brief Write unsigned long to output log.

    \param val unsigned long
    \return *this
  */
  Debug& operator<<(unsigned long val);

  /** \internal

    \brief Write boolean value to output log.

    \param val bool
    \return *this
  */
  Debug& operator<<(bool val);

  /** \internal

    \brief Write float value to output log.

    \param val float
    \return *this
  */
  Debug& operator<<(float val);

  /** \internal

    \brief Write double precision float to output log.

    \param val double
    \return *this
  */
  Debug& operator<<(double val);

  /** \internal

    \brief Write signed short integer to output log.

    \param val signed short integer
    \return *this
  */
  Debug& operator<<(short val);

  /** \internal

    \brief Write unsigned short integer to output log.

    \param val unsigned short integer
    \return *this
  */
  Debug& operator<<(unsigned short val);

  /** \internal

    \brief Write signed 64 bit integer to output log.

    \param val signed 64 bit integer
    \return *this
  */
  Debug& operator<<(__int64 val);

  /** \internal

    \brief Write unsigned 64 bit integer to output log.

    \param val unsigned 64 bit integer
    \return *this
  */
  Debug& operator<<(unsigned __int64 val);

  /** \internal

    \brief Write pointer address to output log.

    \param ptr pointer address
    \return *this
  */
  Debug& operator<<(const void *ptr);

  /** \internal

    \brief Write memory dump to output log.

    \param dump MemDump descriptor, defines what range of memory to dump
    \return *this
  */
  Debug& operator<<(const MemDump &dump);

  /** \internal

    \brief Write HRESULT value to output log.

    \param hres HResult descriptor
    \return *this
  */
  Debug& operator<<(HResult hres);

  /** \internal
    \brief Determines if a log file/group is active or not.

    \param fileOrGroup Name of source file or group to check for. If the
                       string contains any forward or backslashes then anything
                       before them is ignored. If the then remaining string
                       contains any dots anything beyond the first dot is ignored
                       as well.
    \return true if logging is enabled, false if not
  */
  static bool IsLogEnabled(const char *fileOrGroup);

  /**
    \brief Adds a HRESULT translator.

    A HRESULT translator is called whenever a HResult descriptor is passed into
    the Debug log stream. Such a translator can translate the numeric value
    into something more meaningful.

    Translators are differentiated by both function address and user pointer. There
    is however no harm in adding the same translator/user pointer pair twice.

    \param prio priority, translators with a higher priority get called first
    \param func translator address
    \param user optional user pointer which will be passed to the given translator
    \see RemoveHResultTranslator
  */
  static void AddHResultTranslator(unsigned prio, HResultTranslator func, void *user=0);

  /**
    \brief Removes a HRESULT translator.

    If the translator/user pointer pair does not exist nothing is done.

    \param func translator address
    \param user optional user pointer
    \see AddHResultTranslator
  */
  static void RemoveHResultTranslator(HResultTranslator func, void *user=0);

  /**
    \brief Registers a new I/O class factory function.

    This is typically used internally by the DEBUG_IMPLEMENT_IO_INTERFACE
    macro. If there is already an I/O class with the same name registered
    the new one overwrites the old.

    \param io_id name of I/O class as it should be registered with Debug module
    \param descr short class description
    \param func factory function
    \return true (so function can be used in static initializers)
  */
  static bool AddIOFactory(const char *io_id, const char *descr,
                           DebugIOInterface* (*func)(void));

  /**
    \brief Adds a new command group.

    If the command group already exists the new command interface is added at
    the end of the list. A command issued for that group is first passed
    to the 'old' command interface. If that DebugCmdInterface::Execute call
    fails the command is passed on to the next interface within the same
    command group.

    The only exception is the 'help' command. Here all Execute functions are
    always called.

    Ownership of the interface pointer is passed to the Debug module i.e. the
    interface is destroyed when the Debug module shuts down (unless the
    interface is removed using RemoveCommands).

    \param cmdgroup command group this interface implements
    \param cmdif command group interface instance
    \return true (so function can be used in static initializers)
  */
  static bool AddCommands(const char *cmdgroup, DebugCmdInterface *cmdif);

  /**
    \brief Removes a command group.

    \param cmdif command group interface that will be removed
  */
  static void RemoveCommands(DebugCmdInterface *cmdif);

  /**
    \brief Issues a debug command.

    \param cmd command to execute
  */
  static void Command(const char *cmd);

  /**
    \brief Update method, must be called on a regular basis.

    Scans I/O classes for new command input and processes it.
  */
  static void Update(void);

  /** \internal

    \brief Simple recursive pattern matcher.

    \param str string to match
    \param pattern pattern, only wildcard valid is '*'
    \return true if string matches pattern, false if not
  */
  static bool SimpleMatch(const char *str, const char *pattern);

  /**
    \brief Tell debug module about build info.

    All these strings are free form and can be up to 63 chars long.

    \param version official version
    \param internalVersion internal version
    \param buildDate build date & time
  */
  static void SetBuildInfo(const char *version,
                           const char *internalVersion,
                           const char *buildDate);

  /**
    \brief Write build information into log.
  */
  void WriteBuildInfo(void);

private:
  // no assignment, no copy constructor
  Debug(const Debug&);
  Debug& operator=(const Debug&);

  /** \internal

    Undocumented default constructor. Initializes debugging library.
    We can make this private as well so nobody accidently tries to create
    a Debug instance. Actually this function does not do anything -
    initialization is rather performed by PreStaticInit() and
    PostStaticInit().
  */
  Debug(void);

  /** \internal

    This function gets called before any static C++ symbols are
    initialized. Code herein must be extremely careful because all
    global C++ instances are not initialized yet.
  */
  static void PreStaticInit(void);

  /** \internal

    This function gets called after all static C++ symbols have
    been initialized.
  */
  static void PostStaticInit(void);

  /** \internal

    This function gets called if the program exists. Use this
    function for any cleanup purposes (not the destructor, it
    might get called too early).
  */
  static void StaticExit(void);

  /** \internal

    The only debug instance. Actually not used for anything
    except as magic first parameter for overloaded stream operators.
  */
  static Debug Instance;

  /** \internal

    Helper variable for putting PreStaticInit() into the MSVC
    startup list.
  */
  static void *PreStatic;

  /** \internal

    Helper variable for putting PostStaticInit() into the MSVC
    startup list.
  */
  static void *PostStatic;

  /// \internal HResult translator vector entry
  struct HResultTranslatorEntry
  {
    /// priority
    unsigned prio;

    /// translator function
    HResultTranslator func;

    /// user pointer
    void *user;
  };

  /// \internal HResult translator vector
  HResultTranslatorEntry *hrTranslators;

  /// \internal number of HResult translators
  unsigned numHrTranslators;

  /// \internal I/O class/factory list entry
  struct IOFactoryListEntry
  {
    /// pointer to next entry in list
    IOFactoryListEntry *next;

    /// I/O ID
    const char *ioID;

    /// I/O description
    const char *descr;

    /// factory function
    DebugIOInterface* (*factory)(void);

    /// I/O interface (may be null)
    DebugIOInterface *io;

    /// input buffer
    char *input;

    /// used size of input buffer
    unsigned inputUsed;

    /// allocated size of input buffer
    unsigned inputAlloc;
  };

  /** \internal

    First I/O factory list entry. A singly linked list is
    okay for this because looking up I/O IDs is not
    time critical.
  */
  IOFactoryListEntry *firstIOFactory;

  /// \internal command interface list entry
  struct CmdInterfaceListEntry
  {
    /// pointer to next entry in list
    CmdInterfaceListEntry *next;

    /// command group
    const char *group;

    /// interface pointer
    DebugCmdInterface *cmdif;
  };

  /** \internal

    First command interface list entry. A singly linked list is
    okay for this because looking up command groups is not
    time critical.
  */
  CmdInterfaceListEntry *firstCmdGroup;

  /// \internal current stack frame (used by SkipNext)
  static unsigned curStackFrame;

  /** \internal

    Bit mask for frame entry types. Implemented as a bit
    mask since it is also used for the inclusion/exclusion
    list where a single pattern can apply to more than
    one type.
  */
  enum
  {
    /// assert
    FrameTypeAssert = 0x00000001,

    /// check
    FrameTypeCheck  = 0x00000002,

    /// log
    FrameTypeLog    = 0x00000004
  };

  /** \internal

    List of possible statuses for a frame hash entry.
  */
  enum FrameStatus
  {
    /// unknown, must check
    Unknown = 0,

    /// skip this frame
    Skip,

    /// do not skip this frame
    NoSkip
  };

  /** \internal

    \brief Hash table entry for mapping stack frame addresses to
    asserts/checks/logs.
  */
  struct FrameHashEntry
  {
    /// pointer to next entry with same hash
    FrameHashEntry *next;

    /// frame address
    unsigned frameAddr;

    /// frame type (FrameTypeAssert, FrameTypeCheck, or FrameTypeLog)
    unsigned frameType;

    /// file (or group if log)
    const char *fileOrGroup;

    /// line number (undefined for logs)
    int line;

    /// number of times this frame has been hit
    int hits;

    /// frame status
    FrameStatus status;
  };

  /// \internal initial frame hash size (prime number)
  enum { FRAME_HASH_SIZE = 10007 };

  /// \internal frame hash pointers
  FrameHashEntry *frameHash[FRAME_HASH_SIZE];

  /// \internal number of FrameHashEntry structures to allocate at one time
  enum { FRAME_HASH_ALLOC_COUNT = 100 };

  /// \internal next unused FrameHashEntry structure
  FrameHashEntry *nextUnusedFrameHash;

  /// \internal number of available FrameHashEntry structures
  unsigned numAvailableFrameHash;

  /** \internal

    Look up given frame address.

    \param addr frame address
    \return FrameHashEntry found or 0 if nothing found
  */
  __forceinline FrameHashEntry *LookupFrame(unsigned addr)
  {
    for (FrameHashEntry *e=frameHash[addr%FRAME_HASH_SIZE];e;e=e->next)
      if (e->frameAddr==addr)
        return e;
    return 0;
  }

  /** \internal

    Add frame address entry (entry must not exist yet). If
    a log frame is added then a corresponding entry is added
    to the list of known log groups as well.

    \param addr frame address
    \param type frame type
    \param fileOrGroup file (or group name)
    \param line line number
    \return the entry just added
  */
  FrameHashEntry *AddFrameEntry(unsigned addr, unsigned type,
                                const char *fileOrGroup, int line);

  /** \internal

    Updates the frames status variable depending on the current
    pattern list.

    \param entry entry that needs to be updated
  */
  void UpdateFrameStatus(FrameHashEntry &entry);

  /** \internal

    Combines LookupFrame, AddFrameEntry and UpdateFrameStatus.

    \param addr frame address
    \param type frame type
    \param fileOrGroup file (or group name)
    \param line line number
    \return the entry just added (or the already existing entry)
  */
  FrameHashEntry *GetFrameEntry(unsigned addr, unsigned type,
                                const char *fileOrGroup, int line)
  {
    FrameHashEntry *e=LookupFrame(addr);
    if (!e)
      e=AddFrameEntry(addr,type,fileOrGroup,line);
    if (e->status==Unknown)
      UpdateFrameStatus(*e);
    return e;
  }

  /** \internal

    \brief List of known logs.

    A singly linked list is
    okay for this because looking up log groups happens only
    if a new stack frame entry is added (which happens only
    once for each log command).
  */
  struct KnownLogGroupList
  {
    /// next entry
    KnownLogGroupList *next;

    /// name of log group (dynamically allocated memory)
    char *nameGroup;

    /// log description (if any)
    const char *descr;
  };

  /// \internal first log group list entry
  KnownLogGroupList *firstLogGroup;

  /** \internal

    Adds a log group (and optionally description) to the list
    of known groups. If a group with that name already exists
    nothing is done. Takes care of stripping path and extension
    of the passed in group name (if it's a file name).
    Returns translated group name.

    \param fileOrGroup file or log group
    \param descr description, may be nullptr
    \return translated log group name
  */
  const char *AddLogGroup(const char *fileOrGroup, const char *descr);

  /// \internal I/O buffers for all I/O string types
  struct
  {
    /// buffer
    char *buffer;

    /// used buffer size
    unsigned used;

    /// allocated buffer size
    unsigned alloc;

    /// has last character been CR?
    bool lastWasCR;
  } ioBuffer[DebugIOInterface::StringType::MAX];

  /// \internal current I/O string type we're writing
  DebugIOInterface::StringType curType;

  /// \internal current I/O string source (fixed size, careful!)
  char curSource[256];

  /// \internal used to enable/disable all asserts/checks/logs
  int disableAssertsEtc;

  /** \internal

    Starts new output stream with the given type and source.

    \param type string type
    \param fmt wsprintf format string
  */
  void StartOutput(DebugIOInterface::StringType type, const char *fmt, ...);

  /** \internal

    Adds the given string to the currently active output buffer.

    \param str string
    \param len string length
  */
  void AddOutput(const char *str, unsigned len);

  /** \internal

    Flushes current I/O buffer to all output handlers.

    \param defaultLog if true and no I/O class is active then data
           is written to default.log
  */
  void FlushOutput(bool defaultLog=true);

  /// \internal pointer to currently active frame
  FrameHashEntry *curFrameEntry;

  /// \internal pattern list entry
  struct PatternListEntry
  {
    /// next entry
    PatternListEntry *next;

    /// frame type(s)
    unsigned frameTypes;

    /// active (true) or inactive (false)?
    bool isActive;

    /// pattern itself (dynamic allocated memory)
    char *pattern;
  };

  /** \internal

    First pattern list list entry. A singly linked list is
    okay for this because checking patterns is a costly
    operation anyway and is therefore cached.
  */
  PatternListEntry *firstPatternEntry;

  /// \internal last pattern list entry for fast additions to list at end
  PatternListEntry *lastPatternEntry;

  /** \internal

    Adds a new pattern list entry. This function does not check if
    a matching pattern entry exists already.

    \param types frame type(s)
    \param isActive active (true) or inactive (false)?
    \param pattern pattern itself
  */
  void AddPatternEntry(unsigned types, bool isActive, const char *pattern);

  /** \internal

    Executes the given command.

    \param cmdstart start of command
    \param cmdend end of command (not including this character)
  */
  void ExecCommand(const char *cmdstart, const char *cmdend);

  /** \internal

    \brief Checks if main program is running windowed or not.

    If the decisison can not be made an windowed program is assumed.

    \return true if windowed, false if full screen
  */
  bool IsWindowed(void);

  /// \internal name of current command group
  char curCommandGroup[100];

  /// \internal if true then always flush after each line written
  bool alwaysFlush;

  /// \internal if true then put timestamps before each new line
  bool timeStamp;

  /// \internal the one and only stack walker
  DebugStackwalk m_stackWalk;

  /// \internal current integer prefix to use
  char m_prefix[16];

  /// \internal current integer radix to use
  int m_radix;

  /// \internal official version
  char m_version[64];

  /// \internal internal version
  char m_intVersion[64];

  /// \internal build date/time
  char m_buildDate[64];

  /// \internal output width
  int m_width;

  /// \internal fill char
  char m_fillChar;

  /// \internal <0 if fullscreen, >0 if windowed, ==0 if not checked yet
  char m_isWindowed;
};

/// \addtogroup debug_fn Debugging functions
///@{
/**
  \brief Determines default commands to be executed at startup.

  This function returns a list of default commands which are executed on
  startup if no .dbgcmd file is read.

  Currently this function returns:
  \code
"debug.io flat add"
  \endcode

  In order to provide a different set of commands simply put another
  version of DebugGetDefaultCommands in a CPP file. This is basically
  identical to having a .dbgcmd file in the current directory. For some
  situations (e.g. passing EXEs off to QA) it is however safer to have
  these startup commands build into the EXE itself rather than relying
  on QA having the right .dbgcmd file in place.

  As an example a new version of this function might return:
  \code
"debug.io flat add\nio flat copy q:\logfiles\n"
  \endcode

  \return list of commands, separated by \\n
  \note This function is executed after all static variables have been initialized.
*/
const char *DebugGetDefaultCommands(void);

///@} end of debug_fn group
