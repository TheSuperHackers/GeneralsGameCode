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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_debug.cpp $
// $Author: mhoffe $
// $Revision: #2 $
// $DateTime: 2003/07/09 10:57:23 $
//
// (c) 2003 Electronic Arts
//
// Debug class implementation
//////////////////////////////////////////////////////////////////////////////
#include "_pch.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <new>      // needed for placement new prototype

// a little dummy variable that makes the linker actually include
// us...
extern "C" bool __DebugIncludeInLink1;
bool __DebugIncludeInLink1;

// This part is a little tricky (and not portable to other compilers).
// MSVC initializes all static C++ variables by calling a list of 
// function pointers contained in data segments called .CRT$XCA to
// .CRT$XCZ. We jam in our own two functions at the very beginning
// and end of this list (B and Y respectively since the A and Z segments
// contain list delimiters).
#pragma data_seg(".CRT$XCB")
void *Debug::PreStatic=&Debug::PreStaticInit;
#pragma data_seg(".CRT$XCY")
void *Debug::PostStatic=&Debug::PostStaticInit;
#pragma data_seg()

Debug::LogDescription::LogDescription(const char *fileOrGroup, const char *description)
{
  Debug::Instance.AddLogGroup(fileOrGroup,description);
}

// our global Debug instance
Debug Debug::Instance;

// more class static members
unsigned Debug::curStackFrame;

// this constructor is empty on purpose because all construction
// work is done in PreStaticInit (and some in PostStaticInit)
Debug::Debug(void)
{
  // do not put any code in here (but it's good for keeping module global todo's)
  /// @todo what about frame based logging?
  /// @todo have new DLOG with category, add DWARN, DPERF, DERR etc. based on that,
  ///       make it possible to enable/disable categories by adding category to log ID
}

void Debug::PreStaticInit(void)
{
  // do not change any member variables that have constructors
  // because they are not constructed yet!

  // make sure this function gets called on exit
  // (we might still have to call it manually if there's
  // an exception and we're not calling exit)
  atexit(StaticExit);

  // init vars
  Instance.hrTranslators=NULL;
  Instance.numHrTranslators=0;
  Instance.firstIOFactory=NULL;
  Instance.firstCmdGroup=NULL;
  memset(Instance.frameHash,0,sizeof(Instance.frameHash));
  Instance.nextUnusedFrameHash=NULL;
  Instance.numAvailableFrameHash=0;
  Instance.firstLogGroup=NULL;
  memset(Instance.ioBuffer,0,sizeof(Instance.ioBuffer));
  Instance.curType=DebugIOInterface::StringType::MAX;
  *Instance.curSource=0;
  Instance.disableAssertsEtc=0;
  Instance.curFrameEntry=NULL;
  Instance.firstPatternEntry=NULL;
  Instance.lastPatternEntry=NULL;
  *Instance.curCommandGroup=0;
  Instance.alwaysFlush=false;
  Instance.timeStamp=false;
  Instance.m_radix=10;
  Instance.m_fillChar=' ';
  
  /// install exception handler
  SetUnhandledExceptionFilter(DebugExceptionhandler::ExceptionFilter);
}

void Debug::PostStaticInit(void)
{
  InstallExceptionHandler();

  // register our default IO classes
  AddIOFactory("con","Console window",DebugIOCon::Create);
  AddIOFactory("flat","Flat local file(s)",DebugIOFlat::Create);
  AddIOFactory("net","Network via named pipe",DebugIONet::Create);
  AddIOFactory("ods","OutputDebugString function",DebugIOOds::Create);

  // add debug command handler
  AddCommands("debug",new (DebugAllocMemory(sizeof(DebugCmdInterfaceDebug))) DebugCmdInterfaceDebug);

  /// exec dbgcmd file
  char ioBuffer[2048];
  GetModuleFileName(NULL,ioBuffer,sizeof(ioBuffer));
  char *q=strrchr(ioBuffer,'.');
  if (q)
    strcpy(q,".dbgcmd");
  HANDLE h=CreateFile(ioBuffer,GENERIC_READ,0,NULL,OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,NULL);
  if (h==INVALID_HANDLE_VALUE)
    h=CreateFile("default.dbgcmd",GENERIC_READ,0,NULL,OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,NULL);
  if (h!=INVALID_HANDLE_VALUE)
  {
    char cmdBuffer[512];
    unsigned long ioCur=0,ioUsed=0,cmdCur=0;
    ReadFile(h,ioBuffer,sizeof(ioBuffer),&ioUsed,NULL);
    for (;;)
    {
      if (ioCur==ioUsed)
      {
        ReadFile(h,ioBuffer,sizeof(ioBuffer),&ioUsed,NULL);
        ioCur=0;
      }
      if (ioCur==ioUsed||ioBuffer[ioCur]=='\n'||ioBuffer[ioCur]=='\r')
      {
        if (cmdCur)
        {
          Instance.ExecCommand(cmdBuffer,cmdBuffer+cmdCur);
          cmdCur=0;
        }
        if (ioCur==ioUsed)
          break;
        ioCur++;
      }
      else
      {
        if (cmdCur<sizeof(cmdBuffer))
          cmdBuffer[cmdCur++]=ioBuffer[ioCur];
        ioCur++;
      }
    }
    CloseHandle(h);
  }
  else
  {
    // exec default commands
    const char *p=DebugGetDefaultCommands();
    while (p&&*p)
    {
      const char *q=strchr(p,'\n');
      if (!q)
        q=p+strlen(p);
      if (p!=q)
      {
        Instance.ExecCommand(p,q);
        p=*q?q+1:NULL;
      }
    }
  }

  // check: are we using an old dbghelp.dll?
  if (DebugStackwalk::IsOldDbghelp())
  {
    // give a serious hint
    Instance.StartOutput(DebugIOInterface::Other,"");
    Instance << RepeatChar('=',79) <<
      "\nYou are using an older version of the DBGHELP.DLL library.\n"
      "Please update to the newest available version in order to\n"
      "get reliable stack and symbol information.\n\n";

    char buf[256];
    GetModuleFileName((HMODULE)DebugStackwalk::GetDbghelpHandle(),buf,sizeof(buf));
    Instance <<
      "Hint: The DLL got loaded as:\n" << buf << "\n" << RepeatChar('=',79) << "\n\n";

    // flush output only if there is already an active I/O class
    Instance.FlushOutput(false);
  }
}

