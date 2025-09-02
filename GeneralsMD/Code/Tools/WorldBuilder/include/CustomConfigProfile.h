// CustomConfigProfile.h
#pragma once
#include <afx.h>   // for CString
#include "Common/GlobalData.h"

class CustomConfigProfile
{
public:
    static CString GetIniFile(CString filename)
    {
        CString path(TheGlobalData->getPath_UserData().str());
        path += "\\" + filename;
        return path;
    }
    static int ReadInt(const char* section, const char* key, int defVal = 0, CString filename = "Worldbuilder.ini")
    {
        return GetPrivateProfileInt(section, key, defVal, GetIniFile(filename));
    }

    static CString ReadString(const char* section, const char* key, const char* defVal = "", CString filename = "Worldbuilder.ini")
    {
        char buff[512] = {0};
        GetPrivateProfileString(section, key, defVal, buff, sizeof(buff), GetIniFile(filename));
        return CString(buff);
    }

    static void WriteInt(const char* section, const char* key, int val, CString filename = "Worldbuilder.ini")
    {
        char buff[64];
        sprintf(buff, "%d", val);
        WritePrivateProfileString(section, key, buff, GetIniFile(filename));
    }

    static void WriteString(const char* section, const char* key, const char* val, CString filename = "Worldbuilder.ini")
    {
        WritePrivateProfileString(section, key, val, GetIniFile(filename));
    }
};
