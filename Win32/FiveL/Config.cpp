// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-
//////////////////////////////////////////////////////////////////////////////
//
//   (c) Copyright 1999, Trustees of Dartmouth College, All rights reserved.
//        Interactive Media Lab, Dartmouth Medical School
//
//			$Author$
//          $Date$
//          $Revision$
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Config.cpp : 
//

#include "stdafx.h"

#include <fstream.h>

#include <ctype.h>
#include <direct.h>

#include "Config.h"
#include "LUtil.h"
#include "Globals.h"

#define OPT_INSTALL_DIR		1
#define OPT_SCRIPT_FILE		2
#define OPT_MOVIE_DIR		3 
#define OPT_CONFIG_FILE		4

// local globals
static char	*usage_str = "Illegal command line <%s>\n\nUsage: 5L.exe [-m movie_dir] [-d install_dir] [-s script_file] [-c config_file] [-x]";

//
//	Config classes
//
Config::Config()
{
}

Config::~Config()
{
}

//
// 	ConfigManager classes
// 

//
//	ConfigManager - constructor
//
ConfigManager::ConfigManager()
{
	m_NumModules = 0;
	m_PlayMedia = true;
	m_Configs = NULL;
}

//
//	~ConfigManager
//
ConfigManager::~ConfigManager()
{
	if (m_Configs)
		delete [] m_Configs;
}

