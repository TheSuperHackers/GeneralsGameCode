/*
**	Command & Conquer Generals(tm)
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


Streamer::Streamer() : std::streambuf()
{
  //int state= unbuffered();
  //unbuffered(0);  // 0 = buffered, 1 = unbuffered
}
 
Streamer::~Streamer()
{
  sync();
  //delete[](base());
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
    if ((//unbuffered() && 
        c=='\n' || pptr() >= epptr())
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
#if 1
  if (pbase()==NULL)
  {
    char *buf=new char[(2*STREAMER_BUFSIZ)];   // deleted by destructor
    memset(buf,0,2*STREAMER_BUFSIZ);

    // Buffer
    setbuf(
       buf,         // base pointer
       STREAMER_BUFSIZ  // ebuf pointer (end of buffer);
       );          // 0 = manual deletion of buff 

    // Get area
    setg(
        buf,   // eback 
        buf,   // gptr
        buf);  // egptr

    buf+=STREAMER_BUFSIZ;
    // Put area
    setp(buf,buf+STREAMER_BUFSIZ);
    return(1);
  }
  else
    return(0);
#endif
  return 1;
}


int Streamer::sync()
{
  if (pptr()<=pbase()) {
    return(0);
  }

  int wlen=(int)(uintptr_t)(pptr()-pbase());

  if (Output_Device)
  {
    Output_Device->print(pbase(),wlen);
  }

#if 0
  if (unbuffered()) {
    setp(pbase(),pbase());
  }
  else 
#endif
  {
    setp(pbase(),pbase()+STREAMER_BUFSIZ);
  }
  return(0);
}
