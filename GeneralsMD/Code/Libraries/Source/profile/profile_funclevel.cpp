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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/profile_funclevel.cpp $
// $Author: mhoffe $
// $Revision: #4 $
// $DateTime: 2003/08/14 13:43:29 $
//
// (c) 2003 Electronic Arts
//
// Function level profiling
//////////////////////////////////////////////////////////////////////////////
#include "_pch.h"
#include "../debug/debug.h"
#include <new>

#ifdef HAS_PROFILE

// TLS index (-1 if not yet initialized)
static int TLSIndex=-1;

// our own fast critical section
static ProfileFastCS cs;

// Unfortunately VC 6 doesn't support _pleave (or _pexit) so
// we have to come up with our own type of implementation here...
static void __declspec(naked) _pleave(void)
{
  ProfileFuncLevelTracer *p;
  unsigned curESP,leaveAddr;

  _asm
  {
    push ebp        // this will be overwritten down there...
    push ebp        // setup standard stack frame
    push eax
    mov  ebp,esp
    mov  eax,esp
    sub  esp,3*4    // 3 local DWord vars
    pushad
    mov  [curESP],eax
  }

  curESP+=8;
  p=(ProfileFuncLevelTracer *)TlsGetValue(TLSIndex);
  leaveAddr=p->Leave(curESP);
  *(unsigned *)(curESP)=leaveAddr;

  _asm
  {
    popad
    add  esp,3*4    // must match sub esp above
    pop  eax
    pop  ebp
    ret
  }
}

extern "C" void __declspec(naked) _cdecl _penter(void)
{
  unsigned callerFunc,ESPonReturn,callerRet;
  ProfileFuncLevelTracer *p;

  _asm 
  {
    push ebp
    push eax
    mov  ebp,esp
    mov  eax,esp
    sub  esp,4*4      // 4 local DWord vars
    pushad

    // calc return address
    add  eax,4+4      // account for push ebp and push eax
    mov  ebx,[eax]    // grab return address
    mov  [callerFunc],ebx

    // get some more stuff
    add  eax,4
    mov  [ESPonReturn],eax
    mov  ebx,[eax]
    mov  [callerRet],ebx

    // jam in our exit code
    mov  dword ptr [eax],offset _pleave
  }

  // do we need a new stack tracer?
  if (TLSIndex==-1)
    TLSIndex=TlsAlloc();
  p=(ProfileFuncLevelTracer *)TlsGetValue(TLSIndex);
  if (!p)
  {
    p=(ProfileFuncLevelTracer *)ProfileAllocMemory(sizeof(ProfileFuncLevelTracer));
    new (p) ProfileFuncLevelTracer;
    TlsSetValue(TLSIndex,p);
  }

  // enter function
  p->Enter(callerFunc-5,ESPonReturn,callerRet);

  // cleanup
  _asm
  {
    popad
    add  esp,4*4      // must match sub esp above
    pop  eax
    pop  ebp
    ret
  }
}

ProfileFuncLevelTracer *ProfileFuncLevelTracer::head=NULL;
bool ProfileFuncLevelTracer::shuttingDown=false;
int ProfileFuncLevelTracer::curFrame=0;
unsigned ProfileFuncLevelTracer::frameRecordMask;
bool ProfileFuncLevelTracer::recordCaller=false;

ProfileFuncLevelTracer::ProfileFuncLevelTracer(void):
  stack(NULL), usedStack(0), totalStack(0), maxDepth(0)
{
  ProfileFastCS::Lock lock(cs);

  next=head;
  head=this;
}

ProfileFuncLevelTracer::~ProfileFuncLevelTracer()
{
  // yes, I know we leak...
}

void ProfileFuncLevelTracer::Enter(unsigned addr, unsigned esp, unsigned ret)
{
  // must stack grow?
  if (usedStack>=totalStack)
    stack=(StackEntry *)ProfileReAllocMemory(stack,(totalStack+=100)*sizeof(StackEntry));

  // save info
  Function *f=func.Find(addr);
  if (!f)
  {
    // new function
    f=(Function *)ProfileAllocMemory(sizeof(Function));
    new (f) Function(this);
    f->addr=addr;
    f->glob.callCount=f->glob.tickPure=f->glob.tickTotal=0;
    for (int i=0;i<MAX_FRAME_RECORDS;i++)
      f->cur[i].callCount=f->cur[i].tickPure=f->cur[i].tickTotal=0;
    f->depth=0;
    func.Insert(f);
  }

  StackEntry &s=stack[usedStack++];
  s.func=f;
  s.esp=esp;
  s.retVal=ret;
  ProfileGetTime(s.tickEnter);
  s.tickSubTime=0;
  f->depth++;

  // new max depth?
  if (usedStack>=maxDepth)
    maxDepth=usedStack;

  DLOG_GROUP(profile_stack,Debug::RepeatChar(' ',usedStack-1) 
                          << Debug::Hex() << this 
                          << " Enter " << Debug::Width(8) << addr 
                          << " ESP "   << Debug::Width(8) << esp
                          << " return "   << Debug::Width(8) << ret
                          << " level " << Debug::Dec() << usedStack
                          );
}

