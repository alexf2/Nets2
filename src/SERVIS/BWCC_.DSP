# Microsoft Developer Studio Project File - Name="bwcc_" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=bwcc_ - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bwcc_.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bwcc_.mak" CFG="bwcc_ - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bwcc_ - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "bwcc_ - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bwcc_ - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "f:\work\nets\exe"
# PROP Intermediate_Dir "f:\work\nets\exe"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G3 /Zp1 /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386 /out:"f:\work\nets\exe/Servis.dll"

!ELSEIF  "$(CFG)" == "bwcc_ - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "f:\work\nets\exe"
# PROP Intermediate_Dir "f:\work\nets\exe"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G3 /Zp1 /MDd /W3 /Gm /GR /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /out:"f:\work\nets\exe/Servis.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "bwcc_ - Win32 Release"
# Name "bwcc_ - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Expert\Autorc.cpp
# End Source File
# Begin Source File

SOURCE=.\borctl.cpp
# End Source File
# Begin Source File

SOURCE=.\bwcc_.cpp
# End Source File
# Begin Source File

SOURCE=.\Mymenu.cpp
# End Source File
# Begin Source File

SOURCE=.\pathex.cpp
# End Source File
# Begin Source File

SOURCE=.\servis.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# End Source File
# Begin Source File

SOURCE=.\texcept.cpp
# End Source File
# End Group
# Begin Group "Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\EXPERT\Autorc.hpp
# End Source File
# Begin Source File

SOURCE=.\borctl.hpp
# End Source File
# Begin Source File

SOURCE=.\bwcc.hpp
# End Source File
# Begin Source File

SOURCE=.\Mymenu.hpp
# End Source File
# Begin Source File

SOURCE=.\pathex.hpp
# End Source File
# Begin Source File

SOURCE=.\servis.hpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\texcept.hpp
# End Source File
# End Group
# Begin Group "Rc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bwcc.rc

!IF  "$(CFG)" == "bwcc_ - Win32 Release"

!ELSEIF  "$(CFG)" == "bwcc_ - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rc\Cancel.ico
# End Source File
# Begin Source File

SOURCE=..\..\Expert\Rc\icon1.ico
# End Source File
# Begin Source File

SOURCE=..\..\Expert\Rc\icon2.ico
# End Source File
# Begin Source File

SOURCE=..\..\EXPERT\Rc\Mask.bmp
# End Source File
# Begin Source File

SOURCE=..\..\EXPERT\Rc\Mask2.bmp
# End Source File
# Begin Source File

SOURCE=.\rc\Yes.ico
# End Source File
# End Group
# End Target
# End Project
