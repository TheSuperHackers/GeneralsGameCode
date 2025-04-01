# Microsoft Developer Studio Project File - Name="W3DView" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=W3DView - Win32 Compressed
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "W3DView.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "W3DView.mak" CFG="W3DView - Win32 Compressed"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "W3DView - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "W3DView - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "W3DView - Win32 Profile" (based on "Win32 (x86) Application")
!MESSAGE "W3DView - Win32 Compressed" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Commando/Code/Tools/W3DView", XRWAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "W3DView - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /Ob2 /I "..\..\miles6\include" /I "..\..\wwdebug" /I "..\..\wwmath" /I "..\..\ww3d2" /I "..\..\wwphys" /I "..\..\wwlib" /I "..\..\wwsaveload" /I "..\..\wwaudio" /I "..\..\directx\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 mss32.lib wwsaveload.lib wwdebug.lib wwmath.lib ww3d2.lib wwlib.lib vfw32.lib winmm.lib version.lib wwaudio.lib d3dx8.lib d3d8.lib /nologo /subsystem:windows /machine:I386 /out:"Run/W3DView.exe" /libpath:"..\..\Libs\release" /libpath:"..\..\miles6\lib\win" /libpath:"..\..\directx\lib"

!ELSEIF  "$(CFG)" == "W3DView - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\miles6\include" /I "..\..\wwdebug" /I "..\..\wwmath" /I "..\..\ww3d2" /I "..\..\wwphys" /I "..\..\wwlib" /I "..\..\wwsaveload" /I "..\..\wwaudio" /I "..\..\directx\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WWDEBUG" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 
# ADD LINK32 mss32.lib wwsaveload.lib wwdebug.lib wwmath.lib ww3d2.lib wwlib.lib vfw32.lib winmm.lib version.lib wwaudio.lib d3dx8.lib d3d8.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Run/W3DViewD.exe"  /libpath:"..\..\Libs\debug" /libpath:"..\..\miles6\lib\win" /libpath:"..\..\directx\lib"

!ELSEIF  "$(CFG)" == "W3DView - Win32 Profile"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Profile"
# PROP BASE Intermediate_Dir "Profile"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\sr\sr.h" /I "..\..\srsdk1x\include" /I "..\..\wwdebug" /I "..\..\wwmath" /I "..\..\ww3d" /I "..\..\wwphys" /I "..\..\wwlib" /I "..\wwctrl" /I "..\..\wwsaveload" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /Ob2 /I "..\..\miles6\include" /I "..\..\wwdebug" /I "..\..\wwmath" /I "..\..\ww3d2" /I "..\..\wwphys" /I "..\..\wwlib" /I "..\..\wwsaveload" /I "..\..\wwaudio" /I "..\..\directx\include" /D "WIN32" /D "WWDEBUG" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wwsaveload.lib wwctrl.lib sr.lib wwdebug.lib wwmath.lib ww3d.lib wwlib.lib vfw32.lib winmm.lib version.lib /nologo /subsystem:windows /machine:I386 /out:"Profile/W3DView.exe" /libpath:"..\..\Libs\release" /libpath:"..\..\srsdk1x\msvc6\lib\release"
# ADD LINK32 ..\..\miles6\lib\win\mss32.lib wwsaveload.lib wwdebug.lib wwmath.lib ww3d2.lib wwlib.lib vfw32.lib winmm.lib version.lib wwaudio.lib d3dx8.lib d3d8.lib /nologo /subsystem:windows /incremental:yes /debug /machine:I386 /out:"Profile\W3DView.exe" /libpath:"..\..\Libs\profile" /libpath:"..\..\srsdk1x\msvc6\lib\release" /libpath:"..\..\directx\lib"

