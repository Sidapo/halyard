// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-
// @BEGIN_LICENSE
//
// Halyard - Multimedia authoring and playback system
// Copyright 1993-2008 Trustees of Dartmouth College
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

// Needed for RegisterCommonPrimitives.
#include "CommonHeaders.h"
#include "TPrimitives.h"
#include "TCommonPrimitives.h"

// Needed to implement primitives.
#include "sha1.h"
#include "TStyleSheet.h"
#include "TStateDB.h"
#include "TStateListenerManager.h"

using namespace Halyard;


//=========================================================================
//  Mapping bytes to hex strings
//=========================================================================
//  Let's just encode this as a table of strings, and avoid messing with
//  snprintf, _snprintf, and all the portability headaches it creates.

static const char *byte_to_hex_string[256] = {
    "00", "01", "02", "03", "04", "05", "06", "07",
    "08", "09", "0a", "0b", "0c", "0d", "0e", "0f",
    "10", "11", "12", "13", "14", "15", "16", "17",
    "18", "19", "1a", "1b", "1c", "1d", "1e", "1f",
    "20", "21", "22", "23", "24", "25", "26", "27",
    "28", "29", "2a", "2b", "2c", "2d", "2e", "2f",
    "30", "31", "32", "33", "34", "35", "36", "37",
    "38", "39", "3a", "3b", "3c", "3d", "3e", "3f",
    "40", "41", "42", "43", "44", "45", "46", "47",
    "48", "49", "4a", "4b", "4c", "4d", "4e", "4f",
    "50", "51", "52", "53", "54", "55", "56", "57",
    "58", "59", "5a", "5b", "5c", "5d", "5e", "5f",
    "60", "61", "62", "63", "64", "65", "66", "67",
    "68", "69", "6a", "6b", "6c", "6d", "6e", "6f",
    "70", "71", "72", "73", "74", "75", "76", "77",
    "78", "79", "7a", "7b", "7c", "7d", "7e", "7f",
    "80", "81", "82", "83", "84", "85", "86", "87",
    "88", "89", "8a", "8b", "8c", "8d", "8e", "8f",
    "90", "91", "92", "93", "94", "95", "96", "97",
    "98", "99", "9a", "9b", "9c", "9d", "9e", "9f",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "a8", "a9", "aa", "ab", "ac", "ad", "ae", "af",
    "b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7",
    "b8", "b9", "ba", "bb", "bc", "bd", "be", "bf",
    "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7",
    "c8", "c9", "ca", "cb", "cc", "cd", "ce", "cf",
    "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
    "d8", "d9", "da", "db", "dc", "dd", "de", "df",
    "e0", "e1", "e2", "e3", "e4", "e5", "e6", "e7",
    "e8", "e9", "ea", "eb", "ec", "ed", "ee", "ef",
    "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",
    "f8", "f9", "fa", "fb", "fc", "fd", "fe", "ff"
};


//=========================================================================
//  Support Methods
//=========================================================================

void Halyard::UpdateSpecialVariablesForGraphic(const TRect &bounds)
{
	TPoint p(bounds.Right(), bounds.Bottom());
	gVariableManager.Set("_Graphic_X", (int32) p.X());
	gVariableManager.Set("_Graphic_Y", (int32) p.Y());
}

void Halyard::UpdateSpecialVariablesForText(const TPoint &bottomLeft)
{
	TPoint p = bottomLeft;
	gVariableManager.Set("_INCR_X", (int32) p.X());
	gVariableManager.Set("_INCR_Y", (int32) p.Y());
}


//=========================================================================
//  Implementation of Common Primitives
//=========================================================================


//-------------------------------------------------------------------------
// (HavePrimitive name:STRING)
//-------------------------------------------------------------------------
// Check to see whether the specified primitive exists.  Helpful in
// writing code which runs under different versions of the engine.

DEFINE_PRIMITIVE(HavePrimitive)
{
	std::string name;
	inArgs >> SymbolName(name);
	::SkipPrimitiveLogging();
	::SetPrimitiveResult(gPrimitiveManager.DoesPrimitiveExist(name));
}


//-------------------------------------------------------------------------
// (RunInitialCommands)
//-------------------------------------------------------------------------
// Run the initial commands that we need to run once script startup is
// finished.

DEFINE_PRIMITIVE(RunInitialCommands)
{
	TInterpreterManager *manager = TInterpreterManager::GetInstance();
    manager->RunInitialCommands();
}


//-------------------------------------------------------------------------
// (Idle blocking)
//-------------------------------------------------------------------------
// Performs either a blocking or non-blocking idle.

