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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_io_net.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Debug I/O class net (Network destination via named pipe)
//////////////////////////////////////////////////////////////////////////////
#include "_pch.h"
#include <new>      // needed for placement new prototype

DebugIONet::DebugIONet(void)
{
}

DebugIONet::~DebugIONet()
{
  if (m_pipe!=INVALID_HANDLE_VALUE)
    CloseHandle(m_pipe);
}

int DebugIONet::Read(char *buf, int maxchar)
{
  if (m_pipe==INVALID_HANDLE_VALUE)
    return 0;

  DWORD mode=PIPE_READMODE_MESSAGE|PIPE_NOWAIT;
  SetNamedPipeHandleState(m_pipe,&mode,NULL,NULL);

  DWORD read;
  if (!ReadFile(m_pipe,buf,maxchar-1,&read,NULL))
    read=0;
  mode=PIPE_READMODE_MESSAGE|PIPE_WAIT;
  SetNamedPipeHandleState(m_pipe,&mode,NULL,NULL);

  return read;
}

void DebugIONet::Write(StringType type, const char *src, const char *str)
{
  if (m_pipe==INVALID_HANDLE_VALUE)
    return;

  DWORD dummy;
  WriteFile(m_pipe,&type,1,&dummy,NULL);

  unsigned len;
  len=src?strlen(src):0;
  WriteFile(m_pipe,&len,4,&dummy,NULL);
  if (len)
    WriteFile(m_pipe,src,len,&dummy,NULL);

  len=strlen(str);
  WriteFile(m_pipe,&len,4,&dummy,NULL);
  if (len)
    WriteFile(m_pipe,str,len,&dummy,NULL);
}

void DebugIONet::EmergencyFlush(void)
{
}

void DebugIONet::Execute(class Debug& dbg, const char *cmd, bool structuredCmd,
                         unsigned argn, const char * const * argv)
{
  if (!cmd||!strcmp(cmd,"help"))
  {
    dbg << "net I/O help:\n"
           "  add [ <machine> ]\n"
           "    create net I/O (optionally specifying the machine to connect to)\n";
  }
  else if (!strcmp(cmd,"add"))
  {
    const char *machine=argn?argv[0]:".";

    char buf[256];
    wsprintf(buf,"\\\\%s\\pipe\\ea_debug_v1",machine);
    m_pipe=CreateFile(buf,GENERIC_READ|GENERIC_WRITE,
                      0,NULL,OPEN_EXISTING,0,NULL);
    if (m_pipe==INVALID_HANDLE_VALUE)
    {
      dbg << "Could not connect to given machine.\n";
      return;
    }

    // we're reading messages
    DWORD mode=PIPE_READMODE_MESSAGE;
    SetNamedPipeHandleState(m_pipe,&mode,NULL,NULL);

    // write welcome message
    char comp[128];
    mode=sizeof(comp);
    GetComputerName(comp,&mode);
    wsprintf(buf,"Client at %s\n",comp);
    Write(Other,NULL,buf);
  }
}

DebugIOInterface *DebugIONet::Create(void)
{
  return new (DebugAllocMemory(sizeof(DebugIONet))) DebugIONet();
}

void DebugIONet::Delete(void)
{
  this->~DebugIONet();
  DebugFreeMemory(this);
}