unsigned ProfileFuncLevelTracer::Leave(unsigned esp)
{
  // get current "time"
  __int64 cur;
  ProfileGetTime(cur);

  while (usedStack>0)
  {
    // leave current function
    usedStack--;
    StackEntry &s=stack[usedStack],
               &sPrev=stack[usedStack-1];

    Function *f=s.func;

    // decrease call depth
    // note: add global time only if call depth is 0
    f->depth--;

    // insert caller
    if (recordCaller&&usedStack)
      f->glob.caller.Insert(sPrev.func->addr,1);

    // inc call counter
    f->glob.callCount++;

    // add total time
    __int64 delta=cur-s.tickEnter;
    if (!f->depth)
      f->glob.tickTotal+=delta;

    // add pure time
    f->glob.tickPure+=delta-s.tickSubTime;

    // add sub time for higher function
    if (usedStack)
      sPrev.tickSubTime+=delta;

    // frame based profiling?
    if (frameRecordMask)
    {
      unsigned mask=frameRecordMask;
      for (unsigned i=0;i<MAX_FRAME_RECORDS;i++)
      {
        if (mask&1)
        {
          if (recordCaller&&usedStack>0)
            f->cur[i].caller.Insert(sPrev.func->addr,1);
          f->cur[i].callCount++;
          if (!f->depth)
            f->cur[i].tickTotal+=delta;
          f->cur[i].tickPure+=delta-s.tickSubTime;
        }
        if (!(mask>>=1))
          break;
      }
    }

    // exit if address match (somewhat...)
    if (s.esp==esp)
      break;

    // catching those nasty ret<n>...
    if (s.esp<esp&&
        (esp-s.esp)%4==0&&
        (esp-s.esp)<256)
      break;

    // emit warning
    DCRASH("ESP " << Debug::Hex() << esp << " does not match " << stack[usedStack].esp << Debug>>Dec());
  }

  DLOG_GROUP(profile_stack,Debug::RepeatChar(' ',usedStack-1) 
                          << Debug::Hex() << this 
                          << " Leave " << Debug::Width(8) << "" 
                          << " ESP "   << Debug::Width(8) << stack[usedStack].esp
                          << " return "   << Debug::Width(8) << stack[usedStack].retVal
                          << " level " << Debug::Dec() << usedStack
                          );

  return stack[usedStack].retVal;
}

void ProfileFuncLevelTracer::Shutdown(void)
{
  if (frameRecordMask)
  {
    for (unsigned i=0;i<MAX_FRAME_RECORDS;i++)
      if (frameRecordMask&(1<<i))
        for (ProfileFuncLevelTracer *p=head;p;p=p->next)
          p->FrameEnd(i,-1);
  }
}

int ProfileFuncLevelTracer::FrameStart(void)
{
  ProfileFastCS::Lock lock(cs);

  for (unsigned i=0;i<MAX_FRAME_RECORDS;i++)
    if (!(frameRecordMask&(1<<i)))
      break;
  if (i==MAX_FRAME_RECORDS)
    return -1;

  for (ProfileFuncLevelTracer *p=head;p;p=p->next)
  {
    Function *f;
    for (int k=0;(f=p->func.Enumerate(k))!=NULL;k++)
    {
      Profile &p=f->cur[i];
      p.caller.Clear();
      p.callCount=p.tickPure=p.tickTotal=0;
    }
  }

  frameRecordMask|=1<<i;
  return i;
}