//
//	Init - Process the command line and read the config file.
//
bool ConfigManager::Init(LPSTR inCmdLine)
{ 
	ifstream	theConfigFile;
	ifstream	userProfile;
	char		argBuf[MAX_PATH];
	char		*strPtr; 
	char		*chPtr;
	int32		i;   
	int			argCnt;
	int			theOption;
	bool		getArg = false;
	bool		done = false;
	char		errString[256];		// used to print error message
    
    // make sure our path variables are empty
    m_InstallDir = ""; 
    m_ConfigFile = "";
    m_GraphicsDir = "";
    m_LocalMediaDir = "";
    m_MediaDrive = "";

	m_DLSUser = "DefaultUser";
    
   // command line processing
    // -x -d installDir -m movieDir -s scriptFile -c configFile
    // installDir is optional, will use current dir by default
    // movieDir is optional, won't play movies if there isn't one
    // scriptFile is optional, will default to start.scr
    // configFile is optional, will default to 5L_config
    // -x means don't play media (or even look for them)
    //
    if (strcmp(inCmdLine, "")) // command line exists 
    {	
    	strPtr = inCmdLine;
    	
    	while (not done)
    	{
    		strPtr = strstr(strPtr, "-");
    		if (strPtr != NULL)
    		{
    			strPtr++;
    			switch (*strPtr++)
    			{ 
    				case 'c':
    					theOption = OPT_CONFIG_FILE;
    					getArg = true;
    					break;
    				case 'd':
						// -d: installation directory
    					theOption = OPT_INSTALL_DIR;
    					getArg = true;
    					break;
    				case 's':
						// -s: script file name
    				    theOption = OPT_SCRIPT_FILE;
    				    getArg = true;
    				    break;
    				case 'm':
    					theOption = OPT_MOVIE_DIR;
    					getArg = true;
    					break;
    				case 'x':
    					m_PlayMedia = false;
    					getArg = false;
    					break;  
    				default:
    					sprintf(errString, "Unknown command-line option \"%s\"", inCmdLine);
						AlertMsg(errString, true);
						//gLog.Error(usage_str, inCmdLine); 
    					return (false);
    					break;
    			}
    			
    			// skip whitespace
    			while (isspace(*strPtr))
    				strPtr++;
    			
    			if (getArg)
    			{
	    			argCnt = 0;

	    			// read in the argument	
	    			while ((not isspace (*strPtr)) and (*strPtr != '\0'))
					{
						
    					argBuf[argCnt++] = *strPtr++;
					}

	    			argBuf[argCnt] = '\0'; 
	    			
					switch (theOption)
	    			{
	    				case OPT_CONFIG_FILE:
	    					m_ConfigFile = (const char *) argBuf;
	    					break;
	    				case OPT_INSTALL_DIR:
	    					m_InstallDir = (const char *) argBuf;
	    					break;
	    				case OPT_SCRIPT_FILE: 
	    					m_CurScript = (const char *) argBuf;
	    					break;
	    				case OPT_MOVIE_DIR:
	    					m_CurMediaDir = (const char *) argBuf;
	    					break;
	    			}
		    	}
    		}
    		else
    			done = true;
    	} 
    }	// done with the command line
    
    // see if we need defaults for anything
 
 	// if no install dir, use current directory
	if (m_InstallDir.IsEmpty())
	{
		DWORD	dirLen;

		dirLen = ::GetCurrentDirectory(MAX_PATH, argBuf);
		//strPtr = _getcwd(argBuf, MAX_PATH);
		if (dirLen == 0)
		{
			//gLog.Error("Error: Couldn't get current working directory");
			AlertMsg("Couldn't get current working directory", true);
			return (false);
		}
		
		m_InstallDir = (const char *) argBuf;
	}

	// make sure the install dir is a path
	if (not m_InstallDir.EndsWith('\\'))
		m_InstallDir += '\\';
   
    if (m_CurScript.IsEmpty())
    	m_CurScript = "start";			// default to start.scr
	else
	{
		chPtr = strstr(m_CurScript.GetBuffer(), ".");
		if (chPtr != NULL)
		{
			*chPtr = '\0';
			m_CurScript.Update();
		}
	}
    	
	// default the config file to config
	if (m_ConfigFile.IsEmpty())
	{
		m_ConfigFile = m_InstallDir;
		m_ConfigFile += "config";
	}

	// figure out the CD-ROM drive
	{
		TString		theDisk;
		char		buffer[256];
		int			driveStrLen;
		bool		have_CD = false;

		driveStrLen = ::GetLogicalDriveStrings(256, buffer);
		if (driveStrLen != 0)
		{
			for (int i = 0; i < driveStrLen; i++)
			{
				if (buffer[i] == '\0')
				{
					// have a drive
					if (::GetDriveType(theDisk.GetString()) == DRIVE_CDROM)
					{
						m_MediaDrive = theDisk;
						have_CD = true;
						break;
					}
					else
						theDisk.Empty();
				}
				else
					theDisk += buffer[i];
			}
		}

		if (not have_CD)
		{
			HKEY		hKey = NULL;
			TString		theKey;
			char		theBuffer[64];
			DWORD		theSize;
			DWORD		theType;
			int32		error;

			theKey = "Software\\IML";

			error = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, theKey.GetString(), 0, KEY_READ, &hKey);	
			if (error == ERROR_SUCCESS)
			{

				theKey = "CD_DRIVE";
				theSize = 64;
				error = ::RegQueryValueEx(hKey, theKey, NULL, &theType, (unsigned char *) theBuffer, &theSize);
				if (error == ERROR_SUCCESS)
				{
					have_CD = true;
					m_MediaDrive = theBuffer;
				}
			}

			// if we don't have it in the Registry, try WIN.INI
			if (not have_CD)
			{
				// get our CD drive letter
				::GetProfileString("IML", "cdDrive", "d:\\", argBuf, 256);
				m_MediaDrive = argBuf;
			}
		}
	}
   
    // should we have a default media dir??
    if (m_CurMediaDir.IsEmpty())
    {
    	m_CurMediaDir = m_MediaDrive;
    	m_CurMediaDir += "media\\";
    }

	if (not m_CurMediaDir.EndsWith('\\'))
		m_CurMediaDir += '\\';
    
    // set up search directories
    m_GraphicsDir = m_InstallDir;
    m_GraphicsDir += "graphics\\";
    m_LocalMediaDir = m_InstallDir;
    m_LocalMediaDir += "media\\";

	// Pull user information for user.profile if it exists
	TString userProfileName = m_InstallDir;
	userProfileName += DLS_USER_PROFILE;
	userProfile.open(userProfileName, ios::in | ios::nocreate);
	
	if (!userProfile.fail())
	{
		while (!userProfile.eof()) {
			TString sLine;
			char lineBuf[128];

			userProfile.getline(lineBuf, 128, '\n');
			sLine = lineBuf;
			if (sLine.StartsWith("name=", false)) {
				int index = sLine.Find('"');
				m_DLSUser = sLine.Mid(index+1);
				m_DLSUser.RChop();	// chop trailing quote
				break;
			}
		}
		userProfile.close();
	}

    // now try to read the config file to get information about
    // other scripts in the title
    theConfigFile.open(m_ConfigFile.GetString(), ios::in | ios::nocreate);

	if (theConfigFile.is_open())
	{	
		theConfigFile >> m_NumModules;
		
		if (m_NumModules > 0)
		{
			m_Configs = new Config[m_NumModules];
			
			if (m_Configs == NULL)
				m_NumModules = 0;
			else
			{
				for (i = 0; i < m_NumModules; i++)
				{
					theConfigFile >> m_Configs[i].m_ScriptName;
    		        
    		        // strip the .pic off
					chPtr = strstr(m_Configs[i].m_ScriptName.GetBuffer(), ".");
					if (chPtr != NULL)
					{
						*chPtr = '\0';
						m_Configs[i].m_ScriptName.Update();
					}

					theConfigFile >> m_Configs[i].m_MediaDir;
					m_Configs[i].m_MediaDir += "\\";
				}
			}
		}
		
		theConfigFile.close(); 
	}
	return (true);
}

