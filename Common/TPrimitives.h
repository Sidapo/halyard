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

#ifndef TPrimitives_H
#define TPrimitives_H

#include "TInterpreter.h"

BEGIN_NAMESPACE_FIVEL

// These are the primitive types which can currently be passed as
// arguments to a 5L command.
//
// String
//   std::string
//
// Integer
//   int16
//   int32
//   uint32
//   double
// 
// Structures
//   Point
//   Rectangle
//   Color
//
// Manipulators
//   open - scan forward for an open parentheses
//   close - scan forward for a close parentheses, skipping nested pairs
//   discard - discard the next token
//   ValueOrPercent - Read in a value, or a percentage of another value
//
// Implicit
//   thunk - a zero-argument callback

class SymbolName;
class ValueOrPercent;


//////////
/// TArgumentList provides an abstract interface to the argument lists
/// passed to a 5L primitive function.  To allow a new TInterpreter
/// class to call 5L primitives, you'll need to implement all this
/// class's virtual methods.
///
/// You have a lot of flexibility in how you implement the Get* methods.
/// For example, these methods might automatically coerce arguments to
/// the correct data type on the fly, if that's appropriate for the
/// scripting language in question.
///
class TArgumentList
{
	//////////
	/// The arguments passed to a primitive will be stored as TValues
	/// in a TValueList.
    ///
	TValueList mArgList;

	//////////
	/// A pointer to the current TValue object in mArgList
    ///
	TValueList::iterator mArgPtr;

	friend TArgumentList &operator>>(TArgumentList &inArgs,
									 const SymbolName &inVoP);
	friend TArgumentList &operator>>(TArgumentList &inArgs,
									 const ValueOrPercent &inVoP);

	//////////
	/// Keeps track of function name and evaluated parameters for Debug.log
	///
	std::string mDebugString;

	//////////
	/// Log the value of a parameter for future retrieval by EndLog.
	/// Note that this function will get called anyway, even if nobody
	/// calls BeginLog or EndLog.
	///
	void LogParameter(const std::string &inParameterValue);

	void LogTValueParameter(const TValue &inParameterValue);

protected:
	//////////
	/// Fetch the next argument.
	///
	virtual TValue GetNextArg() {
		if(!HasMoreArguments())
			THROW("Not enough arguments");
		return *mArgPtr++;
	}

    //////////
	/// Return the next argument as a string.
	///
	virtual std::string GetStringArg();

	//////////
	/// Return the next argument as a symbol.  Symbols are basically the
	/// same as strings, but they're typically used to name options in APIs,
	/// and some languages (such as Scheme) want to make a distinction.  If
	/// your language doesn't support symbols, make strings and symbols
	/// equivalent.
	///
	virtual std::string GetSymbolArg();

	//////////
	/// Return the next argument as a singed, 32-bit integer.
	///
	virtual int32 GetInt32Arg();

	//////////
	/// Return the next argument as an unsinged, 32-bit integer.
	///
	virtual uint32 GetUInt32Arg();

	//////////
	/// Return the next argument as a boolean value.
	///
	virtual bool GetBoolArg();

	//////////
	/// Return the next argument as a double.
	///
	virtual double GetDoubleArg();

	//////////
	/// Return the next argument as a point.
	///
	virtual TPoint GetPointArg();

	//////////
	/// Return the next argument as a rectangle.
	///
	virtual TRect GetRectArg();

	//////////
	/// Return the next argument as a polygon.
	///
	virtual TPolygon GetPolygonArg();

	//////////
	/// Return the next argument as a color.
	///
	virtual GraphicsTools::Color GetColorArg();

	//////////
	/// Return the next argument as either a value or a percentage.
	///
	virtual void GetValueOrPercentArg(bool &outIsPercent,
									  int32 &outValue);

	//////////
	/// Return the next argument as a callback.  This object
	/// is allocated on the heap, and must be destroyed by the
	/// caller (typically the primitive function) using delete.
	///
	virtual TCallbackPtr GetCallbackArg();