void Debug::StaticExit(void)
{
  // yes, we do leave memory 'leaks' but Win32 will take care of these

  // however, I/O classes must be actively shut down
  if (Instance.curType!=DebugIOInterface::StringType::MAX)
    Instance.FlushOutput();
  for (IOFactoryListEntry *io=Instance.firstIOFactory;io;io=io->next)
    if (io->io)
    {
      io->io->Delete();
      io->io=NULL;
    }

  // and command group interfaces...
  for (CmdInterfaceListEntry *cmd=Instance.firstCmdGroup;cmd;cmd=cmd->next)
    if (cmd->cmdif)
    {
      cmd->cmdif->Delete();
      cmd->cmdif=NULL;
    }
}

Debug& Debug::operator<<(RepeatChar &c)
{
  if (c.m_count>=10)
  {
    char help[10];
    memset(help,c.m_char,10);
    while ((c.m_count-=10)>=0)
      AddOutput(help,10);
  }
  while (c.m_count-->0)
    AddOutput(&c.m_char,1);
  return *this;
}

Debug::Format::Format(const char *format, ...)
{
  va_list va;
  va_start(va,format);
  _vsnprintf(m_buffer,sizeof(m_buffer)-1,format,va);
  va_end(va);
}

Debug::~Debug()
{
  // again, do not put any code in here
}

static void LocalSETranslator(unsigned, struct _EXCEPTION_POINTERS *pExPtrs)
{
  // simply call our regular exception handler
  DebugExceptionhandler::ExceptionFilter(pExPtrs);
}

void Debug::InstallExceptionHandler(void)
{
  _set_se_translator(LocalSETranslator);
}

bool Debug::SkipNext(void)
{
  // this is typically set while an assertion
  // is running
  if (Instance.disableAssertsEtc)
    return true;

  // do not implement this function inline, we do need
  // a valid frame pointer here!
  unsigned help;
  _asm 
  {
    mov eax,[ebp+4]   // return address
    mov help,eax
  };
  curStackFrame=help;

  // do we know if to skip the following code?
  FrameHashEntry *e=Instance.LookupFrame(curStackFrame);
  if (!e||                // unknown frame, will be added later
       e->status==NoSkip) // frame known but active
    return false;

  // status is unknown, must update
  if (e->status==Unknown)
    Instance.UpdateFrameStatus(*e);

  // now we now wether to skip or not
  return e->status==Skip;
}

Debug& Debug::AssertBegin(const char *file, int line, const char *expr)
{
  // avoid infinite recursion...
  ++Instance.disableAssertsEtc;

  // anything to flush first?
  if (Instance.curType!=DebugIOInterface::StringType::MAX)
    Instance.FlushOutput();

  // set new output
  __ASSERT(Instance.curFrameEntry==NULL);
  Instance.curFrameEntry=Instance.GetFrameEntry(curStackFrame,FrameTypeAssert,file,line);
  if (Instance.curFrameEntry->status==NoSkip)
  {
    Instance.StartOutput(DebugIOInterface::StringType::Assert,"%s(%i)",
                  Instance.curFrameEntry->fileOrGroup,
                  Instance.curFrameEntry->line);
    ++Instance.curFrameEntry->hits;

    // if there is a \code\ section in the filename truncate
    // everything before that (including \code\)
    const char *p=strstr(file,"\\code\\");
    p=p?p+6:file;

    Instance << "\n" << RepeatChar('=',80) << "\nAssertion failed in " << p << ", line " << line 
             << ",\nexpression " << expr;
  }

  return Instance;
}

bool Debug::AssertDone(void)
{
  --disableAssertsEtc;

  // did we have an active assertion?
  if (curType==DebugIOInterface::StringType::Assert)
  {
    __ASSERT(curFrameEntry!=NULL);

    // hit info?
    if (curFrameEntry->hits>1)
      (*this) << " (hit #" << curFrameEntry->hits << ")";

    // need CR?
    if (!ioBuffer[curType].lastWasCR)
      operator<<("\n");

    // yes, duplicate message
    const char *addInfo="\nPress 'abort' to abort the program,\n"
                        "'retry' for breaking into the debugger, or\n"
                        "'ignore' for ignoring this assertion for the\n"
                        "time being (stops logging this assertion as well).";
    char *help=(char *)DebugAllocMemory(ioBuffer[curType].used+strlen(addInfo)+1);
    strcpy(help,ioBuffer[curType].buffer+82);
    strcat(help,addInfo);
    
    // First hit? Then do a stack trace
    if (curFrameEntry->hits==1)
    {
      DebugStackwalk::Signature sig;
      if (m_stackWalk.StackWalk(sig))
        (*this) << sig;
    }

    // ... and flush out
    operator<<("\n\n");
    FlushOutput();

    // show dialog box only if running windowed
    if (IsWindowed())
    {
      /// @todo replace MessageBox with custom dialog w/ 4 options: abort, skip 1, skip all, break

      // now display message, wait for user input
      int result=MessageBox(NULL,help,"Assertion failed",
                            MB_ABORTRETRYIGNORE|MB_ICONSTOP|MB_TASKMODAL|MB_SETFOREGROUND);
      switch(result)
      {
        case IDABORT:
          curFrameEntry=NULL;
          exit(1);
          break;
        case IDIGNORE:
          {
            // build 'pattern'
            char help[200];
            __ASSERT(strlen(curFrameEntry->fileOrGroup)<190);
            wsprintf(help,"%s(%i)",curFrameEntry->fileOrGroup,
                                   curFrameEntry->line);
            AddPatternEntry(FrameTypeAssert,false,help);
            curFrameEntry->status=Skip;
          }
          break;
        case IDRETRY:
          _asm int 0x03
          break;
        default:
          ((void)0);
      }
    }
    else 
    {
      // we're running fullscreen

      // hit too often?
      if (curFrameEntry->hits==MAX_CHECK_HITS)
      {
        // yup, turn off then
        StartOutput(DebugIOInterface::StringType::Other,"");
        Instance << "Assert hit too often - turning check off.\n";
        FlushOutput();

        // build 'pattern'
        char help[200];
        __ASSERT(strlen(curFrameEntry->fileOrGroup)<190);
        wsprintf(help,"%s(%i)",curFrameEntry->fileOrGroup,
                               curFrameEntry->line);
        AddPatternEntry(FrameTypeAssert,false,help);

        curFrameEntry->status=Skip;
      }
    }
  }

  curFrameEntry=NULL;
  return false;
}

