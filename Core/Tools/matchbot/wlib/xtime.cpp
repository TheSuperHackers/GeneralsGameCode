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

/****************************************************************************\

xtime                      Neal Kettler

Improved version of the Wtime library (V->W->X...)

Handles signed times better and dates long
long long after you'll be dead.

\****************************************************************************/

#include <ctype.h>
#include <time.h>
#ifndef _WINDOWS
#include <sys/time.h>
#endif
#include "xtime.h"

static const char *DAYS[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

static const char *FULLDAYS[]={"Sunday","Monday","Tuesday","Wednesday","Thursday",
               "Friday","Saturday"};

static const char *MONTHS[]={"Jan","Feb","Mar","Apr","May","Jun","Jul",
               "Aug","Sep","Oct","Nov","Dec"};

static const char *FULLMONTHS[]={"January","February","March","April","May","June",
               "July","August","September","October","November","December"};


#define IS_LEAP(y) ((y) % 4) == 0 && (! ((y) % 100) == 0 || ((y) % 400) == 0)
#define LEAPS_THRU_END_OF(y) ((y) / 4 - (y) / 100 + (y) / 400)


/////////////// Utility functions ///////////////////

//
// Return the daycount since year 0 for the specified date.
// month = 1-12, day = 1-31 year = 0...
//
static sint32 Get_Day(int month, int day, int year)
{
  time_t days;

  static int DaysAtMonth[] =
      {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

  /* Add up number of straight days + number of leap days + days */
  /* up to the month + the day of month. */

  days = (year * 365) + (year / 4) - (year / 100) + (year / 400) +
      DaysAtMonth[month-1] + day;

  /* If we haven't hit Feb 29, and this is a leap year, we need to */
  /* subtract out the leap day that was added above for this year */

  if (month < 3 && IS_LEAP(year))
      --days;
    return(days);
}



//
// Get the year from a daycount since year 0
// Also get the daycount since the start of the year
//
// Ayecarumba what a pain in the ass!
//
static bit8 Get_Date_From_Day(sint32 days, OUT sint32 &year, OUT sint32 &yday)
{
  //register long int rem;
  register long int y;
  //register const unsigned short int *ip;

  if (days <= 365)
  {
    year=0;
    yday=days+1;  // 1 based
    return(TRUE);
  }

  y = 1;
  days-=365;

  days--;  // zero based

  //
  // As far as I can tell there's no non-iteritive way to
  //   do this...
  //
  while (days < 0 || days >= (IS_LEAP (y) ? 366 : 365))
  {
    /* Guess a corrected year, assuming 365 days per year.  */
    long int yg = y + days / 365 - (days % 365 < 0);

    /* Adjust DAYS and Y to match the guessed year.  */
    days -= ((yg - y) * 365
        + LEAPS_THRU_END_OF (yg - 1)
        - LEAPS_THRU_END_OF (y - 1));
      y = yg;
  }
  year=y;
  yday=days+1;  // 1 based
  return(TRUE);
}


//
// Get the max day of a given month in a given year
//
int Max_Day(int month, int year)
{
  static char dayTable[2][13] = {
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
  };
  bit8 isleap=IS_LEAP(year);
  return(dayTable[isleap][month]);
}



/**********************************************************
int main(int argc, char *argv[])
{
  Xtime wtime;
  int month,mday,year,hour,minute,second,day;
  int i;
  int dayoffset;
  int yr;
  int yday;
  int ydaycount;

  wtime.set(719528,0);

***********************************************
  for (year=0; year<10000; year++)
  {
    ydaycount=1;
    for (month=1; month<=12; month++)
    {
      for (day=1; day<=Max_Day(month,year); day++)
      {
        dayoffset=Get_Day(month,day,year);
        assert (GetDateFromDay(dayoffset,yr,yday)==TRUE);

        //printf("Yday=%d YdayCount=%d\n",yday,ydaycount);

        if (yr!=year)
        {
          printf("MO=%d  DAY=%d  YEAR=%d  YR=%d\n",month,day,year,yr);
          assert(0);
        }
        if (yday != ydaycount)
        {
          printf("MO=%d  DAY=%d  YEAR=%d  YR=%d\n",month,day,year,yr);
          printf("Yday=%d YdayCount=%d\n",yday,ydaycount);
          assert(0);
        }
        ydaycount++;
      }
    }
    printf("(%d) ",year);
  }
***************************************

  ///////wtime.addSeconds((60*60*24)-(60*60*8));
  ////////wtime.addSeconds(-(60*60*8));  // timezone delta

  dayoffset=Get_Day(1,1,1970);
  printf("DAYOFFSET = %d\n",dayoffset);


  wtime.getTime(month, mday, year, hour, minute, second);
  printf("\n%s %d  %d  %d:%02d:%02d\n\n",
    MONTHS[month-1],mday,year,hour,minute,second);

  struct  timeval   unixtime;
  struct  timezone  unixtzone;
  time_t    ttime;
  tm        tmtime;

  memset(&tmtime,0,sizeof(tmtime));
  ttime=0;
  unixtime.tv_sec=0;
  unixtime.tv_usec=0;

  //gettimeofday(&unixtime,&unixtzone);
  //ttime=time(NULL);
  tmtime=*gmtime(&ttime);

  printf("TIME->CTIME = %s\n",ctime(&ttime));
  printf("GTOD->CTIE = %s\n",ctime(&(unixtime.tv_sec)));
  printf("TIME->GMT->ASCTIME = %s\n",asctime(&tmtime));
}
***************************************************************/



//
// Construct with current clock time
//
Xtime::Xtime(void)
{
  update();
}

//
// Copy constructor
//
Xtime::Xtime( Xtime &other )
{
  day_=other.day_;
  msec_=other.msec_;
}

//
// Set to a time_t (1970-2038)
//
Xtime::Xtime( time_t other )
{
  day_=719528;  // days from year 0 to Jan1, 1970
  // Add seconds from time_t
  addSeconds(other);
  msec_=0;
}

Xtime::~Xtime()
{
}

//
// Add some number of seconds to the time (seconds can be negative)
//
void Xtime::addSeconds(sint32 seconds)
{
  // Add to day counter first
  day_+=(seconds/86400);
  msec_+=(seconds % 86400)*1000;

  // Now normalize in case msec is > 1 days worth
  normalize();
}

//
// If msec is > 1 days worth, adjust the day count
//   & decrement the msec counter until OK.
//
void Xtime::normalize(void)
{
  day_+=(msec_/86400000);
  msec_%=86400000;

  while (msec_ < 0)
  {
    day_--;
    msec_+=86400000;
  }
}

//
// Update time to hold the current clock time
// (breaks in 2038 :-)
//
void Xtime::update(void)
{
  day_=719528;  // day_s from year 0 to Jan1, 1970
  msec_=0;

 #ifdef _WINDOWS
  struct _timeb    wintime;
  _ftime(&wintime);
  addSeconds(wintime.time);
  msec_+=wintime.millitm;
 #endif
 #ifndef _WINDOWS
  struct timeval   unixtime;
  struct timezone  unixtzone;
  gettimeofday(&unixtime,&unixtzone);
  addSeconds(unixtime.tv_sec);
  msec_+=(unixtime.tv_usec/1000);
 #endif

  // Now normalize in case msec is > 1 days worth
  normalize();
}


// This takes the standard Microsoft time formatting string
// make sure the out string is big enough
// An example format would be "mm/dd/yy hh:mm:ss"
// CHANGE:  Joe Howes  06/30/99
// To specify 12-hour format, use "aa" instead of "hh".
// The hours will be 12 hour and the string will be
// appended with " AM" or " PM".
bit8 Xtime::FormatTime(char *out, const char *format)
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
      sprintf(out+strlen(out),"%ld/%ld/%02ld %ld:%02ld:%02ld",getMonth(),
        getMDay(),getYear()%100,getHour(),getMinute(),getSecond());
      ptr+=1;
    }
    else if (strncmp(ptr,"dddddd",6)==0)
    {
      sprintf(out+strlen(out),"%s %02ld, %ld",FULLMONTHS[getMonth()-1],
        getMDay(),getYear());
      ptr+=6;
    }
    else if (strncmp(ptr,"ddddd",5)==0)
    {
      sprintf(out+strlen(out),"%ld/%ld/%02ld",getMonth(),getMDay(),
        getYear()%100);
      ptr+=5;
    }
    /*else if (strncmp(ptr,"dddd",4)==0)
    {
      sprintf(out+strlen(out),"%s",FULLDAYS[getWDay()-1]);
      ptr+=4;
    }
    else if (strncmp(ptr,"ddd",3)==0)
    {
      sprintf(out+strlen(out),"%s",DAYS[getWDay()-1]);
      ptr+=3;
    }*/
    else if (strncmp(ptr,"dd",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",getMDay());
      ptr+=2;
    }
    else if (strncmp(ptr,"d",1)==0)
    {
      sprintf(out+strlen(out),"%ld",getMDay());
      ptr+=1;
    }
    /*else if (strncmp(ptr,"ww",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",getYWeek());
      ptr+=2;
    }*/
    /*else if (strncmp(ptr,"w",1)==0)
    {
      sprintf(out+strlen(out),"%ld",getWDay());
      ptr+=1;
    }*/
    else if (strncmp(ptr,"mmmm",4)==0)
    {
      sprintf(out+strlen(out),"%s",FULLMONTHS[getMonth()-1]);
      ptr+=4;
    }
    else if (strncmp(ptr,"mmm",3)==0)
    {
      sprintf(out+strlen(out),"%s",MONTHS[getMonth()-1]);
      ptr+=3;
    }
    else if (strncmp(ptr,"mm",2)==0)
    {
      if (lastWasH==1)
        sprintf(out+strlen(out),"%02ld",getMinute());
      else
        sprintf(out+strlen(out),"%02ld",getMonth());
      ptr+=2;
    }
    else if (strncmp(ptr,"m",1)==0)
    {
      if (lastWasH==1)
        sprintf(out+strlen(out),"%ld",getMinute());
      else
        sprintf(out+strlen(out),"%ld",getMonth());
      ptr+=1;
    }
    else if (strncmp(ptr,"q",1)==0)
    {
      sprintf(out+strlen(out),"%ld",((getMonth()-1)/4)+1);  // GetQuarter
      ptr+=1;
    }
    else if (strncmp(ptr,"yyyy",4)==0)
    {
      sprintf(out+strlen(out),"%ld",getYear());
      ptr+=4;
    }
    else if (strncmp(ptr,"yy",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",getYear()%100);
      ptr+=2;
    }
    /*else if (strncmp(ptr,"y",1)==0)
    {
      sprintf(out+strlen(out),"%ld",getYDay());
      ptr+=1;
    }*/
    else if (strncmp(ptr,"hh",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",getHour());
      lastWasH=2;  // needs to be 1 after top of loop decs it
      ptr+=2;
    }
    else if (strncmp(ptr,"h",1)==0)
    {
      sprintf(out+strlen(out),"%ld",getHour());
      lastWasH=2;  // needs to be 1 after top of loop decs it
      ptr+=1;
    }
    else if (strncmp(ptr,"nn",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",getMinute());
      ptr+=2;
    }
    else if (strncmp(ptr,"n",1)==0)
    {
      sprintf(out+strlen(out),"%ld",getMinute());
      ptr+=1;
    }
    else if (strncmp(ptr,"ss",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",getSecond());
      ptr+=2;
    }
    else if (strncmp(ptr,"s",1)==0)
    {
      sprintf(out+strlen(out),"%ld",getSecond());
      ptr+=1;
    }
    else if (strncmp(ptr,"ttttt",5)==0)
    {
      sprintf(out+strlen(out),"%ld:%02ld:%02ld",getHour(),getMinute(),
        getSecond());
      ptr+=5;
    }
    else if (strncmp(ptr,"aa",2)==0)
    {
      uint32 tmp = (getHour() <= 12) ? getHour() : getHour() - 12;
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
    if( getHour() < 12 )
      strcpy(ampm, " AM");
    else
      strcpy(ampm, " PM");
    sprintf(out+strlen(out), "%s", ampm);
  }
  return(TRUE);
}

/**************************************

 REPLACE THIS CRAP

// Parses a date string that's in modified RFC 1123 format
// Can have a +minutes after the normal time
// eg: Thu, 20 Jun 1996 17:33:49 +100
// Returns true if successfully parsed, false otherwise
bit8 Xtime::ParseDate(char *in)
{
  int i;
  uint32 minOffset;
  struct tm t;
  char *ptr=in;
  while ((!isgraph(*ptr))&&(*ptr!=0)) ptr++;  // skip to start of string
  if (*ptr==0) return(FALSE);
  t.tm_wday_=-1;
  for (i=0; i<7; i++)  // parse day_ of week
    if (strncmp(ptr,DAYS[i],strlen(DAYS[i]))==0)
      t.tm_wday_=i;
  if (t.tm_wday_==-1)
    return(FALSE);
  while ((!isdigit(*ptr))&&(*ptr!=0)) ptr++;  // skip to day_ of month
  if (*ptr==0) return(FALSE);
  t.tm_mday_=atoi(ptr);
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
  t.tm_isdst=-1;    // day_light savings info isn't available

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



// In addition to PrintTime & PrintDate there is the 'Print' function
//   which prints both in RFC 1123 format

void Xtime::PrintTime(FILE *out) const
{
  char string[80];
  PrintTime(string);
  fprintf(out,"%s",string);
}

void Xtime::PrintTime(char *out) const
{
  sprintf(out," %02lu:%02lu:%02lu",GetHour(),GetMinute(),GetSecond());
}

void Xtime::PrintDate(FILE *out) const
{
  char string[80];
  PrintDate(string);
  fprintf(out,"%s",string);
}

void Xtime::PrintDate(char *out) const
{
  sprintf(out,"%s, %lu %s %lu",DAYS[GetWDay()-1],GetMDay(),MONTHS[GetMonth()-1],
    GetYear());
}
********************************************/

// Get day_s since year 0
sint32 Xtime::getDay(void) const
{
  return(day_);
}

// Get msecs since start of day
sint32 Xtime::getMsec(void) const
{
  return(msec_);
}

// Set days since year 0
void Xtime::setDay(sint32 newday)
{
  day_=newday;
}

// Set msec since start of this day
void Xtime::setMsec(sint32 newmsec)
{
  msec_=newmsec;
}

// Set both
void Xtime::set(sint32 newday, sint32 newmsec)
{
  day_=newday;
  msec_=newmsec;
}


//
// Get a timeval ptr from a Xtime class
// May fail if timeval can't hold a year this big or small
//
bit8 Xtime::getTimeval(struct timeval &tv)
{
  // A timeval can only hold dates from 1970-2038
  if ((day_ < 719528) || (day_ >= 719528+24855))
    return(FALSE);

  // Compute seconds since Jan 1, 1970
  uint32 seconds=day_-719528;
  seconds*=(60*60*24);
  seconds+=(msec_/1000);

  tv.tv_sec=seconds;
  tv.tv_usec=(msec_%1000)*1000;
  return(TRUE);
}

//
// Set the time
//
bit8 Xtime::setTime(int month, int mday, int year, int hour, int minute, int second)
{
  day_=Get_Day(month,mday,year);
  msec_=(hour*1000*60*60)+(minute*1000*60)+(second*1000);
  return(TRUE);
}



int Xtime::getYDay(void) const   // Day of Year  (1-366)  (366 = leap yr)
{
	int year;
	sint32 dayofyear;
	if (Get_Date_From_Day(day_,year,dayofyear)==FALSE)
		return(-1);
	return dayofyear;
}


//
// Get all the components of the time in the usual normalized format.
//
// Most of the uglyness is in Get_Date_From_Day()
//
bit8 Xtime::getTime(int &month, int &mday, int &year, int &hour, int &minute, int &second) const
{
  int i;
  sint32 dayofyear;
  if (Get_Date_From_Day(day_,year,dayofyear)==FALSE)
    return(FALSE);

  static int DaysAtMonth[2][12] = {
        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},   // normal
        {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}    // leap year
  };

  month=0;

  bit8 isleap=IS_LEAP(year);
  for (i=0; i<12; i++)
  {
    if (DaysAtMonth[isleap][i] >= dayofyear)
      break;
    month=i;
  }
  month++;  // 1 based

  mday=dayofyear-DaysAtMonth[isleap][month-1];

  // Whew! Now all we have to do is figure out H/M/S from the msec!
  hour=(msec_/3600000)%24;  // 1000*60*60
  minute=(msec_/60000)%60;  // 1000*60
  second=(msec_/ 1000)%60;  // 1000

  return(TRUE);
}

//
// These are for getting components of the time in the
//   standard ranges.  Like Day 1-31, Second 0-59, etc...
//
int Xtime::getSecond(void) const
{
  int month,mday,year,hour,minute,second;
  getTime(month, mday, year, hour, minute, second);
  return(second);
}
int Xtime::getMinute(void) const
{
  int month,mday,year,hour,minute,second;
  getTime(month, mday, year, hour, minute, second);
  return(minute);
}
int Xtime::getHour(void) const
{
  int month,mday,year,hour,minute,second;
  getTime(month, mday, year, hour, minute, second);
  return(hour);
}
int Xtime::getMDay(void) const
{
  int month,mday,year,hour,minute,second;
  getTime(month, mday, year, hour, minute, second);
  return(mday);
}

int Xtime::getMonth(void) const
{
  int month,mday,year,hour,minute,second;
  getTime(month, mday, year, hour, minute, second);
  return(month);
}

// based at year 0 (real 0, not 1970)
int Xtime::getYear(void) const
{
  int month,mday,year,hour,minute,second;
  getTime(month, mday, year, hour, minute, second);
  return(year);
}


//
// Set the seconds value (0-59)
//
bit8 Xtime::setSecond(sint32 sec)
{
  sint32 second=(msec_/ 1000)%60;
  msec_-=(second*1000);
  msec_+=(sec*1000);
  return(TRUE);
}

//
// Set the minutes value (0-59)
//
bit8 Xtime::setMinute(sint32 min)
{
  sint32 minute=(msec_/60000)%60;  // 1000*60
  msec_-=(minute*60000);
  msec_+=(min*60000);
  return(TRUE);
}

//
// Set the minutes value (0-23)
//
bit8 Xtime::setHour(sint32 hour)
{
  hour=(msec_/3600000)%24;  // 1000*60*60
  msec_-=(hour*3600000);
  msec_+=(hour*3600000);
  return(TRUE);
}

//
// Set the year value
// Results are undefined if you're moving from Feb 29, to a non leap year
//
bit8 Xtime::setYear(sint32 _year)
{
  // extract the date
  int month,mday,year,hour,min,sec;
  getTime(month,mday,year,hour,min,sec);

  // modify & rebuild
  year=_year;
  day_=Get_Day(month,mday,year);
  return(TRUE);
}

//
// Modify the month
//
bit8 Xtime::setMonth(sint32 _month)
{
  // extract the date
  int month,mday,year,hour,min,sec;
  getTime(month,mday,year,hour,min,sec);

  // modify & rebuild
  month=_month;
  day_=Get_Day(month,mday,year);
  return(TRUE);
}


//
// Modify the day of the month
//
bit8 Xtime::setMDay(sint32 _mday)
{
  // extract the date
  int month,mday,year,hour,min,sec;
  getTime(month,mday,year,hour,min,sec);

  // modify & rebuild
  mday=_mday;
  day_=Get_Day(month,mday,year);
  return(TRUE);
}


//
// Compare two times.  The time better be normalized
//   which it would be unless you've put bad stuff in it!
//
// 1 = *this > other
//-1 = *this < other
// 0 = *this == other
int   Xtime::compare(const Xtime &other) const
{
  if ((day_==other.day_)&&(msec_==other.msec_))
    return(0);        // equal

  else if (day_>other.day_)
    return(1);
  else if (day_<other.day_)
    return(-1);
  else if (msec_>other.msec_)
    return(1);
  else
    return(-1);
}


bit8 Xtime::operator == ( const Xtime &other ) const
{
  bit8 retval=compare(other);
  if (retval==0)
    return(TRUE);
  else
    return(FALSE);
}

bit8 Xtime::operator != ( const Xtime &other ) const
{
  bit8 retval=compare(other);
  if (retval==0)
    return(FALSE);
  else
    return(TRUE);
}

bit8 Xtime::operator < ( const Xtime &other ) const
{
  int retval=compare(other);
  if (retval==-1)
    return(TRUE);
  else
    return(FALSE);
}

bit8 Xtime::operator > ( const Xtime &other ) const
{
  int retval=compare(other);
  if (retval==1)
    return(TRUE);
  else
    return(FALSE);
}

bit8 Xtime::operator <= ( const Xtime &other ) const
{
  int retval=compare(other);
  if ((retval==-1)||(retval==0))
    return(TRUE);
  else
    return(FALSE);
}

bit8 Xtime::operator >= ( const Xtime &other ) const
{
  int retval=compare(other);
  if ((retval==1)||(retval==0))
    return(TRUE);
  else
    return(FALSE);
}


Xtime &Xtime::operator += (const Xtime &other)
{
  day_+=other.day_;
  msec_+=other.msec_;
  normalize();
  return *this;
}

Xtime &Xtime::operator -= (const Xtime &other)
{
  day_-=other.day_;
  msec_-=other.msec_;
  normalize();
  return *this;
}

Xtime Xtime::operator - (Xtime &other)
{
  Xtime temp(*this);
  temp-=other;
  return(temp);
}

Xtime Xtime::operator + (Xtime &other)
{
  Xtime temp(*this);
  temp+=other;
  return(temp);
}


Xtime   &Xtime::operator = (const Xtime &other)
{
  day_=other.day_;
  msec_=other.msec_;
  return *this;
}


Xtime &Xtime::operator += (const time_t other)
{
  addSeconds(other);
  return *this;
}


Xtime &Xtime::operator -= (const time_t other)
{
  addSeconds(-((sint32)other));
  return *this;
}


Xtime Xtime::operator - (time_t other)
{
  Xtime temp(*this);
  temp-=other;
  return(temp);
}


Xtime Xtime::operator + (time_t other)
{
  Xtime temp(*this);
  temp+=other;
  return(temp);
}

Xtime &Xtime::operator = (const time_t other)
{
  msec_=0;
  day_=719528;  // Jan 1, 1970
  addSeconds(other);
  return *this;
}