//
//	SwitchScripts -
//
bool ConfigManager::SwitchScripts(int32 inScriptNum)
{
	bool	retValue = false;

	if ((inScriptNum > 0) and (m_NumModules > 0) and (inScriptNum <= m_NumModules))
	{
		m_CurScript = m_Configs[inScriptNum - 1].m_ScriptName;
		m_CurMediaDir = m_MediaDrive;
		m_CurMediaDir += m_Configs[inScriptNum - 1].m_MediaDir;

		retValue = true;
	}
	
	return (retValue);
}

//
//	GetGraphicsPath - Return the graphics path to use for the given graphic.
//
TString ConfigManager::GetGraphicsPath(TString &inName)
{
	TString		fullPath;
	TString		newName = inName;

	// make sure it has an extension
	if (not inName.Contains("."))
	{
		// no extension, put one on there
		TString	graphicsExtension = gVariableManager.GetString("_BaseGraphicsExtension");
		if (not graphicsExtension.Equal("0"))
			newName += graphicsExtension;
	}

	// see if the name contains a colon
	if (newName.Contains(":"))
	{
		// already have a full path, just see if CD is there
		if (newName.Contains("CD:", false))
		{
			// replace with the actual letter of the CD drive
			fullPath = m_MediaDrive;

			fullPath += newName.Mid(4);

			return (fullPath);
		}

		return (newName);
	}

	// see if the name is a partial path (doesn't start with drive specifier
	//	but has a forward slash in it)
	if (newName.Contains("\\"))
	{
		// partial path, put the installation directory on the front
		fullPath += m_InstallDir;
		fullPath += newName;

		return (fullPath);
	}

	TString		graphicsLocation = gVariableManager.GetString("_BaseGraphicsLocation");
	if (not graphicsLocation.Equal("0"))
	{
		if (graphicsLocation.Contains("CD:", false))
		{
			// replace with the actual letter of the CD drive
			fullPath = m_MediaDrive;

			// get the rest of the path
			fullPath += graphicsLocation.Mid(4);
		}
		else
			fullPath = graphicsLocation;
	}
	else
	{
		// generate a local path to it
		fullPath = m_InstallDir;
		fullPath += "Graphics\\";
	}

	fullPath += newName;

	return (fullPath);
}
//
//	GetVideoPath - Return the video path to use for the given clip.
//		Sequence:
//			1. Have a full URL already (starts with http: or rtsp:), do nothing.
//			2. Have a full path, do nothing.
//			3. Have a partial path, put the installation directory on the front.
//			4. Put the contents of _BaseVideoLocation on the front of the clip name.		
//
TString ConfigManager::GetVideoPath(TString &inName)
{
	TString		fullPath;
	
	if ((inName.Contains("http:", false))
		or (inName.Contains("rtsp:", false)))
	{
		// already have a full URL, nothing to do
		return (inName);
	}

	// see if the name contains a colon
	if (inName.Contains(":"))
	{
		// already have a full path, just see if CD is there
		if (inName.Contains("CD:", false))
		{
			// replace with the actual letter of the CD drive
			fullPath = m_MediaDrive;

			fullPath += inName.Mid(4);

			return (fullPath);
		}

		return (inName);
	}

	// see if the name is a partial path (doesn't start with drive specifier
	//	but has a forward slash in it)
	if (inName.Contains("\\"))
	{
		// partial path, put the installation directory on the front
		fullPath += m_InstallDir;
		fullPath += inName;

		return (fullPath);
	}

	TString		videoLocation = gVariableManager.GetString("_BaseVideoLocation");
	if (not videoLocation.Equal("0"))
	{
		if (videoLocation.Contains("CD:", false))
		{
			// replace with the actual letter of the CD drive
			fullPath = m_MediaDrive;

			// get the rest of the path
			fullPath += videoLocation.Mid(4);
		}
		else
			fullPath = videoLocation;
	}
	else
	{
		// generate a local path to it
		fullPath = m_InstallDir;
		fullPath += "media\\";
	}

	fullPath += inName;

	return (fullPath);
}

