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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_except.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Unhandled exception handler
//////////////////////////////////////////////////////////////////////////////
#include "_pch.h"
#include <commctrl.h>

DebugExceptionhandler::DebugExceptionhandler(void)
{
  // don't do anything here!
}

const char *DebugExceptionhandler::GetExceptionType(struct _EXCEPTION_POINTERS *exptr, char *explanation)
{
  #define EX(code,text)   \
    case EXCEPTION_##code: strcpy(explanation,text); return "EXCEPTION_" #code;

  switch(exptr->ExceptionRecord->ExceptionCode)
  {
		case EXCEPTION_ACCESS_VIOLATION:
      wsprintf(explanation,
             "The thread tried to read from or write to a virtual\n"
             "address for which it does not have the appropriate access.\n"
             "Access address 0x%08x was %s.",
                exptr->ExceptionRecord->ExceptionInformation[1],
                exptr->ExceptionRecord->ExceptionInformation[0]?"written to":"read from");
      return "EXCEPTION_ACCESS_VIOLATION";
		EX(ARRAY_BOUNDS_EXCEEDED,"The thread tried to access an array element that\n"
                             "is out of bounds and the underlying hardware\n"
                             "supports bounds checking.")
		EX(BREAKPOINT,"A breakpoint was encountered.")
		EX(DATATYPE_MISALIGNMENT,"The thread tried to read or write data that is\n"
                             "misaligned on hardware that does not provide alignment.\n"
                             "For example, 16-bit values must be aligned on\n"
                             "2-byte boundaries; 32-bit values on 4-byte\n"
                             "boundaries, and so on.")
		EX(FLT_DENORMAL_OPERAND,"One of the operands in a floating-point operation is\n"
                            "denormal. A denormal value is one that is too small\n"
                            "to represent as a standard floating-point value.")
		EX(FLT_DIVIDE_BY_ZERO,"The thread tried to divide a floating-point\n"
                          "value by a floating-point divisor of zero.")
		EX(FLT_INEXACT_RESULT,"The result of a floating-point operation\n"
                          "cannot be represented exactly as a decimal fraction.")
		EX(FLT_INVALID_OPERATION,"Some strange unknown floating point operation was attempted.")
		EX(FLT_OVERFLOW,"The exponent of a floating-point operation is greater\n"
                    "than the magnitude allowed by the corresponding type.")
		EX(FLT_STACK_CHECK,"The stack overflowed or underflowed as the result\n"
                       "of a floating-point operation.")
		EX(FLT_UNDERFLOW,"The exponent of a floating-point operation is less\n"
                     "than the magnitude allowed by the corresponding type.")
    EX(GUARD_PAGE,"A guard page was accessed.")
		EX(ILLEGAL_INSTRUCTION,"The thread tried to execute an invalid instruction.")
		EX(IN_PAGE_ERROR,"The thread tried to access a page that was not\n"
                     "present, and the system was unable to load the page.\n"
                     "For example, this exception might occur if a network "
                     "connection is lost while running a program over the network.")
		EX(INT_DIVIDE_BY_ZERO,"The thread tried to divide an integer value by\n"
                          "an integer divisor of zero.")
		EX(INT_OVERFLOW,"The result of an integer operation caused a carry\n"
                    "out of the most significant bit of the result.")
		EX(INVALID_DISPOSITION,"An exception handler returned an invalid disposition\n"
                           "to the exception dispatcher. Programmers using a\n"
                           "high-level language such as C should never encounter\n"
                           "this exception.")
    EX(INVALID_HANDLE,"An invalid Windows handle was used.")
		EX(NONCONTINUABLE_EXCEPTION,"The thread tried to continue execution after\n"
                                "a noncontinuable exception occurred.")
		EX(PRIV_INSTRUCTION,"The thread tried to execute an instruction whose\n"
                        "operation is not allowed in the current machine mode.")
		EX(SINGLE_STEP,"A trace trap or other single-instruction mechanism\n"
                   "signaled that one instruction has been executed.")
		EX(STACK_OVERFLOW,"The thread used up its stack.")
    case 0xE06D7363: strcpy(explanation,"Microsoft C++ Exception"); return "EXCEPTION_MS";
    default:
      wsprintf(explanation,"Unknown exception code 0x%08x",exptr->ExceptionRecord->ExceptionCode);
      return "EXCEPTION_UNKNOWN";
  }

  #undef EX
}