DEFINE_PRIMITIVE(Idle)
{
	// Recover our TInterpreterManager.
	TInterpreterManager *manager = TInterpreterManager::GetInstance();
	ASSERT(manager);

	// Should our idle loop block until any events are received from 
	// the user?
	bool block;
	inArgs >> block;

	// Call our stored idle procedure and let the GUI run for a bit.
	manager->DoIdle(block);

	// Logging this primitive would only clutter the debug log.  We need
	// to do this *after* calling DoIdle, so that it doesn't get
	// confused with any internal primitive calls.
	::SkipPrimitiveLogging();
}


//-------------------------------------------------------------------------
// (Log file:STRING msg:STRING [level:STRING = "log"])
//-------------------------------------------------------------------------
// Logs the second argument to the file specified by the first.
// Available logs: debug, halyard.  Available log levels:
// fatalerror, error, caution, log.

DEFINE_PRIMITIVE(Log)
{
	// Logging this primitive call would be redundant and ugly.
	::SkipPrimitiveLogging();

	std::string log_name, msg, level;
	level = "log";
	inArgs >> SymbolName(log_name) >> msg;
	if (inArgs.HasMoreArguments())
		inArgs >> SymbolName(level);
	log_name = ::MakeStringLowercase(log_name);
	level = ::MakeStringLowercase(level);

	// Figure out which log file to use.
	TLogger *log = &gLog;
	if (log_name == "halyard")
		log = &gLog;
	else if (log_name == "debug")
		log = &gDebugLog;
	else
		gLog.Caution("No such log file: %s", log_name.c_str());

	// Report the problem using the appropriate log level.
	if (level == "log")
		log->Log("%s", msg.c_str());
	else if (level == "caution")
		log->Caution("%s", msg.c_str());
	else if (level == "error")
		log->Error("%s", msg.c_str());
	else if (level == "fatalerror")
		log->FatalError("%s", msg.c_str());
	else if (level == "environmenterror")
		log->EnvironmentError("%s", msg.c_str());
	else
	{
		gLog.Error("Unknown logging level: %s", level.c_str());
		gLog.FatalError("%s", msg.c_str());
	}
}


//-------------------------------------------------------------------------
// (PolygonContains poly pt)
//-------------------------------------------------------------------------
// Determines if pt lies within poly

DEFINE_PRIMITIVE(PolygonContains)
{
	::SkipPrimitiveLogging();

	TPolygon poly;
	TPoint pt;
	inArgs >> poly >> pt;
	::SetPrimitiveResult(poly.Contains(pt));
}

//-------------------------------------------------------------------------
// (SetTyped VARIABLE TYPE [NEWVALUE])
//-------------------------------------------------------------------------
// Set the value of VARIABLE to NEWVALUE, using the specified TYPE.  If
// TYPE is "null", then NEWVALUE must be omitted.


DEFINE_PRIMITIVE(SetTyped)
{
	::SkipPrimitiveLogging();

	TValue val;
	std::string vname;
	inArgs >> SymbolName(vname) >> val;

	gVariableManager.Set(vname, val);
}


//-------------------------------------------------------------------------
// (Get VARIABLE)
//-------------------------------------------------------------------------
// Returns the value stored in the variable, preserving type information.

DEFINE_PRIMITIVE(Get)
{
	::SkipPrimitiveLogging();

	std::string vname;
	inArgs >> SymbolName(vname);

   	::SetPrimitiveResult(gVariableManager.Get(vname.c_str()));
}


//-------------------------------------------------------------------------
// (VariableInitialized NAME)
//-------------------------------------------------------------------------
// Determine whether a variable has been initialized.

DEFINE_PRIMITIVE(VariableInitialized)
{
	::SkipPrimitiveLogging();

	std::string vname;
	inArgs >> SymbolName(vname);

	::SetPrimitiveResult(gVariableManager.VariableExists(vname.c_str()));
}


//-------------------------------------------------------------------------
// (DefStyle NAME ...)
//-------------------------------------------------------------------------
// Create a stylesheet with the given name.

DEFINE_PRIMITIVE(DefStyle)
{
	gStyleSheetManager.AddStyleSheet(inArgs);
}


//-------------------------------------------------------------------------
// (IsLazyLoadingEnabled)
//-------------------------------------------------------------------------
// Is lazy loading turned on in the engine?

DEFINE_PRIMITIVE(IsLazyLoadingEnabled) {
    bool enabled(TInterpreterManager::GetInstance()->IsLazyLoadingEnabled());
    ::SetPrimitiveResult(enabled);
}


