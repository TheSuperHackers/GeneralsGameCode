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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_cmd.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Debug command group 'debug'
//////////////////////////////////////////////////////////////////////////////
#include "_pch.h"
#include <process.h>

bool DebugCmdInterfaceDebug::Execute(class Debug& dbg, const char *cmd, 
                                     CommandMode cmdmode, unsigned argn, 
                                     const char * const * argv)
{
  // just for convenience...
  bool normalMode=cmdmode==CommandMode::Normal;

  if (!strcmp(cmd,"help"))
  {
    if (!normalMode)
      return true;
    
    if (!argn)
    {
      dbg << "debug group help:\n"
             "  list, io, alwaysflush, timestamp, exit, clear, add, view\n";
      return true;
    }
    else if (!strcmp(argv[0],"list"))
    {
      dbg << "list (g|l|d|a|c) [ <pattern> ]\n"
             "\n"
             "Shows some or all items of a specific type.\n"
             "\n"
             "The following items are supported:\n"
             "- g: command groups\n"
             "- l: log groups (only those encountered yet)\n"
             "- d: log groups with descriptions (only those that have descriptions)\n"
             "- a: asserts/crashes (only those hit yet)\n"
             "- c: checks (only those failed yet)\n"
             "\n"
             "If a pattern is specified only items matching\n"
             "that pattern are shown. A pattern can contain\n"
             "any character, letter, or a wildcard '*'.\n"
             "\n"
             "Please note that assert, crashes, and check items have\n"
             "their line number appended to the current file name,\n"
             "e.g. debug.cpp(13).\n";
      return true;
    }
    else if (!strcmp(argv[0],"io"))
    {
      dbg << "io <I/O Class> <cmd> { <param> }]\n"
             "\n"
             "Issues a I/O class command. I/O class commands are used\n"
             "for determining where all log output should be sent. \n"
             "Please check the list of \ref debug_ioclasses for a list\n"
             "of existing I/O classes.\n"
             "\n"
             "Each existing I/O class must accept at least the\n"
             "following two commands: 'add' and 'remove'. Usually\n"
             "after a class has been added it reacts to the 'help'\n"
             "command as well.\n"
             "\n"
             "If the command is entered without any parameters a list\n"
             "of active I/O classes is shown. Typing 'io ?' retrieves\n"
             "a list of possible I/O classes.\n";
      return true;
    }
    else if (!strcmp(argv[0],"alwaysflush"))
    {
      dbg << "alwaysflush [ (+|-) ]\n\n"
             "Enables/disables flushing after each new entry in\n"
             "the log file (default: off).\n";
      return true;
    }
    else if (!strcmp(argv[0],"timestamp"))
    {
      dbg << "timestamp [ (+|-) ]\n\n"
             "Enables/disables timestamping each log entry\n"
             "(default: off).\n";
      return true;
    }
    else if (!strcmp(argv[0],"exit"))
    {
      dbg << "exit\n\nExits program immediately.\n";
      return true;
    }
    else if (!strcmp(argv[0],"clear"))
    {
      dbg << "clear (l|a|c)\n\n"
             "Clears the given inclusion/exclusion list\n"
             "(l=logs, a=asserts/crashes, c=checks).\n";
      return true;
    }
    else if (!strcmp(argv[0],"add"))
    {
      dbg << "add (l|a|c) (+|-) <pattern>\n"
             "\n"
             "Adds a pattern to the given list (l=logs, \n"
             "a=asserts/crashes, c=checks). By default all\n"
             "asserts, crashes, and checks are active, all logs\n"
             "inactive. Each item is then checked \n"
             "against all pattern in the respective\n"
             "list. If a match is found the active/inactive\n"
             "state is modified accordingly (+ for active,\n"
             "- for inactive). The final state is always\n"
             "the last match.";
      return true;
    }
    else if (!strcmp(argv[0],"view"))
    {
      dbg << "view [ (l|a|c) ]\n\n"
             "Shows the active pattern for the given list\n"
             "(l=logs, a=asserts/crashes, c=checks).\n";
      return true;
    }
    return false;
  }
  if (!strcmp(cmd,"list"))
  {
    const char *pattern=argn>=2?argv[1]:"*";

    switch(argn?*argv[0]:0)
    {
      case 'g':
        {
          if (normalMode)
            dbg << "Command groups:\n";
          for (Debug::CmdInterfaceListEntry *cur=dbg.firstCmdGroup;cur;cur=cur->next)
            if (Debug::SimpleMatch(cur->group,pattern))
              dbg << cur->group << "\n";
        }
        break;
      case 'l':
      case 'd':
        {
          if (normalMode)
            dbg << "Logs:\n";
          for (Debug::KnownLogGroupList *cur=dbg.firstLogGroup;cur;cur=cur->next)
            if (Debug::SimpleMatch(cur->nameGroup,pattern)&&
                (*argv[0]=='l'||cur->descr))
            {
              dbg << cur->nameGroup;
              if (cur->descr)
                dbg << " (" << cur->descr << ")";
              dbg << "\n";
            }
        }
        break;
      case 'a':
      case 'c':
        {
          if (normalMode)
            dbg << (*argv[0]=='a'?"Asserts/Crashes:\n":"Checks:\n");
          unsigned mask=*argv[0]=='a'?Debug::FrameTypeAssert:Debug::FrameTypeCheck;
          for (unsigned k=0;k<Debug::FRAME_HASH_SIZE;k++)
          {
            for (Debug::FrameHashEntry *cur=dbg.frameHash[k];cur;cur=cur->next)
            {
              if (!(cur->frameType&mask))
                continue;

              char help[256];
              wsprintf(help,"%s(%i)",cur->fileOrGroup,cur->line);
              if (Debug::SimpleMatch(help,pattern))
              {
                dbg << help << " (" << cur->hits << " hits)";
                if (cur->status==Debug::Skip)
                  dbg << " [off]";
                dbg << "\n";
              }
            }
          }
        }
        break;
      default:
        dbg << "Unknown item type, see help.";
    }

    return true;
  }
  if (!strcmp(cmd,"io"))
  {
    // cmd: io
    if (!argn||!strcmp(argv[0],"?"))
    {
      // show active/all I/O classes
      if (normalMode)
        dbg << (argn?"Possible:\n":"Active:\n");

      bool hadItem=false;
      for (Debug::IOFactoryListEntry *cur=dbg.firstIOFactory;cur;cur=cur->next)
      {
        if (!argn&&!cur->io)
          continue;

        hadItem=true;
        dbg << cur->ioID << " (" << cur->descr << ")\n";
      }
      if (normalMode&&!hadItem)
        dbg << "(none)\n";
    }
    else
    {
      // regular I/O command

      // find I/O class
      for (Debug::IOFactoryListEntry *cur=dbg.firstIOFactory;cur;cur=cur->next)
        if (!strcmp(argv[0],cur->ioID))
          break;
      if (!cur)
      {
        dbg << "Unknown I/O class " << argv[0];
        return true; // still return true because we knew the command
      }

      if (argn>1)
      {
        // 'add' command?
        if (!strcmp(argv[1],"add"))
        {
          if (cur->io)
          {
            dbg << "I/O class already added";
            return true;
          }
          cur->io=cur->factory();
          if (!cur->io)
          {
            dbg << "I/O class factory failed";
            return true;
          }
        }
        // 'remove' command?
        if (!strcmp(argv[1],"remove"))
        {
          if (cur->io)
          {
            cur->io->Delete();
            cur->io=NULL;
          }
          return true;
        }
      }
      
      // now pass along I/O command
      if (!cur->io)
      {
        dbg << "Add I/O class first";
        return true;
      }

      cur->io->Execute(dbg,argn>1?argv[1]:NULL,!normalMode,argn>1?argn-2:0,argv+2);
    }
    return true;
  }
  if (!strcmp(cmd,"alwaysflush"))
  {
    if (argn)
    {
      if (*argv[0]=='+')
        dbg.alwaysFlush=true;
      if (*argv[0]=='-')
        dbg.alwaysFlush=false;
    }
    if (normalMode)
      dbg << "Always flush: " << (dbg.alwaysFlush?"on":"off");
    else
      dbg << (dbg.alwaysFlush?"1":"0");

    return true;
  }
  if (!strcmp(cmd,"timestamp"))
  {
    if (argn)
    {
      if (*argv[0]=='+')
        dbg.timeStamp=true;
      if (*argv[0]=='-')
        dbg.timeStamp=false;
    }
    if (normalMode)
      dbg << "Timestamp: " << (dbg.timeStamp?"on":"off");
    else
      dbg << (dbg.timeStamp?"1":"0");

    return true;
  }
  if (!strcmp(cmd,"exit"))
  {
    exit(1);
    return true;
  }
  if (!strcmp(cmd,"clear")||
      !strcmp(cmd,"add")||
      !strcmp(cmd,"view"))
  {
    unsigned mask=0;
    if (argn)
    {
      for (const char *p=argv[0];*p;p++)
      {
        switch(*p)
        {
          case 'l': mask|=Debug::FrameTypeLog; break;
          case 'a': mask|=Debug::FrameTypeAssert; break;
          case 'c': mask|=Debug::FrameTypeCheck; break;
        }
      }
    }
    if (!mask)
      mask=0xffffffff;

    bool modified=false;
    if (!strcmp(cmd,"clear"))
    {
      // remove some (or all) pattern
      const char *pattern=argn<2?"*":argv[1];
      for (Debug::PatternListEntry **entryPtr=&dbg.firstPatternEntry;*entryPtr;)
      {
        if ( (((*entryPtr)->frameTypes&mask)!=0)
            && Debug::SimpleMatch((*entryPtr)->pattern,pattern) )
        {
          // remove this entry
          modified=true;
          Debug::PatternListEntry *cur=*entryPtr;
          *entryPtr=cur->next;
          DebugFreeMemory(cur->pattern);
          DebugFreeMemory(cur);
        }
        else
          entryPtr=&((*entryPtr)->next);
      }

      // must fixup lastPatternEntry now
      if (dbg.firstPatternEntry)
      {
        for (Debug::PatternListEntry *cur=dbg.firstPatternEntry;cur->next;cur=cur->next);
        dbg.lastPatternEntry=cur;
      }
      else
        dbg.lastPatternEntry=NULL;
    }
    if (!strcmp(cmd,"add"))
    {
      // add a pattern
      if (argn<3)
        dbg << "Please specify mode and pattern";
      else
      {
        dbg.AddPatternEntry(mask,*argv[1]=='+',argv[2]);
        modified=true;
      }
    }
    if (!strcmp(cmd,"view"))
    {
      // show list of defined patterns
      for (Debug::PatternListEntry *cur=dbg.firstPatternEntry;cur;cur=cur->next)
      {
        if (!(cur->frameTypes&mask))
          continue;

        if (cur->frameTypes&Debug::FrameTypeLog) dbg << "l";
        if (cur->frameTypes&Debug::FrameTypeAssert) dbg << "a";
        if (cur->frameTypes&Debug::FrameTypeCheck) dbg << "c";

        dbg << (cur->isActive?" + ":" - ") << cur->pattern << "\n";
      }
    }

    if (modified)
    {
      // pattern list was modified, set all frame entries statuses to Unknown
      for (unsigned k=0;k<Debug::FRAME_HASH_SIZE;k++)
        for (Debug::FrameHashEntry *cur=dbg.frameHash[k];cur;cur=cur->next)
          cur->status=Debug::Unknown;
    }
    return true;
  }

  // unknown command
  return false;
}