Debug& Debug::CheckBegin(const char *file, int line, const char *expr)
{
  // avoid infinite recursion...
  ++Instance.disableAssertsEtc;

  // anything to flush first?
  if (Instance.curType!=DebugIOInterface::StringType::MAX)
    Instance.FlushOutput();

  // set new output
  __ASSERT(Instance.curFrameEntry==NULL);
  Instance.curFrameEntry=Instance.GetFrameEntry(curStackFrame,FrameTypeCheck,file,line);
  if (Instance.curFrameEntry->status==NoSkip)
  {
    ++Instance.curFrameEntry->hits;
    Instance.StartOutput(DebugIOInterface::StringType::Check,"%s(%i)",
                  Instance.curFrameEntry->fileOrGroup,
                  Instance.curFrameEntry->line);

    // if there is a \code\ section in the filename truncate
    // everything before that (including \code\)
    const char *p=strstr(file,"\\code\\");
    p=p?p+6:file;

    Instance << "\n" << RepeatChar('=',80) << "\nCheck failed in " << p << ", line " << line 
             << ",\nexpression " << expr;
  }

  return Instance;
}

bool Debug::CheckDone(void)
{
  --disableAssertsEtc;

  // did we have an active check?
  if (curType==DebugIOInterface::StringType::Check)
  {
    __ASSERT(curFrameEntry!=NULL);

    // hit info?
    if (curFrameEntry->hits>1)
      (*this) << " (hit #" << curFrameEntry->hits << ")";

    // need CR?
    if (!ioBuffer[curType].lastWasCR)
      operator<<("\n");

    // First hit? Then do a stack trace
    if (curFrameEntry->hits==1)
    {
      DebugStackwalk::Signature sig;
      if (m_stackWalk.StackWalk(sig))
        (*this) << sig;
    }

    // flush out
    operator<<("\n\n");
    FlushOutput();

    // hit too often?
    if (curFrameEntry->hits==MAX_CHECK_HITS)
    {
      // yup, turn off then
      StartOutput(DebugIOInterface::StringType::Other,"");
      Instance << "Check hit too often - turning check off.\n";
      FlushOutput();

      // build 'pattern'
      char help[200];
      __ASSERT(strlen(curFrameEntry->fileOrGroup)<190);
      wsprintf(help,"%s(%i)",curFrameEntry->fileOrGroup,
                             curFrameEntry->line);
      AddPatternEntry(FrameTypeCheck,false,help);

      curFrameEntry->status=Skip;
    }
  }

  curFrameEntry=NULL;
  return false;
}

Debug& Debug::LogBegin(const char *fileOrGroup)
{
  // avoid infinite recursion...
  ++Instance.disableAssertsEtc;

  // anything to flush first?
  if (Instance.curType!=DebugIOInterface::StringType::MAX&&
      Instance.curType!=DebugIOInterface::StringType::Log)
    Instance.FlushOutput();

  // set new output
  __ASSERT(Instance.curFrameEntry==NULL);
  Instance.curFrameEntry=Instance.GetFrameEntry(curStackFrame,FrameTypeLog,fileOrGroup,0);
  if (Instance.curFrameEntry->status==NoSkip)
  {
    ++Instance.curFrameEntry->hits;

    // we're doing all this extra work so that DLOGs can be spread across
    // multiple calls
    if (Instance.curType==DebugIOInterface::StringType::Log&&
        strcmp(Instance.curSource,Instance.curFrameEntry->fileOrGroup))
      Instance.FlushOutput();

    if (Instance.curType!=DebugIOInterface::StringType::Log)
      Instance.StartOutput(DebugIOInterface::StringType::Log,"%s",
                    Instance.curFrameEntry->fileOrGroup);
  }
  else if (Instance.curType!=DebugIOInterface::StringType::MAX)
    Instance.FlushOutput();
  
  return Instance;
}

bool Debug::LogDone(void)
{
  --disableAssertsEtc;

  // we're not flushing here on intention!

  curFrameEntry=NULL;
  return false;
}

Debug& Debug::CrashBegin(const char *file, int line)
{
  // avoid infinite recursion...
  ++Instance.disableAssertsEtc;

  // anything to flush first?
  if (Instance.curType!=DebugIOInterface::StringType::MAX)
    Instance.FlushOutput();

  // set new output
  __ASSERT(Instance.curFrameEntry==NULL);
  Instance.curFrameEntry=Instance.GetFrameEntry(curStackFrame,FrameTypeAssert,file,line);
  if (Instance.curFrameEntry->status==NoSkip)
  {
    Instance.StartOutput(DebugIOInterface::StringType::Crash,"%s(%i)",file,line);
    ++Instance.curFrameEntry->hits;

    Instance << "\n" << RepeatChar('=',80) << "\n";
    if (file)
    {
      // if there is a \code\ section in the filename truncate
      // everything before that (including \code\)
      const char *p=strstr(file,"\\code\\");
      p=p?p+6:file;

      Instance << "Crash in " << p << ", line " << line 
               << ", reason:\n";
    }
  }

  return Instance;
}

