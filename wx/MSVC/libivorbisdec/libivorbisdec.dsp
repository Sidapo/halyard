# Microsoft Developer Studio Project File - Name="libivorbisdec" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libivorbisdec - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libivorbisdec.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libivorbisdec.mak" CFG="libivorbisdec - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libivorbisdec - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libivorbisdec - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libivorbisdec - Win32 Release"

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
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_WIN32" /D inline=__inline /D LITTLE_ENDIAN=1 /D BIG_ENDIAN=2 /D BYTE_ORDER=LITTLE_ENDIAN /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libivorbisdec - Win32 Debug"

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
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /Gm /GX /Zi /O2 /I "../../../libs/libivorbisdec" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_WIN32" /D inline=__inline /D LITTLE_ENDIAN=1 /D BIG_ENDIAN=2 /D BYTE_ORDER=LITTLE_ENDIAN /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libivorbisdec - Win32 Release"
# Name "libivorbisdec - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\bitwise.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\block.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\codebook.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\floor0.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\floor1.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\framing.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\info.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\mapping0.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\mdct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\registry.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\res012.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\sharedbook.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\synthesis.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\vorbisfile.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\window.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\backends.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\codebook.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\codec_internal.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\ivorbisfile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\lsp_lookup.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\mdct.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\mdct_lookup.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\misc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\ogg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\os.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\registry.h
# End Source File
# Begin Source File

SOURCE=..\..\src\VorbisFile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\window.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\libivorbisdec\window_lookup.h
# End Source File
# End Group
# End Target
# End Project
