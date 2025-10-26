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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_stack.h $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Stack walker
//////////////////////////////////////////////////////////////////////////////

#pragma once

/// \brief stack walker class (singleton)
class DebugStackwalk
{
  friend class Debug;

  DebugStackwalk(const DebugStackwalk&);
  DebugStackwalk& operator=(DebugStackwalk&);

  // private so that only Debug can create and destroy us
  DebugStackwalk(void);
  ~DebugStackwalk();

public:

  /// \brief a stack trace signature
  class Signature
  {
    // makes life easier :)
    friend class DebugStackwalk;

    /// max # of possible addresses
    enum { MAX_ADDR = 256 };

    /// number of addresses
    unsigned m_numAddr;

    /// addresses
    unsigned m_addr[MAX_ADDR];

  public:
    explicit Signature(void): m_numAddr(0) {}
    Signature(const Signature &src);
    Signature& operator=(const Signature& src);

    /**
      \brief Determine the number of addresses in this signature.

      \return number of addresses in this signature
    */
    unsigned Size(void) const { return m_numAddr; }

    /**
      \brief Get a single address from the signature.

      The address at index 0 is always on top of the stack.

      \param n index, 0..Size()-1
      \return signature address
    */
    unsigned GetAddress(int n) const;

    /**
      \brief Strong ordering operator.

      Signatures are ordered by looking at the bottom of the stack
      first. Implemented inline so that STL algorithms using this
      operator can be compiled more efficiently.
    */
    bool operator<(const Signature &other) const
    {
      unsigned m=m_numAddr<other.m_numAddr?m_numAddr:other.m_numAddr;
      for (unsigned k=0;k<m;k++)
      {
        if (m_addr[m_numAddr-k-1]<other.m_addr[other.m_numAddr-k-1])
          return true;
        if (m_addr[m_numAddr-k-1]>other.m_addr[other.m_numAddr-k-1])
          return false;
      }
      return m_numAddr<other.m_numAddr;
    }

    /**
      \brief Determines symbol for given address.

      The data is returned in the form
      \<addr\> \<module\>+NN, \<sym\>+NN, \<file\>:\<line\>+NN

      \param addr function address
      \param buf return buffer
      \param bufSize size of return buffer, minimum is 64 bytes (256 recommended)
    */
    static void GetSymbol(unsigned addr, char *buf, unsigned bufSize);

    /**
      \brief Determines symbol for given address.

      \param addr function address
      \param bufMod module buffer, may be NULL
      \param sizeMod size of buffer, including NUL, minimum 16 if given
      \param relMod relative address within module, may be NULL
      \param bufSym symbol buffer, may be NULL
      \param sizeSym size of buffer, including NUL
      \param relSym relative address within symbol, may be NULL
      \param bufFile file name buffer, may be NULL
      \param sizeFile size of buffer, including NUL
      \param line line number, may be NULL
      \param relLine relative address within line, may be NULL
    */
    static void GetSymbol(unsigned addr,
                          char *bufMod, unsigned sizeMod, unsigned *relMod,
                          char *bufSym, unsigned sizeSym, unsigned *relSym,
                          char *bufFile, unsigned sizeFile, unsigned *line, unsigned *relLine);
  };

  /** \internal
    \brief Returns dbghelp.dll DLL handle.

    \return dbghelp.dll DLL handle
  */
  static void *GetDbghelpHandle(void);

  /** \internal
    \brief Checks if dbghelp.dll version is old.

    \return true if old version, false if not
  */
  static bool IsOldDbghelp(void);

  /**
    \brief Walks the stack from the given address.

    \param sig stack signature to return
    \param ctx processor context, if NULL then use current address
    \return number of addresses found
  */
  static int StackWalk(Signature &sig, struct _CONTEXT *ctx=0);
};

/**
  \brief Dumps a complete signature with symbols.

  \param dbg debug instance
  \param sig signature
  \return debug instance
*/
Debug& operator<<(Debug &dbg, const DebugStackwalk::Signature &sig);