bool Debug::CrashDone(bool die)
{
  --disableAssertsEtc;

  // did we have an active assertion?
  if (curType==DebugIOInterface::StringType::Crash)
  {
    __ASSERT(curFrameEntry!=NULL);

    // hit info?
    if (curFrameEntry->hits>1)
      (*this) << " (hit #" << curFrameEntry->hits << ")";

    // need CR?
    if (!ioBuffer[curType].lastWasCR)
      operator<<("\n");

    // duplicate message
    const char *addInfo=
      "\nBecause of the severity of this error the "
      "game will now exit.";
#ifdef HAS_LOGS
    if (IsWindowed()&&!die)
      addInfo=
        "\nPress 'abort' to abort the program,\n"
        "'retry' for breaking into the debugger, or\n"
        "'ignore' for ignoring this assertion for the\n"
        "time being (stops logging this assertion as well).";
#endif
    char *help=(char *)DebugAllocMemory(ioBuffer[curType].used+strlen(addInfo)+1);
    strcpy(help,ioBuffer[curType].buffer+82);
    strcat(help,addInfo);
    
    // First hit? Then do a stack trace
    if (curFrameEntry->hits==1)
    {
      DebugStackwalk::Signature sig;
      if (m_stackWalk.StackWalk(sig))
        (*this) << sig;
    }

    // ... and flush out
    operator<<("\n\n");
    FlushOutput();

    // now display message, wait for user input
#ifdef HAS_LOGS
    // show dialog box only if running windowed
    if (!die)
    {
      if (IsWindowed())
      {
        /// @todo replace MessageBox with custom dialog w/ 4 options: abort, skip 1, skip all, break

        // now display message, wait for user input
        int result=MessageBox(NULL,help,"Crash hit",
                              MB_ABORTRETRYIGNORE|MB_ICONSTOP|MB_TASKMODAL|MB_SETFOREGROUND);
        switch(result)
        {
          case IDABORT:
            curFrameEntry=NULL;
            exit(1);
            break;
          case IDIGNORE:
            {
              // build 'pattern'
              char help[200];
              __ASSERT(strlen(curFrameEntry->fileOrGroup)<190);
              wsprintf(help,"%s(%i)",curFrameEntry->fileOrGroup,
                                     curFrameEntry->line);
              AddPatternEntry(FrameTypeAssert,false,help);
              curFrameEntry->status=Skip;
            }
            break;
          case IDRETRY:
            _asm int 0x03
            break;
          default:
            ((void)0);
        }
      }
      else
      {
        // running fullscreen...

        // hit too often?
        if (curFrameEntry->hits==MAX_CHECK_HITS)
        {
          // yup, turn off then
          StartOutput(DebugIOInterface::StringType::Other,"");
          Instance << "Crash hit too often - turning check off.\n";
          FlushOutput();

          // build 'pattern'
          char help[200];
          __ASSERT(strlen(curFrameEntry->fileOrGroup)<190);
          wsprintf(help,"%s(%i)",curFrameEntry->fileOrGroup,
                                 curFrameEntry->line);
          AddPatternEntry(FrameTypeAssert,false,help);

          curFrameEntry->status=Skip;
        }
      }
    }
    else
#endif
    {
      MessageBox(NULL,help,"Game crash",
                          MB_OK|MB_ICONSTOP|MB_TASKMODAL|MB_SETFOREGROUND);
      curFrameEntry=NULL;
      _exit(1);
    }
  }

  curFrameEntry=NULL;
  return false;
}

Debug& Debug::operator<<(const char *str)
{
  if (curType==DebugIOInterface::StringType::MAX)
    // yes, this is valid and simply means not to
    // write anything...
    return *this;

  // buffer large enough?
  if (!str)
    str="[NULL]";
  else if (!*str)
    return *this;

  unsigned len=strlen(str);

  // forced width?
  if (len<m_width)
  {
    for (unsigned k=len;k<m_width;k++)
      AddOutput(&m_fillChar,1);
  }

  // reset width after each insertion
  m_width=0;

  AddOutput(str,len);

  return *this;
}

void Debug::SetPrefixAndRadix(const char *prefix, int radix)
{
  strncpy(m_prefix,prefix?prefix:"",sizeof(m_prefix)-1);
  m_prefix[sizeof(m_prefix)-1]=0;
  m_radix=radix;
}

Debug& Debug::operator<<(int val)
{
  // usually having a fixed size buffer and a function
  // that doesn't check for buffer overflow isn't a good idea
  // but in this case we know how long it can be at max...
  char help[1+32+1]; // sign, 32 digits (binary), NUL
  AddOutput(m_prefix,strlen(m_prefix));
  return (*this) << _itoa(val,help,m_radix);
}

Debug& Debug::operator<<(unsigned val)
{
  // usually having a fixed size buffer and a function
  // that doesn't check for buffer overflow isn't a good idea
  // but in this case we know how long it can be at max...
  char help[32+1]; // 32 digits, NUL
  AddOutput(m_prefix,strlen(m_prefix));
  return (*this) << _ultoa(val,help,m_radix);
}

Debug& Debug::operator<<(long val)
{
  // usually having a fixed size buffer and a function
  // that doesn't check for buffer overflow isn't a good idea
  // but in this case we know how long it can be at max...
  char help[1+32+1]; // sign, 32 digits, NUL
  AddOutput(m_prefix,strlen(m_prefix));
  return (*this) << _itoa(val,help,m_radix);
}

Debug& Debug::operator<<(unsigned long val)
{
  // usually having a fixed size buffer and a function
  // that doesn't check for buffer overflow isn't a good idea
  // but in this case we know how long it can be at max...
  char help[32+1]; // 32 digits, NUL
  AddOutput(m_prefix,strlen(m_prefix));
  return (*this) << _ultoa(val,help,m_radix);
}

Debug& Debug::operator<<(bool val)
{
  return (*this) << (val?"true":"false");
}

Debug& Debug::operator<<(float val)
{
  /// @todo_opt shouldn't use snprintf here - brings in most of the old C IO lib...
  char help[200];
  _snprintf(help,sizeof(help),"%f",val);
  return (*this) << help;
}

Debug& Debug::operator<<(double val)
{
  /// @todo_opt shouldn't use snprintf here - brings in most of the old C IO lib...
  char help[200];
  _snprintf(help,sizeof(help),"%f",val);
  return (*this) << help;
}

Debug& Debug::operator<<(short val)
{
  // usually having a fixed size buffer and a function
  // that doesn't check for buffer overflow isn't a good idea
  // but in this case we know how long it can be at max...
  char help[1+16+1]; // sign, 16 digits, NUL
  AddOutput(m_prefix,strlen(m_prefix));
  return (*this) << _itoa(val,help,m_radix);
}

