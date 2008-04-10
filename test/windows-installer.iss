;; Set this to 1 to include various useful debugging stuff, and 0
;; to make a final build.
#define INCLUDE_DEBUGGING_SUPPORT 1

;; Set this to 1 to build a CD installer, and 0 otherwise.
#ifndef CD_INSTALLER
#define CD_INSTALLER 0
#endif

[Files]
Source: qtcheck.dll; Flags: dontcopy
;; The MANIFEST.* files are generated by our release-building
;; scripts, and should not be checked into CVS.
Source: MANIFEST.base; DestDir: {app}; Flags: skipifsourcedoesntexist; Components: base
Source: MANIFEST.media; DestDir: {app}; Flags: skipifsourcedoesntexist; Components: media
#if INCLUDE_DEBUGGING_SUPPORT
Source: MANIFEST.debug; DestDir: {app}; Flags: skipifsourcedoesntexist; Components: debug
#endif
;; The release.spec file is also generated by the build script.
Source: release.spec; DestDir: {app}; Flags: skipifsourcedoesntexist; Components: base
Source: release.spec.sig; DestDir: {app}; Flags: skipifsourcedoesntexist; Components: base
Source: TRUST-PRECOMPILED; DestDir: {app}; Flags: skipifsourcedoesntexist; Components: base
Source: libmzgc2.dll; DestDir: {app}; Components: base
Source: libmzsch2.dll; DestDir: {app}; Components: base
Source: data.tam; DestDir: {app}; Components: base
;;Source: splash.png; DestDir: {app}
;;Source: icon16.png; DestDir: {app}
;;Source: icon32.png; DestDir: {app}
Source: Tamale.exe; DestDir: {app}; Components: base
Source: wxref_gl.dll; DestDir: {app}; Components: base
Source: wxref_soft.dll; DestDir: {app}; Components: base
Source: dbghelp.dll; DestDir: {app}; Components: base
Source: zlib.dll; DestDir: {app}; Components: base
Source: curl.exe; DestDir: {app}; Components: base
Source: gpgv.exe; DestDir: {app}; Components: base
Source: trustedkeys.gpg; DestDir: {app}; Components: base
Source: UpdateInstaller.exe; DestDir: {app}; Components: base
Source: Fonts\*; DestDir: {app}\Fonts; Excludes: CVS,.cvsignore,*.bak,.#*,#*,*~,cache.dat; Flags: recursesubdirs; Components: base
Source: Graphics\*; DestDir: {app}\Graphics; Excludes: CVS,.cvsignore,*.bak,.#*,#*,*~,*.psd; Flags: recursesubdirs nocompression; Components: base
Source: Runtime\*; DestDir: {app}\Runtime; Excludes: CVS,.cvsignore,*.bak,.#*,#*,*~,compiled; Flags: recursesubdirs; Components: base
Source: Scripts\*; DestDir: {app}\Scripts; Excludes: CVS,.cvsignore,*.bak,.#*,#*,*~,compiled; Flags: recursesubdirs; Components: base
;; Install and touch all the *.zo files so they have newer timestamps than
;; the *.ss files.
Source: Runtime\*.zo; DestDir: {app}\Runtime; Flags: recursesubdirs touch; Components: base
Source: Runtime\*.dep; DestDir: {app}\Runtime; Flags: recursesubdirs touch; Components: base
Source: Scripts\*.zo; DestDir: {app}\Scripts; Flags: recursesubdirs touch skipifsourcedoesntexist; Components: base
Source: Scripts\*.dep; DestDir: {app}\Scripts; Flags: recursesubdirs touch skipifsourcedoesntexist; Components: base
;; Source: baseq2\*; DestDir: {app}\baseq2; Excludes: CVS,.cvsignore,*.bak,.#*,#*,*~,*.exe,*.map,*.bat,snapshots; Flags: recursesubdirs nocompression; Components: base
Source: testq2\*; DestDir: {app}\testq2; Excludes: CVS,.cvsignore,*.bak,.#*,#*,*~,*.exe,*.map,*.bat,snapshots,*.psd; Flags: recursesubdirs; Components: base
;; All media except *.mp3 and *.mov files must be built into the installer. This is
;; because we can't stream anything but QuickTime media formats.
Source: LocalMedia\*; DestDir: {app}\LocalMedia; Flags: recursesubdirs nocompression; Components: base; Excludes: CVS,.cvsignore,*.bak,.#*,#*,*~
#if CD_INSTALLER
Source: Media\*.mov; DestDir: {app}\Media; Flags: recursesubdirs skipifsourcedoesntexist nocompression; Components: media
Source: Media\*.mp3; DestDir: {app}\Media; Flags: recursesubdirs skipifsourcedoesntexist nocompression; Components: media
#else
;; The *.mp3 and *.mov files may be in a directory on the CD, or missing for a
;; web install.
Source: {src}\Media\*; DestDir: {app}\Media; Flags: recursesubdirs external; Components: media
#endif
#if INCLUDE_DEBUGGING_SUPPORT
Source: Tamale_d.exe; DestDir: {app}; Components: debug
Source: libmzgc2_d.dll; DestDir: {app}; Components: debug
Source: libmzsch2_d.dll; DestDir: {app}; Components: debug
Source: 5L.prefs; DestDir: {app}; Components: debug
#endif
Source: {src}\QuickTimeFullInstaller6_5.exe; DestDir: {tmp}; Components: quicktime; Flags: deleteafterinstall external
Source: "\\Mccay\Mccay\program\archive\program_sources\Installers\QuickTime6\QuickTimeInstaller.ini"; DestDir: {tmp}; Components: quicktime; Flags: deleteafterinstall
Source: "\\Mccay\Mccay\program\archive\program_sources\Installers\On2 VP3\VP3 3.2.6.1. Windows\On2_VP3.qtx"; DestDir: {sys}\QuickTime; Components: vp3; Flags: uninsneveruninstall
[Types]
Name: regular; Description: Regular Install
Name: custom; Description: Custom Install; Flags: iscustom
[Components]
Name: base; Description: Core Files; Flags: fixed; Types: custom regular
#if CD_INSTALLER
Name: media; Description: Video Files; Types: custom regular
#else
Name: media; Description: Video Files (optional); Types: custom regular; Check: DirCheck(ExpandConstant('{src}\Media'))
#endif
#if INCLUDE_DEBUGGING_SUPPORT
Name: debug; Description: Debugging Support; Types: custom regular
#endif
Name: quicktime; Description: QuickTime; Flags: fixed disablenouninstallwarning; Types: custom regular; Check: NeedQuickTime; ExtraDiskSpaceRequired: 4194304
Name: vp3; Description: On2 VP3 QuickTime Codec; Flags: fixed disablenouninstallwarning; Types: custom regular; Check: NeedVP3
[Setup]
MinVersion=4.1.1998,5.0.2195
AppCopyright=Copyright 2006 Trustees of Dartmouth College
AppName=Tamale Test
AppVerName=Tamale Test
LicenseFile=LICENSE.txt
PrivilegesRequired=admin
DefaultDirName={pf}\Tamale Test
AlwaysShowComponentsList=false
ShowLanguageDialog=yes
#if CD_INSTALLER
OutputDir=cd_installer
#else
OutputDir=.
#endif
OutputBaseFilename=Tamale_Test_Setup
DefaultGroupName=Tamale Test
#if CD_INSTALLER
DiskSpanning=true
ReserveBytes=20971520
DiskSliceSize=732954624
#endif
AppID={{B46C75D0-CC42-4F32-9F92-5D1500EDE4CB}
[Run]
Filename: {tmp}\QuickTimeFullInstaller6_5.exe; Components: quicktime; StatusMsg: Installing QuickTime...
[Icons]
Name: {group}\Tamale Test; Filename: {app}\Tamale.exe; IconIndex: 0; Flags: createonlyiffileexists; Parameters: """{app}"""; WorkingDir: {app}
Name: {group}\Tamale Test (DO NOT USE: Debugging Only); Filename: {app}\Tamale_d.exe; IconIndex: 0; Flags: createonlyiffileexists; Parameters: """{app}"""; WorkingDir: {app}
[Code]
const
    // Display the currently installed versions of various components.
    // (For debugging purposes only.)
    DebugShowVersions = false;
    // Minimum QuickTime version.  See below for a URL explaining what
    // this means.
    MinimumQuickTimeVersion = $06508000;
    MinimumQuickTimeVersionString = '6.5';
    QuickTimeInstaller = '{src}\QuickTimeFullInstaller6_5.exe';
    // Minimum VP3 version.  This is a nasty, in-house hacked version
    // with minimum support for QuickTime 6; there should be a better
    // one available in the QuickTime update service.
    MinimumVP3Version = 131075;
    // Minimum Flash version.  This is actually the name of an
    // ActiveX interface that will be provided by any compatible
    // version of the Flash Player.
    MinimumFlashVersion = 'ShockwaveFlash.ShockwaveFlash.7';

var
    // Cached checks for QuickTime components.
    QuickTimeVersionOK, VP3VersionOK: Boolean;


//-------------------------------------------------------------------------
//  Basic Checks
//-------------------------------------------------------------------------

// Never use the specified component.
function NeverUse: Boolean;
begin
    Result := false;
end;

// Check to see whether a directory exists.
function DirCheck(DirName: String): Boolean;
begin
    Result := DirExists(DirName);
end;

// Check to see whether a file exists.
function FileCheck(FileName: String): Boolean;
begin
    Result := FileExists(FileName);
end;

// Check to see whether we can create the specified ActiveX control.
function HaveActiveXControl(const ControlName: String): Boolean;
var
    Player: Variant;
begin
    Result := true;
    try
        Player := CreateOleObject(ControlName);
    except
        Result := false;
    end;
end;


//-------------------------------------------------------------------------
//  Flash Support
//-------------------------------------------------------------------------

// Check to see whether we should update MacroMedia's Flash Player.
function NeedFlashPlayer(): Boolean;
begin
    Result := not HaveActiveXControl(MinimumFlashVersion);
end;


//-------------------------------------------------------------------------
//  QuickTime Support
//-------------------------------------------------------------------------
//  This is more annoying than it should be, because we're not allowed to
//  include the QuickTime installer with web downloads, and we need an
//  external DLL to figure out what is already installed.

// Check to see whether we have a QuickTime installer.
function HaveQuickTimeInstaller(): Boolean;
begin
    Result := FileCheck(ExpandConstant(QuickTimeInstaller));
end;

// Import QuickTime-related functions from qtcheck.dll.  The *Version
// functions return weirdly encoded version numbers; see
// http://developer.apple.com/technotes/tn/tn1197.html for some discussion
// of how these numbers are interpreted.
procedure QuickTimeCheckSetup();
external 'QuickTimeCheckSetup@files:qtcheck.dll stdcall';
function QuickTimeVersion(): Integer;
external 'QuickTimeVersion@files:qtcheck.dll stdcall';
function QuickTimeComponentVersion(ctype, csubtype: String): Integer;
external 'QuickTimeComponentVersion@files:qtcheck.dll stdcall';
procedure QuickTimeCheckCleanup();
external 'QuickTimeCheckCleanup@files:qtcheck.dll stdcall';

// Figure out a bunch of facts about QuickTime and cache them for later.
procedure CacheQuickTimeChecks();
var
    QTVer, VP3Ver: Integer;
begin
    // Initialize our DLL.
    QuickTimeCheckSetup();

    // Look up the versions we care about.
    QTVer := QuickTimeVersion();
    VP3Ver := QuickTimeComponentVersion('imco', 'VP31');

    // Clean up our DLL.
    QuickTimeCheckCleanup();

    // Display what we've learned.  To make sense of these numbers, you'll
    // need to convert them into hexadecimal.
    if DebugShowVersions then begin
      MsgBox('QuickTime: '+IntToStr(QTVer)+' VP3: '+IntToStr(VP3Ver),
        mbInformation, MB_OK);
    end;

    // Default our cached values to false;
    QuickTimeVersionOK := false;
    VP3VersionOK := false;
    if QTVer >= MinimumQuickTimeVersion then begin
        QuickTimeVersionOK := true;
    end;
    if VP3Ver >= MinimumVP3Version then begin
        VP3VersionOK := true;
    end;
end;

// Do we need to install or update QuickTime?
function NeedQuickTime(): Boolean;
begin
    Result := not QuickTimeVersionOK;
end;

// Do we need to install or update our VP3 codec?
function NeedVP3(): Boolean;
begin
    Result := not VP3VersionOK;
end;


//-------------------------------------------------------------------------
//  Installer Logic
//-------------------------------------------------------------------------

// This is called immediately after the installer starts.
function InitializeSetup(): Boolean;
begin
    // Assume, by default, that we want to run the installer.
    Result := true;

    // Load our DLL and figure out a bunch of useful information
    // about QuickTime *once*.  We'll use this information throughout
    // the install process.
    CacheQuickTimeChecks();

    // Make sure we'll be able to do something intelligent about
    // about QuickTime.
    if NeedQuickTime() and not HaveQuickTimeInstaller() then begin
        MsgBox(ExpandConstant('Please install QuickTime ' +
                              MinimumQuickTimeVersionString + ' ' +
                              'or later before installing this program.'),
               mbError, MB_OK);
        Result := false;
    end;
end;

// It work be really nice if this routine worked, but it doesn't, because
// QuickTime tends to turn into a half-installed mess if the user hits
// "Cancel" on the QuickTime installer.
//
//procedure CheckQuickTimeInstall();
//begin
//	CacheQuickTimeChecks(); // Re-run our QuickTime checks.
//	if NeedQuickTime() then begin
//		MsgBox('Your copy of QuickTime may not be fully installed. ' +
//		       'Before running this program, please ' +
//		       'make sure QuickTime is installed.',
//		       mbError, MB_OK);
//	end;
//end;