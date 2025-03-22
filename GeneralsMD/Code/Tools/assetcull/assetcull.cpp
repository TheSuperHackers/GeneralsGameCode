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
// $File: //depot/GeneralsMD/Staging/code/Tools/assetcull/assetcull.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/28 14:54:04 $
//
// (c) 2003 Electronic Arts
//
// simple recursive directory compare tool for asset culling
//////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include <sys/stat.h>
#include <stdio.h>
#include <io.h>

/*
  Usage: assetcull <dir1> <dir2> <bat-file>

  Description:
    All files in <dir1> and <dir2> (and subdirectories) are compared
    binary. If an identical file exists it is removed from <dir1>
    and a corresponding DEL line is written to the given batch file.
*/

static bool filesEqual(const char *fn1, const char *fn2)
{
  // make sure both files exist and are of same size
  struct _stat s1,s2;
  if (_stat(fn1,&s1)||
      _stat(fn2,&s2))
    return false;
  if (s1.st_size!=s2.st_size)
    return false;

  // must compare byte-by-byte
  FILE *f1=fopen(fn1,"rb");
  if (!f1)
    return false;
  FILE *f2=fopen(fn2,"rb");
  if (!f2)
    return false;

  static char buf1[16384],buf2[16384];
  for (unsigned k=0;k<s1.st_size;)
  {
    unsigned cur=s1.st_size-k;
    if (cur>sizeof(buf1))
      cur=sizeof(buf1);
    if (fread(buf1,cur,1,f1)!=1||
        fread(buf2,cur,1,f2)!=1)
      break;
    if (memcmp(buf1,buf2,cur))
      break;
    k+=cur;
  }
  fclose(f1);
  fclose(f2);
  return k==s1.st_size;
}

static int recursiveCull(FILE *batchFile,
                         const char *dir1, const char *dir2,
                         const char *relDir)
{
  // sub directory must exist both in dir1 and dir2
  // (but we're walking dir2 only later)
  std::string work;
  work=dir1; work+=relDir; work+="*.*";
  _finddata_t fd;
  long h=_findfirst(work.c_str(),&fd);
  if (h==-1)
    return 0;
  _findclose(h);

  work=dir2; work+=relDir; work+="*.*";
  h=_findfirst(work.c_str(),&fd);
  if (h==-1)
    return 0;

  // walk dir2, collect sub directories and check for duplicate
  // files
  std::vector<std::string> subdir,dupfiles;
  int deleted=0;
  for (;;)
  {
    if (fd.attrib&_A_SUBDIR)
    {
      if (strcmp(fd.name,".")&&
          strcmp(fd.name,".."))
        subdir.push_back(fd.name);
    }
    else
    {
      std::string work1,work2;
      work1=dir1; work1+=relDir; work1+=fd.name;
      work2=dir2; work2+=relDir; work2+=fd.name;
      if (filesEqual(work1.c_str(),work2.c_str()))
        dupfiles.push_back(fd.name);
    }
    if (_findnext(h,&fd))
      break;
  }
  _findclose(h);

  // remove duplicate files, at to batch file
  // (we can't just delete them inside the find loop because - at
  // least theoretically - that could screw up that find process...)
  for (std::vector<std::string>::iterator i=dupfiles.begin();i!=dupfiles.end();++i)
  {
    std::string work;
    work=dir1; work+=relDir; work+=*i;
    _chmod(work.c_str(),_S_IREAD|_S_IWRITE);
    if (_unlink(work.c_str()))
      fprintf(stderr,"Error: Can't delete %s\n",work.c_str());
    else
      deleted++;
    fprintf(batchFile,"attrib -r \"%s\"\n",work.c_str());
    fprintf(batchFile,"del -r \"%s\"\n",work.c_str());
  }

  // walk subdirectories
  for (i=subdir.begin();i!=subdir.end();++i)
  {
    std::string work;
    work=relDir;
    work+=*i;
    work+="\\";
    deleted+=recursiveCull(batchFile,dir1,dir2,work.c_str());
  }

  // done!
  return deleted;
}

int main(int argc, char *argv[])
{
  if (argc!=4)
  {
    printf("Usage: assetcull <dir1> <dir2> <bat-file>\n\n"
           "Description:\n"
           "  All files in <dir1> and <dir2> (and subdirectories) are compared\n"
           "  binary. If an identical file exists it is removed from <dir1>\n"
           "  and a corresponding DEL line is written to the given batch file.\n"
          );
    return 10;
  }

  FILE *f=fopen(argv[3],"wt");
  if (!f)
  {
    printf("Error: Can't create %s\n",argv[3]);
    return 10;
  }

  int n=recursiveCull(f,argv[1],argv[2],"\\.\\");
  fclose(f);
  printf("assetcull: %i files culled.\n",n);

  return 0;
}
