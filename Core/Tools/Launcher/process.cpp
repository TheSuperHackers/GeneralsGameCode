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

#include "process.h"

Process::Process()
{
  directory[0]=0;
  command[0]=0;
  args[0]=0;
  hProcess=nullptr;
  hThread=nullptr;
}

// Create a process
bit8 Create_Process(Process &process)
{
    int                      retval;
    STARTUPINFO              si;
    PROCESS_INFORMATION      piProcess;
    ZeroMemory(&si,sizeof(si));
    si.cb=sizeof(si);

    char cmdargs[513];
    memset(cmdargs,0,513);
    strcpy(cmdargs,process.command);
    strcat(cmdargs,process.args);

    DBGMSG("PROCESS CMD="<<cmdargs<<"  DIR="<<process.directory);

    retval=CreateProcess(nullptr,cmdargs,nullptr,nullptr,FALSE, 0  ,nullptr, nullptr/*process.directory*/,&si,&piProcess);

    DBGMSG("("<<retval<<") New process:  HANDLE " << (void *)piProcess.hProcess << "   ID "
      << (DWORD)piProcess.dwProcessId);
    DBGMSG("("<<retval<<") New thread:  HANDLE " << (void *)piProcess.hThread << "   ID "
      << (DWORD)piProcess.dwThreadId);
    if (retval==0)
    {
      char message_buffer[256];
	   FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &message_buffer[0], 256, nullptr );
      DBGMSG("ERR: "<<message_buffer);
    }
    process.hProcess=piProcess.hProcess;
		process.dwProcessID = piProcess.dwProcessId;
    process.hThread=piProcess.hThread;
		process.dwThreadID = piProcess.dwThreadId;
    return(TRUE);
}

//
// Wait for a process to complete, and fill in the exit code
//
bit8 Wait_Process(Process &process, DWORD *exit_code)
{
  DWORD retval;
  retval=WaitForSingleObject(process.hProcess,INFINITE);
  if (exit_code != nullptr)
    *exit_code=-1;
  if (retval==WAIT_OBJECT_0)  // process exited
  {
		// MDC 1/10/2003 Inserting sleep here to let game exit before applying patch
		Sleep(3000);

    if (exit_code != nullptr)
      GetExitCodeProcess(process.hProcess,exit_code);
    return(TRUE);
  }
  else                        // can this happen?
    return(FALSE);
}


//
// Get the process to run from the config object
//
bit8 Read_Process_Info(ConfigFile &config,OUT Process &info, IN const char *key)
{

 Wstring keyStr = "RUN";
 if (key)
	 keyStr = key;

 Wstring procinfo;
 if (config.getString(keyStr.get(), procinfo)==FALSE)
 {
   DBGMSG("Couldn't read the RUN line");
   return(FALSE);
 }
 int          offset=0;
 Wstring      dir;
 Wstring      executable;
 Wstring      args;
 offset=procinfo.getToken(offset," ",dir);
 offset=procinfo.getToken(offset," ",executable);
 args=procinfo;
 args.remove(0,offset);

 ///
 ///
 DBGMSG("RUN: EXE = "<<executable.get()<<"  DIR = "<<dir.get()<<
   "  ARGS = "<<args.get());
 strcpy(info.command,executable.get());
 strcpy(info.directory,dir.get());
 strcpy(info.args,args.get());
 return(TRUE);


/*********************************************************
  FILE     *in;
  char      string[256];
  bit8      found_space;
  int       i;

  if ((in=fopen(config,"r"))==nullptr)
    return(FALSE);

  while(fgets(string,256,in))
  {
    i=0;
    while ((isspace(string[i]))&&(i<int(strlen(string))))
      i++;

    // do nothing with empty line or commented line
    if ((string[i]=='#')||(i==int(strlen(string)-1)))
      continue;

    strcpy(info.directory,string);
    found_space=FALSE;
    for (; i<int(strlen(string)); i++)
    {
      if (isspace(info.directory[i]))
      {
        info.directory[i]=0;
        found_space=TRUE;
      }
      else if (found_space)
        break;
    }

    strcpy(info.command,string+i);
    for (i=0; i<int(strlen(info.command)); i++)
      if ((info.command[i]=='\n')||(info.command[i]=='\r'))
        info.command[i]=0;

    //printf("DIR = '%s'  CMD = '%s'\n",info.directory,info.command);

    // We only have 1 process for this
    break;
  }
  fclose(in);
  return(TRUE);
**********************************************/
}
