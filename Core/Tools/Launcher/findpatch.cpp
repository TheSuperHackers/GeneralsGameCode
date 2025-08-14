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

#include "findpatch.h"

//
// Locate a patch file
//  If a patch can be found then TRUE is returned and the name is filled in,
//  otherwise FALSE is returned.
//
// Patch Types:
//   - *.rtp = RTPatch file that can be applied right now
//   - *.exe = Executable that should be put in the RunOnce registry entry & reboot
//   - *.exn = Executable that should be run right now
//   - *.web = Link to a web page that will have the patch
//   - else  = File is ignored, possibly a resource file for one of the other types
//
int Find_Patch(OUT char *filename,int maxlen, ConfigFile &config)
{
  WIN32_FIND_DATA     findData;
  char                string[128];
  HANDLE              hFile;
  const char         *extensions[]={"web","exe","exn","rtp",NULL};
  int                 i;
  int                 skuIndex=0;
  Wstring             key;
  Wstring             path;
  Wstring             sku;
  char                gamePath[MAX_PATH] = "";
  bit8                ok;


  while(1)
  {
    //
    // Loop through the apps we're responsible for
    //
    skuIndex++;
    ok=Get_App_Dir(gamePath,MAX_PATH,config,skuIndex);
    if (ok==FALSE)
      break;

    i=0;
    while(extensions[i++])
    {
      _chdir(gamePath);  // goto the directory with the game

      // should probably get the registry entry for the wchat install path
      sprintf(string,"patches\\*.%s",extensions[i]);
      hFile=FindFirstFile(string,&findData);
      if (hFile!=INVALID_HANDLE_VALUE)
      {
        _getcwd(filename,MAX_PATH);
        strcat(filename,"\\patches\\");
        strcat(filename,findData.cFileName);
        FindClose(hFile);
        return(skuIndex);
      }
      sprintf(string,"download\\*.%s",extensions[i]);
      hFile=FindFirstFile(string,&findData);
      if (hFile!=INVALID_HANDLE_VALUE)
      {
        _getcwd(filename,MAX_PATH);
        strcat(filename,"\\download\\");
        strcat(filename,findData.cFileName);
        FindClose(hFile);
        return(skuIndex);
      }
    }
  }
  return(FALSE);
}


//
// Get the directory for the N'th application in the config file
//
// Returns FALSE if not in the config file or invalid for some reason.
//
bit8 Get_App_Dir(OUT char *filename,int maxlen, ConfigFile &config,int index)
{
  char                string[128];
  Wstring             key;
  Wstring             path;
  Wstring             sku;
  int                 temp;
  char                gamePath[MAX_PATH];


  sprintf(string,"SKU%d",index);

  // Can't find this product
  if (config.getString(string,key)==FALSE)
    return(FALSE);


  DBGMSG("KEY = "<<key.get());
  // Get the InstallPath from the specified registry key
  temp=0;
  temp=key.getToken(temp," ",sku);
  path=key;
  path.remove(0,temp);
  while((*(path.get()))==' ')  // remove leading spaces
    path.remove(0,1);


  DBGMSG("CONFIG:   SKU = "<<sku.get()<<"  PATH = '"<<path.get()<<"'");
  HKEY   regKey;
  LONG   regRetval;
  /////////////DWORD  regPrevious;
  regRetval=RegOpenKeyEx(HKEY_LOCAL_MACHINE,path.get(),0,KEY_READ,&regKey);
  if (regRetval!=ERROR_SUCCESS)
  {
    DBGMSG("RegOpenKey failed");
    return(FALSE);
  }
  DWORD  type;
  DWORD  length=MAX_PATH;
  regRetval=RegQueryValueEx(regKey,"InstallPath",NULL,&type,(uint8 *)gamePath,
      &length);
  DBGMSG("GAME PATH = "<<gamePath);
  if ((regRetval!=ERROR_SUCCESS)||(type!=REG_SZ))
  {
    DBGMSG("Reg failure");
    return(FALSE);
  }

  // Find the last '\\' in a string and put a 0 after it
  //  If you only put a directory in the InstallPath key instead of a full
  //  path to a file, you better end the directory with a trailing '\\'!!!
  char *cptr=gamePath;
  char *tempPtr;
  while( (tempPtr=strchr(cptr,'\\')) !=NULL)
    cptr=tempPtr+1;
  if (cptr)
    *cptr=0;

  DBGMSG("Game path = "<<gamePath);
  strncpy(filename,gamePath,maxlen);

  return(TRUE);
}





//
// Delete any patch files
//
void Delete_Patches(ConfigFile &config)
{
  char dir[MAX_PATH];
  int  i=1;
  WIN32_FIND_DATA     findData;
  HANDLE              hFile;

  DBGMSG("IN DELPATCH");

  //
  // Loop through all the application directories in the config file
  //
  while (Get_App_Dir(dir,MAX_PATH,config,i++)==TRUE)
  {
    // Make sure path is at least 3 for "c:\".  I really hope nobody's
    //   dumb enough to install a game to the root directory. (It's OK though
    //   since only the '\patches' folder is cleared out.
    if (strlen(dir)<3)
      continue;

    //
    // Delete everything in case a .exe patch had some data files it used.
    //
    strcat(dir,"patches\\*.*");

    DBGMSG("DELPATCH: "<<dir);

    hFile=FindFirstFile(dir,&findData);
    if (hFile!=INVALID_HANDLE_VALUE)
    {
      if (findData.cFileName[0]!='.')
      {
        //_unlink(findData.cFileName);
        DBGMSG("UNLINK: "<<findData.cFileName);
      }
      while(FindNextFile(hFile,&findData))
      {
        if (findData.cFileName[0]!='.')
        {
          //_unlink(findData.cFileName);
          DBGMSG("UNLINK: "<<findData.cFileName);
        }
      }
    }  // If there's at least one file
    FindClose(hFile);
  }  // while there's apps in config
  return;
}
