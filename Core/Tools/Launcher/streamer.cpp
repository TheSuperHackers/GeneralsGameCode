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


#include "streamer.h"
#ifdef _WIN32
  #include <windows.h>
#endif

#if defined(USING_STLPORT) || (defined(_MSC_VER) && _MSC_VER < 1300)
#define STREAMER_UNBUFFERED unbuffered()
#else
#define STREAMER_UNBUFFERED 0
#endif

Streamer::Streamer() : streambuf(), Output_Device(nullptr), Buf(nullptr)
{
#if defined(USING_STLPORT) || (defined(_MSC_VER) && _MSC_VER < 1300)
  int state=unbuffered();
  unbuffered(0);  // 0 = buffered, 1 = unbuffered
#else
  static_assert(STREAMER_UNBUFFERED==0, "std::streambuf is assumed to be buffered by default");
#endif
}

Streamer::~Streamer()
{
  ///////// calling sync seems to cause crashes here on Win32
  //sync();
  /////////

  delete[] Buf;
}

int Streamer::setOutputDevice(OutputDevice *device)
{
  Output_Device=device;
  return(0);
}


// put n chars from string into buffer
int Streamer::xsputn(const char* buf, int size) //implementation of sputn
{
  if (size<=0)  // Nothing to do
    return(0);

  const unsigned char *ptr=(const unsigned char *)buf;
  for (int i=0; i<size; i++, ptr++)
  {
    if(*ptr=='\n')
    {
      if (overflow(*ptr)==EOF)
        return(i);
    }
    else if (sputc(*ptr)==EOF)
      return(i);
  }
  return(size);
}

// Flush the buffer and make more room if needed
int Streamer::overflow(int c)
{
  if (c==EOF)
    return(sync());
  if ((pbase()==0) && (doallocate()==0))
    return(EOF);
  if((pptr() >= epptr()) && (sync()==EOF))
    return(EOF);
  else {
    sputc(c);
    if ((STREAMER_UNBUFFERED && c=='\n' || pptr() >= epptr())
        && sync()==EOF) {
      return(EOF);
    }
    return(c);
  }
}

// This is a write only stream, this should never happen
int Streamer::underflow(void)
{
  return(EOF);
}

int Streamer::doallocate()
{
  if (Buf==nullptr)
  {
    Buf=new char[(2*STREAMER_BUFSIZ)];   // deleted by destructor
    memset(Buf,0,2*STREAMER_BUFSIZ);

    // Buffer
#if defined(USING_STLPORT) || (defined(_MSC_VER) && _MSC_VER < 1300)
    setb(
       Buf,         // base pointer
       Buf+STREAMER_BUFSIZ,  // ebuf pointer (end of buffer);
       0);          // 0 = manual deletion of buff
#else
    pubsetbuf(Buf, 2*STREAMER_BUFSIZ);
#endif

    // Get area
    setg(
        Buf,   // eback
        Buf,   // gptr
        Buf);  // egptr

    Buf+=STREAMER_BUFSIZ;
    // Put area
    setp(Buf,Buf+STREAMER_BUFSIZ);
    return(1);
  }
  else
    return(0);
}


int Streamer::sync()
{
  if (pptr()<=pbase()) {
    return(0);
  }

  int wlen=pptr()-pbase();

  if (Output_Device)
  {
    Output_Device->print(pbase(),wlen);
  }

  if (STREAMER_UNBUFFERED) {
    setp(pbase(),pbase());
  }
  else {
    setp(pbase(),pbase()+STREAMER_BUFSIZ);
  }
  return(0);
}