//
//	GetAudioPath - 
//
TString ConfigManager::GetAudioPath(TString &inName)
{
	TString		fullPath;
	
	if ((inName.Contains("http:", false))
		or (inName.Contains("rtsp:", false)))
	{
		// already have a full URL, nothing to do
		return (inName);
	}

	// see if the name contains a colon
	if (inName.Contains(":"))
	{
		// already have a full path, just see if CD is there
		if (inName.Contains("CD:\\", false))
		{
			// replace with the actual letter of the CD drive
			fullPath = m_MediaDrive;

			fullPath += inName.Mid(4);

			return (fullPath);
		}
		
		return (inName);
	}

	// see if the name is a partial path (doesn't start with drive specifier
	//	but has a back slash in it)
	if (inName.Contains("\\"))
	{
		// partial path, put the installation directory on the front
		fullPath += m_InstallDir;
		fullPath += inName;

		return (fullPath);
	}


	TString		audioLocation = gVariableManager.GetString("_BaseAudioLocation");
	if (audioLocation.Equal("0"))
		audioLocation = gVariableManager.GetString("_BaseVideoLocation");

	if (not audioLocation.Equal("0"))
	{
		if (audioLocation.Contains("CD:\\", false))
		{
			// replace with the actual letter of the CD drive
			fullPath = m_MediaDrive;

			// get the rest of the path
			fullPath += audioLocation.Mid(4);
		}
		else
			fullPath = audioLocation;
	}
	else
	{
		// generate a local path to it
		fullPath = m_InstallDir;
		fullPath += "media\\";
	}

	fullPath += inName;

	return (fullPath);
}

//bool ConfigManager::GetCDDrives()
//{
//	TString		*cdStr = NULL;
//	TString		theDisk;
//	char		buffer[256];
//	int			driveStrLen;
//	bool		have_CD = false;
//
//	driveStrLen = ::GetLogicalDriveStrings(256, buffer);
//	if (driveStrLen != 0)
//	{
//		for (int i = 0; i < driveStrLen; i++)
//		{
//			if (buffer[i] == '\0')
//			{
//				// have a drive
//				if (::GetDriveType(theDisk.GetString()) == DRIVE_CDROM)
//				{
//					//m_MediaDrive = theDisk;
//					// add each CD to our list of CD drives
//					cdStr = new TString(theDisk);
//					if (cdStr != NULL)
//						m_CDDrives.Add(cdStr);
//
//					have_CD = true;
//				}
//				else
//					theDisk.Empty();
//			}
//			else
//				theDisk += buffer[i];
//		}
//	}
//
//	return (have_CD);
//}
 
