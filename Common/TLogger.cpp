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
// TLogger.cpp : 
//

#include "THeader.h"
#include "TLogger.h"
#include "TString.h"

#include <time.h>

#define FATAL_HEADER	"Fatal Error: "
#define ERROR_HEADER	"Error: "
#define CAUTION_HEADER	"Caution: "

#define FormatMsg(Format)	\
	va_list	argPtr;			\
	va_start(argPtr, (Format));		\
	wvsprintf(m_LogBuffer, (Format), argPtr);	\
	va_end(argPtr)

TLogger::TLogger()
{ 
	m_LogOpen = false; 
	m_OpenFailed = false;
	m_CautionAlert = false; 
	m_Append = false;
	m_LogMask = LOG_ALL;
}

TLogger::~TLogger()
{
	if (m_LogOpen)
		m_Log.close();
}

//
//	Init - Initialize the log file. 
//
void TLogger::Init(const char *Path, const char *Name, 
				   bool OpenFile /* = true */, bool Append /* = false */)
{
	ASSERT(not m_LogOpen);
	ASSERT(Path != NULL);
	ASSERT(Name != NULL);

	m_Append = Append;

	m_FileName = Path;
	if (not m_FileName.EndsWith('\\'))
		m_FileName += '\\';
	m_FileName += Name;
	m_FileName += ".log";

	if (OpenFile)
	{
		if (m_Append)
			m_Log.open(m_FileName.GetString(), std::ios::out | std::ios::app);
		else
			m_Log.open(m_FileName.GetString(), std::ios::out);

		if (not m_Log.fail())
			m_LogOpen = true;
		else
			m_OpenFailed = true;

		if (m_LogOpen and m_Append)
		{
			// put welcome line
			m_Log << std::endl;
			TimeStamp();
		}
	}
}

//
//	Init - Initialize the log file. Use the current directory.
//
void TLogger::Init(const char *Name, bool OpenFile /* = true */)
{	
	char		pathBuf[MAX_PATH];
	DWORD		retLen = 0;

	retLen = ::GetCurrentDirectory(MAX_PATH, pathBuf);
	if (retLen > 0)
		Init(pathBuf, Name, OpenFile);
}

void TLogger::Log(int32 Mask, const char *Format, ...)
{
	if (!m_LogOpen)
		return;

	if (not ShouldLog(Mask))
		return;

	FormatMsg(Format);
	LogBuffer(NULL);	
}

void TLogger::Log(const char *Format, ...)
{
	if (!m_LogOpen)
		return;

	FormatMsg(Format);
	LogBuffer(NULL);
}

void TLogger::Error(const char *Format, ...)
{
	if (!m_LogOpen)
		return;

	FormatMsg(Format);
	LogBuffer(ERROR_HEADER);
	AlertBuffer(true);
}

void TLogger::Caution(const char *Format, ...)
{
	if (!m_LogOpen)
		return;

	FormatMsg(Format);
	LogBuffer(CAUTION_HEADER);
	if (m_CautionAlert)
		AlertBuffer(false);
}

void TLogger::FatalError(const char *Format, ...)
{
	if (!m_LogOpen)
		return;

	FormatMsg(Format);
	LogBuffer(FATAL_HEADER);
	AlertBuffer(true);
//	ShutDown(true);
}

//
//	CheckLog - Check that the log is open, if it isn't and 
//		we haven't failed at opening it once then try to open
//		it again.
//
/*
bool TLogger::CheckLog()
{
	if (m_LogOpen)
		return (true);

	if (m_OpenFailed)
		return (false);

	if (m_FileName.IsEmpty())
		return (false);

	// else try and open it
	if (m_Append)
		m_Log.open(m_FileName.GetString(), std::ios::out | std::ios::app);
	else
		m_Log.open(m_FileName.GetString(), std::ios::out);
	if (m_Log.fail())
		m_OpenFailed = true;
	else
		m_LogOpen = true;

	if (m_LogOpen and m_Append)
	{
		// put welcome line
		m_Log << std::endl;
		TimeStamp();
	}

	return (m_LogOpen);
}
*/

//
//	LogBuffer - 
//
void TLogger::LogBuffer(const char *Header)
{
	if (m_LogOpen)
	{
		if (Header != NULL)
			m_Log << Header;

		m_Log << m_LogBuffer << std::endl;
	}
}

//
//	AlertBuffer -
//
void TLogger::AlertBuffer(bool isError /* = false */)
{
	uint32		alertType;

	alertType = MB_SYSTEMMODAL | MB_OK;
	if (isError)
		alertType |= MB_ICONSTOP;
	else
		alertType |= MB_ICONINFORMATION;

	::MessageBox(::GetFocus(), m_LogBuffer, NULL, alertType);
}

//
//	TimeStamp - Put a time stamp in the log
//
void TLogger::TimeStamp(void)
{
	time_t	timeNow;
	char	*timeStrPtr;

	time(&timeNow);
	timeStrPtr = ctime(&timeNow);
	m_Log << timeStrPtr << std::endl;
}


/*
 $Log$
 Revision 1.3  2002/02/19 12:35:11  tvw
 Bugs #494 and #495 are addressed in this update.

 (1) 5L.prefs configuration file introduced
 (2) 5L_d.exe will no longer be part of CVS codebase, 5L.prefs allows for
     running in different modes.
 (3) Dozens of compile-time switches were removed in favor of
     having a single executable and parameters in the 5L.prefs file.
 (4) CryptStream was updated to support encrypting/decrypting any file.
 (5) Clear file streaming is no longer supported by CryptStream

 For more details, refer to ReleaseNotes.txt

 Revision 1.2  2002/01/23 20:39:19  tvw
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

 Revision 1.1  2000/04/06 17:06:10  chuck
 Initial check-in

 Revision 1.2  1999/09/24 19:57:19  chuck
 Initial revision

*/