!ELSEIF  "$(CFG)" == "W3DView - Win32 Compressed"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Compressed"
# PROP BASE Intermediate_Dir "Compressed"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Compressed"
# PROP Intermediate_Dir "Compressed"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\miles6\include" /I "..\..\wwdebug" /I "..\..\wwmath" /I "..\..\ww3d2" /I "..\..\wwphys" /I "..\..\wwlib" /I "..\wwctrl" /I "..\..\wwsaveload" /I "..\..\wwaudio" /I "..\..\directx\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WWDEBUG" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\miles6\include" /I "..\..\wwdebug" /I "..\..\wwmath" /I "..\..\ww3d2" /I "..\..\wwphys" /I "..\..\wwlib" /I "..\wwctrl" /I "..\..\wwsaveload" /I "..\..\wwaudio" /I "..\..\directx\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WWDEBUG" /D "VIEW_COMPRESSED" /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wwctrld.lib mss32.lib wwsaveload.lib wwdebug.lib wwmath.lib ww3d2.lib wwlib.lib vfw32.lib winmm.lib version.lib wwaudio.lib d3dx8.lib d3d8.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Compressed/W3DViewC.exe"  /libpath:"..\..\Libs\debug" /libpath:"..\..\miles6\lib\win" /libpath:"..\..\directx\lib"
# ADD LINK32 wwctrld.lib mss32.lib wwsaveload.lib wwdebug.lib wwmath.lib ww3d2C.lib wwlib.lib vfw32.lib winmm.lib version.lib wwaudio.lib d3dx8.lib d3d8.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Compressed\W3DViewC.exe"  /libpath:"..\..\Libs\debug" /libpath:"..\..\miles6\lib\win" /libpath:"..\..\directx\lib"

!ENDIF 

# Begin Target

# Name "W3DView - Win32 Release"
# Name "W3DView - Win32 Debug"
# Name "W3DView - Win32 Profile"
# Name "W3DView - Win32 Compressed"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AddToLineupDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\AdvancedAnimSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\AggregateNameDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\AmbientLightDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimatedSoundOptionsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimationPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimationSpeed.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimMixingPage.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimReportPage.cpp
# End Source File
# Begin Source File

SOURCE=.\AssetInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\AssetPropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=.\BackgroundBMPDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\BackgroundColorDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\BackgroundObjectDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\BoneMgrDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\CameraDistanceDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\CameraSettingsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorBar.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorPicker.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorPickerDialogClass.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorSelectionDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\DataTreeView.cpp
# End Source File
# Begin Source File

SOURCE=.\DeviceSelectionDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\DialogToolbar.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectoryDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\EditLODDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\EmitterColorPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\EmitterFramePropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\EmitterGeneralPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\EmitterInstanceList.cpp
# End Source File
# Begin Source File

SOURCE=.\EmitterLineGroupPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\EmitterLinePropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\EmitterParticlePropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\EmitterPhysicsPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\EmitterPropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=.\EmitterRotationPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\EmitterSizePropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\EmitterUserPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\GammaDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Globals.cpp
# End Source File
# Begin Source File

SOURCE=.\GraphicView.cpp
# End Source File
# Begin Source File

SOURCE=.\HierarchyPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\OpacitySettingsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\OpacityVectorDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleBlurTimeKeyDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleFrameKeyDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleRotationKeyDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSizeDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PlaySoundDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ResolutionDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\RestrictedFileDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\RingColorPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\RingGeneralPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\RingPropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=.\RingSizePropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\SaveSettingsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ScaleDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SceneLightDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenCursor.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundEditDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SphereColorPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\SphereGeneralPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\SpherePropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=.\SphereSizePropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TexturePathDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Toolbar.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils.cpp
# End Source File
# Begin Source File

SOURCE=.\Vector3RndCombo.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewerAssetMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewerScene.cpp
# End Source File
# Begin Source File

SOURCE=.\VolumeRandomDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\W3DView.cpp
# End Source File
# Begin Source File

SOURCE=.\W3DView.rc
# End Source File
# Begin Source File

SOURCE=.\W3DViewDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\W3DViewView.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AddToLineupDialog.h
# End Source File
# Begin Source File

SOURCE=.\AdvancedAnimSheet.h
# End Source File
# Begin Source File

SOURCE=.\AggregateNameDialog.h
# End Source File
# Begin Source File

SOURCE=.\AmbientLightDialog.h
# End Source File
# Begin Source File

SOURCE=.\AnimatedSoundOptionsDialog.h
# End Source File
# Begin Source File

