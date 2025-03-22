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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_stack.cpp $
// $Author: KMorness $
// $Revision: #2 $
// $DateTime: 2005/01/19 15:02:33 $
//
// (c) 2003 Electronic Arts
//
// Stack walker
//////////////////////////////////////////////////////////////////////////////
#include "_pch.h"
#include <imagehlp.h>

// Definitions to allow run-time linking to the dbghelp.dll functions.

#define DBGHELP(name,ret,par) typedef ret (WINAPI *name##Type) par;
#include "debug_stack.inl"
#undef DBGHELP

#define DBGHELP(name,ret,par) name##Type _##name;
static union 
{
  struct  
  {
#include "debug_stack.inl"
  };
  unsigned funcPtr[1];
} gDbg;
#undef DBGHELP

#define DBGHELP(name,ret,par) #name,
static char const *DebughelpFunctionNames[] =
{
#include "debug_stack.inl"
	NULL
};
#undef DBGHELP

// local dbghelp.dll module handle
static HMODULE g_dbghelp;

// local flag that is true if we're using an old dbghelp.dll version
static bool g_oldDbghelp;

static void InitDbghelp(void)
{
  // already called?
  if (g_dbghelp)
    return;

	// firstly check for dbghelp.dll in the EXE directory
	char dbgHelpPath[256];
	if (GetModuleFileName(NULL,dbgHelpPath,sizeof(dbgHelpPath)))
	{
		char *slash=strrchr(dbgHelpPath,'\\');
		if (slash)
		{
			strcpy(slash+1,"DBGHELP.DLL");
			g_dbghelp=::LoadLibrary(dbgHelpPath);
		}
	}
	if (!g_dbghelp)
		// load any version we can
		g_dbghelp=::LoadLibrary("DBGHELP.DLL");
  
  if (!g_dbghelp)
    return;

  // Get function addresses
  unsigned *funcptr=gDbg.funcPtr;
  for (unsigned k=0;DebughelpFunctionNames[k];++k,++funcptr)
  {
    *funcptr=(unsigned)GetProcAddress(g_dbghelp,DebughelpFunctionNames[k]);
    if (!*funcptr)
      break;
  }
  if (DebughelpFunctionNames[k])
  {
    // not all functions found -> clear them all
    while (funcptr!=gDbg.funcPtr)
      *--funcptr=NULL;
  }
  else
  {
    // Set options
    gDbg._SymSetOptions(gDbg._SymGetOptions()|SYMOPT_DEFERRED_LOADS|SYMOPT_LOAD_LINES);

    // Init module
    gDbg._SymInitialize((HANDLE)GetCurrentProcessId(),NULL,TRUE);

    // Check: are we using a newer version of dbghelp.dll?
    // (older versions have some serious issues.. err... bugs)
    if (!GetProcAddress(g_dbghelp,"SymEnumSymbolsForAddr"))
      g_oldDbghelp=true;
  }
}

//////////////////////////////////////////////////////////////////////////////

DebugStackwalk::Signature::Signature(const Signature &src)
{
  *this=src;
}

DebugStackwalk::Signature& DebugStackwalk::Signature::operator=(const Signature& src)
{
  if (&src!=this)
  {
    m_numAddr=src.m_numAddr;
    memcpy(m_addr,src.m_addr,m_numAddr*sizeof(*m_addr));
  }
  return *this;
}

unsigned DebugStackwalk::Signature::GetAddress(int n) const
{
  DFAIL_IF_MSG(n<0||n>=MAX_ADDR,n << "/" << MAX_ADDR) return 0;
  return m_addr[n];
}

