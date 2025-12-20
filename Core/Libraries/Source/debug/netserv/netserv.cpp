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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/netserv/netserv.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Simple console based server for NET I/O connections
//////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>

// Note: This implementation is quick'n'ugly. I've developed this
//       program basically just for testing the net I/O class.

static char m_input[256];
static unsigned m_inputUsed;

static void InitConsole(void)
{
  AllocConsole();

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
}

static char *InputConsole(void)
{
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
    ci[k].Attributes=BACKGROUND_BLUE|FOREGROUND_BLUE|FOREGROUND_GREEN
                    |FOREGROUND_RED|FOREGROUND_INTENSITY;
  }

  // fake another cursor
  if (GetTickCount()&512)
    ci[m_inputUsed].Attributes=BACKGROUND_BLUE|BACKGROUND_GREEN
                              |BACKGROUND_RED|BACKGROUND_INTENSITY|FOREGROUND_BLUE;

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
    m_input[--m_inputUsed]=0;
    m_inputUsed=0;
    return m_input;
  }

  return 0;
}

class Pipe
{
  Pipe(const Pipe&);
  Pipe& operator=(const Pipe&);

  HANDLE m_pipe;
  bool m_connected;
  int m_state;
  int m_stringType;
  int m_len;
  char *m_src;
  char *m_str;

public:
  Pipe(void):
    m_pipe(INVALID_HANDLE_VALUE),
    m_src(NULL), m_str(NULL), m_stringType(0)
  {
  }

  ~Pipe()
  {
    if (m_pipe!=INVALID_HANDLE_VALUE)
    {
      DisconnectNamedPipe(m_pipe);
      CloseHandle(m_pipe);
      free(m_src);
      free(m_str);
    }
  }

  bool Create(const char *name)
  {
    m_pipe=CreateNamedPipe(name,
                           PIPE_ACCESS_DUPLEX,
                           PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_NOWAIT,
                           PIPE_UNLIMITED_INSTANCES,1024,1024,0,NULL);
    m_connected=false;
    return m_pipe!=INVALID_HANDLE_VALUE;
  }

  bool Connected(void)
  {
    if (!m_connected)
    {
      ConnectNamedPipe(m_pipe,NULL);
      if (GetLastError()==ERROR_PIPE_CONNECTED)
      {
        DWORD dwDummy;
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),"\n<connect>\n",11,&dwDummy,NULL);
        m_connected=true;
        m_state=0;
      }
    }
    return m_connected;
  }

  void Write(char msg)
  {
    DWORD dummy;
    if (!WriteFile(m_pipe,&msg,1,&dummy,NULL)||!dummy)
    {
      char sp[30];
      wsprintf(sp,"%c:%i/%i\n",msg,dummy,GetLastError());

      DWORD dwDummy;
      WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),sp,strlen(sp),&dwDummy,NULL);
    }
  }

  const char *Read(void)
  {
    DWORD read;
    switch(m_state)
    {
      case 0:
        if (!ReadFile(m_pipe,&m_stringType,1,&read,NULL))
          break;
        if (read==1)
          m_state++;
        return NULL;
      case 1:
      case 3:
        if (!ReadFile(m_pipe,&m_len,4,&read,NULL))
          break;
        if (read==4)
        {
          if (m_state==1)
            m_src=(char *)realloc(m_src,m_len+1);
          else
            m_str=(char *)realloc(m_str,m_len+1);
          m_state++;
        }
        return NULL;
      case 2:
        if (!ReadFile(m_pipe,m_src,m_len,&read,NULL))
          break;
        if (read==m_len)
        {
          m_src[m_len]=0;
          m_state++;
        }
        return NULL;
      case 4:
        if (!ReadFile(m_pipe,m_str,m_len,&read,NULL))
          break;
        if (read==m_len)
        {
          m_str[m_len]=0;
          m_state=0;
          return m_str;
        }
        return NULL;
    }

    if (GetLastError()==ERROR_BROKEN_PIPE)
    {
      DisconnectNamedPipe(m_pipe);

      DWORD dwDummy;
      WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),"\n<disconnect>\n",14,&dwDummy,NULL);
      m_connected=false;
    }

    return NULL;
  }
};

int CALLBACK WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
  InitConsole();

  char buf1[200],buf2[400];
  DWORD dwDummy=sizeof(buf1);
  GetComputerName(buf1,&dwDummy);
  WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),buf2,
            wsprintf(buf2,"\n\nSimple debug.net Server ready. Enter 'quit' to exit.\n\nLocal machine: %s\n\n",buf1),
            &dwDummy,NULL);

  Pipe p[10];
  for (int k=0;k<10;k++)
    if (!p[k].Create("\\\\.\\pipe\\ea_debug_v1"))
    {
      char msg[200];
      wsprintf(msg,"Can't create named pipe (Code %i).",GetLastError());
      MessageBox(NULL,msg,"Error",MB_OK);
      return 1;
    }

  for (;;)
  {
    char *input=InputConsole();
    if (input)
    {
      if (strcmp(input,"quit") == 0)
        break;
    }

    for (int k=0;k<10;k++)
    {
      if (!p[k].Connected())
        continue;

      const char *msg=p[k].Read();
      if (msg)
      {
        DWORD dwDummy;
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),msg,strlen(msg),&dwDummy,NULL);
      }

      if (input)
      {
        for (unsigned i=0;input[i];i++)
          p[k].Write(input[i]);
        p[k].Write('\n');
      }
    }

    Sleep(10);
  }

  return 0;
}
