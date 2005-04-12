# Microsoft Developer Studio Project File - Name="AMXBot" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=AMXBot - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AMXBot.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AMXBot.mak" CFG="AMXBot - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AMXBot - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "AMXBot - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AMXBot - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "AMXBot___Win32_Release"
# PROP BASE Intermediate_Dir "AMXBot___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Objs"
# PROP Intermediate_Dir "Objs"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "Small/amx/amx ." /I "H:\Programmation\__MyProjects\amxbot_svn\amxbot\src" /I "..\src ..\Small\amx\amx\\" /I "..\src" /I "..\src\Small\amx\amx\\" /I "..\src\Ruby\ruby" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AMX_NODYNALOAD" /D "AMX_CONSOLE" /D "HAVE_STRING_H" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 comctl32.lib shlwapi.lib WS2_32.LIB kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"..\bin\dryon.exe"
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=compress executable
PostBuild_Cmds=upx -9 ../amxbot/amxbot.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "AMXBot - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "AMXBot___Win32_Debug"
# PROP BASE Intermediate_Dir "AMXBot___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Objs"
# PROP Intermediate_Dir "Objs"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Ob2 /I "..\Small\amx\amx\\" /I "..\src" /I "..\src\Small\amx\amx\\" /I "..\src\Ruby\ruby" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AMX_NODYNALOAD" /D "AMX_CONSOLE" /D "HAVE_STRING_H" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib shlwapi.lib WS2_32.LIB kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"..\bin\dryon.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "AMXBot - Win32 Release"
# Name "AMXBot - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\Small\amx\amx\amx.c
# End Source File
# Begin Source File

SOURCE=..\src\Small\amx_script.cpp
# End Source File
# Begin Source File

SOURCE=..\src\amxbot.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Small\amx\amx\amxcore.c
# End Source File
# Begin Source File

SOURCE=..\src\basebot.cpp
# End Source File
# Begin Source File

SOURCE=.\bot.rc
# End Source File
# Begin Source File

SOURCE=..\src\cfg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\log.cpp
# End Source File
# Begin Source File

SOURCE=..\src\match.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Small\natives.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Small\natives_base.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Small\natives_strings.cpp
# End Source File
# Begin Source File

SOURCE=..\src\regex.c
# End Source File
# Begin Source File

SOURCE=..\src\Ruby\ruby_embed.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Ruby\ruby_script.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Small\sc_comp.cpp
# End Source File
# Begin Source File

SOURCE=..\src\script.cpp
# End Source File
# Begin Source File

SOURCE=..\src\serverlist.cpp
# End Source File
# Begin Source File

SOURCE=..\src\sockets.cpp
# End Source File
# Begin Source File

SOURCE=..\src\termwin.cpp
# End Source File
# Begin Source File

SOURCE=..\src\thread.cpp
# End Source File
# Begin Source File

SOURCE=..\src\tokens.cpp
# End Source File
# Begin Source File

SOURCE=..\src\userfile.cpp
# End Source File
# Begin Source File

SOURCE=..\src\utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\Small\amx_script.h
# End Source File
# Begin Source File

SOURCE=..\src\amxbot.h
# End Source File
# Begin Source File

SOURCE=..\src\basebot.h
# End Source File
# Begin Source File

SOURCE=..\src\cfg.h
# End Source File
# Begin Source File

SOURCE=..\src\config.h
# End Source File
# Begin Source File

SOURCE=..\src\log.h
# End Source File
# Begin Source File

SOURCE=..\src\match.h
# End Source File
# Begin Source File

SOURCE=..\src\Small\natives.h
# End Source File
# Begin Source File

SOURCE=..\src\regex.h
# End Source File
# Begin Source File

SOURCE=..\src\Ruby\ruby_embed.h
# End Source File
# Begin Source File

SOURCE=..\src\Ruby\ruby_script.h
# End Source File
# Begin Source File

SOURCE=..\src\script.h
# End Source File
# Begin Source File

SOURCE=..\src\serverlist.h
# End Source File
# Begin Source File

SOURCE=..\src\sockets.h
# End Source File
# Begin Source File

SOURCE=..\src\termwin.h
# End Source File
# Begin Source File

SOURCE=..\src\thread.h
# End Source File
# Begin Source File

SOURCE=..\src\tokens.h
# End Source File
# Begin Source File

SOURCE=..\src\userfile.h
# End Source File
# Begin Source File

SOURCE=..\src\utils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# End Group
# End Target
# End Project
