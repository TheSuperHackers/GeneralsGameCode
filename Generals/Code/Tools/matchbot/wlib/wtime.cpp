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

/****************************************************************************\
wtime                      Neal Kettler
\****************************************************************************/

#include <ctype.h>
#include "wtime.h"

static const char *DAYS[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

static const char *FULLDAYS[]={"Sunday","Monday","Tuesday","Wednesday","Thursday",
               "Friday","Saturday"};

static const char *MONTHS[]={"Jan","Feb","Mar","Apr","May","Jun","Jul",
               "Aug","Sep","Oct","Nov","Dec"};

static const char *FULLMONTHS[]={"January","February","March","April","May","June",
               "July","August","September","October","November","December"};

// MDC: Windows doesn't provide a localtime_r, so make our own...
#ifdef _WINDOWS
#ifdef _REENTRANT
#include "critsec.h"
static CritSec localtime_critsec;
#undef localtime
_CRTIMP struct tm *localtime(const time_t *clockval);
#endif // _REENTRANT

static struct tm *localtime_r(const time_t *clockval, struct tm *res) {
#ifdef _REENTRANT
	localtime_critsec.lock();
#endif
	struct tm *static_tm = localtime(clockval);
	res = (struct tm *)memcpy(res, static_tm, sizeof(tm));
#ifdef _REENTRANT
	localtime_critsec.unlock();
#endif
	return res;
}
#endif // _WINDOWS

Wtime::Wtime(void)
{
  Update();
}

Wtime::Wtime( Wtime &other )
{
  sign=other.sign;
  sec=other.sec;
  usec=other.usec;
}

Wtime::Wtime( uint32 other )
{
  sign=POSITIVE;
  sec=other;
  usec=0;
}

Wtime::~Wtime()
{
}

void Wtime::Update(void)
{
 sign=POSITIVE;
 #ifdef _WINDOWS
  struct _timeb    wintime;
  _ftime(&wintime);
  sec=wintime.time;
  usec=(wintime.millitm)*1000;
 #endif
 #ifndef _WINDOWS
  struct timeval   unixtime;
  struct timezone  unixtzone;
  gettimeofday(&unixtime,&unixtzone);
  sec=unixtime.tv_sec;
  usec=unixtime.tv_usec;
 #endif
}


// Parses a date string that's in modified RFC 1123 format
// Can have a +minutes after the normal time
// eg: Thu, 20 Jun 1996 17:33:49 +100
// Returns true if successfully parsed, false otherwise
bit8 Wtime::ParseDate(char *in)
{
  int i;
  uint32 minOffset;
  struct tm t;
  char *ptr=in;
  while ((!isgraph(*ptr))&&(*ptr!=0)) ptr++;  // skip to start of string
  if (*ptr==0) return(FALSE);
  t.tm_wday=-1;
  for (i=0; i<7; i++)  // parse day of week
    if (strncmp(ptr,DAYS[i],strlen(DAYS[i]))==0)
      t.tm_wday=i;
  if (t.tm_wday==-1)
    return(FALSE);
  while ((!isdigit(*ptr))&&(*ptr!=0)) ptr++;  // skip to day of month
  if (*ptr==0) return(FALSE);
  t.tm_mday=atoi(ptr);
  while ((!isalpha(*ptr))&&(*ptr!=0)) ptr++;  // skip to month
  if (*ptr==0) return(FALSE);
  t.tm_mon=-1;
  for (i=0; i<12; i++)  // match month
    if (strncmp(ptr,MONTHS[i],strlen(MONTHS[i]))==0) t.tm_mon=i;
  if (t.tm_mon==-1) return(FALSE);
  while ((!isdigit(*ptr))&&(*ptr!=0)) ptr++;
  if (*ptr==0) return(FALSE);
  t.tm_year=atoi(ptr);
  if (t.tm_year<70)  // if they specify a 2 digit year, we'll be nice
    t.tm_year+=2000;
  else if (t.tm_year<100)
    t.tm_year+=1900;
  if (t.tm_year>2200)  // I doubt my code will be around for another 203 years
    return(FALSE);
  while ((isdigit(*ptr))&&(*ptr!=0)) ptr++;  // skip to end of year
  if (*ptr==0) return(FALSE);

  while ((!isgraph(*ptr))&&(*ptr!=0)) ptr++;  // skip to start of time
  if (*ptr==0) return(FALSE);

  t.tm_hour=atoi(ptr);
  while ((*ptr!=':')&&(*ptr!=0)) ptr++;
  ptr++; // skip past colon
  if (*ptr==0) return(FALSE);
  t.tm_min=atoi(ptr);
  while ((*ptr!=':')&&(*ptr!=0)) ptr++;
  ptr++; // skip past colon
  if (*ptr==0) return(FALSE);
  t.tm_sec=atoi(ptr);
  t.tm_year%=100;   // 1996 is stored as 96, not 1996
  t.tm_isdst=-1;    // daylight savings info isn't available

  sec=(uint32)(mktime(&t));
  if ((sint32)sec==-1)
    return(FALSE);


  // The next part of the time is OPTIONAL (+minutes)

  // first skip past the seconds 
  while ((isdigit(*ptr))&&(*ptr!=0)) ptr++;
  if (*ptr==0) return(TRUE);

  // skip past any spaces 
  while ((isspace(*ptr))&&(*ptr!=0)) ptr++;
  if (*ptr!='+')
  {
    //printf("\nNOPE ptr was '%s'\n",ptr);
    return(TRUE);
  }
  ptr++;
  if (*ptr==0)
  {
    //printf("\nPTR WAS 0\n");
    return(TRUE);
  }
 
  minOffset=atol(ptr);
  //printf("\n\nAdding %d minutes!\n\n",minOffset);
  sec+=minOffset*60;  // add the minutes as seconds
  return(TRUE);
}


// This takes the standard Microsoft time formatting string
// make sure the out string is big enough
// An example format would be "mm/dd/yy hh:mm:ss"
// CHANGE:  Joe Howes  06/30/99
// To specify 12-hour format, use "aa" instead of "hh".
// The hours will be 12 hour and the string will be
// appended with " AM" or " PM".
bit8 Wtime::FormatTime(char *out, const char *format)
{
  int lastWasH=0;
  int ampmflag = 0;
  out[0]=0;
  const char *ptr=format;

  if (*ptr=='"') ptr++;  // skip past open quote if exists

  while (*ptr!=0)
  {
    if (lastWasH>0)
      lastWasH--;

    if (isspace(*ptr))
    {
      if (lastWasH==1) lastWasH=2;
      sprintf(out+strlen(out),"%c",*ptr);
      ptr+=1;
    } 
    else if (strncmp(ptr,"\"",1)==0)
    {
      break;
    }
    else if (strncmp(ptr,":",1)==0)
    {
      if (lastWasH==1) lastWasH=2;
      sprintf(out+strlen(out),":");
      ptr+=1;
    }
    else if (strncmp(ptr,"/",1)==0)
    {
      sprintf(out+strlen(out),"/");
      ptr+=1;
    }
    else if (strncmp(ptr,"c",1)==0)
    {
      sprintf(out+strlen(out),"%ld/%ld/%02ld %ld:%02ld:%02ld",GetMonth(),
        GetMDay(),GetYear()%100,GetHour(),GetMinute(),GetSecond());
      ptr+=1;
    }
    else if (strncmp(ptr,"dddddd",6)==0)
    {
      sprintf(out+strlen(out),"%s %02ld, %ld",FULLMONTHS[GetMonth()-1],
        GetMDay(),GetYear());
      ptr+=6;
    }
    else if (strncmp(ptr,"ddddd",5)==0)
    {
      sprintf(out+strlen(out),"%ld/%ld/%02ld",GetMonth(),GetMDay(),
        GetYear()%100);
      ptr+=5;
    }
    else if (strncmp(ptr,"dddd",4)==0)
    {
      sprintf(out+strlen(out),"%s",FULLDAYS[GetWDay()-1]);
      ptr+=4;
    }
    else if (strncmp(ptr,"ddd",3)==0)
    {
      sprintf(out+strlen(out),"%s",DAYS[GetWDay()-1]);
      ptr+=3;
    }
    else if (strncmp(ptr,"dd",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",GetMDay());
      ptr+=2;
    }
    else if (strncmp(ptr,"d",1)==0)
    {
      sprintf(out+strlen(out),"%ld",GetMDay());
      ptr+=1;
    }
    else if (strncmp(ptr,"ww",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",GetYWeek());
      ptr+=2;
    }
    else if (strncmp(ptr,"w",1)==0)
    {
      sprintf(out+strlen(out),"%ld",GetWDay());
      ptr+=1;
    }
    else if (strncmp(ptr,"mmmm",4)==0)
    {
      sprintf(out+strlen(out),"%s",FULLMONTHS[GetMonth()-1]);
      ptr+=4;
    }
    else if (strncmp(ptr,"mmm",3)==0)
    {
      sprintf(out+strlen(out),"%s",MONTHS[GetMonth()-1]);
      ptr+=3;
    }
    else if (strncmp(ptr,"mm",2)==0)
    {
      if (lastWasH==1)
        sprintf(out+strlen(out),"%02ld",GetMinute());
      else
        sprintf(out+strlen(out),"%02ld",GetMonth());
      ptr+=2;
    }
    else if (strncmp(ptr,"m",1)==0)
    {
      if (lastWasH==1)
        sprintf(out+strlen(out),"%ld",GetMinute());
      else
        sprintf(out+strlen(out),"%ld",GetMonth());
      ptr+=1;
    }
    else if (strncmp(ptr,"q",1)==0)
    {
      sprintf(out+strlen(out),"%ld",((GetMonth()-1)/4)+1);  // GetQuarter
      ptr+=1;
    } 
    else if (strncmp(ptr,"yyyy",4)==0)
    {
      sprintf(out+strlen(out),"%ld",GetYear());
      ptr+=4;
    }
    else if (strncmp(ptr,"yy",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",GetYear()%100);
      ptr+=2;
    } 
    else if (strncmp(ptr,"y",1)==0)
    {
      sprintf(out+strlen(out),"%ld",GetYDay());
      ptr+=1;
    }
    else if (strncmp(ptr,"hh",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",GetHour());
      lastWasH=2;  // needs to be 1 after top of loop decs it
      ptr+=2;
    }
    else if (strncmp(ptr,"h",1)==0)
    {
      sprintf(out+strlen(out),"%ld",GetHour());
      lastWasH=2;  // needs to be 1 after top of loop decs it
      ptr+=1;
    }
    else if (strncmp(ptr,"nn",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",GetMinute());
      ptr+=2;
    }
    else if (strncmp(ptr,"n",1)==0)
    {
      sprintf(out+strlen(out),"%ld",GetMinute());
      ptr+=1;
    }
    else if (strncmp(ptr,"ss",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",GetSecond());
      ptr+=2;
    }
    else if (strncmp(ptr,"s",1)==0)
    {
      sprintf(out+strlen(out),"%ld",GetSecond());
      ptr+=1;
    }
    else if (strncmp(ptr,"ttttt",5)==0)
    {
      sprintf(out+strlen(out),"%ld:%02ld:%02ld",GetHour(),GetMinute(),
        GetSecond());
      ptr+=5;
    }
    else if (strncmp(ptr,"aa",2)==0)
    {
      uint32 tmp = (GetHour() <= 12) ? GetHour() : GetHour() - 12;
      sprintf(out+strlen(out),"%02ld", tmp);
      lastWasH=2;  // needs to be 1 after top of loop decs it
      ptr+=2;
      ampmflag = 1;
    }
   else  // an unknown char, move to next
     ptr++;
  }
  if(ampmflag)
  {
    char ampm[4];
    if( GetHour() < 12 )
      strcpy(ampm, " AM");
    else
      strcpy(ampm, " PM");
    sprintf(out+strlen(out), "%s", ampm);
  }
  return(TRUE);
}



// In addition to PrintTime & PrintDate there is the 'Print' function
//   which prints both in RFC 1123 format

void Wtime::PrintTime(FILE *out) const
{
  char string[80];
  PrintTime(string);
  fprintf(out,"%s",string);
}

void Wtime::PrintTime(char *out) const
{
  sprintf(out," %02lu:%02lu:%02lu",GetHour(),GetMinute(),GetSecond());
}

void Wtime::PrintDate(FILE *out) const
{
  char string[80];
  PrintDate(string);
  fprintf(out,"%s",string);
}

void Wtime::PrintDate(char *out) const
{
  sprintf(out,"%s, %lu %s %lu",DAYS[GetWDay()-1],GetMDay(),MONTHS[GetMonth()-1],
    GetYear());
}

uint32 Wtime::GetSec(void) const
{
  return(sec);
}

uint32 Wtime::GetUsec(void) const
{
  return(usec);
}

void Wtime::SetSec(uint32 newsec)
{
  sec=newsec;
}

void Wtime::SetUsec(uint32 newusec)
{
  usec=newusec;
}

void Wtime::Set(uint32 newsec, uint32 newusec)
{
  sec=newsec;
  usec=newusec;
}

// Get a timeval ptr from a Wtime class
struct timeval *Wtime::GetTimeval(void)
{
  static struct timeval tv;
  tv.tv_sec=sec;
  tv.tv_usec=usec;
  return(&tv);
}

// Get a timeval ptr from a Wtime class
void Wtime::GetTimevalMT(struct timeval &tv)
{
  tv.tv_sec=sec;
  tv.tv_usec=usec;
}


uint32 Wtime::GetSecond(void) const
{
  struct tm  t;
  struct tm *tptr;
  tptr=localtime_r((time_t *)&sec,&t);
  return(tptr->tm_sec);
}
uint32 Wtime::GetMinute(void) const
{
  struct tm  t;
  struct tm *tptr;
  tptr=localtime_r((time_t *)&sec,&t);
  return(tptr->tm_min);
}
uint32 Wtime::GetHour(void) const
{
  struct tm  t;
  struct tm *tptr;
  tptr=localtime_r((time_t *)&sec,&t);
  return(tptr->tm_hour);
}
uint32 Wtime::GetMDay(void) const
{
  struct tm  t;
  struct tm *tptr;
  tptr=localtime_r((time_t *)&sec,&t);
  return(tptr->tm_mday);
}
uint32 Wtime::GetWDay(void) const
{
  struct tm  t;
  struct tm *tptr;
  tptr=localtime_r((time_t *)&sec,&t);
  return(tptr->tm_wday+1);
}
uint32 Wtime::GetYDay(void) const
{
  struct tm  t;
  struct tm *tptr;
  tptr=localtime_r((time_t *)&sec,&t);
  return(tptr->tm_yday+1);
}
uint32 Wtime::GetYWeek(void) const
{
  uint32 yweek;
  uint32 yday=GetYDay();
  uint32 wday=GetWDay();
  //phase holds the first weekday of the year.  If (Jan 1 = Sun) phase = 0
  sint32 phase=((wday-yday)%7);
  if (phase<0) phase+=7;
  yweek=((yday+phase-1)/7)+1; 
  return(yweek);
}
uint32 Wtime::GetMonth(void) const
{
  struct tm  t;
  struct tm *tptr;
  tptr=localtime_r((time_t *)&sec,&t);
  return(tptr->tm_mon+1);
}

uint32 Wtime::GetYear(void) const
{
  struct tm  t;
  struct tm *tptr;
  tptr=localtime_r((time_t *)&sec,&t);
  if ((tptr->tm_year)>=70)
    return((tptr->tm_year)+1900);
  else
    return((tptr->tm_year)+2000);
}


bit8 Wtime::GetSign(void) const
{
  return(sign);
}

// 1 = *this > other
//-1 = *this < other
// 0 = *this == other
int   Wtime::Compare(const Wtime &other) const
{
  if ((sec==other.sec)&&(usec==other.usec))
    return(0);        // equal

  else if (sec>other.sec)
    return(1);
  else if (sec<other.sec)
    return(-1);
  else if (usec>other.usec)
    return(1);
  else
    return(-1);
}
  

bit8 Wtime::operator == ( const Wtime &other ) const
{
  bit8 retval=Compare(other);
  if (retval==0)
    return(TRUE);
  else
    return(FALSE);
}

bit8 Wtime::operator != ( const Wtime &other ) const
{
  bit8 retval=Compare(other);
  if (retval==0)
    return(FALSE);
  else
    return(TRUE);
}

bit8 Wtime::operator < ( const Wtime &other ) const
{
  int retval=Compare(other);
  if (retval==-1)
    return(TRUE);
  else
    return(FALSE);
}

bit8 Wtime::operator > ( const Wtime &other ) const
{
  int retval=Compare(other);
  if (retval==1)
    return(TRUE);
  else
    return(FALSE);
}

bit8 Wtime::operator <= ( const Wtime &other ) const
{
  int retval=Compare(other);
  if ((retval==-1)||(retval==0))
    return(TRUE);
  else
    return(FALSE);
}

bit8 Wtime::operator >= ( const Wtime &other ) const
{
  int retval=Compare(other);
  if ((retval==1)||(retval==0))
    return(TRUE);
  else
    return(FALSE);
}


// None of the operators pay attention to sign
//   only the functions that begin with 'Signed'
void Wtime::SignedAdd(const Wtime &other)
{
  Wtime temp;

  if ((sign==POSITIVE)&&(other.sign==POSITIVE))
  {
    *this+=other;
    sign=POSITIVE;
  }
  else if ((sign==POSITIVE)&&(other.sign==NEGATIVE))
  {
    if (*this>other)
    {
      *this-=other;
      sign=POSITIVE;
    }
    else
    {
      temp=other;
      temp-=*this;
      *this=temp;
      sign=NEGATIVE;
    }
  }
  else if ((sign==NEGATIVE)&&(other.sign==POSITIVE))
  {
    if (*this<other)
    {
      temp=other;
      temp-=*this;
      *this=temp;
      sign=POSITIVE;
    }
    else
    {
      *this-=other;
      sign=NEGATIVE;
    }
  }
  else if ((sign==NEGATIVE)&&(other.sign==NEGATIVE))
  {
    *this+=other;
    sign=NEGATIVE;
  }
}



// None of the operators pay attention to sign
//   only the functions that begin with 'Signed'
void Wtime::SignedSubtract(const Wtime &other)
{
  Wtime temp;

  if ((sign==POSITIVE)&&(other.sign==NEGATIVE))
  {
    *this+=other;
    sign=POSITIVE;
  }
  else if ((sign==POSITIVE)&&(other.sign==POSITIVE))
  {
    if (*this>other)
    {
      *this-=other;
      sign=POSITIVE;
    }
    else
    {
      temp=other;
      temp-=*this;
      *this=temp;
      sign=NEGATIVE;
    }
  }
  else if ((sign==NEGATIVE)&&(other.sign==NEGATIVE))
  {
    if (*this<other)
    {
      temp=other;
      temp-=*this;
      *this=temp;
      sign=POSITIVE;
    }
    else
    {
      *this-=other;
      sign=NEGATIVE;
    }
  }
  else if ((sign==NEGATIVE)&&(other.sign==POSITIVE))
  {
    *this+=other;
    sign=NEGATIVE;
  }
}



Wtime &Wtime::operator += (const Wtime &other)
{
  sec+=other.sec;
  usec+=other.usec;
  if (usec>1000000)
  {
    sec++;
    usec-=1000000;
  }
  return *this;
}

Wtime &Wtime::operator -= (const Wtime &other)
{
  sint32 temp;
  if (Compare(other)==-1)
  {
    sec=0;                  // can't handle negative time 
    usec=0;
    return *this;
  }
  sec-=other.sec;
  temp=(sint32)usec;
  temp-=(sint32)other.usec;
  if (temp<0)
  {
    sec--;
    temp+=1000000;
  }
  usec=temp;
  return *this;
}

Wtime Wtime::operator - (Wtime &other)
{
  Wtime temp(*this);
  temp-=other;
  return(temp);
}

Wtime Wtime::operator + (Wtime &other)
{
  Wtime temp(*this);
  temp+=other;
  return(temp);
}


Wtime   &Wtime::operator = (const Wtime &other)
{
  sign=other.sign;
  sec=other.sec;
  usec=other.usec;
  return *this;
}


Wtime &Wtime::operator += (const uint32 other)
{
  sec+=other;
  return *this;
}


Wtime &Wtime::operator -= (const uint32 other)
{
  sec-=other;
  return *this;
}


Wtime Wtime::operator - (uint32 other)
{
  Wtime temp(*this);
  temp-=other;
  return(temp);
}


Wtime Wtime::operator + (uint32 other)
{
  Wtime temp(*this);
  temp+=other;
  return(temp);
}


Wtime &Wtime::operator = (const uint32 other)
{
  sign=POSITIVE;
  sec=other;
  usec=0;
  return *this;
}
