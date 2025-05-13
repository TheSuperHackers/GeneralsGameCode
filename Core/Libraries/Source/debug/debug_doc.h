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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_doc.h $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// additional Doxygen module documentation
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#  pragma once
#endif
#ifndef DEBUG_DOC_H // Include guard
#define DEBUG_DOC_H

// This generates a small main page for Doxygen if a module only
// documentation is built. 
#ifndef DOXYGEN_GLOBAL_DOC
/**
  \mainpage %Debug module

  The following pages contain the most useful information:
  - \ref module_debug
  - \ref debug_macros
  - \ref debug_stream
  - \ref debug_cmd

  \internal This is the internal module documentation meant only for
            programmers who are working on the module internals.
*/
#endif

/**
  \page module_debug Debug module overview
  
  \section overview Overview

  The Debug module contains all functions necessary to perform debugging related tasks,
  e.g. checking assertions or logging. 

  \section logging Logging

  Logging can be sent to a number of different locations:
  - a flat log file
  - a console window
  - a debugger via OutputDebugString
  - to a remote program (via a named pipe)
  - to a number of separated log files

  By default logging is enabled to the flat log file.

  \section cmd Commands

  The debug module is controlled via extendable commands. Commands are
  typically put into logical groups. A command can then be executed 
  in a number of ways:
  - group.command \<param\> (explicitly specifies which group to use)
  - command \<param\> (assumes that the command resides in the same group
    as the previous one)
  - group. (switches the current group)

  Every group has to respond to the command 'help'.

  See also: \ref debug_cmd

  \section dbgcmd .dbgcmd file

  On startup the debug module tries to open a file called \<exe\>.dbgcmd -
  if found then all lines in this module are directly interpreted as commands.
  If the file is not found then default.dbgcmd is tried instead.
  This allows for setting up a default environment.
*/

/**
  \page debug_cmd Debug commands

  Notation used:
  - [ abc ] means that 'abc' may or may not be specified
  - { abc } means that 'abc' may be specified any number of times (or not at all)
  - (a|b|c) means that either 'a', 'b' or 'c' must be specified once

  The following commands are already build in (group: debug):

  <table><tr>
    <td><b>Command</b></td>
    <td><b>Parameters</b></td>
    <td><b>Description</b></td>
  </tr><tr>

    <!---------------------------------->
    <td valign=top>list</td>
    <td valign=top>(g|l|d|a|c) [ \<pattern\> ]</td>
    <td>
      Shows some or all items of a specific type.

      The following items are supported:
      - g: command groups
      - l: log groups (only those encountered yet)
      - d: log groups with descriptions (only those that have descriptions)
      - a: asserts/crashes (only those hit yet)
      - c: checks (only those failed yet)

      If a pattern is specified only items matching
      that pattern are shown. A pattern can contain
      any character, letter, or a wildcard '*'.

      Please note that assert, crashes, and check items have
      their line number appended to the current file name,
      e.g. debug.cpp(13).

      Example:
      \code
> list g
debug
> list l test*
test
test_log2
> list d test*
test_log2: test log group w/ description
      \endcode
    </td>
  </tr><tr>

    <!---------------------------------->
    <td valign=top>help</td>
    <td valign=top></td>
    <td>
      Shows help regarding debug group commands.

      Example:
      \code
> help
Help info debug group:
...
      \endcode
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>io</td>
    <td valign=top>[ \<I/O Class\> \<cmd\> { \<param\> }] </td>
    <td>
      Issues a I/O class command. I/O class commands are used
      for determining where all log output should be sent. 
      Please check the list of \ref debug_ioclasses for a list
      of existing I/O classes.

      Each existing I/O class must accept at least the
      following two commands: 'add' and 'remove'. Parameters
      for these commands and additional commands are defined 
      on this page: \ref debug_ioclasses

      If the command is entered without any parameters a list
      of active I/O classes is shown. Typing 'io ?' retrieves
      a list of possible I/O classes.

      Example:
      \code
> io
Active:
  flat (flat log file)
  ods (OutputDebugString)
> io ods remove
> io ?
Possible:
  flat (flat/split log file)
  con (console window)
  ods (OutputDebugString)
  net (Network destination via named pipe)
      \endcode
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>alwaysflush</td>
    <td valign=top>[ (+|-) ]</td>
    <td>
      Enables/disables flushing after each new entry in
      the log file (default: off)

      Example:
      \code
> alwaysflush +
Flush: on
      \endcode
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>timestamp</td>
    <td valign=top>[ (+|-) ]</td>
    <td>
      Enables/disables timestamping each log entry
      (default: off).

      Example:
      \code
> timestamp +
Timestamping: on
      \endcode
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>exit</td>
    <td valign=top></td>
    <td>
      Exits program immediately.
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>clear</td>
    <td valign=top>(l|a|c)</td>
    <td>
      Clears the given inclusion/exclusion list 
      (l=logs, a=asserts/crashes, c=checks).
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>add</td>
    <td valign=top>(l|a|c) (+|-) \<pattern\></td>
    <td>
      Adds a pattern to the given list (l=logs, 
      a=asserts/crashes, c=checks). By default all
      asserts, crashes, and checks are active, all logs
      inactive. Each item is then checked 
      against all pattern in the respective
      list. If a match is found the active/inactive
      state is modified accordingly (+ for active, - 
      for inactive). The final state is always
      the last match.

      Example:
      \code
> clear l
> add l + test*
> add l - *45*
      \endcode

      This would now enable logging on all log groups
      that start with 'test' but don't have '45' in
      the group name.
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>view</td>
    <td valign=top>[ (l|a|c) ]</td>
    <td>
      Shows the active pattern for the given list
      (l=logs, a=asserts/crashes, c=checks).
    </td>

  <!-- keep this as template for new commands -->
  </tr><tr>

    <!---------------------------------->
    <td valign=top></td>
    <td valign=top></td>
    <td>
    </td>

  </tr></table>

*/

