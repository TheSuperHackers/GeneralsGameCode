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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_io_con.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Debug I/O class con (console window)
//////////////////////////////////////////////////////////////////////////////
#include "_pch.h"
#include <stdlib.h>
#include <new>      // needed for placement new prototype

DebugIOCon::DebugIOCon(void):
  m_inputUsed(0), m_inputRead(0)
{
  // check: is there already a console window open?
  m_allocatedConsole=AllocConsole()!=0;
  if (m_allocatedConsole)
  {
    HANDLE h=GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(h,0);

    // make screen buffer same size as currently displayed area
    // (prevents that our input line gets scrolled out of view)
    h=GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(h,&info);

    COORD newSize;
    newSize.X=info.srWindow.Right+1;
    newSize.Y=info.srWindow.Bottom+1;
    SetConsoleScreenBufferSize(h,newSize);

    // hide cursor
    CONSOLE_CURSOR_INFO ci;
    ci.dwSize=1;
    ci.bVisible=FALSE;
    SetConsoleCursorInfo(h,&ci);

    Write(StringType::Other,NULL,"\n\nEA/Debug console open\n\n");
  }
}

DebugIOCon::~DebugIOCon()
{
  // close console if we allocated it
  if (m_allocatedConsole)
    FreeConsole();
}

int DebugIOCon::Read(char *buf, int maxchar)
{
  // We are not supporting reading from the console
  // unless we allocated that console ourselves.
  // The reason for that is that if we didn't allocate
  // the console ourselves we're running as a console
  // process and if we'd be reading data from the console
  // we would probably be snatching away keyboard data
  // for the process that is using that console.
  if (!m_allocatedConsole)
    return 0;

  // are we doing a continuous read?
  if (m_inputRead)
  {
    int numRead;
    if (maxchar>m_inputUsed-m_inputRead)
    {
      // return all
      numRead=m_inputUsed-m_inputRead;
      memcpy(buf,m_input+m_inputRead,numRead);
      m_inputRead=m_inputUsed=0;
    }
    else
    {
      // return partially
      numRead=maxchar;
      memcpy(buf,m_input+m_inputRead,numRead);
      m_inputRead+=maxchar;
    }
    return numRead;
  }

  // update our input buffer
  HANDLE h=GetStdHandle(STD_INPUT_HANDLE);
  bool returnChars=false;
  for (;;)
  {
    DWORD dwRecords;
    if (!GetNumberOfConsoleInputEvents(h,&dwRecords))
      break;
    if (!dwRecords)
      break;

    INPUT_RECORD record;
    ReadConsoleInput(h,&record,1,&dwRecords);
    if (record.EventType!=KEY_EVENT)
      continue;

    KEY_EVENT_RECORD &key=record.Event.KeyEvent;
    if (!key.bKeyDown||!key.uChar.AsciiChar)
      continue;

    if (key.uChar.AsciiChar=='\r'||
        key.uChar.AsciiChar=='\n')
    {
      m_input[m_inputUsed++]='\n';
      returnChars=true;
      break;
    }

    /// @todo_opt if somebody wants this can be improved by adding support for cursor keys, history, etc

    if (key.uChar.AsciiChar=='\b')
    {
      if (m_inputUsed)
        m_inputUsed--;
    }
    else if (((unsigned char)key.uChar.AsciiChar)>=' ')
    {
      if (m_inputUsed<sizeof(m_input)-1)
        m_input[m_inputUsed++]=key.uChar.AsciiChar;
    }
  }

  // update screen
  h=GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetConsoleScreenBufferInfo(h,&info);
  CHAR_INFO ci[sizeof(m_input)+1];
  for (unsigned k=0;k<=sizeof(m_input);k++)
  {
    ci[k].Char.AsciiChar=k<m_inputUsed?m_input[k]:' ';
    ci[k].Attributes=BACKGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_GREEN
                    |FOREGROUND_RED|FOREGROUND_INTENSITY;
  }

  // fake another cursor
  if (GetTickCount()&512)
    ci[m_inputUsed].Attributes=BACKGROUND_BLUE|BACKGROUND_GREEN
                              |BACKGROUND_RED|BACKGROUND_INTENSITY|FOREGROUND_GREEN;

  COORD srcSize,srcCoord;
  srcSize.X=sizeof(m_input); srcSize.Y=1;
  srcCoord.X=srcCoord.Y=0;

  SMALL_RECT r;
  r.Left=r.Top=r.Bottom=0; r.Right=info.dwSize.X-1;
  WriteConsoleOutput(h,ci+(m_inputUsed<=info.dwSize.X?0:m_inputUsed-info.dwSize.X),
                      srcSize,srcCoord,&r);

  // return data now?
  if (returnChars&&m_inputUsed>1)
  {
    *buf=*m_input;
    m_inputRead=1;
    return 1;
  }

  return 0;
}

void DebugIOCon::Write(StringType type, const char *src, const char *str)
{
  if (type==StringType::StructuredCmdReply||!str)
    return;

  DWORD dwDummy;
  WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),str,strlen(str),&dwDummy,NULL);
}

void DebugIOCon::Execute(class Debug& dbg, const char *cmd, bool structuredCmd,
                         unsigned argn, const char * const * argv)
{
  if (!cmd||!strcmp(cmd,"help"))
  {
    dbg << "con I/O help:\n"
           "  add [ <width> [ <height> ] ]\n"
           "    create con I/O (optionally specifying the window size)\n";
  }
  else if (!strcmp(cmd,"add"))
  {
    if (argn>0&&m_allocatedConsole)
    {
      // resize our console area
      HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
      COORD newSize;
      newSize.X=atoi(argv[0]);
      newSize.Y=argn>1?atoi(argv[1]):25;
      SMALL_RECT sr;
      sr.Left=sr.Top=0;
      sr.Right=newSize.X-1;
      sr.Bottom=newSize.Y-1;
      SetConsoleWindowInfo(h,TRUE,&sr);
      SetConsoleScreenBufferSize(h,newSize);
      SetConsoleWindowInfo(h,TRUE,&sr);
    }
  }
}

DebugIOInterface *DebugIOCon::Create(void)
{
  return new (DebugAllocMemory(sizeof(DebugIOCon))) DebugIOCon();
}

void DebugIOCon::Delete(void)
{
  this->~DebugIOCon();
  DebugFreeMemory(this);
}