void DebugExceptionhandler::LogExceptionLocation(Debug &dbg, struct _EXCEPTION_POINTERS *exptr)
{
  struct _CONTEXT &ctx=*exptr->ContextRecord;

  char buf[512];
  DebugStackwalk::Signature::GetSymbol(ctx.Eip,buf,sizeof(buf));
  dbg << "Exception occured at\n" << buf << ".";
}

void DebugExceptionhandler::LogRegisters(Debug &dbg, struct _EXCEPTION_POINTERS *exptr)
{
  struct _CONTEXT &ctx=*exptr->ContextRecord;

  dbg << Debug::FillChar('0')
      << Debug::Hex()
      <<  "EAX:" << Debug::Width(8) << ctx.Eax 
      << " EBX:" << Debug::Width(8) << ctx.Ebx
      << " ECX:" << Debug::Width(8) << ctx.Ecx << "\n"
      <<  "EDX:" << Debug::Width(8) << ctx.Edx 
      << " ESI:" << Debug::Width(8) << ctx.Esi
      << " EDI:" << Debug::Width(8) << ctx.Edi << "\n"
      <<  "EIP:" << Debug::Width(8) << ctx.Eip 
      << " ESP:" << Debug::Width(8) << ctx.Esp
      << " EBP:" << Debug::Width(8) << ctx.Ebp << "\n"
      <<  "Flags:" << Debug::Bin() << Debug::Width(32) << ctx.EFlags << Debug::Hex() << "\n"
      <<  "CS:" << Debug::Width(4) << ctx.SegCs
      << " DS:" << Debug::Width(4) << ctx.SegDs
      << " SS:" << Debug::Width(4) << ctx.SegSs
      << "\nES:" << Debug::Width(4) << ctx.SegEs
      << " FS:" << Debug::Width(4) << ctx.SegFs
      << " GS:" << Debug::Width(4) << ctx.SegGs << "\n" << Debug::FillChar() << Debug::Dec();
}

void DebugExceptionhandler::LogFPURegisters(Debug &dbg, struct _EXCEPTION_POINTERS *exptr)
{
  struct _CONTEXT &ctx=*exptr->ContextRecord;

  if (!(ctx.ContextFlags&CONTEXT_FLOATING_POINT))
  {
    dbg << "FP registers not available\n";
    return;
  }

  FLOATING_SAVE_AREA &flt=ctx.FloatSave;
  dbg << Debug::Bin() << Debug::FillChar('0')
      << "CW:" << Debug::Width(16) << (flt.ControlWord&0xffff) << "\n"
      << "SW:" << Debug::Width(16) << (flt.StatusWord&0xffff) << "\n"
      << "TW:" << Debug::Width(16) << (flt.TagWord&0xffff) << "\n"
      << Debug::Hex() 
      << "ErrOfs:      " << Debug::Width(8) << flt.ErrorOffset
      << " ErrSel:  "    << Debug::Width(8) << flt.ErrorSelector << "\n"
      << "DataOfs:     " << Debug::Width(8) << flt.DataOffset
      << " DataSel: "    << Debug::Width(8) << flt.DataSelector << "\n"
#if !defined(WOW64_SIZE_OF_80387_REGISTERS)
      << "Cr0NpxState: " << Debug::Width(8) << flt.Cr0NpxState << "\n"
#endif
  ;

  for (unsigned k=0;k<SIZE_OF_80387_REGISTERS/10;++k)
  {
    dbg << Debug::Dec() << "ST(" << k << ") ";
    dbg.SetPrefixAndRadix("",16);

    BYTE *value=flt.RegisterArea+k*10;
    for (unsigned i=0;i<10;i++)
      dbg << Debug::Width(2) << value[i];

    double fpVal;

    // convert from temporary real (10 byte) to double
    _asm
    {
      mov eax,value
      fld tbyte ptr [eax]
      fstp qword ptr [fpVal]
    }

    dbg << " " << fpVal << "\n";
  }
  dbg << Debug::FillChar() << Debug::Dec();
}

