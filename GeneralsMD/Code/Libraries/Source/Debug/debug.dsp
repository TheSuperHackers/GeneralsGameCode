# Microsoft Developer Studio Project File - Name="debug" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=debug - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "debug.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "debug.mak" CFG="debug - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "debug - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "debug - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "debug - Win32 Internal" (based on "Win32 (x86) Static Library")
!MESSAGE "debug - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "debug"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "debug - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /Zi /Ot /Og /Oi /Oy /Ob2 /Gy /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"_pch.h" /FD /GF /Gs /c
# SUBTRACT CPP /Oa
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=copying
PostBuild_Cmds=copy release\debug.lib ..\..\lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "debug - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"_pch.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\debugdebug.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=copying
PostBuild_Cmds=copy debug\debugdebug.lib ..\..\lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "debug - Win32 Internal"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Internal"
# PROP BASE Intermediate_Dir "Internal"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Internal"
# PROP Intermediate_Dir "Internal"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /WX /Zi /Ot /Oa /Og /Oi /Oy /Ob2 /Gy /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"_pch.h" /FD /GF /Gs /c
# ADD CPP /nologo /G6 /MD /W3 /WX /Zi /Ot /Og /Oi /Oy /Ob2 /Gy /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_INTERNAL" /Yu"_pch.h" /FD /GF /Gs /c
# SUBTRACT CPP /Oa
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Internal\debuginternal.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=copying
PostBuild_Cmds=copy internal\debuginternal.lib ..\..\lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "debug - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Profile"
# PROP BASE Intermediate_Dir "Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /WX /Zi /Ot /Oa /Og /Oi /Oy /Ob2 /Gy /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"_pch.h" /FD /GF /Gs /c
# ADD CPP /nologo /G6 /MD /W3 /WX /Zi /Ot /Og /Oi /Oy /Ob2 /Gy /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_PROFILE" /Yu"_pch.h" /FD /GF /Gs /c
# SUBTRACT CPP /Oa
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Profile\debugprofile.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=copying
PostBuild_Cmds=copy profile\debugprofile.lib ..\..\lib
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "debug - Win32 Release"
# Name "debug - Win32 Debug"
# Name "debug - Win32 Internal"
# Name "debug - Win32 Profile"
# Begin Group "Precompiled header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\_pch.cpp
# ADD CPP /Yc"_pch.h"
# End Source File
# Begin Source File

SOURCE=.\_pch.h
# End Source File
# End Group
# Begin Group "Doxygen"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\compile_doxygen.bat
# End Source File
# Begin Source File

SOURCE=.\debug.dox
# End Source File
# Begin Source File

SOURCE=.\debug_priv.dox
# End Source File
# End Group
# Begin Group "IO Classes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\debug_io.h
# End Source File
# Begin Source File

SOURCE=.\debug_io_con.cpp
# End Source File
# Begin Source File

SOURCE=.\debug_io_flat.cpp
# End Source File
# Begin Source File

SOURCE=.\debug_io_net.cpp
# End Source File
# Begin Source File

SOURCE=.\debug_io_ods.cpp
# End Source File
# Begin Source File

SOURCE=.\internal_io.h
# End Source File
# End Group
# Begin Group "Exception handler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\debug_except.cpp
# End Source File
# Begin Source File

SOURCE=.\internal_except.h
# End Source File
# Begin Source File

SOURCE=.\rc_exception.inl
# End Source File
# End Group
# Begin Group "Stack walk"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\debug_stack.cpp
# End Source File
# Begin Source File

SOURCE=.\debug_stack.h
# End Source File
# Begin Source File

SOURCE=.\debug_stack.inl
# End Source File
# End Group
# Begin Group "Command interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\debug_cmd.cpp
# End Source File
# Begin Source File

SOURCE=.\debug_cmd.h
# End Source File
# Begin Source File

SOURCE=.\debug_getdefaultcommands.cpp
# End Source File
# End Group
# Begin Group "Proxy header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Include\rts\debug.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\debug.cpp
# ADD CPP /FAcs
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\debug_debug.cpp

!IF  "$(CFG)" == "debug - Win32 Release"

!ELSEIF  "$(CFG)" == "debug - Win32 Debug"

!ELSEIF  "$(CFG)" == "debug - Win32 Internal"

# ADD CPP /FAcs

!ELSEIF  "$(CFG)" == "debug - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\debug_debug.h
# End Source File
# Begin Source File

SOURCE=.\debug_doc.h
# End Source File
# Begin Source File

SOURCE=.\debug_internal.cpp
# End Source File
# Begin Source File

SOURCE=.\debug_macro.h
# End Source File
# Begin Source File

SOURCE=.\debug_purecall.cpp
# End Source File
# Begin Source File

SOURCE=.\internal.h
# End Source File
# End Target
# End Project
