// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-
// @BEGIN_LICENSE
//
// Tamale - Multimedia authoring and playback system
// Copyright 1993-2004 Trustees of Dartmouth College
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// @END_LICENSE

#if !defined (_TLogger_h_)
#define _TLogger_h_

#include <fstream>

#include "FileSystem.h"

BEGIN_NAMESPACE_FIVEL

#define LOG_NONE		0x00000000
#define LOG_ALL			0xFFFFFFFF

#define LOG_BUFFER_SIZE	10240

//////////
/// A class for logging output to a file.
///
/// \author Chuck Officer
/// \author Sean Sharp
/// \author ...and others
///
class TLogger
{
public:
	//////////
	// Constructor.
	//
	TLogger();
	
	//////////
	// Constructor.
	//
	~TLogger();

	//////////
	// Initialize the log file.  Assumes path is current directory.
	//
	// [in] Name - name of the log file (no file extension)
	// [in_optional] OpenFile - if true opens the file on init (default true)
	// [in_optional] Append - if true, opens file for append only
    //                        (defualt false)
	//
	void	Init(const char *Name, bool OpenFile = true, bool Append = false);

	//////////
	// Initialize the log file.
	//
	// [in] inLogFile - location where the log file should be written
	// [in_optional] OpenFile - if true, opens the file on init (default true)
	// [in_optional] Append - if true, opens file for append only
    //                        (defualt false)
	//
	void	Init(const FileSystem::Path &inLogFile, bool OpenFile = true,
				 bool Append = false);

    //////////
	// Log a general message.
	//
	// [in] Mask - a mask to check against the log mask before logging
	// [in] Format - a printf format string (e.g. "Count is %d.", count)
	//
	void	Log(int32 Mask, const char *Format, ...);
	
	//////////
	// Log a general message.
	//
	// [in] Format - a printf format string (e.g. "Count is %d.", count)
	//
	void	Log(const char *Format, ...);	
	
	//////////
	// Log an error message.  Prepends ERROR_HEADER.
	//
	// [in] Format - a printf format string (e.g. "Count is %d.", count)
	//
	void	Error(const char *Format, ...);
	
	//////////
	// Log a caution message.  Prepends CAUTION_HEADER.
	//
	// [in] Format - a printf format string (e.g. "Count is %d.", count)
	//
	void	Caution(const char *Format, ...);
	
	//////////
	// Log a fatal error message.  Prepends FATAL_HEADER and calls Shutdown().
	//
	// [in] Format - a printf format string (e.g. "Count is %d.", count)
	//
	void	FatalError(const char *Format, ...);
	
	//////////
	// Put a time stamp in the log.
	//
	void	TimeStamp(void);

	//////////
	// Turn all logging off.
	//
	inline void LogNothing(void) { m_LogMask = LOG_NONE; }
	
	//////////
	// Logical "or" the log mask with the given mask.
	//
	// [in] Mask - the mask
	//
	inline void AddLogMask(int32 Mask) { m_LogMask |= Mask; }
	
	//////////
	// Turn all loggin on.
	//
	inline void LogEverything(void) { m_LogMask = LOG_ALL; }

private:
	//////////
	// File output stream
	//
	std::ofstream	m_Log;
	
	//////////
	// Filename for log output
	//
	std::string m_FileName;
	
	//////////
	// Log buffer
	//
	char		m_LogBuffer[LOG_BUFFER_SIZE];
	
	//////////
	// Has the log file been opened for writing?
	//
	bool		m_LogOpen;
	
	//////////
	// Was there an error when opening the log file for writing?
	//
	bool		m_OpenFailed;
	
	//////////
	// If true, an alert message box is displayed for caution messages in 
	// addition to logging the message.
	//
	bool		m_CautionAlert;
	
	//////////
	// Is the log output in append mode?
	//
	bool		m_Append;
	
	//////////
	// Logger mask
	//
	uint32		m_LogMask;

	//////////
	// Either NULL, or a function which can be used to unfade the screen,
	// etc., before displaying errors.
	//
	static void (*s_ErrorPrepFunction)();

#ifdef FIVEL_PLATFORM_MACINTOSH
	//////////
	// True if and only if the Macintosh Toolbox has been
	// properly initialized.
	//
	static bool	s_ToolboxIsInitialized;
#endif

	// Deprecated
	//
	//bool		CheckLog();
	
	//////////
	// Write the contents of m_LogBuffer to the log file.
	//
	// [in] Header - a header to precede the log buffer contents
	//
	void		LogBuffer(const char *Header);
	
	//////////
	// Display an alert message box with the contents of m_LogBuffer.
	//
	// [in] inError - Is it an error message?
	//
	void		AlertBuffer(bool isError = false);

	//////////
	// Should a log message with a given mask be logged?
	//
	// [in] Mask - a mask to check against
	// [out] return - true if the message should be logged, false otherwise
	//
	inline bool	ShouldLog(int32 Mask) 
	{ 
		if (m_LogOpen)
			if ((m_LogMask & Mask) != 0)
				return (true);
		return (false);
	}

public:
	//////////
	// Open up all the log files which will be required by our program.
	//
	// [in] inOpenDebugLog - Should we open up the debugging log as well?
	//
	static void OpenStandardLogs(bool inOpenDebugLog = false);

	//////////
    // We may need to unfade the screen or perform other cleanup
    // before displaying an error.  This method will call any
    // function registered with RegisterErrorPrepFunction, below,
    // or simply do nothing if no such function has been registered.
    //
    static void PrepareToDisplayError();

	//////////
	// Install a function to be called before displaying dialog
	// boxes, triggering assertions, etc.  This function can
	// be called using PrepareToDisplayError, above.
	//
	static void RegisterErrorPrepFunction(void (*inFunc)());

#ifdef FIVEL_PLATFORM_MACINTOSH
	//////////
	// Tell the logging subsystem that the toolbox has been initialized, and
	// that it's OK to use dialogs.
	//
	static void MarkToolboxAsInitialized() { s_ToolboxIsInitialized = true; }
#endif
};

//////////
// This log is used to log ordinary, relatively important events.  This
// file typically exists on a normal user's system.
//
extern TLogger gLog;

//////////
// This log is used to log low-level debugging events.  This file typically
// exists on a developer's system.
//
extern TLogger gDebugLog;

//////////
// This log is used to log missing media items.  This file typically exists
// on a normal user's system *if* some media is unavailable.
//
extern TLogger gMissingMediaLog;

END_NAMESPACE_FIVEL

#endif // _TLogger_h_