// include exception dialog box
#include "rc_exception.inl"

// stupid dialog box function needs this
static struct _EXCEPTION_POINTERS *exPtrs;

// and this saves us from re-generating register/version info again...
static char regInfo[1024],verInfo[256];

// and this saves us from doing a stack walk twice
static DebugStackwalk::Signature sig;

static BOOL CALLBACK ExceptionDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
    case WM_INITDIALOG:
      break;
    case WM_COMMAND:
      if (LOWORD(wParam)==IDOK)
        EndDialog(hWnd,IDOK);
    default:
      return FALSE;
  }

  // init dialog box

  // version
  SendDlgItemMessage(hWnd,103,WM_SETTEXT,0,(LPARAM)verInfo);

  // registers
  char *p=regInfo;
  for (char *q=p;;q++)
  {
    if (!*q||*q=='\n')
    {
      bool quit=!*q; *q=0;
      SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)p);
      if (quit)
        break;
      p=q+1;
    }
  }

  // yes, this generates a GDI leak but we're crashing anyway
  SendDlgItemMessage(hWnd,105,WM_SETFONT,(WPARAM)CreateFont(13,0,0,0,FW_NORMAL,
                FALSE,FALSE,FALSE,ANSI_CHARSET,
                OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY,FIXED_PITCH|FF_MODERN,NULL),MAKELPARAM(TRUE,0));

  // exception type
  SendDlgItemMessage(hWnd,100,WM_SETTEXT,0,(LPARAM)
            DebugExceptionhandler::GetExceptionType(exPtrs,regInfo));
  SendDlgItemMessage(hWnd,101,WM_SETTEXT,0,(LPARAM)regInfo);

  // address
  struct _CONTEXT &ctx=*exPtrs->ContextRecord;
  DebugStackwalk::Signature::GetSymbol(ctx.Eip,regInfo,sizeof(regInfo));
  SendDlgItemMessage(hWnd,102,WM_SETTEXT,0,(LPARAM)regInfo);

  // stack 
  // (this code is a little messy because we're dealing with a raw list control)
  HWND list;
  list=GetDlgItem(hWnd,104);
  if (!sig.Size())
  {
    LVCOLUMN c;
    c.mask=LVCF_TEXT|LVCF_WIDTH;
    char columnText[] = "";
    c.pszText=columnText;
    c.cx=690;
    ListView_InsertColumn(list,0,&c);

    LVITEM item;
    item.iItem=0;
    item.iSubItem=0;
    item.mask=LVIF_TEXT;
    char itemText[] = "No stack data available - check for dbghelp.dll";
    item.pszText=itemText;

    item.iItem=ListView_InsertItem(list,&item);
  }
  else
  {
    // add columns first
    LVCOLUMN c;
    c.mask=LVCF_TEXT|LVCF_WIDTH;
    char columnText1[] = "";
    c.pszText=columnText1;
    c.cx=0; // first column is empty (can't right-align 1st column)
    ListView_InsertColumn(list,0,&c);

    c.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
    char columnText2[] = "Address";
    c.pszText=columnText2;
    c.cx=60;
    c.fmt=LVCFMT_RIGHT;
    ListView_InsertColumn(list,1,&c);

    c.mask=LVCF_TEXT|LVCF_WIDTH;
    char columnText3[] = "Module";
    c.pszText=columnText3;
    c.cx=120;
    ListView_InsertColumn(list,2,&c);

    char columnText4[] = "Symbol";
    c.pszText=columnText4;
    c.cx=300;
    ListView_InsertColumn(list,3,&c);

    char columnText5[] = "File";
    c.pszText=columnText5;
    c.cx=130;
    ListView_InsertColumn(list,4,&c);

    char columnText6[] = "Line";
    c.pszText=columnText6;
    c.cx=80;
    ListView_InsertColumn(list,5,&c);

    // now add stack walk lines
    for (unsigned k=0;k<sig.Size();k++)
    {
      DebugStackwalk::Signature::GetSymbol(sig.GetAddress(k),regInfo,sizeof(regInfo));
      
      LVITEM item;
      item.iItem=k;
      item.iSubItem=0;
      item.mask=0;
      item.iItem=ListView_InsertItem(list,&item);
      item.mask=LVIF_TEXT;

      item.iSubItem++;
      item.pszText=strtok(regInfo," ");
      ListView_SetItem(list,&item);

      item.iSubItem++;
      item.pszText=strtok(NULL,",");
      ListView_SetItem(list,&item);

      item.iSubItem++;
      item.pszText=strtok(NULL,",");
      ListView_SetItem(list,&item);

      item.iSubItem++;
      item.pszText=strtok(NULL,":");
      ListView_SetItem(list,&item);
      
      item.iSubItem++;
      item.pszText=strtok(NULL,"");
      ListView_SetItem(list,&item);
    }
  }

  return TRUE;
}