void ProfileFuncLevelTracer::FrameEnd(int which, int mixIndex)
{
  DFAIL_IF(which<0||which>=MAX_FRAME_RECORDS)
    return;
  DFAIL_IF(!(frameRecordMask&(1<<which))) 
    return;
  DFAIL_IF(mixIndex>=curFrame)
    return;

  ProfileFastCS::Lock lock(cs);
  
  frameRecordMask^=1<<which;
  if (mixIndex<0)
    curFrame++;
  for (ProfileFuncLevelTracer *p=head;p;p=p->next)
  {
    Function *f;
    for (int k=0;(f=p->func.Enumerate(k))!=NULL;k++)
    {
      Profile &p=f->cur[which];
      if (p.callCount)
      {
        if (mixIndex<0)
          f->frame.Append(curFrame,p);
        else
          f->frame.MixIn(mixIndex,p);
      }
      p.caller.Clear();
      p.callCount=p.tickPure=p.tickTotal=0;
    }
  }
}

void ProfileFuncLevelTracer::ClearTotals(void)
{
  ProfileFastCS::Lock lock(cs);

  for (ProfileFuncLevelTracer *p=head;p;p=p->next)
  {
    Function *f;
    for (int k=0;(f=p->func.Enumerate(k))!=NULL;k++)
    {
      f->glob.caller.Clear();
      f->glob.callCount=0;
      f->glob.tickPure=0;
      f->glob.tickTotal=0;
    }
  }
}

ProfileFuncLevelTracer::UnsignedMap::UnsignedMap(void):
  e(NULL), alloc(0), used(0), writeLock(false)
{
  memset(hash,0,sizeof(hash));
}

ProfileFuncLevelTracer::UnsignedMap::~UnsignedMap()
{
  Clear();
}

void ProfileFuncLevelTracer::UnsignedMap::Clear(void)
{
  ProfileFreeMemory(e);
  e=NULL;
  alloc=used=0;
  memset(hash,0,sizeof(hash));
}

void ProfileFuncLevelTracer::UnsignedMap::_Insert(unsigned at, unsigned val, int countAdd)
{
  DFAIL_IF(writeLock) return;

  // realloc list?
  if (used==alloc)
  {
    // must fixup pointers...
    unsigned delta=unsigned(e);
    e=(Entry *)ProfileReAllocMemory(e,((alloc+=64)*sizeof(Entry)));
    delta=unsigned(e)-delta;
    if (used&&delta)
    {
      for (unsigned k=0;k<HASH_SIZE;k++)
        if (hash[k])
          ((unsigned &)hash[k])+=delta;
      for (k=0;k<used;k++)
        if (e[k].next)
          ((unsigned &)e[k].next)+=delta;
    }
  }

  // add new item
  e[used].val=val;
  e[used].count=countAdd;
  e[used].next=hash[at];
  hash[at]=e+used++;
}

unsigned ProfileFuncLevelTracer::UnsignedMap::Enumerate(int index)
{
  if (index<0||index>=(int)used)
    return 0;
  return e[index].val;
}

unsigned ProfileFuncLevelTracer::UnsignedMap::GetCount(int index)
{
  if (index<0||index>=(int)used)
    return 0;
  return e[index].count;
}

void ProfileFuncLevelTracer::UnsignedMap::Copy(const UnsignedMap &src)
{
  Clear();
  if (src.e)
  { 
    alloc=used=src.used;
    e=(Entry *)ProfileAllocMemory(alloc*sizeof(Entry));
    memcpy(e,src.e,alloc*sizeof(Entry));
    writeLock=true;
  }
}

void ProfileFuncLevelTracer::UnsignedMap::MixIn(const UnsignedMap &src)
{
  writeLock=false;
  for (unsigned k=0;k<src.used;k++)
    Insert(src.e[k].val,src.e[k].count);
  writeLock=true;
}

ProfileFuncLevelTracer::ProfileMap::ProfileMap(void):
  root(NULL), tail(&root)
{
}

ProfileFuncLevelTracer::ProfileMap::~ProfileMap()
{
  while (root)
  {
    List *next=root->next;
    root->~List();
    ProfileFreeMemory(root);
    root=next;
  }
}

ProfileFuncLevelTracer::Profile *ProfileFuncLevelTracer::ProfileMap::Find(int frame)
{
  for (List *p=root;p&&p->frame<frame;p=p->next);
  return p&&p->frame==frame?&p->p:NULL;
}

void ProfileFuncLevelTracer::ProfileMap::Append(int frame, const Profile &p)
{
  List *newEntry=(List *)ProfileAllocMemory(sizeof(List));
  new (newEntry) List;
  newEntry->frame=frame;
  newEntry->p.Copy(p);
  newEntry->next=NULL;
  *tail=newEntry;
  tail=&newEntry->next;
}

