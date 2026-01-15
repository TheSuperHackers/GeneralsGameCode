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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_macro.h $
// $Author: mhoffe $
// $Revision: #2 $
// $DateTime: 2003/07/09 10:57:23 $
//
// (c) 2003 Electronic Arts
//
// Debugging macros
//////////////////////////////////////////////////////////////////////////////

#pragma once

// I'm putting the documentation for the following macros
// here because Doxygen otherwise includes the values of each macro
// in the documentation. I don't want that (it would expose some
// internals) so I'm putting all comments for the following macros
// here...
#ifdef DOXYGEN // defined if Doxygen is running

/**
  \addtogroup debug_macros Debugging and logging macros

  This module defines a number of macros. Usually only these macros should be
  used to access any debug functionality.

  All these macros are defined if RTS_DEBUG is defined. Otherwise
  all of them (with the exception of DCHECK and DCHECK_MSG) will be removed.
*/
///@{

  /**
    \brief Regular assert macro.

    This macro will generate an error message on screen if the evaluated expression
    returns false. The user will then have the choice of aborting the program,
    continuing once or continuing with completely ignoring that specific assertion.

    Assertions are completely removed if RTS_DEBUG is not defined.

    \param expr expression, trigger assert window if false
  */
  #define DASSERT(expr)

  /**
    \brief Assert macro with custom message.

    Like \ref DASSERT but a custom message will be given in addition to the expression.
    The message can be specified in a stream-like notation, e.g.
    \code
      DASSERT_MSG( n>=0 && n<MAX_N, "n:" << n << ", max " << MAX_N);
    \endcode
    For more information see the \ref debug_stream page.

    Assertions are completely removed if RTS_DEBUG is not defined.

    \param expr expression, trigger assert window if false
    \param msg custom message stream, see \ref debug_stream

  */
  #define DASSERT_MSG(expr,msg)

  /**
    \brief Compile time assertion.

    This assertion will fail during compile time. It can be used anywhere and will
    (of course) remain in release compiles as well. No code is generated in any case.

    If a compile time assertion fails the compiler will give an error message like
    this:
    \code
debug.cpp(14) : error C2027: use of undefined type 'StaticAssertionFailed<0>'
    \endcode

    If the expression given can't be evaluated at compile time the error message
    looks like this:
    \code
debug.cpp(15) : error C2027: use of undefined type 'StaticAssertionFailed< ?? >'
    \endcode

    \param expr expression which can be evaluated at compile time
  */
  #define DCTASSERT(expr)

  /**
    \brief Function macro, checks if the given expression is true, returns false if not.

    Additionally a message is written to the log file. This macro is usually used
    like this:
    \code
      if ( !DCHECK( n>=0 && n<MAX_N ) ) return;
    \endcode

    \note Checks will remain in place even in release builds!

    \param expr expression which will always be checked
  */
  #define DCHECK(expr)

  /**
    \brief Function macro, \ref DCHECK with additional message on failure.

    For more information see the \ref debug_stream page.

    \note Even though the expression check will remain in place in release builds the
          \a msg part is completely removed.

    \param expr expression which will always be checked
    \param msg custom message stream, see \ref debug_stream
  */
  #define DCHECK_MSG(expr,msg)

  /**
    \brief Macro for precondition-checks.

    Internally expands to
    \code
if (!DCHECK(!(cond)))
    \endcode
    so it can be used e.g. like this:
    \code
DFAIL_IF(!ptrval) return;
    \endcode

    \param cond condition which is checked for failure
  */
  #define DFAIL_IF(cond)

  /**
    \brief Convenience macro, \ref DFAIL_IF with additional message on failure.

    Internally expands to
    \code
if (!DCHECK_MSG(!(cond),msg))
    \endcode
    so it can be used e.g. like this:
    \code
DFAIL_IF_MSG(!ptrval,"pointer must not be null") return;
    \endcode

    \param cond condition which is checked for failure
    \param msg custom message stream, see \ref debug_stream
  */
  #define DFAIL_IF_MSG(cond,msg)

  /**
    \brief Writes a message to the log file.

    It also generates a logging group with the name of the current file (but without
    path or extension), e.g. if DLOG("hello world") is written in here a logging group
    called 'debug' would be created.

    Logs can be turned on and off based on individual logging groups.

    \param what message, see \ref debug_stream
  */
  #define DLOG(what)

  /**
    \brief Adds a description for the current file if used for logging.

    \note This macro must be used in a CPP file in file scope only.
    \param descr a short static description of what gets logged in the current file
  */
  #define DLOG_DESCR(descr)

  /**
    \brief Writes a message to the log file using a custom log group.

    Works just like \ref DLOG but instead of using the current file as a logging group
    the logging group is explicitly specified.

    \note Specifiy the group directly without quotes, e.g.
      \code
        DLOG_GROUP(my_log_group,"hello world")
      \endcode

    \param group logging group, without quotes
    \param what message, see \ref debug_stream
  */
  #define DLOG_GROUP(group,what)

  /**
    \brief Adds a description for a custom log group

    \note This macro must be used in a CPP file in file scope only.
    \param group logging group, without quotes
    \param descr a short static description of what gets logged in the specified log group
  */
  #define DLOG_GROUP_DESCR(group,descr)

  /**
    \brief Unconditionally aborts the program in debug/internal builds.

    A DCRASH can not be disabled by the user.

    \param msg custom message stream, see \ref debug_stream
  */
  #define DCRASH(msg)

  /**
    \brief Unconditional program exit that is active in release builds as well.

    \note In a release build the crash message will not include the file name or
          line number where the DCRASH_RELEASE was specified.

    \param msg custom message stream, see \ref debug_stream
  */
  #define DCRASH_RELEASE(msg)

  /**
    \brief Unconditional assert.

    Basically makes sense only as default clause in switch statements to make
    sure that no case is missed, e.g.
    \code
      switch(n)
      {
        case MY_DEF_VAL_A:
          // ...
          break;
        case MY_DEF_VAL_B:
          // ...
          break;
        default:
          DFAIL();
      }
    \endcode
    Gets removed in release builds.
  */
  #define DFAIL()

  /**
    \brief Function macro, determines if logging is active for current file or not.

    This function macro can be safely used without any #ifdefs:
    \code
      if (D_ISLOG())
      {
        // do some computationally expensive logging here...
      }
    \endcode

    In release builds this macro expands to \a false effectively removing
    the whole section.

    \return true if logging is active, false if not
  */
  #define D_ISLOG()

  /**
    \brief Function macro, determines if logging is active for the specified log group or not.

    Works just like \ref D_ISLOG but instead of using the current file as a logging group
    the logging group is explicitly specified.

    \param group logging group, without quotes
    \return true if logging is active, false if not
  */
  #define D_ISLOG_GROUP(group)

