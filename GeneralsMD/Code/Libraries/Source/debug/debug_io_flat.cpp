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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_io_flat.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Debug I/O class flat (flat or split log file)
//////////////////////////////////////////////////////////////////////////////
#include "_pch.h"
#include <stdlib.h>
#include <new>      // needed for placement new prototype

DebugIOFlat::OutputStream::OutputStream(const char *filename, unsigned maxSize):
  m_bufferUsed(0), m_nextChar(0)
{
  m_fileName=(char *)DebugAllocMemory(strlen(filename)+1);
  strcpy(m_fileName,filename);

  m_limitedFileSize=maxSize>0;
  m_bufferSize=m_limitedFileSize?maxSize:0x10000;
  m_buffer=(char *)DebugAllocMemory(m_bufferSize);

  if (!m_limitedFileSize)
    m_fileHandle=CreateFile(m_fileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,
                            NULL);
}

DebugIOFlat::OutputStream::~OutputStream()
{
}

DebugIOFlat::OutputStream *DebugIOFlat::OutputStream::Create(const char *filename, unsigned maxSize)
{
  return new (DebugAllocMemory(sizeof(OutputStream))) OutputStream(filename,maxSize);
}

void DebugIOFlat::OutputStream::Delete(const char *path)
{
  Flush();
  if (!m_limitedFileSize)
    CloseHandle(m_fileHandle);

  if (path&&*path)
  {
    // copy file to given path
    int run=-1;
    char *ext=strrchr(m_fileName,'.');
    if (!ext)
      ext=m_fileName+strlen(m_fileName);
    char *fileNameOnly=strrchr(m_fileName,'\\');
    fileNameOnly=fileNameOnly?fileNameOnly+1:m_fileName;

    unsigned pathLen=strlen(path);
    for (;;)
    {
      // absolute path?
      char help[512];
      if (path[0]&&(path[1]==':'||(path[0]=='\\'&&path[1]=='\\')))
      {
        strcpy(help,path);
        strcpy(help+pathLen,fileNameOnly);
        help[ext-fileNameOnly+pathLen]=0;
      }
      else
      {
        // no, relative path given
        strcpy(help,m_fileName);
        strcpy(help+(fileNameOnly-m_fileName),path);
        strcpy(help+(fileNameOnly-m_fileName)+pathLen,fileNameOnly);
        help[ext-fileNameOnly+pathLen+(fileNameOnly-m_fileName)]=0;
      }
      if (++run)
        wsprintf(help+strlen(help),"(%i)%s",run,ext);
      else
        strcat(help,ext);
      if (CopyFile(m_fileName,help,TRUE))
        break;
      if (GetLastError()!=ERROR_FILE_EXISTS)
        break;
    }
  }

  DebugFreeMemory(m_buffer);
  DebugFreeMemory(m_fileName);

  this->~OutputStream();
  DebugFreeMemory(this);
}

void DebugIOFlat::OutputStream::Write(const char *src)
{
  if (!src)
  {
    // flush request, flush only if unlimited file size
    if (!m_limitedFileSize)
      Flush();
  }
  else
  {
    unsigned len=strlen(src);

    while (len>m_bufferSize)
    {
      InternalWrite(src,m_bufferSize);
      src+=m_bufferSize;
      len-=m_bufferSize;
    }
    InternalWrite(src,len);
  }
}

void DebugIOFlat::OutputStream::InternalWrite(const char *src, unsigned len)
{
  __ASSERT(len<=m_bufferSize);

  // unlimited log file length?
  if (!m_limitedFileSize)
  {
    if (m_bufferUsed+len>m_bufferSize)
      Flush();
    memcpy(m_buffer+m_bufferUsed,src,len);
    m_bufferUsed+=len;
  }
  else
  {
    // just write to ring buffer
    if ((m_bufferUsed+=len)>m_bufferSize)
      m_bufferUsed=m_bufferSize;

    while (len)
    {
      unsigned toWrite;
      if (m_nextChar+len>m_bufferSize)
        toWrite=m_bufferSize-m_nextChar;
      else
        toWrite=len;
      memcpy(m_buffer+m_nextChar,src,toWrite);
      if ((m_nextChar+=toWrite)>=m_bufferSize)
        m_nextChar=0;
      src+=toWrite;
      len-=toWrite;
    }
  }
}

