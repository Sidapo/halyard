// -*- Mode: C++; tab-width: 4; -*-

#if !defined (TMac5LInterpreter_H)
#define TMac5LInterpreter_H

#include "TCommon.h"
#include "TInterpreter.h"
#include "TString.h"
#include "lang/old5l/TIndex.h"

BEGIN_NAMESPACE_FIVEL

//////////
// This class implements the Windows version of the 5L language.
//
class TMac5LInterpreter : public TInterpreter
{
private:
	// Have we been asked to shut down the interpreter?
	bool mKilled;

	// Unload all our cards, macros, headers, etc.
	// TODO - Rename to PurgeScriptData, or something like that.
	void CleanupIndexes();

public:
	TMac5LInterpreter(const TString &inStartScript);
	virtual ~TMac5LInterpreter();

	// These methods are documented in our parent class.
	virtual void Run(SystemIdleProc inIdleProc);
	virtual void KillInterpreter();
	virtual void Pause();
	virtual void WakeUp();
	virtual bool Paused();
	virtual void Timeout(const char *inName, int32 inTime);
	virtual void Nap(int32 inTime);
	virtual bool Napping();
	virtual void KillNap();
	virtual void KillCurrentCard();
	virtual void JumpToCardByName(const char *inName);
	virtual std::string CurCardName();
	virtual std::string PrevCardName();
};

//////////
// When invoked, this callback runs a regular 5L command.  This callback
// remains valid across a ReloadScript (but that doesn't matter much,
// since all the touchzones get cleared).
//
class TMac5LCallback : public TCallback
{
	TString mCommand;

public:
	//////////
	// Create a new callback.
	//
	// [in] inCommand - The command to run.
	//
	TMac5LCallback(const TString &inCommand);

	//////////
	// Run the appropriate commands.
	//
	virtual void Run();
	
	//////////
	// Return the code string associated with this callback.
	//
	virtual std::string PrintableRepresentation();
	
	//////////
	// Create a TCallback object from a 5L command string.
	// 
	// [in] inCmd - A command string of the form "(foo ...)".
	// [out] return - A TCallback object allocated on the heap.
	//                The caller must take delete it when finished.
	//
	static TCallback *MakeCallback(const TString &inCmd);
};

//////////
// A simple TInterpreterManager subclass which makes the old 5L
// interpreter look like other, more typical interpreters.
//
class TMac5LInterpreterManager : public TInterpreterManager
{
	TPrimitiveTlfProcessor mDefStyleProcessor;
	TPrimitiveTlfProcessor mHeaderProcessor;

public:
	//////////
	// Create a new TMac5LInterpreterManager with the specified idle
	// procedure.
	//
	TMac5LInterpreterManager(TInterpreter::SystemIdleProc inIdleProc);

protected:
	// See our parent class for documentation of these methods.
	virtual TInterpreter *MakeInterpreter();
};

END_NAMESPACE_FIVEL

#endif // TMac5LInterpreter_H