	//////////
	/// Return the next argument as a list.  This object
	/// is allocated on the heap, and must be destroyed by the
	/// caller (typically the primitive function) using delete.
	///
	virtual TArgumentList *GetListArg();

public:
	TArgumentList() {}
	TArgumentList(TValueList inVal);
	virtual ~TArgumentList() {}
		
	//////////
	/// Are there any more arguments left?
	///	
	virtual bool HasMoreArguments() { 
		return (mArgPtr != mArgList.end());
	}

	// These functions provide handy wrapper functions
	// for the protected Get* functions above.
	friend TArgumentList &operator>>(TArgumentList &args, std::string &out);
	friend TArgumentList &operator>>(TArgumentList &args, int16 &out);
	friend TArgumentList &operator>>(TArgumentList &args, int32 &out);
	friend TArgumentList &operator>>(TArgumentList &args, bool &out);
	friend TArgumentList &operator>>(TArgumentList &args, uint32 &out);
	friend TArgumentList &operator>>(TArgumentList &args, double &out);
	friend TArgumentList &operator>>(TArgumentList &args, TRect &out);
	friend TArgumentList &operator>>(TArgumentList &args, TPolygon &out);
	friend TArgumentList &operator>>(TArgumentList &args, TPoint &out);
	friend TArgumentList &operator>>(TArgumentList &args,
									 GraphicsTools::Color &out);
	friend TArgumentList &operator>>(TArgumentList &args, TCallbackPtr &out);
	friend TArgumentList &operator>>(TArgumentList &args, TArgumentList* &out);
	friend TArgumentList &operator>>(TArgumentList &args, TValue &out);

	//////////
	/// Ask the TArgumentList list to start logging all the parameters
	/// extracted from it.  You can retrieve this log data from EndLog.
	///
	/// \param inFunctionName  The name of the functions to which this
	///                       TArgumentList corresponds.
	///
	void BeginLog(const std::string &inFunctionName);

	//////////
	/// Stop logging parameters extracted from this TArgumentList, and
	/// return them (together with the function name), as though they
	/// were a Scheme function call.
	/// 
	/// \return  The complete entry for Debug.log
	///
	std::string EndLog();
};

//////////
/// An input manipulator which reads a symbol from the input stream and
/// stores it in the specified std::string object.
///
class SymbolName
{
	std::string &mName;

public:
	SymbolName(std::string &outName) : mName(outName) { }

	friend TArgumentList &operator>>(TArgumentList &inArgs,
									 const SymbolName &inSymbolName);
};

//////////
/// An input manipulator which parses either (1) percentage values
/// relative to some base or (2) absolute values of the form "4".
/// Call it as:
///   int result;
///   stream >> ValueOrPercent(10, &result);
/// When passed 20%, this will return 2.  When passed "4", this
/// will return "4".
///
class ValueOrPercent
{
	int32 mBaseValue;
	int32 *mOutputValue;

public:
	ValueOrPercent(int32 baseValue, int32 *outputValue)
		: mBaseValue(baseValue), mOutputValue(outputValue) {}

	friend TArgumentList &operator>>(TArgumentList &inArgs,
									 const ValueOrPercent &inVoP);
};

//////////
/// The TPrimitiveManager class maintains a set of primitive engine
/// functions.  These functions can be called from our scripting
/// language.
///
class TPrimitiveManager
{
public:
	//////////
	/// A PrimitiveFunc implements a single primitive.
	///
	/// \param inArgs  The arguments to the primitive.
	///
	typedef void (*PrimitiveFunc)(TArgumentList &inArgs);

private:
	//////////
	/// The big table of all our primitive functions.
	///
	std::map<std::string,PrimitiveFunc> mPrimitiveMap;

public:
	//////////
	/// Register a primitive with the primitive manager.
	///
	/// \param inName  The name of the primitive, in lowercase.
	/// \param inFunc  The function which implements this primitive.
	///
	void RegisterPrimitive(const std::string &inName, PrimitiveFunc inFunc);

	//////////
	/// Does a primitive with the given name exist?
	///
	/// \param inName  The name of the primitive, in lowercase.
	/// \return  Whether the given primitive exists.
	///
	bool DoesPrimitiveExist(const std::string &inName);

