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

#include "debug.h"
#include "debug_stack.h"
#include <windows.h>
#include "WWLib/stringex.h"
#include <imagehlp.h>
#include <cstdio>
// Only pulls the pointer-sized-int typedef (uintptr_t); avoids dragging
// BaseTypeCore.h's warning-as-error pragmas into a file that never had them.
#include <Utility/stdint_adapter.h>
#include "Lib/arch_context.h"

// imagehlp.h (via dbghelp.h's psdk_inc/_dbg_common.h) #defines StackWalk to
// StackWalk64 on 64-bit builds, because _IMAGEHLP64 is set whenever _WIN64
// is defined. DebugStackwalk::StackWalk below is our own class method, not
// a direct call into the Win32 API (that goes through the gDbg._StackWalk
// function pointer instead), so the platform macro must not be allowed to
// rewrite its name. debug_stack.h is included above, before this macro
// exists, so the class declaration is unaffected; without this #undef the
// out-of-line definition further down would be silently renamed to
// StackWalk64 and no longer match its own declaration.
//
// This fix is order-dependent: it only protects code that appears *after*
// this point in this translation unit. If a future #include added below
// this line (directly or transitively) pulls in <imagehlp.h>/<dbghelp.h>
// again, or otherwise redefines StackWalk, the mismatch this guards
// against comes back with no compiler warning -- #undef is silent by
// design. Keep this as the last DbgHelp-related include in the file, or
// re-apply the #undef immediately after whatever reintroduces the macro.
#ifdef StackWalk
#undef StackWalk
#endif

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
  // Overlays the struct above, whose members are actual function pointers
  // (8 bytes on Win64). Must be pointer-sized or the aliasing/stride used
  // by InitDbghelp() below only covers half of each slot on 64-bit.
  uintptr_t funcPtr[1];
} gDbg;
#undef DBGHELP

// GetProcAddress'd against DBGHELP.DLL by InitDbghelp() below, one name per
// DBGHELP() entry in debug_stack.inl, in the same order as the gDbg struct
// above.
//
// 64-bit dbghelp.dll only exports the ...64 form of an entry point whose
// address parameter is DWORD64 -- StackWalk, SymFunctionTableAccess,
// SymGetModuleBase, SymGetSymFromAddr and SymGetLineFromAddr here.
// GetProcAddress with the un-suffixed name returns NULL for those on x64,
// which would leave the matching gDbg._SymXxx pointer null and silently
// disable that part of the stack walker rather than fail to compile.
// SymInitialize, SymGetOptions, SymSetOptions and SymCleanup take no address
// parameter and are exported under the same name on every architecture
// (verified against mingw-w64's psdk_inc/_dbg_common.h: no #define
// redirects them under _IMAGEHLP64), so they are unchanged.
#if defined(_WIN64) || defined(__x86_64__)
#define DBGHELP(name,ret,par) DBGHELP_APINAME_##name,
#define DBGHELP_APINAME_SymInitialize          "SymInitialize"
#define DBGHELP_APINAME_SymGetOptions          "SymGetOptions"
#define DBGHELP_APINAME_SymSetOptions          "SymSetOptions"
#define DBGHELP_APINAME_StackWalk              "StackWalk64"
#define DBGHELP_APINAME_SymFunctionTableAccess "SymFunctionTableAccess64"
#define DBGHELP_APINAME_SymGetModuleBase       "SymGetModuleBase64"
#define DBGHELP_APINAME_SymGetSymFromAddr      "SymGetSymFromAddr64"
#define DBGHELP_APINAME_SymGetLineFromAddr     "SymGetLineFromAddr64"
#define DBGHELP_APINAME_SymCleanup             "SymCleanup"
static char const *const DebughelpFunctionNames[] =
{
#include "debug_stack.inl"
	nullptr
};
#undef DBGHELP_APINAME_SymInitialize
#undef DBGHELP_APINAME_SymGetOptions
#undef DBGHELP_APINAME_SymSetOptions
#undef DBGHELP_APINAME_StackWalk
#undef DBGHELP_APINAME_SymFunctionTableAccess
#undef DBGHELP_APINAME_SymGetModuleBase
#undef DBGHELP_APINAME_SymGetSymFromAddr
#undef DBGHELP_APINAME_SymGetLineFromAddr
#undef DBGHELP_APINAME_SymCleanup
#else
#define DBGHELP(name,ret,par) #name,
static char const *const DebughelpFunctionNames[] =
{
#include "debug_stack.inl"
	nullptr
};
#endif
#undef DBGHELP

// local dbghelp.dll module handle
static HMODULE g_dbghelp;

// local flag that is true if we're using an old dbghelp.dll version
static bool g_oldDbghelp;

