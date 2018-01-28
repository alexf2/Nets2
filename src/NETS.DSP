# Microsoft Developer Studio Project File - Name="nets" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=nets - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nets.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nets.mak" CFG="nets - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nets - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "nets - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nets - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "exe"
# PROP Intermediate_Dir "exe"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G3 /Zp1 /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /incremental:yes

!ELSEIF  "$(CFG)" == "nets - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "exe"
# PROP Intermediate_Dir "exe"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G3 /Zp1 /MDd /W3 /Gm /GR /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /incremental:no /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "nets - Win32 Release"
# Name "nets - Win32 Debug"
# Begin Group "SOURCE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\About.cpp
# End Source File
# Begin Source File

SOURCE=.\Autorc.cpp
# End Source File
# Begin Source File

SOURCE=.\cfg.cpp
# End Source File
# Begin Source File

SOURCE=.\Coding.cpp
# End Source File
# Begin Source File

SOURCE=.\drv2dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\drvsel.cpp
# End Source File
# Begin Source File

SOURCE=.\fdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\fsrv.cpp
# End Source File
# Begin Source File

SOURCE=.\ind.cpp
# End Source File
# Begin Source File

SOURCE=.\localus.cpp
# End Source File
# Begin Source File

SOURCE=.\ncpane.cpp
# End Source File
# Begin Source File

SOURCE=.\Net.cpp
# End Source File
# Begin Source File

SOURCE=.\nframe.cpp
# End Source File
# Begin Source File

SOURCE=.\optdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\phus.cpp
# End Source File
# Begin Source File

SOURCE=.\Polinom.cpp
# End Source File
# Begin Source File

SOURCE=.\ruser.cpp
# End Source File
# Begin Source File

SOURCE=.\server.cpp
# End Source File
# Begin Source File

SOURCE=.\Stdafx.cpp
# End Source File
# End Group
# Begin Group "HEADER"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\About.hpp
# End Source File
# Begin Source File

SOURCE=.\Autorc.hpp
# End Source File
# Begin Source File

SOURCE=.\cfg.hpp
# End Source File
# Begin Source File

SOURCE=.\Coding.hpp
# End Source File
# Begin Source File

SOURCE=.\drv2dlg.hpp
# End Source File
# Begin Source File

SOURCE=.\drvsel.hpp
# End Source File
# Begin Source File

SOURCE=.\fdlg.hpp
# End Source File
# Begin Source File

SOURCE=.\fsrv.hpp
# End Source File
# Begin Source File

SOURCE=.\ind.hpp
# End Source File
# Begin Source File

SOURCE=.\localus.hpp
# End Source File
# Begin Source File

SOURCE=.\ncpane.hpp
# End Source File
# Begin Source File

SOURCE=.\Net.hpp
# End Source File
# Begin Source File

SOURCE=.\nframe.hpp
# End Source File
# Begin Source File

SOURCE=.\optdlg.hpp
# End Source File
# Begin Source File

SOURCE=.\phus.hpp
# End Source File
# Begin Source File

SOURCE=.\polinom.h
# End Source File
# Begin Source File

SOURCE=.\ruser.hpp
# End Source File
# Begin Source File

SOURCE=.\server.hpp
# End Source File
# Begin Source File

SOURCE=.\Stdafx.h
# End Source File
# End Group
# Begin Group "RC"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Rc\About.bmp
# End Source File
# Begin Source File

SOURCE=.\Rc\About256.img
# End Source File
# Begin Source File

SOURCE=.\Rc\cdrom.bmp
# End Source File
# Begin Source File

SOURCE=.\Rc\connect.bmp
# End Source File
# Begin Source File

SOURCE=.\Rc\copy.bmp
# End Source File
# Begin Source File

SOURCE=.\Rc\direct.bmp
# End Source File
# Begin Source File

SOURCE=.\Rc\drive.bmp
# End Source File
# Begin Source File

SOURCE=.\Rc\erase.bmp
# End Source File
# Begin Source File

SOURCE=.\Rc\Exit.bmp
# End Source File
# Begin Source File

SOURCE=.\Rc\fixed.bmp
# End Source File
# Begin Source File

SOURCE=.\Rc\icon_com.ico
# End Source File
# Begin Source File

SOURCE=.\Rc\move.bmp
# End Source File
# Begin Source File

SOURCE=.\Rc\net.ico
# End Source File
# Begin Source File

SOURCE=.\Rc\nets.rc
# End Source File
# Begin Source File

SOURCE=.\Rc\options.bmp
# End Source File
# Begin Source File

SOURCE=.\Rc\rembl.bmp
# End Source File
# Begin Source File

SOURCE=.\Rc\remote.bmp
# End Source File
# End Group
# End Target
# End Project