	//////////
	/// Call the specified primitive.  This function throws
	/// all sorts of exciting exceptions, so you should probably
	/// wrap it in a try/catch block and deal with any problems
	/// that arise.
	///
	/// \param inName  The name of the primitive, in lowercase.
	/// \param inArgs  The arguments to the primitive.
	///
	void CallPrimitive(const std::string &inName, TArgumentList &inArgs);
};

//////////
/// The global object in charge of managing our primitive functions.
///
extern TPrimitiveManager gPrimitiveManager;

//////////
/// A handy macro for declaring a 5L primitive function and registering
/// it with the gPrimitiveManager, all in one fell swoop.  There are
/// several bits of pre-processor wizardry going on here:
///
///   1) The 'do { ... } while (0)' makes C++ treat the multiple
///      statements in the body as a single statement.
///   2) The '#NAME' construct converts the argument token to
///      to a string literal.
///   3) The '##' construct glues two adjacent tokens together.
///
/// Call it as follows:
///
///   REGISTER_5L_PRIMITIVE_WITH_NAME("log", LogMessage); // register "log"
///   REGISTER_5L_PRIMITIVE(LogMessage); // register "logmessage"
///
#define REGISTER_5L_PRIMITIVE_WITH_NAME(NAME, TOKEN) \
	do { \
		extern void DoPrim_ ## TOKEN(TArgumentList &inArgs); \
		gPrimitiveManager.RegisterPrimitive(NAME, &DoPrim_ ## TOKEN); \
	} while (0)

#define REGISTER_5L_PRIMITIVE(NAME) \
	REGISTER_5L_PRIMITIVE_WITH_NAME(MakeStringLowercase(#NAME), NAME)

//////////
/// Use this macro in place of a function prototype when implementing a
/// 5L primitive.  This will shield you from changes to the standard
/// prototype.  (For an explanation of the macro grik, see
/// REGISTER_5L_PRIMITIVE.)
///
/// Use it as follows:
///
///   DEFINE_5L_PRIMITIVE(LogMessage)
///   {
///       std::string message;
///       inArgs >> message;
///       gDebugLog.Log("%s", message.c_str());
///   }
///
#define DEFINE_5L_PRIMITIVE(NAME) \
	BEGIN_NAMESPACE_FIVEL \
	extern void DoPrim_ ## NAME(TArgumentList &inArgs); \
	END_NAMESPACE_FIVEL \
	void FIVEL_NS DoPrim_ ## NAME(TArgumentList &inArgs)

//////////
/// Set the return value of the current primitive to the
/// value of the specified variable.
///
/// \param inVariable  The variable whose value we should use.
///
inline void SetPrimitiveResult(const TValue &inVariable)
{
	gVariableManager.Set("_result", inVariable);
}

#define FIVEL_ERROR_CODE_VAR ("_errorcode")
#define FIVEL_ERROR_MSG_VAR  ("_errormsg")
#define FIVEL_SKIP_LOGGING_VAR ("_skip_logging")

//////////
/// Set the global error value.  Interpreter modules which support
/// exceptions will probably want to check this and throw an exception.
///
/// \param inValue  The boolean value to return.
///
inline void SetPrimitiveError(const char *inErrorName,
							  const char *inErrorMessage)
{
	gVariableManager.Set(FIVEL_ERROR_CODE_VAR, inErrorName);
	gVariableManager.Set(FIVEL_ERROR_MSG_VAR, inErrorMessage);
	gDebugLog.Log("PRIMITIVE ERROR: %s: %s", inErrorName, inErrorMessage);
}

//////////
/// Tell the primitive manager not to log the primitive call currently
/// in progress.
///
inline void SkipPrimitiveLogging()
{
	// We don't use an engine variable for any particular reason here--
	// a global would work fine as well.
	gVariableManager.Set(FIVEL_SKIP_LOGGING_VAR, true);
}

END_NAMESPACE_FIVEL

#endif // TPrimitives_H