void DebugIOFlat::OutputStream::Flush(void)
{
  if (!m_limitedFileSize)
  {
    // simple flush to file
    DWORD written;
    WriteFile(m_fileHandle,m_buffer,m_bufferUsed,&written,NULL);
    m_bufferUsed=0;
  }
  else
  {
    // create file, write ring buffer
    m_fileHandle=CreateFile(m_fileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,
                            NULL);
    DWORD written;
    if (m_bufferUsed<m_bufferSize)
      WriteFile(m_fileHandle,m_buffer,m_bufferUsed,&written,NULL);
    else
    {
      WriteFile(m_fileHandle,m_buffer+m_nextChar,m_bufferUsed-m_nextChar,&written,NULL);
      WriteFile(m_fileHandle,m_buffer,m_nextChar,&written,NULL);
    }
    CloseHandle(m_fileHandle);
  }
}

//////////////////////////////////////////////////////////////////////////////

void DebugIOFlat::ExpandMagic(const char *src, const char *splitName, char *buf)
{
  // barf if too long
  if (strlen(src)>250)
    src="*eMN";

  // non-magic name?
  if (*src!='*')
  {
    // just return input name
    if (splitName)
    {
      // must jam in split name before extension
      const char *p=strrchr(src,'.');
      if (!p)
        p=src+strlen(src);
      strncpy(buf,src,p-src);
      buf[p-src]='-';
      strcpy(buf+(p-src)+1,splitName);
      strcat(buf,p);
    }
    else
      strcpy(buf,src);

    return;
  }

  // translate magic name
  src++;
  char *dst=buf;
  while (*src)
  {
    if (dst-buf>250)
      break;

    if (*src>='A'&&*src<='Z'&&(*src!='N'||splitName))
      *dst++='-';

    char help[256];
    DWORD size=sizeof(help);
    *help=0;

    switch(*src++)
    {
      case 'e':
      case 'E':
        GetModuleFileName(NULL,help,sizeof(help));
        break;
      case 'm':
      case 'M':
        GetComputerName(help,&size);
        break;
      case 'u':
      case 'U':
        GetUserName(help,&size);
        break;
      case 't':
      case 'T':
        {
          SYSTEMTIME systime;
          GetLocalTime(&systime);

          wsprintf(help,"%04i%02i%02i-%02i%02i-%02i",
                   systime.wYear,systime.wMonth,systime.wDay,
                   systime.wHour,systime.wMinute,systime.wSecond);
        }
        break;
      case 'n':
      case 'N':
        if (splitName&&strlen(splitName)<250)
          strcpy(help,splitName);
        break;
      default:
        *dst++=src[-1];
    }

    unsigned len=strlen(help);
    if (dst-buf+len>250)
      break;
    strcpy(dst,help);
    dst+=len;
  }
  strcpy(dst,".log");
}

DebugIOFlat::DebugIOFlat(void):
  m_firstStream(NULL), m_firstSplit(NULL),
  m_lastStreamPtr(&m_firstStream), m_lastSplitPtr(&m_firstSplit)
{
  *m_copyDir=0;
}

DebugIOFlat::~DebugIOFlat()
{
  for (SplitListEntry *cur=m_firstSplit;cur;)
  {
    SplitListEntry *kill=cur;
    cur=cur->next;
    DebugFreeMemory(kill);
  }
  m_firstSplit=NULL;

  for (StreamListEntry *stream=m_firstStream;stream;)
  {
    StreamListEntry *kill=stream;
    stream=stream->next;
    kill->stream->Delete(m_copyDir);
    DebugFreeMemory(kill);
  }
}

void DebugIOFlat::Write(StringType type, const char *src, const char *str)
{
  for (SplitListEntry *cur=m_firstSplit;cur;cur=cur->next)
  {
    if (!(cur->stringTypes&(1<<type)))
      continue;
    if (src&&*src&&!Debug::SimpleMatch(src,cur->items))
      continue;
    cur->stream->Write(str);
    break;
  }
  if (!cur)
    m_firstStream->stream->Write(str);
}

void DebugIOFlat::EmergencyFlush(void)
{
  for (StreamListEntry *cur=m_firstStream;cur;cur=cur->next)
    cur->stream->Flush();
}