/*
 $Log$
 Revision 1.8  2002/07/25 22:25:36  emk
   * Made new CryptStream auto_ptr code work under Windows.
   * PURIFY: Fixed memory leak in TBTree::Add of duplicate node.  We now
     notify the user if there are duplicate cards, macros, etc.
   * PURIFY: Fixed memory leak in TBTree destructor.
   * PURIFY: Fixed memory leak in ConfigManager destructor.
   * PURIFY: Fixed memory leaks when deleting DIBs.
   * PURIFY: Made sure we deleted offscreen GWorld when exiting.
   * PURIFY: Fixed memory leak in LBrowser.
   * PURIFY: Fixed memory leak in LFileBundle.
   * PURIFY: Fixed uninitialized memory reads when View methods were
     called before View::Init.
   * PURIFY: Made View::Draw a no-op before View::Init is called.
     (It seems that Windows causes us to call Draw too early.)

 Revision 1.7  2002/06/20 16:32:54  emk
 Merged the 'FiveL_3_3_4_refactor_lang_1' branch back into the trunk.  This
 branch contained the following enhancements:

   * Most of the communication between the interpreter and the
     engine now goes through the interfaces defined in
     TInterpreter.h and TPrimitive.h.  Among other things, this
     refactoring makes will make it easier to (1) change the interpreter
     from 5L to Scheme and (2) add portable primitives that work
     the same on both platforms.
   * A new system for handling callbacks.

 I also slipped in the following, unrelated enhancements:

   * MacOS X fixes.  Classic Mac5L once again runs under OS X, and
     there is a new, not-yet-ready-for-prime-time Carbonized build.
   * Bug fixes from the "Fix for 3.4" list.

 Revision 1.6.8.2  2002/06/05 08:50:52  emk
 A small detour - Moved responsibility for script, palette and data directories
 from Config.{h,cpp} to FileSystem.{h,cpp}.

 Revision 1.6.8.1  2002/06/05 07:05:30  emk
 Began isolating the 5L-language-specific code in Win5L:

   * Created a TInterpreter class, which will eventually become the
     interface to all language-related features.
   * Moved ssharp's developer preference support out of Config.{h,cpp}
     (which are tighly tied to the language) and into TDeveloperPrefs.{h,cpp},
     where they will be isolated and easy to port to other platforms.

 Revision 1.6  2002/03/05 10:25:41  tvw
 Added new option to 5L.prefs to optionally allow multiple
 instances of 5L to run.

 Revision 1.5  2002/02/19 12:35:12  tvw
 Bugs #494 and #495 are addressed in this update.

 (1) 5L.prefs configuration file introduced
 (2) 5L_d.exe will no longer be part of CVS codebase, 5L.prefs allows for
     running in different modes.
 (3) Dozens of compile-time switches were removed in favor of
     having a single executable and parameters in the 5L.prefs file.
 (4) CryptStream was updated to support encrypting/decrypting any file.
 (5) Clear file streaming is no longer supported by CryptStream

 For more details, refer to ReleaseNotes.txt

 Revision 1.4  2002/02/05 14:55:41  tvw
 Backed out changes for -D command-line option.
 The DLS client was fixed, eliminating the need for this option.

 Revision 1.3  2002/01/24 19:22:41  tvw
 Fixed bug (#531) in -D command-line option causing
 system registry read error.

 Revision 1.2  2002/01/23 20:39:20  tvw
 A group of changes to support a new stable build.

 (1) Only a single instance of the FiveL executable may run.

 (2) New command-line option "-D" used to lookup the installation directory in the system registry.
     Note: Underscores will be parsed as spaces(" ").
     Ex: FiveL -D HIV_Prevention_Counseling

 (3) Slow down the flash on buttpcx so it can be seen on
     fast machines.  A 200 mS pause was added.

 (4) Several bugfixes to prevent possible crashes when error
     conditions occur.

 Revision 1.1  2001/09/24 15:11:00  tvw
 FiveL v3.00 Build 10

 First commit of /iml/FiveL/Release branch.

 There are now seperate branches for development and release
 codebases.

 Development - /iml/FiveL/Dev
 Release - /iml/FiveL/Release

 Revision 1.7  2000/08/08 19:03:40  chuck
 no message

 Revision 1.6  2000/05/11 12:54:54  chuck
 v 2.01 b2

 Revision 1.5  2000/04/07 17:05:15  chuck
 v 2.01 build 1

 Revision 1.4  2000/02/02 15:15:32  chuck
 no message

 Revision 1.3  1999/09/28 15:13:53  chuck
 Do our own CD-ROM drive detection.

 Revision 1.2  1999/09/24 19:57:18  chuck
 Initial revision

*/