Debug& Debug::operator<<(unsigned short val)
{
  // usually having a fixed size buffer and a function
  // that doesn't check for buffer overflow isn't a good idea
  // but in this case we know how long it can be at max...
  char help[16+1]; // 16 digits, NUL
  AddOutput(m_prefix,strlen(m_prefix));
  return (*this) << _itoa(val,help,m_radix);
}

Debug& Debug::operator<<(__int64 val)
{
  // usually having a fixed size buffer and a function
  // that doesn't check for buffer overflow isn't a good idea
  // but in this case we know how long it can be at max...
  char help[1+64+1]; // sign, 64 digits, NUL
  AddOutput(m_prefix,strlen(m_prefix));
  return (*this) << _i64toa(val,help,m_radix);
}

Debug& Debug::operator<<(unsigned __int64 val)
{
  // usually having a fixed size buffer and a function
  // that doesn't check for buffer overflow isn't a good idea
  // but in this case we know how long it can be at max...
  char help[64+1]; // sign, 64 digits, NUL
  AddOutput(m_prefix,strlen(m_prefix));
  return (*this) << _ui64toa(val,help,m_radix);
}

Debug& Debug::operator<<(const void *ptr)
{
  (*this) << "ptr:";
  if (ptr)
  {
    char help[9];
    (*this) << "0x" << _ultoa((unsigned long)ptr,help,16);
  }
  else
    (*this) << "NULL";
  return *this;
}

Debug& Debug::operator<<(const MemDump &dump)
{
  if (curType==DebugIOInterface::StringType::MAX)
    return *this;

  // need CR?
  if (!ioBuffer[curType].lastWasCR)
    operator<<("\n");

  // How many items per line? We're assuming an output
  // width of 73 chars. Left border is address thus
  // leaving 65 chars effectively. If character dump is
  // enabled then an additional space is needed (64 chars
  // then).
  unsigned itemPerLine=(dump.m_withChars?64:65)/
                          (1+2*dump.m_bytePerItem+(dump.m_withChars?1:0));
  if (!itemPerLine)
    itemPerLine=1;

  // now dump line by line
  const unsigned char *cur=dump.m_startPtr;
  for (unsigned i=0;i<dump.m_numItems;i+=itemPerLine,cur+=itemPerLine*dump.m_bytePerItem)
  {
    // address
    char buf[9];
    sprintf(buf,"%08x",dump.m_absAddr?unsigned(cur):cur-dump.m_startPtr);
    operator<<(buf);

    // items
    const unsigned char *curByte=cur;
    for (unsigned k=0;k<itemPerLine;k++,curByte+=dump.m_bytePerItem)
    {
      operator<<(" ");

      if (k+i>=dump.m_numItems)
      {
        for (unsigned l=dump.m_bytePerItem;l;--l)
          operator<<("  ");
      }
      else if (IsBadReadPtr(curByte,dump.m_bytePerItem))
      {
        for (unsigned l=dump.m_bytePerItem;l;--l)
          operator<<("??");
      }
      else
      {
        curByte+=dump.m_bytePerItem;
        for (unsigned l=0;l<dump.m_bytePerItem;++l)
        {
          sprintf(buf,"%02x",*--curByte);
          operator<<(buf);
        }
      }
    }

    // characters
    if (!dump.m_withChars)
      continue;
    operator<<(" ");
    curByte=cur;
    for (k=0;k<itemPerLine;k++,curByte+=dump.m_bytePerItem)
    {
      if (k+i>=dump.m_numItems)
        break;
      else if (IsBadReadPtr(curByte,dump.m_bytePerItem))
      {
        for (unsigned l=dump.m_bytePerItem;l;--l)
          operator<<("?");
      }
      else
      {
        buf[1]=0;
        for (unsigned l=0;l<dump.m_bytePerItem;++l)
        {
          *buf=curByte[l]>' '?curByte[l]:'.';
          operator<<(buf);
        }
      }
    }

    operator<<("\n");
  }

  return *this;
}

Debug& Debug::operator<<(HResult hres)
{
  for (unsigned k=0;k<numHrTranslators;k++)
    if (hrTranslators[k].func(*this,hres.m_hresult,hrTranslators[k].user))
      return *this;
  (*this) << "HResult:0x";
  char help[9];
  return (*this) << _ultoa(hres.m_hresult,help,16);
}

bool Debug::IsLogEnabled(const char *fileOrGroup)
{
  // now this isn't great but since IsLogEnabled is supposed
  // to be used from the D_ISLOG macros only and those guarantee
  // that we are having real static strings let's use
  // that strings address as frame address...
  FrameHashEntry *e=Instance.LookupFrame((unsigned)fileOrGroup);
  if (!e)
    e=Instance.AddFrameEntry((unsigned)fileOrGroup,FrameTypeLog,fileOrGroup,0);
  if (e->status==Unknown)
    Instance.UpdateFrameStatus(*e);
  return e->status==NoSkip;
}

void Debug::AddHResultTranslator(unsigned prio, HResultTranslator func, void *user)
{
  // bail out if invalid parameter passed in
  if (!func)
    return;

  // just remove it first (if it's not in there nothing is done)
  // necessary in case we want to 'change' the priority of an
  // existing HR translator
  RemoveHResultTranslator(func,user);

  // now find the right place to insert the translator
  // (slow but this function is not time critical)
  for (unsigned k=0;k<Instance.numHrTranslators;++k)
    if (Instance.hrTranslators[k].prio<prio)
      break;

  // grow & move
  Instance.hrTranslators=(HResultTranslatorEntry *)
    DebugReAllocMemory(Instance.hrTranslators,(Instance.numHrTranslators+1)*sizeof(void *));
  memmove(Instance.hrTranslators+k+1,Instance.hrTranslators+k,(Instance.numHrTranslators-k)*sizeof(void *));

  // add new
  ++Instance.numHrTranslators;
  Instance.hrTranslators[k].prio=prio;
  Instance.hrTranslators[k].func=func;
  Instance.hrTranslators[k].user=user;
}