void DebugIOFlat::Execute(class Debug& dbg, const char *cmd, bool structuredCmd,
                          unsigned argn, const char * const * argv)
{
  if (!cmd||!strcmp(cmd,"help"))
  {
    if (!argn)
      dbg << "flat I/O help:\n"
             "The following I/O commands are defined:\n"
             "  add, copy, splitadd, splitview, splitremove\n"
             "Type in debug.io flat help <cmd> for a detailed command help.\n";
    else if (!strcmp(argv[0],"add"))
      dbg <<
        "add [ <filename> [ <size in kb> ] ]\n\n"
        "Create flat file I/O (optionally specifying file name and file size).\n"
        "If a filename is specified all output is written to that file. Otherwise\n"
        "the magic filename '*eMN' is automatically used. Any existing files with\n"
        "that name are overwritten.\n"
        "\n"
        "Instead of a real file name a 'magic' file name can be used by starting\n"
        "the file name with a '*' followed by any number of special characters:\n"
        "- 'e': inserts EXE name\n"
        "- 'm': inserts machine name\n"
        "- 'u': inserts username\n"
        "- 't': inserts timestamp\n"
        "- 'n': inserts split name (empty if main log file)\n"
        "- '-': inserts '-'\n"
        "- 'E', 'M', 'U', 'T', 'N': same as above but with '-' in front \n"
        "  (for 'N': empty if main log file)\n"
        ".log is automatically appended if using magic file names.\n"
        "\n"
        "If a size is specified then all data is internally written to a fixed \n"
        "size memory based ring buffer. This data is flushed out once the \n"
        "program exits. If no size is given then the size of the log file is not \n"
        "limited and any log data is written out immediately.\n";
    else if (!strcmp(argv[0],"copy"))
      dbg <<
        "copy <directory>\n\n"
        "Copies generated log file(s) into the given directory if the program\n"
        "exists or crashes. If there is already a log file with the same\n"
        "name a unique number is appended to the current log files' name.\n";
    else if (!strcmp(argv[0],"splitadd"))
      dbg <<
        "splitadd <types> <filter> <name> [ <size in kb> ]\n\n"
        "Splits off part of the log data. Multiple splits can be defined. They \n"
        "are written out to the first matching split file.\n"
        "\n"
        "'types' defines one or more string types which should be split off:\n"
        "- a: asserts\n"
        "- c: checks\n"
        "- l: logs\n"
        "- h: crash\n"
        "- x: exceptions\n"
        "- r: replies from commands\n"
        "- o: other messages \n"
        "\n"
        "Next a filter is specified that determines which items are to be \n"
        "filtered (only for string types a, c and l). Items can be listed with\n"
        "then 'list' command. The filter is exactly specified as in that command.\n"
        "\n"
        "The third parameter defines a name for this split. If there is \n"
        "already a split with the same name then both will write to the same \n"
        "destination file.\n"
        "\n"
        "Note: If splits are used and the filename given for 'add' is static \n"
        "or does not contain 'n' or 'N' then the split name is automatically \n"
        "appended to the log file name (before the extension).\n"
        "\n"
        "If a size is specified then all data is internally written to a \n"
        "fixed size memory based ring buffer. This data is flushed out once \n"
        "the program exits.\n"
        "\n"
        "If no size is given then the size of the log file is not limited and \n"
        "any log data is written out immediately.\n";
    else if (!strcmp(argv[0],"splitview"))
      dbg << "splitview\n\n"
             "Shows all existing splits in the order they are evaluated.";
    else if (!strcmp(argv[0],"splitremove"))
      dbg << "splitremove <namepattern>\n\n"
             "Removes all active splits matching the given name pattern.";
    else 
      dbg << "Unknown flat I/O command";
  }
  else if (!strcmp(cmd,"add"))
  {
    // add [ <filename> [ <size in kb> ] ]
    __ASSERT(m_firstStream==NULL);

    strcpy(m_baseFilename,argn?argv[0]:"*eMN");

    char fn[256];
    ExpandMagic(m_baseFilename,NULL,fn);

    m_firstStream=(StreamListEntry *)DebugAllocMemory(sizeof(StreamListEntry));
    m_firstStream->next=NULL;
    m_firstStream->stream=OutputStream::Create(fn,argn>1?atoi(argv[1])*1024:0);
    m_lastStreamPtr=&m_firstStream->next;
  }
  else if (!strcmp(cmd,"copy"))
  {
    // copy <directory>
    if (argn)
    {
      strncpy(m_copyDir,argv[0],sizeof(m_copyDir)-1);
      m_copyDir[sizeof(m_copyDir)-1]=0;
    }
  }
  else if (!strcmp(cmd,"splitadd"))
  {
    // splitadd <types> <filter> <name> [ <size in kb> ]
    if (argn>=3)
    {
      // add entry
      SplitListEntry *cur=(SplitListEntry *)DebugAllocMemory(sizeof(SplitListEntry));
      cur->next=*m_lastSplitPtr;
      m_lastSplitPtr=&cur->next;
      if (!m_firstSplit)
        m_firstSplit=cur;
      cur->stringTypes=0;
      for (const char *p=argv[0];*p;++p)
      {
        switch(*p)
        {
          case 'a': cur->stringTypes|=1<<Assert; break;
          case 'c': cur->stringTypes|=1<<Check; break;
          case 'l': cur->stringTypes|=1<<Log; break;
          case 'h': cur->stringTypes|=1<<Crash; break;
          case 'x': cur->stringTypes|=1<<Exception; break;
          case 'r': cur->stringTypes|=1<<CmdReply; break;
          case 'o': cur->stringTypes|=1<<Other; break;
        }
      }
      if (!cur->stringTypes)
        cur->stringTypes=0xffffffff;

      strncpy(cur->items,argv[1],sizeof(cur->items)-1);
      cur->items[sizeof(cur->items)-1]=0;

      strncpy(cur->name,argv[2],sizeof(cur->name)-1);
      cur->name[sizeof(cur->name)-1]=0;

      // create our filename, search for stream with same filename
      char fn[256];
      ExpandMagic(m_baseFilename,cur->name,fn);
      for (StreamListEntry *stream=m_firstStream;stream;stream=stream->next)
        if (!strcmp(stream->stream->GetFilename(),fn))
          break;
      if (!stream)
      {
        // must create new stream
        stream=(StreamListEntry *)DebugAllocMemory(sizeof(StreamListEntry));
        stream->next=NULL;
        *m_lastStreamPtr=stream;
        m_lastStreamPtr=&stream->next;
        stream->stream=OutputStream::Create(fn,argn>3?atoi(argv[3])*1024:0);
      }
      cur->stream=stream->stream;
    }
  }
  else if (!strcmp(cmd,"splitview"))
  {
    // splitview
    for (SplitListEntry *cur=m_firstSplit;cur;cur=cur->next)
    {
      for (StringType t=Assert;t<MAX;t=(StringType)(t+1))
      {
        if (t==StructuredCmdReply||!(cur->stringTypes&(1<<t)))
          continue;
        switch(t)
        {
          case Assert:    dbg << "a"; break;
          case Check:     dbg << "c"; break;
          case Log:       dbg << "l"; break;
          case Crash:     dbg << "h"; break;
          case Exception: dbg << "x"; break;
          case CmdReply:  dbg << "r"; break;
          case Other:     dbg << "o"; break;
        }
      }
      dbg << " " << cur->items << " " << cur->name << "\n";
    }
  }
  else if (!strcmp(cmd,"splitremove"))
  {
    // splitremove <namepattern>
    const char *pattern=argn<1?"*":argv[0];
    for (SplitListEntry **entryPtr=&m_firstSplit;*entryPtr;)
    {
      if ( Debug::SimpleMatch((*entryPtr)->name,pattern) )
      {
        // remove this entry
        SplitListEntry *cur=*entryPtr;
        *entryPtr=cur->next;
        DebugFreeMemory(cur);
      }
      else
        entryPtr=&((*entryPtr)->next);
    }
    
    // must fixup m_lastSplitPtr now
    if (m_firstSplit)
    {
      for (SplitListEntry *cur=m_firstSplit;cur->next;cur=cur->next);
      m_firstSplit=cur;
    }
    else
      m_firstSplit=NULL;
  }
}

DebugIOInterface *DebugIOFlat::Create(void)
{
  return new (DebugAllocMemory(sizeof(DebugIOFlat))) DebugIOFlat();
}

void DebugIOFlat::Delete(void)
{
  this->~DebugIOFlat();
  DebugFreeMemory(this);
}