SOURCE=.\AnimationPropPage.h
# End Source File
# Begin Source File

SOURCE=.\AnimationSpeed.h
# End Source File
# Begin Source File

SOURCE=.\AnimMixingPage.h
# End Source File
# Begin Source File

SOURCE=.\AnimReportPage.h
# End Source File
# Begin Source File

SOURCE=.\AssetInfo.h
# End Source File
# Begin Source File

SOURCE=.\AssetPropertySheet.h
# End Source File
# Begin Source File

SOURCE=.\AssetTypes.h
# End Source File
# Begin Source File

SOURCE=.\BackgroundBMPDialog.h
# End Source File
# Begin Source File

SOURCE=.\BackgroundColorDialog.h
# End Source File
# Begin Source File

SOURCE=.\BackgroundObjectDialog.h
# End Source File
# Begin Source File

SOURCE=.\BoneMgrDialog.h
# End Source File
# Begin Source File

SOURCE=.\CameraDistanceDialog.h
# End Source File
# Begin Source File

SOURCE=.\CameraSettingsDialog.h
# End Source File
# Begin Source File

SOURCE=.\ColorBar.h
# End Source File
# Begin Source File

SOURCE=.\ColorPicker.h
# End Source File
# Begin Source File

SOURCE=.\ColorPickerDialogClass.h
# End Source File
# Begin Source File

SOURCE=.\ColorSelectionDialog.h
# End Source File
# Begin Source File

SOURCE=.\ColorUtils.h
# End Source File
# Begin Source File

SOURCE=.\DataTreeView.h
# End Source File
# Begin Source File

SOURCE=.\DeviceSelectionDialog.h
# End Source File
# Begin Source File

SOURCE=.\DialogToolbar.h
# End Source File
# Begin Source File

SOURCE=.\DirectoryDialog.h
# End Source File
# Begin Source File

SOURCE=.\EditLODDialog.h
# End Source File
# Begin Source File

SOURCE=.\EmitterColorPropPage.h
# End Source File
# Begin Source File

SOURCE=.\EmitterFramePropPage.h
# End Source File
# Begin Source File

SOURCE=.\EmitterGeneralPropPage.h
# End Source File
# Begin Source File

SOURCE=.\EmitterInstanceList.h
# End Source File
# Begin Source File

SOURCE=.\EmitterLineGroupPropPage.h
# End Source File
# Begin Source File

SOURCE=.\EmitterLinePropPage.h
# End Source File
# Begin Source File

SOURCE=.\EmitterParticlePropPage.h
# End Source File
# Begin Source File

SOURCE=.\EmitterPhysicsPropPage.h
# End Source File
# Begin Source File

SOURCE=.\EmitterPropertySheet.h
# End Source File
# Begin Source File

SOURCE=.\EmitterRotationPropPage.h
# End Source File
# Begin Source File

SOURCE=.\EmitterSizePropPage.h
# End Source File
# Begin Source File

SOURCE=.\EmitterUserPropPage.h
# End Source File
# Begin Source File

SOURCE=.\GammaDialog.h
# End Source File
# Begin Source File

SOURCE=.\Globals.h
# End Source File
# Begin Source File

SOURCE=.\GraphicView.h
# End Source File
# Begin Source File

SOURCE=.\HierarchyPropPage.h
# End Source File
# Begin Source File

SOURCE=.\LODDefs.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MeshPropPage.h
# End Source File
# Begin Source File

SOURCE=.\OpacitySettingsDialog.h
# End Source File
# Begin Source File

SOURCE=.\OpacityVectorDialog.h
# End Source File
# Begin Source File

SOURCE=.\ParticleBlurTimeKeyDialog.h
# End Source File
# Begin Source File

SOURCE=.\ParticleFrameKeyDialog.h
# End Source File
# Begin Source File

SOURCE=.\ParticleRotationKeyDialog.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSizeDialog.h
# End Source File
# Begin Source File

SOURCE=.\PlaySoundDialog.h
# End Source File
# Begin Source File

SOURCE=.\ResolutionDialog.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\RestrictedFileDialog.h
# End Source File
# Begin Source File