void Debug::RemoveHResultTranslator(HResultTranslator func, void *user)
{
  // bail out if invalid parameter passed in
  if (!func)
    return;

  // look for func/user pair
  for (unsigned k=0;k<Instance.numHrTranslators;++k)
    if (Instance.hrTranslators[k].func==func&&
        Instance.hrTranslators[k].user==user)
    {
      // remove it
      memmove(Instance.hrTranslators+k,Instance.hrTranslators+k+1,
        (Instance.numHrTranslators-k-1)*sizeof(void *));
      --Instance.hrTranslators;
      Instance.hrTranslators=(HResultTranslatorEntry *)
        DebugReAllocMemory(Instance.hrTranslators,Instance.numHrTranslators*sizeof(void *));
    }
}

bool Debug::AddIOFactory(const char *io_id, const char *descr, DebugIOInterface* (*func)(void))
{
  // bail out if invalid parameters passed in
  if (!io_id||!func)
    return true;
  
  // allocate & init new list entry
  IOFactoryListEntry *entry=(IOFactoryListEntry *)
                      DebugAllocMemory(sizeof(IOFactoryListEntry));
  entry->next=Instance.firstIOFactory;
  entry->ioID=io_id;
  entry->descr=descr;
  entry->factory=func;
  entry->io=NULL;
  entry->input=NULL;
  entry->inputAlloc=0;
  entry->inputUsed=0;

  // add to list
  Instance.firstIOFactory=entry;

  return true;
}

bool Debug::AddCommands(const char *cmdgroup, DebugCmdInterface *cmdif)
{
  // bail out if invalid parameters passed in
  if (!cmdgroup||!cmdif)
    return true;

  // walk to end of list, add there (unless interface pointer already in list)
  CmdInterfaceListEntry **listptr=&Instance.firstCmdGroup;
  while (*listptr)
  {
    if ((*listptr)->cmdif==cmdif)
      // interface already in list, don't add twice
      return true;
    listptr=&((*listptr)->next);
  }
  
  // allocate & init new list entry
  CmdInterfaceListEntry *entry=(CmdInterfaceListEntry *)
                      DebugAllocMemory(sizeof(CmdInterfaceListEntry));
  entry->next=NULL;
  entry->group=cmdgroup;
  entry->cmdif=cmdif;

  // add to list
  *listptr=entry;

  return true;
}

void Debug::RemoveCommands(DebugCmdInterface *cmdif)
{
  // bail out if invalid parameter passed in
  if (!cmdif)
    return;

  // walk the list, search for interface pointer
  CmdInterfaceListEntry **listptr=&Instance.firstCmdGroup;
  while (*listptr)
  {
    if ((*listptr)->cmdif==cmdif)
    {
      // found it, now remove it
      CmdInterfaceListEntry *cur=*listptr;
      *listptr=cur->next;

      // free list entry
      DebugFreeMemory(cur);

      // done
      break;
    }
    listptr=&((*listptr)->next);
  }
}

void Debug::Command(const char *cmd)
{
  DFAIL_IF(!cmd) return;
  Instance.ExecCommand(cmd,cmd+strlen(cmd));
}

void Debug::Update(void)
{
  // check all existing IO interfaces
  for (IOFactoryListEntry *cur=Instance.firstIOFactory;cur;cur=cur->next)
  {
    if (!cur->io)
      continue;

    // any input?
    bool hadInput=false;
    for (;;)
    {
      if (cur->inputAlloc-cur->inputUsed<64)
        // must grow input buffer...
        cur->input=(char *)DebugReAllocMemory(cur->input,(cur->inputAlloc+=64)+1);
      int numChars=cur->io->Read(cur->input+cur->inputUsed,cur->inputAlloc-cur->inputUsed);
      if (!numChars)
        break;

      cur->inputUsed+=numChars;
      cur->input[cur->inputUsed]=0;
      hadInput=true;
    } 
    
    if (!hadInput)
      // skip then
      continue;

    // else look for completed commands and try to process them
    for (;;)
    {
      char *p=strchr(cur->input,'\n');
      if (!p)
        break;

      Instance.ExecCommand(cur->input,p);
      strcpy(cur->input,p+1);
      cur->inputUsed=strlen(cur->input);
    }
  }
}

Debug::FrameHashEntry* Debug::AddFrameEntry(unsigned addr, unsigned type,
                                            const char *fileOrGroup, int line)
{
  __ASSERT(LookupFrame(addr)==NULL);

  // get new entry
  if (!numAvailableFrameHash)
  {
    numAvailableFrameHash=FRAME_HASH_ALLOC_COUNT;
    nextUnusedFrameHash=(FrameHashEntry *)
      DebugAllocMemory(numAvailableFrameHash*sizeof(FrameHashEntry));
  }
  FrameHashEntry *e=nextUnusedFrameHash++;
  --numAvailableFrameHash;

  // fill entry
  e->next=frameHash[addr%FRAME_HASH_SIZE];
  e->frameAddr=addr;
  e->frameType=type;
  e->line=line;
  e->status=Unknown;
  e->hits=0;

  // log?
  if (type&FrameTypeLog)
  {
    // must add to list of known logs,
    // store translated name 
    e->fileOrGroup=AddLogGroup(fileOrGroup,NULL);
  }
  else
  {
    // no, just add file name (without path though)
    e->fileOrGroup=fileOrGroup?strrchr(fileOrGroup,'\\'):NULL;
    e->fileOrGroup=e->fileOrGroup?e->fileOrGroup+1:fileOrGroup;
  }

  // add to hash
  frameHash[addr%FRAME_HASH_SIZE]=e;
  return e;
}

void Debug::UpdateFrameStatus(FrameHashEntry &entry)
{
  // build pattern match entry
  char help[512];
  if (entry.frameType==FrameTypeAssert||
      entry.frameType==FrameTypeCheck)
    wsprintf(help,"%s(%i)",entry.fileOrGroup,entry.line);
  else
    strcpy(help,entry.fileOrGroup);
  
  // update frame status
  bool active=entry.frameType!=FrameTypeLog;
  for (PatternListEntry *cur=firstPatternEntry;cur;cur=cur->next)
  {
    if (!(cur->frameTypes&entry.frameType))
      continue;
    if (SimpleMatch(help,cur->pattern))
      active=cur->isActive;
  }
  entry.status=active?NoSkip:Skip;
}

