// CustomConfigProfile.h
#pragma once
#include <afx.h>   // for CString

class CustomConfigProfile
{
public:
    static const char* GetIniFile()
    {
        // Save in same folder as worldbuilder.ini
        return "grovesets.ini";
    }

    static int ReadInt(const char* section, const char* key, int defVal = 0)
    {
        return GetPrivateProfileInt(section, key, defVal, GetIniFile());
    }

    static CString ReadString(const char* section, const char* key, const char* defVal = "")
    {
        char buff[512] = {0};
        GetPrivateProfileString(section, key, defVal, buff, sizeof(buff), GetIniFile());
        return CString(buff);
    }

    static void WriteInt(const char* section, const char* key, int val)
    {
        char buff[64];
        sprintf(buff, "%d", val);
        WritePrivateProfileString(section, key, buff, GetIniFile());
    }

    static void WriteString(const char* section, const char* key, const char* val)
    {
        WritePrivateProfileString(section, key, val, GetIniFile());
    }
};
