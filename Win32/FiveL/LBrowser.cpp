//////////////////////////////////////////////////////////////////////////////
//
//   (c) Copyright 2000 Trustees of Dartmouth College, All rights reserved.
//        Interactive Media Lab, Dartmouth Medical School
//
//			$Author$
//          $Date$
//          $Revision$
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// LBrowser.cpp : 
//

#include "stdafx.h"
#include "LBrowser.h"
#include "Globals.h"

LBrowser::LBrowser()
{
	m_HaveBrowserPath = false;
	m_HaveNetscape = false;
}

LBrowser::~LBrowser()
{

}

void LBrowser::Init()
{
	TString		keyStr;
	HKEY		regKey = NULL;
	LONG		err;
	int			exePos;
	int			quotePos;
	char		*chPtr;

	// figure out the default browser
	err = RegOpenKeyEx(HKEY_CLASSES_ROOT, ".html", 0, KEY_READ, &regKey);
	if (err != ERROR_SUCCESS)
		goto done;

	// get the value
	if (not GetString(regKey, keyStr))
		goto done;

	// close the key
	RegCloseKey(regKey);
	regKey = NULL;

	// the value is another key we need to open
	keyStr += "\\shell\\open\\command";

	// get the path to the default browser
	err = RegOpenKeyEx(HKEY_CLASSES_ROOT, keyStr.GetString(), 0, KEY_READ,
		&regKey);
	if (err != ERROR_SUCCESS)
		goto done;

	// get the value
	if (not GetString(regKey, keyStr))
		goto done;

	// close the key
	RegCloseKey(regKey);
	regKey = NULL;

	if (keyStr.Find("netscape", 0, false) != -1)
		m_HaveNetscape = true;

	if (m_HaveNetscape)
	{
		// for Netscape strip off any junk on the end, find .exe 
		// and remove everything after that
		exePos = keyStr.Find(".exe", 0, false);
		if (exePos != -1)
			m_BrowserPath = keyStr.Mid(0, exePos + 4);
		else
			m_BrowserPath = keyStr;
	}
	else
	{
		// for IE just strip off double quotes
		m_BrowserPath = keyStr.Mid(1);
		quotePos = m_BrowserPath.Find("\"");
		if (quotePos != -1)
		{
			chPtr = m_BrowserPath.GetBuffer();
			chPtr[quotePos] = ' ';
		}
	}

	m_HaveBrowserPath = true;

	gDebugLog.Log("Default browser path <%s>", m_BrowserPath.GetString());
	
done:		
	if (regKey != NULL)
		RegCloseKey(regKey);
}

bool LBrowser::GoToUrl(TString &inURL)
{
	if (not m_HaveBrowserPath)
		return (false);

	TString		commandLine;

	//commandLine = "\"";
	commandLine = m_BrowserPath;
	//commandLine += "\" ";
	commandLine += " ";
	commandLine += inURL;

	gDebugLog.Log("Browser: command line -> <%s>", commandLine.GetString());

	// launch it
	STARTUPINFO				startInfo;
	PROCESS_INFORMATION		procInfo;
	::ZeroMemory(&startInfo, sizeof (startInfo));
	startInfo.cb = sizeof (startInfo);

	if (not CreateProcess(NULL, (char *) commandLine.GetString(), NULL, NULL, false,
		0, NULL, NULL, &startInfo, &procInfo))
	{
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);
		
		gLog.Log("Browser: ERROR %s", lpMsgBuf);
		gDebugLog.Log("Browser: ERROR <%s>", lpMsgBuf);

		// Free the buffer.
		LocalFree( lpMsgBuf );

		return (false);
	}

	return (true);
}

bool LBrowser::GetString(HKEY inKey, TString &outValueStr)
{
	TString		resultStr;
	DWORD		size = 256;
	LONG		err = ERROR_MORE_DATA;

	while (err == ERROR_MORE_DATA)
	{ 
		size += 64;
		char*	buf = new char[size];
		DWORD	type;
		err = ::RegQueryValueEx(inKey, NULL, NULL, &type, 
			(unsigned char *) buf, &size);
		if ((err != ERROR_MORE_DATA) && (err != ERROR_SUCCESS))
			return (false);
		else if ((type != REG_SZ) && (type != REG_EXPAND_SZ))
			return (false);
		else
		{
			outValueStr = buf;
			return (true);
		}

		delete buf;
	}

	return (false);
}

/*
 $Log$
 Revision 1.1.2.1  2002/03/13 15:06:56  emk
 Merged changed from 3.1.1 -> 3.2.1 into the 3.2.0.1 codebase,
 because we want these in the stable engine.  Highlights:

   1) FiveL.prefs file support.
   2) Removal of -D command line flag.

 Revision 1.2  2002/02/19 12:35:12  tvw
 Bugs #494 and #495 are addressed in this update.

 (1) 5L.prefs configuration file introduced
 (2) 5L_d.exe will no longer be part of CVS codebase, 5L.prefs allows for
     running in different modes.
 (3) Dozens of compile-time switches were removed in favor of
     having a single executable and parameters in the 5L.prefs file.
 (4) CryptStream was updated to support encrypting/decrypting any file.
 (5) Clear file streaming is no longer supported by CryptStream

 For more details, refer to ReleaseNotes.txt

 Revision 1.1  2001/09/24 15:11:01  tvw
 FiveL v3.00 Build 10

 First commit of /iml/FiveL/Release branch.

 There are now seperate branches for development and release
 codebases.

 Development - /iml/FiveL/Dev
 Release - /iml/FiveL/Release

 Revision 1.1  2000/08/09 14:37:06  chuck
 2.01 Build 5

*/