void DebugStackwalk::Signature::GetSymbol(unsigned addr, char *buf, unsigned bufSize) 
{
  DFAIL_IF(!buf) return;
  DFAIL_IF(bufSize<64||bufSize>=0x80000000) return;

  InitDbghelp();

  char *bufEnd=buf+bufSize;
  *buf=0;
  buf+=wsprintf(buf,"%08x",addr);

  // determine module
  unsigned modBase=gDbg._SymGetModuleBase((HANDLE)GetCurrentProcessId(),addr);
  if (!modBase)
	{
		strcpy(buf," (unknown module)");
    return;
	}

  // illegal code ptr?
	if (IsBadReadPtr((void *)addr,4)||IsBadCodePtr((FARPROC)addr))
	{
		strcpy(buf," (invalid code addr)");
		return;
	}

  char symbolBuffer[512];
  GetModuleFileName((HMODULE)modBase,symbolBuffer,sizeof(symbolBuffer));

  char *p=strrchr(symbolBuffer,'\\'); // use filename only, strip off path
  p=p?p+1:symbolBuffer;
  *buf++=' ';
  strcpy(buf,p);
  buf+=strlen(buf);
  if (bufEnd-buf<32)
    return;
  buf+=wsprintf(buf,"+0x%x",addr-modBase);

  // determine symbol
  PIMAGEHLP_SYMBOL symPtr=(PIMAGEHLP_SYMBOL)symbolBuffer;
  memset(symPtr,0,sizeof(symbolBuffer));
  symPtr->SizeOfStruct=sizeof(IMAGEHLP_SYMBOL);
  symPtr->MaxNameLength=sizeof(symbolBuffer)-sizeof(IMAGEHLP_SYMBOL);
  DWORD displacement;
  if (!gDbg._SymGetSymFromAddr((HANDLE)GetCurrentProcessId(),addr,&displacement,symPtr))
    return;
  if ((unsigned int)(bufEnd-buf)<strlen(symPtr->Name)+16)
    return;
  buf+=wsprintf(buf,", %s+0x%x",symPtr->Name,displacement);

  // and line number
  IMAGEHLP_LINE line;
  memset(&line,0,sizeof(line));
  line.SizeOfStruct=sizeof(line);
  if (!gDbg._SymGetLineFromAddr((HANDLE)GetCurrentProcessId(),addr,&displacement,&line))
    return;

  p=strrchr(line.FileName,'\\'); // use filename only, strip off path
  p=p?p+1:line.FileName;

  if ((unsigned int)(bufEnd-buf)<strlen(p)+16)
    return;
  buf+=wsprintf(buf,", %s:%i+0x%x",p,line.LineNumber,displacement);
}

void DebugStackwalk::Signature::GetSymbol(unsigned addr,
                                          char *bufMod, unsigned sizeMod, unsigned *relMod,
                                          char *bufSym, unsigned sizeSym, unsigned *relSym,
                                          char *bufFile, unsigned sizeFile, unsigned *linePtr, unsigned *relLine)
{
  InitDbghelp();

  if (bufMod) *bufMod=0;
  if (relMod) *relMod=0;
  if (bufSym) *bufSym=0;
  if (relSym) *relSym=0;
  
  if (bufFile) *bufFile=0;
  if (linePtr) *linePtr=0;
  if (relLine) *relLine=0;

  DFAIL_IF(bufMod&&sizeMod<16) return;
  DFAIL_IF(bufSym&&sizeSym<16) return;
  DFAIL_IF(bufFile&&sizeFile<16) return;

  // determine module
  unsigned modBase=gDbg._SymGetModuleBase((HANDLE)GetCurrentProcessId(),addr);
  if (!modBase)
	{
    if (bufMod)
		  strcpy(bufMod,"(unknown mod)");
    if (bufSym)
      strcpy(bufSym,"(unknown)");
    return;
	}

  // illegal code ptr?
	if (IsBadReadPtr((void *)addr,4)||IsBadCodePtr((FARPROC)addr))
	{
    if (bufMod)
		  strcpy(bufMod,"(inv code addr)");
    if (bufSym)
      strcpy(bufSym,"(unknown)");
		return;
	}

  char symbolBuffer[512];
  if (bufMod)
  {
    GetModuleFileName((HMODULE)modBase,symbolBuffer,sizeof(symbolBuffer));

    char *p=strrchr(symbolBuffer,'\\'); // use filename only, strip off path
    p=p?p+1:symbolBuffer;
    strncpy(bufMod,p,sizeMod);
    bufMod[sizeMod-1]=0;
  }
  if (relMod)
    *relMod=addr-modBase;

  // determine symbol
  if (bufSym)
  {
    PIMAGEHLP_SYMBOL symPtr=(PIMAGEHLP_SYMBOL)symbolBuffer;
    memset(symPtr,0,sizeof(symbolBuffer));
    symPtr->SizeOfStruct=sizeof(IMAGEHLP_SYMBOL);
    symPtr->MaxNameLength=sizeof(symbolBuffer)-sizeof(IMAGEHLP_SYMBOL);
    DWORD displacement;
    if (gDbg._SymGetSymFromAddr((HANDLE)GetCurrentProcessId(),addr,&displacement,symPtr))
    {
      strncpy(bufSym,symPtr->Name,sizeSym);
      bufSym[sizeSym-1]=0;
      if (relSym)
        *relSym=displacement;
    }
    else 
      strcpy(bufSym,"(unknown)");
  }

  // and line number
  if (bufFile)
  {
    IMAGEHLP_LINE line;
    memset(&line,0,sizeof(line));
    line.SizeOfStruct=sizeof(line);
    DWORD displacement;
    if (!gDbg._SymGetLineFromAddr((HANDLE)GetCurrentProcessId(),addr,&displacement,&line))
      strcpy(bufFile,"(unknown)");
    else
    {
      char *p=strrchr(line.FileName,'\\'); // use filename only, strip off path
      p=p?p+1:line.FileName;
      strncpy(bufFile,p,sizeFile);
      bufFile[sizeFile-1]=0;
      if (linePtr)
        *linePtr=line.LineNumber;
      if (relLine)
        *relLine=displacement;
    }
  }
}

