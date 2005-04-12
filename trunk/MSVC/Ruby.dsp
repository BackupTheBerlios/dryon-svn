# Microsoft Developer Studio Project File - Name="Ruby" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Ruby - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Ruby.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Ruby.mak" CFG="Ruby - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Ruby - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Ruby - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Ruby - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Objs"
# PROP Intermediate_Dir "Objs"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Ruby - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Ruby___Win32_Debug"
# PROP BASE Intermediate_Dir "Ruby___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Objs"
# PROP Intermediate_Dir "Objs"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Ruby - Win32 Release"
# Name "Ruby - Win32 Debug"
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
# End Target
# End Project
