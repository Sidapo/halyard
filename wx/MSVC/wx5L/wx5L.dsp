# Microsoft Developer Studio Project File - Name="wx5L" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=wx5L - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wx5L.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wx5L.mak" CFG="wx5L - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wx5L - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "wx5L - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wx5L - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "wx5L - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "../../../Common" /I "../../../Common/freetype2/include" /I "../../../Common/libs/boost" /I "../../../Common/libs/wxWindows/include" /I "../../../Common/libs/wxWindows/lib/mswd" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\..\..\Common\libs\wxWindows\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib qtmlclient.lib ../../../Common/libs/wxWindows/lib\zlibd.lib ../../../Common/libs/wxWindows/lib\regexd.lib ../../../Common\libs/wxWindows/lib\pngd.lib ../../../Common/libs/wxWindows/lib\jpegd.lib ../../../Common/libs/wxWindows/lib\tiffd.lib ../../../Common/libs/wxWindows/lib\wxmswd.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "wx5L - Win32 Release"
# Name "wx5L - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\Element.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\FiveLApp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Listener.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Log5L.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\MovieWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Stage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Timecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ToolWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\TQTMovie.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\TWxPrimitives.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Widget.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Zone.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\Element.h
# End Source File
# Begin Source File

SOURCE=..\..\src\FiveLApp.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Listener.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Log5L.h
# End Source File
# Begin Source File

SOURCE=..\..\src\MovieWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Stage.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Timecoder.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ToolWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\src\TQTMovie.h
# End Source File
# Begin Source File

SOURCE=..\..\src\TWxPrimitives.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Widget.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Zone.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\src\ic_5L.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\ic_listener.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\ic_timecoder.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\tb_borders.bmp
# End Source File
# Begin Source File

SOURCE=..\..\src\tb_grid.bmp
# End Source File
# Begin Source File

SOURCE=..\..\src\tb_reload.bmp
# End Source File
# Begin Source File

SOURCE=..\..\src\tb_xy.bmp
# End Source File
# Begin Source File

SOURCE=..\..\src\wx5L.rc
# End Source File
# End Group
# End Target
# End Project