#include <stdio.h>

LONG __stdcall DebugExceptionhandler::ExceptionFilter(struct _EXCEPTION_POINTERS* pExPtrs)
{
  // we should not be calling ourselves! 
  static bool inExceptionFilter;
  if (inExceptionFilter)
  {
    MessageBox(NULL,"Exception in exception handler","Fatal error",MB_OK);
    return EXCEPTION_CONTINUE_SEARCH;
  }
  inExceptionFilter=true;

  if (pExPtrs->ExceptionRecord->ExceptionCode==EXCEPTION_STACK_OVERFLOW)
  {
    // almost everything we are about to do will generate a second
    // stack overflow... double fault... so give at least a little warning
    OutputDebugString("EA/DEBUG: EXCEPTION_STACK_OVERFLOW\n");
  }

  // Let's log some info
  Debug &dbg=Debug::Instance;

  // we're logging an exception
  ++dbg.disableAssertsEtc;
  if (dbg.curType!=DebugIOInterface::StringType::MAX)
    dbg.FlushOutput();
  dbg.StartOutput(DebugIOInterface::StringType::Exception,"");

  // start off with the exception type & location
  dbg << "\n" << Debug::RepeatChar('=',80) << "\n";
  dbg << GetExceptionType(pExPtrs,regInfo) << ":\n" << regInfo << "\n\n";
  LogExceptionLocation(dbg,pExPtrs); dbg << "\n\n";

  // build info must be saved off for dialog...
  unsigned curOfs=dbg.ioBuffer[DebugIOInterface::Exception].used;
  dbg.WriteBuildInfo(); 
  unsigned len=dbg.ioBuffer[DebugIOInterface::Exception].used-curOfs;
  if (len>=sizeof(verInfo))
    len=sizeof(verInfo)-1;
  memcpy(verInfo,dbg.ioBuffer[DebugIOInterface::Exception].buffer+curOfs,len);
  verInfo[len]=0;
  dbg << "\n\n";

  // save off register info as well...
  curOfs=dbg.ioBuffer[DebugIOInterface::Exception].used;
  LogRegisters(dbg,pExPtrs); dbg << "\n";
  LogFPURegisters(dbg,pExPtrs); dbg << "\n";
  len=dbg.ioBuffer[DebugIOInterface::Exception].used-curOfs;
  if (len>=sizeof(regInfo))
    len=sizeof(regInfo)-1;
  memcpy(regInfo,dbg.ioBuffer[DebugIOInterface::Exception].buffer+curOfs,len);
  regInfo[len]=0;

  // now finally add stack & EIP dump
  dbg.m_stackWalk.StackWalk(sig,pExPtrs->ContextRecord);
  dbg << sig << "\n";

  dbg << "Bytes around EIP:" << Debug::MemDump::Char(((char *)(pExPtrs->ContextRecord->Eip))-32,80);

  dbg.FlushOutput();

  // shut down real Debug module now
  // (atexit code never gets called in exception case)
  Debug::StaticExit();

  // Show a dialog box
  InitCommonControls();
  exPtrs=pExPtrs;
  DialogBoxIndirect(NULL,(LPDLGTEMPLATE)rcException,NULL,ExceptionDlgProc);

  // Now die
  return EXCEPTION_EXECUTE_HANDLER;
}