//-------------------------------------------------------------------------
// (MaybeSetIsLazyLoadingEnabled)
//-------------------------------------------------------------------------
// Attempt to turn on lazy loading in the engine.

DEFINE_PRIMITIVE(MaybeSetIsLazyLoadingEnabled) {
    bool enable;
    inArgs >> enable;
    TInterpreterManager::GetInstance()->MaybeSetIsLazyLoadingEnabled(enable);
}


//-------------------------------------------------------------------------
// (MeasureTextAA STYLE TEXT MAX_WIDTH)
//-------------------------------------------------------------------------
// Calculate the width and height required to draw TEXT using STYLE,
// assuming a maximum width of MAX_WIDTH pixels.
//
// This updates all the special variables associated with text.

DEFINE_PRIMITIVE(MeasureTextAA)
{
	std::string style;
	std::string text;
	uint32 max_width;

	inArgs >> SymbolName(style) >> text >> max_width;
	TRect bounds = gStyleSheetManager.Draw(style, text,
                                           GraphicsTools::Point(0, 0),
                                           max_width, NULL);
    ASSERT(bounds.Top() == 0);
    ASSERT(bounds.Left() >= 0);
    
    // Relocate bounds to 0,0, so that we do the right thing when measuring
    // centered text--namely, measure the just the needed width.
    ::SetPrimitiveResult(TRect(0, 0, bounds.Width(), bounds.Height()));
}

DEFINE_PRIMITIVE(NotifyFileLoaded) {
    TInterpreter::GetInstance()->NotifyFileLoaded();
}

DEFINE_PRIMITIVE(NotifyScriptLoaded) {
    TInterpreter::GetInstance()->NotifyScriptLoaded();    
}

DEFINE_PRIMITIVE(Sha1File) {
    std::string path;
    inArgs >> path;
    
    unsigned char digest[20];
    sha1_file(const_cast<char*>(path.c_str()), digest);

    std::string result;
    for (size_t i = 0; i < sizeof(digest); ++i)
        result += byte_to_hex_string[digest[i]];

    ::SetPrimitiveResult(result);
}

DEFINE_PRIMITIVE(StateDbSet) {
	std::string key;
	TValue val;
	inArgs >> SymbolName(key) >> val;
	gStateDB.Set(key, val);
}

DEFINE_PRIMITIVE(StateDbGet) {
	std::string listener_name, key;
    int32 serial_number;
	inArgs >> SymbolName(listener_name) >> serial_number >> SymbolName(key);
    shared_ptr<TStateListener> listener =
        gStateListenerManager.FindListener(listener_name, serial_number);
    ::SetPrimitiveResult(gStateDB.Get(listener.get(), key));
}

DEFINE_PRIMITIVE(StateDbRegisterListener) {
    std::string name;
    TCallbackPtr listener;
    inArgs >> SymbolName(name) >> listener;
    gStateListenerManager.RegisterListener(name, listener);
}

DEFINE_PRIMITIVE(StateDbUnregisterListeners) {
    std::string name;
    inArgs >> SymbolName(name);
    gStateListenerManager.UnregisterListeners(name);
}


//=========================================================================
//  RegisterCommonPrimitives
//=========================================================================
//  Install our portable primitive functions.

void Halyard::RegisterCommonPrimitives()
{
	REGISTER_PRIMITIVE(HavePrimitive);
    REGISTER_PRIMITIVE(RunInitialCommands);
    REGISTER_PRIMITIVE(Idle);
	REGISTER_PRIMITIVE(Log);
	REGISTER_PRIMITIVE(PolygonContains);
	REGISTER_PRIMITIVE(SetTyped);
	REGISTER_PRIMITIVE(Get);
	REGISTER_PRIMITIVE(VariableInitialized);
	REGISTER_PRIMITIVE(DefStyle);
    REGISTER_PRIMITIVE(IsLazyLoadingEnabled);
    REGISTER_PRIMITIVE(MaybeSetIsLazyLoadingEnabled);
	REGISTER_PRIMITIVE(MeasureTextAA);
    REGISTER_PRIMITIVE(NotifyFileLoaded);
    REGISTER_PRIMITIVE(NotifyScriptLoaded);
    REGISTER_PRIMITIVE(Sha1File);
    REGISTER_PRIMITIVE(StateDbSet);
    REGISTER_PRIMITIVE(StateDbGet);
    REGISTER_PRIMITIVE(StateDbRegisterListener);
    REGISTER_PRIMITIVE(StateDbUnregisterListeners);
}