/**
  \addtogroup debug_stream Debug and Logging stream

  The Debug class can act as an output stream for debugging and logging purposes.
  This is used e.g. for displaying custom messages in a failed DASSERT_MSG statement
  or for logging data.

  There are a number of reasons why this module implements this in a stream-like 
  fashion instead of the 'usual' printf method:
  - type safety: using a streams-like approach makes logging statements
    type safe. No more crashes in an ASSERT or log statement because the
    parameter specifiers in the printf format string didn't match with
    the values actually given.
  - extensibility: new stream operators can be implemented outside of the debug
    module

  Syntactically the Debug class acts almost like a ostream class:
  \code
int n=1;
float f=4;
DLOG( "This is a static string and some other stuff: " 
      << n << " " << f << "\n" );
  \endcode

  New types can be added easily, e.g.:
  \code
struct Point2D { int x,y; };

inline Debug& operator<<(Debug& debug, const Point2D &pt)
{
  debug << "(" << pt.x << ";" << pt.y << ")";
  return debug;
}

void test(const Point2D &val)
{
  DLOG( "Current point value is " << val << "\n" );
  DASSERT_MSG( val.x>=0 && val.y>=0, val << " is invalid");
}
  \endcode
  Please note that all these operators should not be cluttered with any #ifdef RTS_DEBUG
  or similar conditional compile statements. It is not necessary to remove these
  operators from the code in release builds (just don't forget to add 'inline' before
  the operator though).

  In addition there are a number of helper classes declared inside of Debug:
  - Debug::MemDump provides functions for inserting a raw memory dump into the Debug stream
  - Debug::HResult dumps out a HRESULT value plus additionally calling custom translator functions
*/