SOURCE=.\RingColorPropPage.h
# End Source File
# Begin Source File

SOURCE=.\RingGeneralPropPage.h
# End Source File
# Begin Source File

SOURCE=.\RingPropertySheet.h
# End Source File
# Begin Source File

SOURCE=.\RingSizePropPage.h
# End Source File
# Begin Source File

SOURCE=.\SaveSettingsDialog.h
# End Source File
# Begin Source File

SOURCE=.\ScaleDialog.h
# End Source File
# Begin Source File

SOURCE=.\SceneLightDialog.h
# End Source File
# Begin Source File

SOURCE=.\ScreenCursor.h
# End Source File
# Begin Source File

SOURCE=.\SoundEditDialog.h
# End Source File
# Begin Source File

SOURCE=.\SphereColorPropPage.h
# End Source File
# Begin Source File

SOURCE=.\SphereGeneralPropPage.h
# End Source File
# Begin Source File

SOURCE=.\SpherePropertySheet.h
# End Source File
# Begin Source File

SOURCE=.\SphereSizePropPage.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TexturePathDialog.h
# End Source File
# Begin Source File

SOURCE=.\Toolbar.h
# End Source File
# Begin Source File

SOURCE=.\Utils.h
# End Source File
# Begin Source File

SOURCE=.\Vector3RndCombo.H
# End Source File
# Begin Source File

SOURCE=.\ViewerAssetMgr.h
# End Source File
# Begin Source File

SOURCE=.\ViewerScene.h
# End Source File
# Begin Source File

SOURCE=.\VolumeRandomDialog.h
# End Source File
# Begin Source File

SOURCE=.\W3DView.h
# End Source File
# Begin Source File

SOURCE=.\W3DViewDoc.h
# End Source File
# Begin Source File

SOURCE=.\W3DViewView.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Res\aggregat.ico
# End Source File
# Begin Source File

SOURCE=.\res\animatio.ico
# End Source File
# Begin Source File

SOURCE=.\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cur00001.cur
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\Res\ffwd.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\ffwdsel.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\frame.ico
# End Source File
# Begin Source File

SOURCE=.\Res\GAMMA.BMP
# End Source File
# Begin Source File

SOURCE=.\res\hierarch.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00003.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00004.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00005.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00006.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00007.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00008.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00009.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00010.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00011.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00012.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00013.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00014.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00015.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00016.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00017.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00018.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\keyframe.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\line.ico
# End Source File
# Begin Source File

SOURCE=.\Res\lineup.ico
# End Source File
# Begin Source File

SOURCE=.\marker.bmp
# End Source File
# Begin Source File

SOURCE=.\res\material.ico
# End Source File
# Begin Source File

SOURCE=.\Res\openfold.ico
# End Source File
# Begin Source File

SOURCE=.\Res\particle.ico
# End Source File
# Begin Source File

SOURCE=.\Res\pause.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\pausesel.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\play.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\playsel.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\reverse.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\reversesel.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\rotatez.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\rotatezsel.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\rotation.ico
# End Source File
# Begin Source File

SOURCE=.\Res\sound.ico
# End Source File
# Begin Source File

SOURCE=.\Res\stop.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\stopsel.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\user1.ico
# End Source File
# Begin Source File

SOURCE=.\res\W3DView.ico
# End Source File
# Begin Source File

SOURCE=.\res\W3DView.rc2
# End Source File
# Begin Source File

SOURCE=.\res\W3DViewDoc.ico
# End Source File
# Begin Source File

SOURCE=.\Res\xdir.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\xdirsel.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\ydir.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\ydirsel.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\zdir.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\zdirsel.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Res\cursor.tga
# End Source File
# Begin Source File

SOURCE=.\Res\grab.tga
# End Source File
# Begin Source File

SOURCE=.\Res\Light.w3d
# End Source File
# Begin Source File

SOURCE=.\Res\orbit.tga
# End Source File
# Begin Source File

SOURCE=.\W3DView.reg
# End Source File
# Begin Source File

SOURCE=.\Res\zoom.tga
# End Source File
# End Target
# End Project
