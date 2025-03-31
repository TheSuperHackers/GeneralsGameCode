# Microsoft Developer Studio Project File - Name="wdump" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=wdump - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wdump.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wdump.mak" CFG="wdump - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wdump - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "wdump - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "wdump - Win32 Profile" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/G/wdump", JDKAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wdump - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\ww3d2" /I "..\..\wwlib" /I "..\..\wwmath" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "G_CODE_BASE" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 wwlib.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libcd.lib" /libpath:"..\libs\release" /libpath:"..\..\libs\release"

!ELSEIF  "$(CFG)" == "wdump - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\ww3d2" /I "..\..\wwlib" /I "..\..\wwmath" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "G_CODE_BASE" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 
# ADD LINK32 wwlib.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /libpath:"..\libs\debug" /libpath:"..\..\libs\debug"
# SUBTRACT LINK32 /incremental:no

!ELSEIF  "$(CFG)" == "wdump - Win32 Profile"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "wdump___"
# PROP BASE Intermediate_Dir "wdump___"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\ww3d" /I "..\..\library" /I "..\ww3d" /I "..\library" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "G_CODE_BASE" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\ww3d2" /I "..\..\wwlib" /I "..\..\wwmath" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "G_CODE_BASE" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 library.lib ww3d.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libcd.lib" /libpath:"..\libs\release" /libpath:"..\..\libs\release"
# ADD LINK32 wwlib.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /libpath:"..\libs\profile" /libpath:"..\..\libs\profile"

!ENDIF 

# Begin Target

# Name "wdump - Win32 Release"
# Name "wdump - Win32 Debug"
# Name "wdump - Win32 Profile"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Chunk_D.cpp
# End Source File
# Begin Source File

SOURCE=.\FindDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\RawfileM.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\WDEView.cpp
# End Source File
# Begin Source File

SOURCE=.\WDLView.cpp
# End Source File
# Begin Source File

SOURCE=.\WDTView.cpp
# End Source File
# Begin Source File

SOURCE=.\wdump.cpp
# End Source File
# Begin Source File

SOURCE=.\wdump.rc
# End Source File
# Begin Source File

SOURCE=.\wdumpDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\wdView.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Chunk_D.h
# End Source File
# Begin Source File

SOURCE=.\FindDialog.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\RawfileM.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\WDEView.h
# End Source File
# Begin Source File

SOURCE=.\WDLView.h
# End Source File
# Begin Source File

SOURCE=.\WDTView.h
# End Source File
# Begin Source File

SOURCE=.\wdump.h
# End Source File
# Begin Source File

SOURCE=.\wdumpDoc.h
# End Source File
# Begin Source File

SOURCE=.\wdView.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\wdump.ico
# End Source File
# Begin Source File

SOURCE=.\res\wdump.rc2
# End Source File
# Begin Source File

SOURCE=.\res\wdumpDoc.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