void ProfileFuncLevelTracer::ProfileMap::MixIn(int frame, const Profile &p)
{
  // search correct list entry
  for (List *oldEntry=root;oldEntry;oldEntry=oldEntry->next)
    if (oldEntry->frame==frame)
      break;
  if (!oldEntry)
    Append(frame,p);
  else
    oldEntry->p.MixIn(p);
}

ProfileFuncLevelTracer::FunctionMap::FunctionMap(void):
  e(NULL), alloc(0), used(0)
{
  memset(hash,0,sizeof(hash));
}

ProfileFuncLevelTracer::FunctionMap::~FunctionMap()
{
  if (e)
  {
    for (unsigned k=0;k<used;k++)
    {
      e[k].funcPtr->~Function();
      ProfileFreeMemory(e[k].funcPtr);
    }
    ProfileFreeMemory(e);
  }
}

void ProfileFuncLevelTracer::FunctionMap::Insert(Function *funcPtr)
{
  // realloc list?
  if (used==alloc)
  {
    // must fixup pointers...
    unsigned delta=unsigned(e);
    e=(Entry *)ProfileReAllocMemory(e,(alloc+=1024)*sizeof(Entry));
    delta=unsigned(e)-delta;
    if (used&&delta)
    {
      for (unsigned k=0;k<HASH_SIZE;k++)
        if (hash[k])
          ((unsigned &)hash[k])+=delta;
      for (k=0;k<used;k++)
        if (e[k].next)
          ((unsigned &)e[k].next)+=delta;
    }
  }

  // add to hash
  unsigned at=(funcPtr->addr/16)%HASH_SIZE;
  e[used].funcPtr=funcPtr;
  e[used].next=hash[at];
  hash[at]=e+used++;
}

ProfileFuncLevelTracer::Function *ProfileFuncLevelTracer::FunctionMap::Enumerate(int index)
{
  if (index<0||index>=(int)used)
    return NULL;
  return e[index].funcPtr;
}

bool ProfileFuncLevel::IdList::Enum(unsigned index, Id &id, unsigned *countPtr) const
{
  if (!m_ptr)
    return false;

  ProfileFuncLevelTracer::Profile &prof=*(ProfileFuncLevelTracer::Profile *)m_ptr;

  unsigned addr;
  if ((addr=prof.caller.Enumerate(index)))
  {
    id.m_funcPtr=prof.tracer->FindFunction(addr);
    if (countPtr)
      *countPtr=prof.caller.GetCount(index);
    return true;
  }
  else
    return false;
}

const char *ProfileFuncLevel::Id::GetSource(void) const
{
  if (!m_funcPtr)
    return NULL;

  ProfileFuncLevelTracer::Function *func=(ProfileFuncLevelTracer::Function *)m_funcPtr;
  if (!func->funcSource)
  {
    char helpFunc[256],helpFile[256];
    unsigned ofsFunc;
    DebugStackwalk::Signature::GetSymbol(func->addr,
                                         NULL,0,NULL,
                                         helpFunc,sizeof(helpFunc),&ofsFunc,
                                         helpFile,sizeof(helpFile),&func->funcLine,NULL);
    
    char help[300];
    wsprintf(help,ofsFunc?"%s+0x%x":"%s",helpFunc,ofsFunc);
    func->funcSource=(char *)ProfileAllocMemory(strlen(helpFile)+1);
    strcpy(func->funcSource,helpFile);
    func->funcName=(char *)ProfileAllocMemory(strlen(help)+1);
    strcpy(func->funcName,help);
  }

  return func->funcSource;
}

const char *ProfileFuncLevel::Id::GetFunction(void) const
{
  if (!m_funcPtr)
    return NULL;
  ProfileFuncLevelTracer::Function *func=(ProfileFuncLevelTracer::Function *)m_funcPtr;
  if (!func->funcSource)
    GetSource();
  return func->funcName;
}

unsigned ProfileFuncLevel::Id::GetAddress(void) const
{
  if (!m_funcPtr)
    return 0;
  ProfileFuncLevelTracer::Function *func=(ProfileFuncLevelTracer::Function *)m_funcPtr;
  return func->addr;
}

unsigned ProfileFuncLevel::Id::GetLine(void) const
{
  if (!m_funcPtr)
    return NULL;
  ProfileFuncLevelTracer::Function *func=(ProfileFuncLevelTracer::Function *)m_funcPtr;
  if (!func->funcSource)
    GetSource();
  return func->funcLine;
}

