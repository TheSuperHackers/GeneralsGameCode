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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_cmd.h $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Debug command interface
//////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef DEBUG_CMD_H // Include guard
#define DEBUG_CMD_H

/**
  \interface DebugCmdInterface debug.h <rts/debug.h>

  \brief Debug command group interface.

  A debug command group interface implementation defines a new
  command group and the commands implemented for this group.

  A Debug command group interface instance must register itself
  using Debug::AddCommands. Ownership is then transfered to
  the Debug module unless the object is manually removed
  by calling Debug::RemoveCommands.

  For convenience the macro \ref DEBUG_CREATE_COMMAND_GROUP
  can be used as well.
*/
class DebugCmdInterface
{
  // no copy/assign op
  DebugCmdInterface(const DebugCmdInterface&);
  DebugCmdInterface& operator=(const DebugCmdInterface&);

protected:
  // nobody can call this destructor (except child classes)
  virtual ~DebugCmdInterface() {}

public:
  // interface only so no functionality here
  explicit DebugCmdInterface(void) {}

  /// possible command modes
  enum CommandMode
  {
    /// normal command mode
    Normal,

    /// structured command mode
    Structured,

    MAX
  };

  /**
    \brief Execute the given command.

    This function is called whenever a command has been issued for
    the command group the interface implements.

    \param dbg debug instance
    \param cmd command issued
    \param cmdmode command mode
    \param argn number of additional arguments passed in
    \param argv argument list
    \return true if command was known, false if not
  */
  virtual bool Execute(class Debug& dbg, const char *cmd, CommandMode cmdmode,
                       unsigned argn, const char * const * argv)=0;

  /**
    \brief Destroys the current command interface.

    Use this function instead of just delete'ing the instance.
  */
  virtual void Delete(void)=0;
};

/**
  \addtogroup debug_cmd_macros Debug command interface helper macros

  These macros can be used if a command interface instance should
  be available right after starting the program (which is usually
  the case).
*/
///@{

#ifdef DOXYGEN

  /**
    \brief Helper macro for creating and registering a command interface
    instance the Debug module.

    \param groupname name of command group this class if for
                 (without quotes)
    \param type type name of class we're implementing
  */
  #define DEBUG_CREATE_COMMAND_GROUP(groupname,type)

#else

  #define DEBUG_CREATE_COMMAND_GROUP(groupname,type) \
    static bool __RegisterDebugCmdGroup_##type=Debug::AddCommands(#groupname,new type);

#endif

///@}

#endif // DEBUG_CMD_H
