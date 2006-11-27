// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-
// @BEGIN_LICENSE
//
// Tamale - Multimedia authoring and playback system
// Copyright 1993-2006 Trustees of Dartmouth College
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

#include "TStyleSheet.h"
#include "TStateDB.h"
#include "TStateListenerManager.h"

USING_NAMESPACE_FIVEL


//=========================================================================
//  RegisterCommonPrimitives
//=========================================================================
//  Install our portable primitive functions.

void FIVEL_NS RegisterCommonPrimitives()
{
	REGISTER_5L_PRIMITIVE(HavePrimitive);
	REGISTER_5L_PRIMITIVE(Log);
	REGISTER_5L_PRIMITIVE(PolygonContains);
	REGISTER_5L_PRIMITIVE(SetTyped);
	REGISTER_5L_PRIMITIVE(Get);
	REGISTER_5L_PRIMITIVE(VariableInitialized);
	REGISTER_5L_PRIMITIVE(DefStyle);
	REGISTER_5L_PRIMITIVE(MeasureTextAA);
    REGISTER_5L_PRIMITIVE(NotifyFileLoaded);
    REGISTER_5L_PRIMITIVE(NotifyScriptLoaded);
    REGISTER_5L_PRIMITIVE(StateDbSet);
    REGISTER_5L_PRIMITIVE(StateDbGet);
    REGISTER_5L_PRIMITIVE(StateDbRegisterListener);
    REGISTER_5L_PRIMITIVE(StateDbUnregisterListeners);
}


//=========================================================================
//  Support Methods
//=========================================================================

void FIVEL_NS UpdateSpecialVariablesForGraphic(const TRect &bounds)
{
	TPoint p(bounds.Right(), bounds.Bottom());
	gVariableManager.Set("_Graphic_X", (int32) p.X());
	gVariableManager.Set("_Graphic_Y", (int32) p.Y());
}

void FIVEL_NS UpdateSpecialVariablesForText(const TPoint &bottomLeft)
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
// Check to see whether the specified 5L primitive exists.  Helpful in
// writing code which runs under different versions of the engine.

DEFINE_5L_PRIMITIVE(HavePrimitive)
{
	std::string name;
	inArgs >> SymbolName(name);
	::SkipPrimitiveLogging();
	::SetPrimitiveResult(gPrimitiveManager.DoesPrimitiveExist(name));
}


//-------------------------------------------------------------------------
// (Log file:STRING msg:STRING [level:STRING = "log"])
//-------------------------------------------------------------------------
// Logs the second argument to the file specified by the first.
// Available logs: debug, 5L.  Available log levels:
// fatalerror, error, caution, log.

DEFINE_5L_PRIMITIVE(Log)
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
	if (log_name == "5l")
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

DEFINE_5L_PRIMITIVE(PolygonContains)
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


DEFINE_5L_PRIMITIVE(SetTyped)
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

DEFINE_5L_PRIMITIVE(Get)
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

DEFINE_5L_PRIMITIVE(VariableInitialized)
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

DEFINE_5L_PRIMITIVE(DefStyle)
{
	gStyleSheetManager.AddStyleSheet(inArgs);
}


//-------------------------------------------------------------------------
// (MeasureTextAA STYLE TEXT MAX_WIDTH)
//-------------------------------------------------------------------------
// Calculate the width and height required to draw TEXT using STYLE,
// assuming a maximum width of MAX_WIDTH pixels.
//
// This updates all the special variables associated with text.

DEFINE_5L_PRIMITIVE(MeasureTextAA)
{
	std::string style;
	std::string text;
	uint32 max_width;

	inArgs >> SymbolName(style) >> text >> max_width;
	TRect bounds = gStyleSheetManager.Draw(style, text,
                                           GraphicsTools::Point(0, 0),
                                           max_width, NULL);
    
    ::SetPrimitiveResult(bounds);
}

DEFINE_5L_PRIMITIVE(NotifyFileLoaded) {
    TInterpreter::GetInstance()->NotifyFileLoaded();
}

DEFINE_5L_PRIMITIVE(NotifyScriptLoaded) {
    TInterpreter::GetInstance()->NotifyScriptLoaded();    
}

DEFINE_5L_PRIMITIVE(StateDbSet) {
	std::string key;
	TValue val;
	inArgs >> SymbolName(key) >> val;
	gStateDB.Set(key, val);
}

DEFINE_5L_PRIMITIVE(StateDbGet) {
	std::string listener_name, key;
    int32 serial_number;
	inArgs >> SymbolName(listener_name) >> serial_number >> SymbolName(key);
    shared_ptr<TStateListener> listener =
        gStateListenerManager.FindListener(listener_name, serial_number);
    ::SetPrimitiveResult(gStateDB.Get(listener.get(), key));
}

DEFINE_5L_PRIMITIVE(StateDbRegisterListener) {
    std::string name;
    TCallbackPtr listener;
    inArgs >> SymbolName(name) >> listener;
    gStateListenerManager.RegisterListener(name, listener);
}

DEFINE_5L_PRIMITIVE(StateDbUnregisterListeners) {
    std::string name;
    inArgs >> SymbolName(name);
    gStateListenerManager.UnregisterListeners(name);
}