Debug& operator<<(Debug &dbg, const DebugStackwalk::Signature &sig)
{
  dbg << sig.Size() << " addresses:\n";

  for (unsigned k=0;k<sig.Size();k++)
  {
    char buf[512];
    sig.GetSymbol(sig.GetAddress(k),buf,sizeof(buf));
    dbg << buf << "\n";
  }

  return dbg;
}

//////////////////////////////////////////////////////////////////////////////

DebugStackwalk::DebugStackwalk(void)
{
  // it doesn't harm to do this here
  InitDbghelp();
}

DebugStackwalk::~DebugStackwalk()
{
}

void *DebugStackwalk::GetDbghelpHandle(void)
{
  return g_dbghelp;
}

bool DebugStackwalk::IsOldDbghelp(void)
{
  return g_oldDbghelp;
}

int DebugStackwalk::StackWalk(Signature &sig, struct _CONTEXT *ctx)
{
  InitDbghelp();

  sig.m_numAddr=0;

  // bail out if no stack walk available
  if (!gDbg._StackWalk)
    return 0;

	// Set up the stack frame structure for the start point of the stack walk (i.e. here).
	STACKFRAME stackFrame;
	memset(&stackFrame,0,sizeof(stackFrame));

	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Mode = AddrModeFlat;

	// Use the context struct if it was provided.
	if (ctx) 
  {
		stackFrame.AddrPC.Offset = ctx->Eip;
		stackFrame.AddrStack.Offset = ctx->Esp;
		stackFrame.AddrFrame.Offset = ctx->Ebp;
	}
  else
  {
    // walk stack back using current call chain
	  unsigned long reg_eip, reg_ebp, reg_esp;
	  __asm 
    {
    here:
		  lea	eax,here
		  mov	reg_eip,eax
		  mov	reg_ebp,ebp
		  mov	reg_esp,esp
	  };
	  stackFrame.AddrPC.Offset = reg_eip;
	  stackFrame.AddrStack.Offset = reg_esp;
	  stackFrame.AddrFrame.Offset = reg_ebp;
  }

	// Walk the stack by the requested number of return address iterations.
  bool skipFirst=!ctx;
  while (sig.m_numAddr<Signature::MAX_ADDR&&
		     gDbg._StackWalk(IMAGE_FILE_MACHINE_I386,GetCurrentProcess(),GetCurrentThread(),
                         &stackFrame,NULL,NULL,gDbg._SymFunctionTableAccess,gDbg._SymGetModuleBase,NULL))
  {
    if (skipFirst)
      skipFirst=false;
    else
      sig.m_addr[sig.m_numAddr++]=stackFrame.AddrPC.Offset;
  }

	return sig.m_numAddr;
}
