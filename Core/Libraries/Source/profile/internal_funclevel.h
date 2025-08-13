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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/internal_funclevel.h $
// $Author: mhoffe $
// $Revision: #4 $
// $DateTime: 2003/08/14 13:43:29 $
//
// (c) 2003 Electronic Arts
//
// Function level profiling (internal header)
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#  pragma once
#endif
#ifndef INTERNAL_FUNCLEVEL_H // Include guard
#define INTERNAL_FUNCLEVEL_H

class ProfileFuncLevelTracer
{
  friend class ProfileCmdInterface;

  // can't copy this
  ProfileFuncLevelTracer(const ProfileFuncLevelTracer&);
  ProfileFuncLevelTracer& operator=(const ProfileFuncLevelTracer&);

public:
  enum
  {
    // # of simultaneous frame recordings
    MAX_FRAME_RECORDS = 4
  };

  /// simple unique unsigned/unsigned map
  class UnsignedMap
  {
    UnsignedMap(const UnsignedMap&);
    UnsignedMap& operator=(const UnsignedMap&);

    struct Entry
    {
      Entry *next;
      unsigned val;
      unsigned count;
    };

    enum
    {
      // do not make this number too large
      // a single function uses approx HASH_SIZE*20 bytes!
      HASH_SIZE = 131
    };

    Entry *e;
    unsigned alloc,used;
    Entry *hash[HASH_SIZE];
    bool writeLock;

    void _Insert(unsigned at, unsigned val, int countAdd);

  public:
    UnsignedMap(void);
    ~UnsignedMap();

    void Clear(void);
    void Insert(unsigned val, int countAdd);
    unsigned Enumerate(int index);
    unsigned GetCount(int index);
    void Copy(const UnsignedMap &src);
    void MixIn(const UnsignedMap &src);
  };

  /// profile entry
  struct Profile
  {
    /// call count
    __int64 callCount;

    /// pure time
    __int64 tickPure;

    /// total time
    __int64 tickTotal;

    /// caller list
    UnsignedMap caller;

    /// tracer for this profile
    ProfileFuncLevelTracer *tracer;

    void Copy(const Profile &src)
    {
      callCount=src.callCount;
      tickPure=src.tickPure;
      tickTotal=src.tickTotal;
      caller.Copy(src.caller);
    }

    void MixIn(const Profile &src)
    {
      callCount+=src.callCount;
      tickPure+=src.tickPure;
      tickTotal+=src.tickTotal;
      caller.MixIn(src.caller);
    }
  };

  /// map of profiles
  class ProfileMap
  {
    ProfileMap(const ProfileMap&);
    ProfileMap& operator=(const ProfileMap&);

    struct List
    {
      List *next;
      int frame;
      Profile p;
    };

    List *root,**tail;

  public:
    ProfileMap(void);
    ~ProfileMap();

    Profile *Find(int frame);
    void Append(int frame, const Profile &p);
    void MixIn(int frame, const Profile &p);
  };

  /// function entry (map address -> Function)
  struct Function
  {
    /// address of this function
    unsigned addr;

    /// global profile
    Profile glob;

    /// profile for current frame(s)
    Profile cur[MAX_FRAME_RECORDS];

    /// frame based profiles
    ProfileMap frame;

    /// current call depth (for recursion)
    int depth;

    /// function source
    char *funcSource;

    /// function source line
    unsigned funcLine;

    /// function name
    char *funcName;

    Function(ProfileFuncLevelTracer *tr)
    {
      glob.tracer=tr;
      for (int k=0;k<MAX_FRAME_RECORDS;k++)
        cur[k].tracer=tr;
      funcSource=funcName=NULL;
      funcLine=0;
    }
  };

  ProfileFuncLevelTracer(void);
  ~ProfileFuncLevelTracer();

  /**
    Enters the function at the given address.

    @param addr function address
    @param esp current ESP value
    @param ret return address for given function
  */
  void Enter(unsigned addr, unsigned esp, unsigned ret);

  /**
    Leaves the function at the ESP value.
    If this does not match with the last Enter call
    the internal stack is unwound until a match is found.

    @param esp current ESP value
    @return return address
  */
  unsigned Leave(unsigned esp);

  /**
    Shutdown function.
  */
  static void Shutdown(void);

  /**
    Starts frame based profiling, starts a new frame.
  */
  static int FrameStart(void);

  /**
    Ends frame based profiling.
  */
  static void FrameEnd(int which, int mixIndex);

  /**
    Clears all total values.
  */
  static void ClearTotals(void);

  /**
    Retrieves the first function level tracer.

    \return first function level tracer
  */
  static ProfileFuncLevelTracer *GetFirst(void)
  {
    return head;
  }

  /**
    Retrieves next function level tracer.

    \return next function level tracer, NULL if none
  */
  ProfileFuncLevelTracer *GetNext(void)
  {
    return next;
  }

  Function *FindFunction(unsigned addr)
  {
    return func.Find(addr);
  }

  Function *EnumFunction(unsigned index)
  {
    return func.Enumerate(index);
  }

private:
  /// single stack entry
  struct StackEntry
  {
    /// function
    Function *func;

    /// ESP value
    unsigned esp;

    /// return value
    unsigned retVal;

    /// enter tick count
    __int64 tickEnter;

    /// time spend in called functions
    __int64 tickSubTime;
  };

  /// map of functions
  class FunctionMap
  {
    FunctionMap(const FunctionMap&);
    FunctionMap& operator=(const FunctionMap&);

    struct Entry
    {
      Entry *next;
      Function *funcPtr;
    };

    enum
    {
      HASH_SIZE = 15551
    };

    Entry *e;
    unsigned alloc,used;
    Entry *hash[HASH_SIZE];

  public:
    FunctionMap(void);
    ~FunctionMap();

    Function *Find(unsigned addr);
    void Insert(Function *funcPtr);
    Function *Enumerate(int index);
  };

  /// head of list
  static ProfileFuncLevelTracer *head;

  /// next tracer object
  ProfileFuncLevelTracer *next;

  /// are we in shutdown mode?
  static bool shuttingDown;

  /// stack
  StackEntry *stack;

  /// number of used entries
  int usedStack;

  /// total number of entries
  int totalStack;

  /// max call depth
  int maxDepth;

  /// current frame
  static int curFrame;

  /// function map
  FunctionMap func;

  /// bit mask, currently recording which cur[] entries?
  static unsigned frameRecordMask;

  /// record caller information?
  static bool recordCaller;
};

inline void ProfileFuncLevelTracer::UnsignedMap::Insert(unsigned val, int countAdd)
{
  // in hash?
  unsigned at=(val/16)%HASH_SIZE;
  for (Entry *e=hash[at];e;e=e->next)
    if (e->val==val)
    {
      e->count+=countAdd;
      return;
    }
  _Insert(at,val,countAdd);
}

inline ProfileFuncLevelTracer::Function *ProfileFuncLevelTracer::FunctionMap::Find(unsigned addr)
{
  for (Entry *e=hash[(addr/16)%HASH_SIZE];e;e=e->next)
    if (e->funcPtr->addr==addr)
      return e->funcPtr;
  return NULL;
}

#endif // INTERNAL_FUNCLEVEL_H