unsigned _int64 ProfileFuncLevel::Id::GetCalls(unsigned frame) const
{
  if (!m_funcPtr)
    return 0;

  ProfileFuncLevelTracer::Function &func=*(ProfileFuncLevelTracer::Function *)m_funcPtr;

  switch(frame)
  {
    case Total:
      return func.glob.callCount;
    default:
      ProfileFuncLevelTracer::Profile *prof=func.frame.Find(frame);
      return prof?prof->callCount:0;
  }
}

unsigned _int64 ProfileFuncLevel::Id::GetTime(unsigned frame) const
{
  if (!m_funcPtr)
    return 0;

  ProfileFuncLevelTracer::Function &func=*(ProfileFuncLevelTracer::Function *)m_funcPtr;

  switch(frame)
  {
    case Total:
      return func.glob.tickTotal;
    default:
      ProfileFuncLevelTracer::Profile *prof=func.frame.Find(frame);
      return prof?prof->tickTotal:0;
  }
}

unsigned _int64 ProfileFuncLevel::Id::GetFunctionTime(unsigned frame) const
{
  if (!m_funcPtr)
    return 0;

  ProfileFuncLevelTracer::Function &func=*(ProfileFuncLevelTracer::Function *)m_funcPtr;

  switch(frame)
  {
    case Total:
      return func.glob.tickPure;
    default:
      ProfileFuncLevelTracer::Profile *prof=func.frame.Find(frame);
      return prof?prof->tickPure:0;
  }
}

ProfileFuncLevel::IdList ProfileFuncLevel::Id::GetCaller(unsigned frame) const
{
  if (!m_funcPtr)
    return IdList();

  ProfileFuncLevelTracer::Function &func=*(ProfileFuncLevelTracer::Function *)m_funcPtr;

  IdList ret;
  switch(frame)
  {
    case Total:
      ret.m_ptr=&func.glob;
      break;
    default:
      ProfileFuncLevelTracer::Profile *prof=func.frame.Find(frame);
      if (prof)
        ret.m_ptr=prof;
  }

  return ret;
}

bool ProfileFuncLevel::Thread::EnumProfile(unsigned index, Id &id) const
{
  if (!m_threadID)
    return false;
  
  ProfileFastCS::Lock lock(cs);

  ProfileFuncLevelTracer::Function *f=m_threadID->EnumFunction(index);
  if (f)
  {
    id.m_funcPtr=f;
    return true;
  }
  else
    return false;
}

bool ProfileFuncLevel::EnumThreads(unsigned index, Thread &thread)
{
  ProfileFastCS::Lock lock(cs);

  for (ProfileFuncLevelTracer *p=ProfileFuncLevelTracer::GetFirst();p;p=p->GetNext())
    if (!index--)
      break;
  if (p)
  {
    thread.m_threadID=p;
    return true;
  }
  else
    return false;
}

ProfileFuncLevel::ProfileFuncLevel(void)
{
}

#else // !defined HAS_PROFILE

bool ProfileFuncLevel::IdList::Enum(unsigned index, Id &id, unsigned *) const
{
  return false;
}

const char *ProfileFuncLevel::Id::GetSource(void) const
{
  return NULL;
}

const char *ProfileFuncLevel::Id::GetFunction(void) const
{
  return NULL;
}

unsigned ProfileFuncLevel::Id::GetAddress(void) const
{
  return 0;
}

unsigned ProfileFuncLevel::Id::GetLine(void) const
{
  return 0;
}

unsigned _int64 ProfileFuncLevel::Id::GetCalls(unsigned frame) const
{
  return 0;
}

unsigned _int64 ProfileFuncLevel::Id::GetTime(unsigned frame) const
{
  return 0;
}

unsigned _int64 ProfileFuncLevel::Id::GetFunctionTime(unsigned frame) const
{
  return 0;
}

ProfileFuncLevel::IdList ProfileFuncLevel::Id::GetCaller(unsigned frame) const
{
  return ProfileFuncLevel::IdList();
}

bool ProfileFuncLevel::Thread::EnumProfile(unsigned index, Id &id) const
{
  return false;
}

bool ProfileFuncLevel::EnumThreads(unsigned index, Thread &thread)
{
  return false;
}

ProfileFuncLevel::ProfileFuncLevel(void)
{
}

#endif // !defined HAS_PROFILE

ProfileFuncLevel ProfileFuncLevel::Instance;
HANDLE ProfileFastCS::testEvent=::CreateEvent(NULL,FALSE,FALSE,"");