const char *Debug::AddLogGroup(const char *fileOrGroup, const char *descr)
{
  // helper buffer for stripping down fileOrGroup
  char help[200];

  // do we need to strip down fileOrGroup?
  const char *p=strrchr(fileOrGroup,'\\');
  const char *q=strchr(p?p:fileOrGroup,'.');
  if (p||q)
  {
    // this extracts everything beyond the last backslash
    // up to the first dot
    p=p?p+1:fileOrGroup;
    if (!q) q=p+strlen(p);
    if (q-p>=sizeof(help))
      q=p+sizeof(help)-1;
    memcpy(help,p,q-p);
    help[q-p]=0;
    fileOrGroup=help;
  }

  // is that log group known?
  for (KnownLogGroupList *cur=firstLogGroup;cur;cur=cur->next)
  {
    if (!strcmp(cur->nameGroup,fileOrGroup))
    {
      // yes, return translated name
      return cur->nameGroup;
    }
  }

  // no, add new entry
  cur=(KnownLogGroupList *)DebugAllocMemory(sizeof(KnownLogGroupList));
  cur->next=firstLogGroup;
  cur->nameGroup=(char *)DebugAllocMemory(strlen(fileOrGroup)+1);
  strcpy(cur->nameGroup,fileOrGroup);
  cur->descr=descr;
  firstLogGroup=cur;
  return cur->nameGroup;
}

void Debug::StartOutput(DebugIOInterface::StringType type, const char *fmt, ...)
{
  if (curType==DebugIOInterface::Log)
    FlushOutput();
  __ASSERT(curType==DebugIOInterface::StringType::MAX);
  curType=type;

  // potentially dangerous (fixed string buffer...)
  va_list va;
  va_start(va,fmt);
  wvsprintf(curSource,fmt,va);
  va_end(va);
  __ASSERT(curSource[sizeof(curSource)-1]==0);
}

void Debug::AddOutput(const char *str, unsigned remainingLen)
{
  // bail out if no valid destination type
  // (valid, can happen if hitting a disabled log for the first time)
  if (curType==DebugIOInterface::StringType::MAX)
    return;

  while (remainingLen)
  {
    // if we're doing timestamps we have to split at each '\n'
    unsigned len;
    if (timeStamp)
    {
      // add timestamp now?
      if (ioBuffer[curType].lastWasCR)
      {
        SYSTEMTIME systime;
        GetLocalTime(&systime);

        char ts[40];
        wsprintf(ts,"[%02i:%02i.%02i.%03i] ",systime.wHour,systime.wMinute,
                      systime.wSecond,systime.wMilliseconds);

        unsigned tsLen=strlen(ts);
        memcpy(ioBuffer[curType].buffer+ioBuffer[curType].used,ts,tsLen+1);
        ioBuffer[curType].used+=tsLen;
      }

      // search for next '\n'
      const char *p=strchr(str,'\n');
      p=p?p+1:str+remainingLen;
      len=p-str;
    }
    else
      len=remainingLen;

    if (ioBuffer[curType].used+len+64>=ioBuffer[curType].alloc)
    {
      // no, must grow buffer
      ioBuffer[curType].alloc+=len+1024;
      ioBuffer[curType].buffer=(char *)
          DebugReAllocMemory(ioBuffer[curType].buffer,ioBuffer[curType].alloc);
    }

    // add to buffer (with NUL)
    memcpy(ioBuffer[curType].buffer+ioBuffer[curType].used,str,len+1);
    ioBuffer[curType].used+=len;

    // last char CR?
    ioBuffer[curType].lastWasCR=str[len-1]=='\n';
    str+=len;
    remainingLen-=len;

    // are we writing a log string?
    if (curType==DebugIOInterface::Log&&ioBuffer[curType].lastWasCR)
    {
      // yes, flush out now
      FlushOutput();
      curType=DebugIOInterface::Log;
    }
  }
}