/**
  \page debug_ioclasses Debug I/O classes

  The %Debug module defines a number of pre-existing I/O classes. New classes can
  be developed and added.

  An I/O class can have additional commands and parameters. This section lists
  the existing I/O classes and their additional commands and parameters. The
  initial part of the command ('io \<class\>') is omitted here.

  \section debug_ioc_flat flat (flat or split log file)

  By default all data is written into a single flag log file. Optionally the
  size of this log file may be limited (implemented by using a memory based
  ring buffer).

  <table><tr>
    <td><b>I/O Command</b></td>
    <td><b>Parameters</b></td>
    <td><b>Description</b></td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>add</td>
    <td valign=top>[ \<filename\> [ \<size in kb\> ] ]</td>
    <td>
      Adds the flat I/O class. If a filename is
      specified all output is written to that file.
      Otherwise the filename \<exe\>-\<machinename\>.log
      (resp. \<exe\>-\<machinename\>-\<splitname\>.log)
      is automatically used. Any existing files with
      that name are overwritten.

      Instead of a real file name a 'magic' file name
      can be used by starting the file name with a '*' 
      followed by any number of special characters:
      - 'e': inserts EXE name
      - 'm': inserts machine name
      - 'u': inserts username
      - 't': inserts timestamp
      - 'n': inserts split name (empty if main log file)
      - '-': inserts '-'
      - 'E', 'M', 'U', 'T', 'N': same as above but with '-'
        in front (for 'N': empty if main log file)
      .log is automatically appended if using magic
      file names.

      So basically 'add' and 'and *eMN' are identical.

      If a size is specified then all data is internally written
      to a fixed size memory based ring buffer. This data is
      flushed out once the program exits.

      If no size is given then the size of the log file
      is not limited and any log data is written out
      immediately.
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>copy</td>
    <td valign=top>\<directory\></td>
    <td>
      Copies generated log file(s) into the given
      directory if the program exists or crashes.
      If there is already a log file with the same
      name a unique number is appended to the
      current log files' name.
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>splitadd</td>
    <td valign=top>\<types\> \<filter\> \<name\> [ \<size in kb\> ]</td>
    <td>
      Splits off part of the log data. Multiple
      splits can be defined. They are written out
      to the first matching split file.

      'types' defines one or more 
      string types which should be split off:
      - a: asserts/crashes
      - c: checks
      - l: logs
      - h: crash
      - x: exceptions
      - r: replies from commands
      - o: other messages 

      Next a filter is specified that determines
      which items are to be filtered (only for string
      types a, c and l). Items can be listed with
      then 'list' command. The filter is exactly specified
      as in that command.

      The third parameter defines a name for this
      split. If there is already a split with the same name
      then both will write to the same destination file.

      Note: If splits are used and the filename given
      for 'add' is static then the split name is automatically 
      appended to the log file name (before the extension).

      If a size is specified then all data is internally written
      to a fixed size memory based ring buffer. This data is
      flushed out once the program exits.

      If no size is given then the size of the log file
      is not limited and any log data is written out
      immediately.
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>splitview</td>
    <td valign=top></td>
    <td>
      Shows all existing splits in the order they are
      evaluated.
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>splitremove</td>
    <td valign=top>\<namepattern\></td>
    <td>
      Removes all active splits matching the given
      name pattern.
    </td>

  <!-- keep this as template for new commands -->
  </tr><tr>

    <!---------------------------------->
    <td valign=top></td>
    <td valign=top></td>
    <td>
    </td>

  </tr></table>

  \section debug_ioc_con con (console window)

  <table><tr>
    <td><b>I/O Command</b></td>
    <td><b>Parameters</b></td>
    <td><b>Description</b></td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>add</td>
    <td valign=top>%</td>
    <td>
      Adds the console window as log output. A new
      console window is created (if it doesn't
      exist already).
    </td>

  <!-- keep this as template for new commands -->
  </tr><tr>

    <!---------------------------------->
    <td valign=top></td>
    <td valign=top></td>
    <td>
    </td>

  </tr></table>

  \section debug_ioc_ods ods (OutputDebugString, for use in debugger)

  <table><tr>
    <td><b>I/O Command</b></td>
    <td><b>Parameters</b></td>
    <td><b>Description</b></td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>add</td>
    <td valign=top>%</td>
    <td>
      %Debug output is sent via OutputDebugString.
    </td>

  <!-- keep this as template for new commands -->
  </tr><tr>

    <!---------------------------------->
    <td valign=top></td>
    <td valign=top></td>
    <td>
    </td>

  </tr></table>

  \section debug_ioc_net net (Network destination via named pipe)

  <table><tr>
    <td><b>I/O Command</b></td>
    <td><b>Parameters</b></td>
    <td><b>Description</b></td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>add</td>
    <td valign=top>\<name\></td>
    <td>
      Tells system which computer to connect to for
      remote logging. By default this is the local
      machine. The debug module tries to connect
      to a named pipe \\\\\<name\>\\pipe\\ea_debug_v1.

      <!-- sorry about all these backslashes but they're used for escaping... -->
    </td>

  <!-- keep this as template for new commands -->
  </tr><tr>

    <!---------------------------------->
    <td valign=top></td>
    <td valign=top></td>
    <td>
    </td>

  </tr></table>

*/

/**
  \page debug_structcmd Structured command reply

  \section debug_structcmd_overview Overview

  All commands are capable of returning replies in two differnt forms: A regular 
  reply or a structured reply.
  A structured reply is basically like the regular reply but with three differences:
  - Structured reply string type \ref DebugIOInterface::StructuredCmdReply
  - There is a 1:1 mapping between command and result i.e. no matter how long a command
    result is there will always be only one call to the \ref DebugIOInterface::Write method
    for that result.
  - The result format is fixed for any given command i.e. it must not change.

  A structured reply is given if the command is invoked with a '!' as first character.

  The motivation behind having structured command replies is to make it possible
  to hide some of the debug command complexity behind a UI.

  \section debug_structcmd_list List of structured command replies for debug command group

  The reply for a structured command always consists of first repeating the
  given command, a CR and then additional optional reply data.

  <table><tr>
    <td valign=top><b>Command</b></td>
    <td><b>Reply</b></td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>list</td>
    <td>
      \<item 1\> CR<br>
      ...<br>
      \<item N\> CR
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>help</td>
    <td>
      regular reply format (unstructured)
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>io, io ?</td>
    <td>
      \<item 1\> CR<br>
      ...<br>
      \<item N\> CR
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>io \<any parameters\></td>
    <td>
      '1' CR (if command ok) <br>
      '0' \<error msg\> CR (if command failed)
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>alwaysflush</td>
    <td>
      '1' CR (if flush on) <br>
      '0' CR (if flush off)
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>timestamp</td>
    <td>
      '1' CR (if timestamp on) <br>
      '0' CR (if timestamp off)
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>exit</td>
    <td>
      none
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>clear</td>
    <td>
      none
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>add</td>
    <td>
      none
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>view</td>
    <td>
      \<item 1\> CR<br>
      ...<br>
      \<item N\> CR
    </td>

  <!-- keep this as template for new commands -->
  </tr><tr>

    <!---------------------------------->
    <td valign=top></td>
    <td>
    </td>

  </tr></table>
*/

#endif // DEBUG_DOC_H