static void InitDbghelp()
{
  // already called?
  if (g_dbghelp)
    return;

	// firstly check for dbghelp.dll in the EXE directory
	char dbgHelpPath[256];
	if (GetModuleFileName(nullptr,dbgHelpPath,sizeof(dbgHelpPath)))
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
  uintptr_t *funcptr=gDbg.funcPtr;
  unsigned k=0;
  for (;DebughelpFunctionNames[k];++k,++funcptr)
  {
    *funcptr=(uintptr_t)GetProcAddress(g_dbghelp,DebughelpFunctionNames[k]);
    if (!*funcptr)
      break;
  }
  if (DebughelpFunctionNames[k])
  {
    // not all functions found -> clear them all
    while (funcptr!=gDbg.funcPtr)
      *--funcptr=0;
  }
  else
  {
    // Set options
    gDbg._SymSetOptions(gDbg._SymGetOptions()|SYMOPT_DEFERRED_LOADS|SYMOPT_LOAD_LINES);

    // Init module
    gDbg._SymInitialize((HANDLE)GetCurrentProcessId(),nullptr,TRUE);

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

uintptr_t DebugStackwalk::Signature::GetAddress(int n) const
{
  DFAIL_IF_MSG(n<0||n>=MAX_ADDR,n << "/" << MAX_ADDR) return 0;
  return m_addr[n];
}

void DebugStackwalk::Signature::GetSymbol(uintptr_t addr, char *buf, unsigned bufSize)
{
  DFAIL_IF(!buf) return;
  DFAIL_IF(bufSize<64||bufSize>=0x80000000) return;

  InitDbghelp();

  char *bufEnd=buf+bufSize;
  *buf=0;
#if defined(_WIN64) || defined(__x86_64__)
  // sprintf (CRT), not wsprintf (User32's own limited formatter, used for
  // every other format string in this function): wsprintf's documented
  // format support does not include a 64-bit-width specifier, and this is
  // the one field in this function that can actually need one.
  buf+=sprintf(buf,"%016llX",(unsigned long long)addr);
#else
  buf+=wsprintf(buf,"%08x",(unsigned)addr);
#endif

  // determine module
  // Pointer-width, not `unsigned`: _SymGetModuleBase resolves to
  // SymGetModuleBase64 on x64 (see debug_stack.inl) and returns a DWORD64;
  // truncating it here would corrupt every `addr-modBase` relative offset
  // computed below.
  uintptr_t modBase=gDbg._SymGetModuleBase((HANDLE)GetCurrentProcessId(),addr);
  if (!modBase)
	{
		strcpy(buf," (unknown module)");
    return;
	}

  // illegal code ptr?
	if (IsBadReadPtr((void *)addr,sizeof(addr))||IsBadCodePtr((FARPROC)addr))
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
  // Cast to unsigned: a module-relative offset is well under 4GB in
  // practice (it's an offset within a single loaded module, not an
  // absolute address), and wsprintf's "%x" is a 32-bit format regardless
  // of argument width -- passing the full uintptr_t here would mismatch
  // the format on x64.
  buf+=wsprintf(buf,"+0x%x",(unsigned)(addr-modBase));

  // determine symbol
  PIMAGEHLP_SYMBOL symPtr=(PIMAGEHLP_SYMBOL)symbolBuffer;
  memset(symPtr,0,sizeof(symbolBuffer));
  symPtr->SizeOfStruct=sizeof(IMAGEHLP_SYMBOL);
  symPtr->MaxNameLength=sizeof(symbolBuffer)-sizeof(IMAGEHLP_SYMBOL);
  DWORD displacement;
#if defined(_WIN64) || defined(__x86_64__)
  // SymGetSymFromAddr64's Displacement out-param is PDWORD64; &displacement
  // (DWORD, 4 bytes) would overflow. Capture into a properly sized local
  // and narrow into displacement, which is then reused below for the
  // SymGetLineFromAddr call, whose Displacement stays PDWORD on x64.
  DWORD64 displacement64;
  if (!gDbg._SymGetSymFromAddr((HANDLE)GetCurrentProcessId(),addr,&displacement64,symPtr))
    return;
  displacement=(DWORD)displacement64;
#else
  if (!gDbg._SymGetSymFromAddr((HANDLE)GetCurrentProcessId(),addr,&displacement,symPtr))
    return;
#endif
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

void DebugStackwalk::Signature::GetSymbol(uintptr_t addr,
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

  // determine module (see the other GetSymbol overload's comment: pointer-
  // width, not `unsigned`, since this resolves to SymGetModuleBase64 on x64)
  uintptr_t modBase=gDbg._SymGetModuleBase((HANDLE)GetCurrentProcessId(),addr);
  if (!modBase)
	{
    if (bufMod)
		  strcpy(bufMod,"(unknown mod)");
    if (bufSym)
      strcpy(bufSym,"(unknown)");
    return;
	}

  // illegal code ptr?
	if (IsBadReadPtr((void *)addr,sizeof(addr))||IsBadCodePtr((FARPROC)addr))
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
    strlcpy(bufMod,p,sizeMod);
  }
  // relMod is `unsigned *`, unchanged: a module-relative offset is well
  // under 4GB in practice, unlike the absolute addr/modBase this is derived
  // from.
  if (relMod)
    *relMod=(unsigned)(addr-modBase);

  // determine symbol
  if (bufSym)
  {
    PIMAGEHLP_SYMBOL symPtr=(PIMAGEHLP_SYMBOL)symbolBuffer;
    memset(symPtr,0,sizeof(symbolBuffer));
    symPtr->SizeOfStruct=sizeof(IMAGEHLP_SYMBOL);
    symPtr->MaxNameLength=sizeof(symbolBuffer)-sizeof(IMAGEHLP_SYMBOL);
    DWORD displacement;
#if defined(_WIN64) || defined(__x86_64__)
    // See the comment on the first _SymGetSymFromAddr call above: its
    // Displacement out-param is PDWORD64 on x64.
    DWORD64 displacement64;
    if (gDbg._SymGetSymFromAddr((HANDLE)GetCurrentProcessId(),addr,&displacement64,symPtr))
    {
      displacement=(DWORD)displacement64;
#else
    if (gDbg._SymGetSymFromAddr((HANDLE)GetCurrentProcessId(),addr,&displacement,symPtr))
    {
#endif
      strlcpy(bufSym,symPtr->Name,sizeSym);
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
      strlcpy(bufFile,p,sizeFile);
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

DebugStackwalk::DebugStackwalk()
{
  // it doesn't harm to do this here
  InitDbghelp();
}

DebugStackwalk::~DebugStackwalk()
{
}

void *DebugStackwalk::GetDbghelpHandle()
{
  return g_dbghelp;
}

bool DebugStackwalk::IsOldDbghelp()
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
		stackFrame.AddrPC.Offset = CTX_PC(*ctx);
		stackFrame.AddrStack.Offset = CTX_STACK(*ctx);
		stackFrame.AddrFrame.Offset = CTX_FRAME(*ctx);
	}
  else
  {
    // walk stack back using current call chain
	  // uintptr_t rather than unsigned long: these feed
	  // stackFrame.AddrPC/AddrFrame/AddrStack.Offset, which are DWORD64 in
	  // STACKFRAME64 (STACKFRAME becomes STACKFRAME64 on x64), and
	  // `unsigned long` stays 32 bits under Win64's LLP64 model.
	  uintptr_t reg_eip, reg_ebp, reg_esp;
#if defined(_MSC_VER) && defined(_M_IX86)
	  __asm
    {
    here:
		  lea	eax,here
		  mov	reg_eip,eax
		  mov	reg_ebp,ebp
		  mov	reg_esp,esp
	  };
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(_M_IX86))
	  __asm__ __volatile__ (
		  "call 1f\n\t"
		  "1: pop %0\n\t"
		  "mov %%ebp, %1\n\t"
		  "mov %%esp, %2"
		  : "=r" (reg_eip), "=r" (reg_ebp), "=r" (reg_esp)
	  );
#else
	  // x86-64 and anything else: RtlCaptureContext fills a CONTEXT with the
	  // caller's register state -- the documented Win64 way to seed a
	  // StackWalk64, needing no inline assembly. Mirrors the eip/ebp/esp set
	  // this function captures (same register set as Except.cpp's
	  // Stack_Walk); the ctx-provided branch above already reads the same
	  // three fields via CTX_PC/CTX_FRAME/CTX_STACK.
	  CONTEXT capture_ctx;
	  RtlCaptureContext(&capture_ctx);
	  reg_eip = (uintptr_t)CTX_PC(capture_ctx);
	  reg_ebp = (uintptr_t)CTX_FRAME(capture_ctx);
	  reg_esp = (uintptr_t)CTX_STACK(capture_ctx);
#endif
	  stackFrame.AddrPC.Offset = reg_eip;
	  stackFrame.AddrStack.Offset = reg_esp;
	  stackFrame.AddrFrame.Offset = reg_ebp;
  }

	// Walk the stack by the requested number of return address iterations.
  bool skipFirst=!ctx;
  while (sig.m_numAddr<Signature::MAX_ADDR&&
		     gDbg._StackWalk(CTX_STACKWALK_MACHINE,GetCurrentProcess(),GetCurrentThread(),
                         &stackFrame,nullptr,nullptr,gDbg._SymFunctionTableAccess,gDbg._SymGetModuleBase,nullptr))
  {
    if (skipFirst)
      skipFirst=false;
    else
      sig.m_addr[sig.m_numAddr++]=stackFrame.AddrPC.Offset;
  }

	return sig.m_numAddr;
}