void Debug::FlushOutput(bool defaultLog)
{
  __ASSERT(curType!=DebugIOInterface::StringType::MAX);

  // bail out early if buffer is still empty
  if (!ioBuffer[curType].used)
  {
    curType=DebugIOInterface::StringType::MAX;
    return;
  }

  // need CR?
  if (!ioBuffer[curType].lastWasCR)
    operator<<("\n");

  // send string to all active I/O interfaces
  bool hadWrite=!defaultLog;
  for (IOFactoryListEntry *cur=firstIOFactory;cur;cur=cur->next)
  {
    if (!cur->io)
      continue;

    hadWrite=true;
    cur->io->Write(curType,curSource,ioBuffer[curType].buffer);

    if (alwaysFlush)
      cur->io->Write(curType,curSource,NULL);
  }

  // written nowhere?
  if (!hadWrite&&curType!=DebugIOInterface::StringType::StructuredCmdReply)
  {
#ifdef HAS_LOGS
    // then force output to a very simple default log file
    // (non-Release builds only)
    HANDLE h=CreateFile("default.log",GENERIC_WRITE,0,NULL,
                        OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    SetFilePointer(h,0,NULL,FILE_END);
    DWORD dwDummy;
    WriteFile(h,ioBuffer[curType].buffer,strlen(ioBuffer[curType].buffer),&dwDummy,NULL);
    CloseHandle(h);
#endif
  }

  // empty buffer etc.
  ioBuffer[curType].used=0;
  *ioBuffer[curType].buffer=0;
  curType=DebugIOInterface::StringType::MAX;
  *curSource=0;
}

void Debug::AddPatternEntry(unsigned types, bool isActive, const char *pattern)
{
  __ASSERT(pattern);

  // alloc new pattern entry
  PatternListEntry *cur=(PatternListEntry *)
      DebugAllocMemory(sizeof(PatternListEntry));

  // init
  cur->next=NULL;
  cur->frameTypes=types;
  cur->isActive=isActive;
  cur->pattern=(char *)DebugAllocMemory(strlen(pattern)+1);
  strcpy(cur->pattern,pattern);

  // add to list
  if (lastPatternEntry)
    lastPatternEntry->next=cur;
  else
    firstPatternEntry=cur;
  lastPatternEntry=cur;
}

bool Debug::SimpleMatch(const char *str, const char *pattern)
{
  __ASSERT(str);
  __ASSERT(pattern);
  while (*str&&*pattern)
  {
    if (*pattern=='*')
    {
      pattern++;
      while (*str)
        if (SimpleMatch(str++,pattern))
          return true;
      return *str==*pattern;
    }
    else 
    {
      if (*str++!=*pattern++)
        return false;
    }
  }

  return *str==*pattern;
}

void Debug::SetBuildInfo(const char *version,
                         const char *internalVersion,
                         const char *buildDate)
{
  if (version)
    strncpy(Instance.m_version,version,sizeof(Instance.m_version)-1);
  if (internalVersion)
    strncpy(Instance.m_intVersion,internalVersion,sizeof(Instance.m_intVersion)-1);
  if (buildDate)
    strncpy(Instance.m_buildDate,buildDate,sizeof(Instance.m_buildDate)-1);
}

void Debug::WriteBuildInfo(void)
{
  operator<<("Version:");
  if (*m_version)
    (*this) << " " << m_version;
  if (*m_intVersion)
    (*this) << " internal " << m_intVersion;
  #if defined(_INTERNAL)
    operator<<(" internal");
  #elif defined(_DEBUG)
    operator<<(" debug");
  #elif defined(_PROFILE)
    operator<<(" profile");
  #else
    operator<<(" release");
  #endif
  if (*m_buildDate)
    (*this) << " build " << m_buildDate;
}

void Debug::ExecCommand(const char *cmdstart, const char *cmdend)
{
  // split off into command and arguments

  // alloc & copy string
  char *strbuf=(char *)DebugAllocMemory(cmdend-cmdstart+1);
  memcpy(strbuf,cmdstart,cmdend-cmdstart);
  strbuf[cmdend-cmdstart]=0;

  // for simplicity I'm using a fixed size argv array here...
  // if there are more arguments given than we have we're
  // just dropping the excess arguments
  char *parts[100];
  int numParts=0;
  char *lastNonWhitespace=NULL;
  char *cur=strbuf;

  // regular reply or structured reply?
  DebugIOInterface::StringType reply;
  DebugCmdInterface::CommandMode mode;
  if (*cur=='!')
  {
    cur++;
    reply=DebugIOInterface::StringType::StructuredCmdReply;
    mode=DebugCmdInterface::CommandMode::Structured;
  }
  else
  {
    reply=DebugIOInterface::StringType::CmdReply;
    mode=DebugCmdInterface::CommandMode::Normal;
  }

  for (;;)
  {
    if (!lastNonWhitespace&&(*cur=='\''||*cur=='"'))
    {
      char quote=*cur++;

      if (numParts<sizeof(parts)/sizeof(*parts))
        parts[numParts++]=cur;

      while (*cur&&*cur!=quote)
        ++cur;
      if (*cur)
        *cur++=0;
    }
    else if (*cur==' '||*cur=='\t'||!*cur||*cur==';')
    {
      if (*cur==';')
        *cur=0;
      if (lastNonWhitespace)
      {
        if (numParts<sizeof(parts)/sizeof(*parts))
          parts[numParts++]=lastNonWhitespace;
        lastNonWhitespace=NULL;
        if (*cur)
          *cur++=0;
      }
      else if (*cur)
        ++cur;
      else
        break;
    }
    else
    {
      if (!lastNonWhitespace)
        lastNonWhitespace=cur;
      ++cur;
    }
  }

  if (numParts)
  {
    // part[0] is the command, part[1..numParts] are arguments

    // split off command group (if any)
    char *p=strchr(parts[0],'.');
    if (p&&p-parts[0]<sizeof(curCommandGroup))
    {
      memcpy(curCommandGroup,parts[0],p-parts[0]);
      curCommandGroup[p-parts[0]]=0;
      ++p;
    }
    else
      p=parts[0];

    StartOutput(reply,"%s.%s",curCommandGroup,p);

    if (mode!=DebugCmdInterface::CommandMode::Structured)
      AddOutput("> ",2);
  
    // repeat current command first
    AddOutput(cmdstart,cmdend-cmdstart);
    AddOutput("\n",1);

    // command group known?
    for (CmdInterfaceListEntry *cur=firstCmdGroup;cur;cur=cur->next)
      if (!strcmp(curCommandGroup,cur->group))
        break;
    if (!cur)
    {
      // nope, show error message
      (*this) << "Unknown command group " << curCommandGroup;
      *p=0;
    }

    if (*p)
    {
      // must have command...

      // search for a matching command handler
      for (CmdInterfaceListEntry *cur=firstCmdGroup;cur;cur=cur->next)
      {
        if (strcmp(curCommandGroup,cur->group))
          continue;

        bool doneCommand=cur->cmdif->Execute(*this,p,mode,numParts-1,parts+1);
        if (doneCommand&&(strcmp(p,"help")||numParts>1))
          break;
      }

      // display error message if command not found, break away
      if (!cur&&mode==DebugCmdInterface::CommandMode::Normal)
      {
        if (strcmp(p,"help"))
          operator<<("Unknown command");
        else if (numParts>1)
          operator<<("Unknown command, help not available");
      }
    }

    // flush output only if there is already an active I/O class
    FlushOutput(false);
  }

  // cleanup
  DebugFreeMemory(strbuf);
}

// little helper to get app window
static BOOL CALLBACK EnumThreadWndProc(HWND hwnd, LPARAM lParam)
{
  *(HWND *)lParam=hwnd;
  return FALSE;
}

bool Debug::IsWindowed(void)
{
  // use cached result if possible
  if (m_isWindowed)
    return m_isWindowed>0;

  // find main app window
  HWND appHWnd=NULL;
  EnumThreadWindows(GetCurrentThreadId(),EnumThreadWndProc,(LPARAM)&appHWnd);
  if (!appHWnd)
  {
    // couldn't find main window, assume we're windowed anyway
    m_isWindowed=1;
    return true;
  }

  // we assume full screen if WS_CAPTION is not set
  m_isWindowed=(GetWindowLong(appHWnd,GWL_STYLE)&WS_CAPTION)?1:-1;
  return m_isWindowed>0;
}

//////////////////////////////////////////////////////////////////////////////

// And finally for a little list of C/C++ runtime replacement functions.

// Abort process due to fatal heap error
void __cdecl _heap_abort(void)
{
  DCRASH_RELEASE("Fatal heap error.");
}
