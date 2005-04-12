# Microsoft Developer Studio Project File - Name="Ruby_dll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Ruby_dll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Ruby_dll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Ruby_dll.mak" CFG="Ruby_dll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Ruby_dll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Ruby_dll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Ruby_dll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Objs/ruby"
# PROP Intermediate_Dir "Objs/ruby"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RUBY_DLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "RUBY_DLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RUBY_DLL_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 WS2_32.LIB kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"..\bin\Ruby_bot.dll"

!ELSEIF  "$(CFG)" == "Ruby_dll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Objs/ruby"
# PROP Intermediate_Dir "Objs/ruby"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RUBY_DLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "RUBY_DLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RUBY_DLL_EXPORTS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 WS2_32.LIB kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /debug /machine:I386 /out:"..\bin\Ruby_bot.dll" /pdbtype:sept
# SUBTRACT LINK32 /force

!ENDIF 

# Begin Target

# Name "Ruby_dll - Win32 Release"
# Name "Ruby_dll - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="..\src\Ruby\ruby\missing\acosh.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\array.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\bignum.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\class.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\compar.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\missing\crypt.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\dir.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\dln.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\dmyext.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\enum.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\missing\erf.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\error.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\eval.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\file.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\gc.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\hash.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\inits.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\io.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\marshal.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\math.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\numeric.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\object.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\pack.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\parse.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\prec.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\process.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\random.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\range.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\re.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\regex.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\ruby.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\signal.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\sprintf.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\st.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\string.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\struct.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\time.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\util.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\variable.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\version.c"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\win32\win32.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="..\src\Ruby\ruby\config.h"
# End Source File
# Begin Source File

SOURCE="..\src\Ruby\ruby\ruby.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