///@}

#elif defined(RTS_DEBUG)

  #define DASSERT(expr) \
    ((void)( Debug::SkipNext() || \
             (expr) || \
             Debug::AssertBegin(__FILE__,__LINE__,#expr).AssertDone() ))

  #define DASSERT_MSG(expr,msg) \
    ((void)( Debug::SkipNext() || \
             (expr) || \
             ( Debug::AssertBegin(__FILE__,__LINE__,#expr) << ": " << msg ).AssertDone() ))

  #define DCHECK(expr) \
    ( (expr) || \
      Debug::SkipNext() || \
      Debug::CheckBegin(__FILE__,__LINE__,#expr).CheckDone() )

  #define DCHECK_MSG(expr,msg) \
    ( (expr) || \
      Debug::SkipNext() || \
      ( Debug::CheckBegin(__FILE__,__LINE__,#expr) << ": " << msg ).CheckDone() )

  #define DFAIL_IF(cond) \
    if (!DCHECK(!(cond)))

  #define DFAIL_IF_MSG(cond,msg) \
    if (!DCHECK_MSG(!(cond),msg))

  #define DLOG(what) \
    ((void)( Debug::SkipNext() || \
             ( Debug::LogBegin(__FILE__) << what ).LogDone() ))

  #define DLOG_DESCR(descr) \
    static Debug::LogDescription _LogDescr_FILE(__FILE__,descr);

  #define DLOG_GROUP(group,what) \
    ((void)( Debug::SkipNext() || \
             ( Debug::LogBegin(#group) << what ).LogDone() ))

  #define DLOG_GROUP_DESCR(group,descr) \
    static Debug::LogDescription(#group,descr) _LogDescr_Grp_##group

  #define DCRASH(msg) \
    (Debug::SkipNext() || (Debug::CrashBegin(__FILE__,__LINE__) << msg).CrashDone(false))

  #define DCRASH_RELEASE(msg) \
    (Debug::SkipNext(),(Debug::CrashBegin(__FILE__,__LINE__) << msg).CrashDone(true))

  #define DFAIL() \
    Debug::AssertBegin(__FILE__,__LINE__,nullptr).AssertDone()

  #define D_ISLOG() \
    Debug::IsLogEnabled(__FILE__)

  #define D_ISLOG_GROUP(group) \
    Debug::IsLogEnabled(#group)

#else

  #define DASSERT(expr)           ((void)0)
  #define DASSERT_MSG(expr,msg)   ((void)0)
  #define DCHECK(expr)            (expr)
  #define DCHECK_MSG(expr,msg)    (expr)
  #define DFAIL_IF(cond)          if (cond)
  #define DFAIL_IF_MSG(cond,msg)  if (cond)
  #define DLOG(what)              ((void)0)
  #define DLOG_DESCR(descr)
  #define DLOG_GROUP(group,what)  ((void)0)
  #define DLOG_GROUP_DESCR(g,d)
  #define DCRASH(msg)             ((void)0)
  #define DCRASH_RELEASE(msg)     (Debug::SkipNext(),(Debug::CrashBegin(nullptr,0) << msg).CrashDone(true))
  #define DFAIL()                 ((void)0)
  #define D_ISLOG()               false
  #define D_ISLOG_GROUP(group)    false

#endif

// put these helper templates in a namespace...
namespace _Debug
{
  template<bool> struct StaticAssertionFailed;
  template<> struct StaticAssertionFailed<true> {};
  template<int x> struct StaticAssertionTest {};

  #define DCTASSERT(expr)         typedef ::_Debug::StaticAssertionTest< \
                                    sizeof(::_Debug::StaticAssertionFailed< (bool)(expr) >) \
                                    > DebugStaticAssertTypedef__;
};

// These are stolen from the WW3D Debug file. REALLY useful. :-)
#define STRING_IT(a) #a
#define TOKEN_IT(a) STRING_IT(,##a)

/**
  The macro MESSAGE allows user to put:
  \code
#pragma MESSAGE("Hello world")
  \endcode
  anywhere in a source file.  The message:
  \code
sourcefname.cpp (123) : Hello world
\endcode
  would be printed if put in sourcefname.cpp on line 123 in compile window like an error.
  You can then use next/prev error	hot keys to see where comment is.  It is not an error and
  will be printed everytime it is compiled.  Very useful to put comments in code that cannot
  be forgotten.
*/
#define MESSAGE(a) message (__FILE__ "(" TOKEN_IT(__LINE__) ") : " a)
