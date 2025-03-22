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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_io.h $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Debug I/O interface
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#  pragma once
#endif
#ifndef DEBUG_IO_H // Include guard
#define DEBUG_IO_H

/**
  \interface DebugIOInterface debug.h <rts/debug.h>
  
  \brief Debug I/O interface.

  A Debug I/O interface implementation must register itself
  using Debug::AddIOFactory. Typically this is done
  by using the \ref DEBUG_DECLARE_IO_INTERFACE and
  \ref DEBUG_IMPLEMENT_IO_INTERFACE macros (recommended, but
  not mandatory.)
*/
class DebugIOInterface
{
  // no copy/assign op
  DebugIOInterface(const &DebugIOInterface);
  DebugIOInterface& operator=(const DebugIOInterface&);

protected:
  /**
    \brief I/O class destructor.

    The destructor must always be protected. Destruction is
    done by calling the Delete member function.
  */
  virtual ~DebugIOInterface() {}

public:
  // interface only so no functionality here
  explicit DebugIOInterface(void) {}

  /// List of possible log string types
  enum StringType
  {
    /// DASSERT etc
    Assert = 0,

    /// DCHECK etc
    Check,

    /// DLOG etc
    Log,

    /// DCRASH etc
    Crash,

    /// Exception
    Exception,

    /// Regular command reply
    CmdReply,

    /// Structured command reply, see \ref debug_structcmd
    StructuredCmdReply,

    /// some other message
    Other,

    MAX
  };

  /**
    \brief Retrieves up to the given number of characters from a command input source.

    This source can be e.g. keyboard or a network pipe. This function must
    not block.

    \param buf buffer to place read characters in
    \param maxchar maximum number of characters to return
    \return numbers of characters written to buffer
    \note There is no terminating NUL char written to the buffer
  */
  virtual int Read(char *buf, int maxchar)=0;

  /**
    \brief Write out some characters differentiated by the log string type.

    \param type possible string type
    \param src string source, may be NULL, content depends on type:
                <table><tr>
                  <td><b>type</b></td><td><b>src</b></td></tr><tr>
                  <td>Assert</td><td>file(line)</td></tr><tr>
                  <td>Check</td><td>file(line)</td></tr><tr>
                  <td>Log</td><td>log group</td></tr><tr>
                  <td>Crash</td><td>file(line)</td></tr><tr>
                  <td>Exception</td><td>NULL</td></tr><tr>
                  <td>CmdReply</td><td>group.command</td></tr><tr>
                  <td>StructuredCmdReply</td><td>group.command</td></tr><tr>
                  <td>Other</td><td>NULL</td>
                </tr></table>
    \param str string to output, NUL delimited, if NULL then simply flush
               output (if applicable)
  */
  virtual void Write(StringType type, const char *src, const char *str)=0;

  /**
    \brief Emergency shutdown function.

    This function gets called during an exception and should perform the
    absolute bare minimum (e.g. just flushing and closing the output file).
  */
  virtual void EmergencyFlush(void)=0;

  /**
    \brief I/O class specific command.

    All io \<class\> commands are passed into this function, with the
    exception of remove which results in simply calling the class
    destructor.

    \param dbg debug instance
    \param cmd command issued
    \param structuredCmd true if structured command reply, false if not
    \param argn number of additional arguments passed in
    \param argv argument list
  */
  virtual void Execute(class Debug& dbg, const char *cmd, bool structuredCmd,
                       unsigned argn, const char * const * argv)=0;

  /**
    \brief Destroys the current I/O class instance.

    Use this function instead of just delete'ing the instance.
  */
  virtual void Delete(void)=0;
};

/**
  \addtogroup debug_io_macros Debug I/O interface helper macros
*/
///@{

#ifdef DOXYGEN

  /**
    \brief Helper macro used in I/O class declaration to declare
    a factory function.

    \param type type name of class we're implementing
    \note This macro changes the access method to private.
  */
  #define DEBUG_DECLARE_IO_INTERFACE(type) 

  /**
    \brief Helper macro for registering I/O class factory with
    the Debug module.

    \param io_id name of I/O class as it should be registered with Debug module
                 (without quotes)
    \param descr short I/O class description
    \param type type name of class we're implementing
  */
  #define DEBUG_IMPLEMENT_IO_INTERFACE(io_id,descr,type) 

#else

  #define DEBUG_DECLARE_IO_INTERFACE(type) \
    public: \
      static bool __RegisterClassFactory; \
      static DebugIOInterface *__ClassFactory(void) { return new type; }

  #define DEBUG_IMPLEMENT_IO_INTERFACE(io_id,descr,type) \
    static bool type::__RegisterClassFactory=Debug::AddIOFactory(#io_id,descr,type::__ClassFactory);

#endif

///@}

#endif // DEBUG_IO_H
