// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#ifndef TPrimitives_H
#define TPrimitives_H

#include <string>
#include <map>

#include "TCommon.h"
#include "TString.h"
#include "TPoint.h"
#include "TRect.h"
#include "TVariable.h"
#include "GraphicsTools.h"
#include "TInterpreter.h"
#include "TTemplateUtils.h"

BEGIN_NAMESPACE_FIVEL

// These are the primitive types which can currently be passed as
// arguments to a 5L command.
//
// String
//   TString
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

//////////
// TArgumentList provides an abstract interface to the argument lists
// passed to a 5L primitive function.  To allow a new TInterpreter
// class to call 5L primitives, you'll need to implement all this
// class's virtual methods.
//
// You have a lot of flexibility in how you implement the Get* methods.
// For example, these methods might automatically coerce arguments to
// the correct data type on the fly, if that's appropriate for the
// scripting language in question.
//
class TArgumentList
{
private:
	//////////
	// Keeps track of function name and evaluated parameters for Debug.log
	//
	std::string mDebugString;

	//////////
	// Log the value of a parameter for future retrieval by EndLog.
	// Note that this function will get called anyway, even if nobody
	// calls BeginLog or EndLog.
	//
	void LogParameter(const std::string &inParameterValue);

protected:
	//////////
	// Return the next argument as a string.
	//
	virtual std::string GetStringArg() = 0;

	//////////
	// Return the next argument as a singed, 32-bit integer.
	//
	virtual int32 GetInt32Arg() = 0;

	//////////
	// Return the next argument as an unsinged, 32-bit integer.
	//
	virtual uint32 GetUInt32Arg() = 0;

	//////////
	// Return the next argument as a double.
	//
	virtual double GetDoubleArg() = 0;

	//////////
	// Return the next argument as a point.
	//
	virtual TPoint GetPointArg() = 0;

	//////////
	// Return the next argument as a rectangle.
	//
	virtual TRect GetRectArg() = 0;

	//////////
	// Return the next argument as a color.
	//
	virtual GraphicsTools::Color GetColorArg() = 0;

	//////////
	// Return the next argument as a percent.
	//
	//virtual int32 GetPercentArg() = 0;

	//////////
	// Return the next argument as a callback.  This object
	// is allocated on the heap, and must be destroyed by the
	// caller (typically the primitive function) using delete.
	//
	virtual TCallback *GetCallbackArg() = 0;

	//////////
	// Return the next argument as a list.  This object
	// is allocated on the heap, and must be destroyed by the
	// caller (typically the primitive function) using delete.
	//
	virtual TArgumentList *GetListArg() = 0;

public:
	virtual ~TArgumentList() {}
	
	//////////
	// Are there any more arguments left?
	//
	virtual bool HasMoreArguments() = 0;

	// These functions provide handy wrapper functions
	// for the protected Get* functions above.
	friend TArgumentList &operator>>(TArgumentList &args, TString &out);
	friend TArgumentList &operator>>(TArgumentList &args, std::string &out);
	friend TArgumentList &operator>>(TArgumentList &args, int16 &out);
	friend TArgumentList &operator>>(TArgumentList &args, int32 &out);
	friend TArgumentList &operator>>(TArgumentList &args, bool &out);
	friend TArgumentList &operator>>(TArgumentList &args, uint32 &out);
	friend TArgumentList &operator>>(TArgumentList &args, double &out);
	friend TArgumentList &operator>>(TArgumentList &args, TRect &out);
	friend TArgumentList &operator>>(TArgumentList &args, TPoint &out);
	friend TArgumentList &operator>>(TArgumentList &args,
									 GraphicsTools::Color &out);
	friend TArgumentList &operator>>(TArgumentList &args, TCallback* &out);
	friend TArgumentList &operator>>(TArgumentList &args, TArgumentList* &out);

	// TODO - Handle the ValueOrPercent manipulator here.

	//////////
	// Ask the TArgumentList list to start logging all the parameters
	// extracted from it.  You can retrieve this log data from EndLog.
	//
	// [in] inFunctionName - The name of the functions to which this
	//                       TArgumentList corresponds.
	//
	void BeginLog(const std::string &inFunctionName);

	//////////
	// Stop logging parameters extracted from this TArgumentList, and
	// return them (together with the function name), as though they
	// were a Scheme function call.
	// 
	// [out] return - The complete entry for Debug.log
	//
	std::string EndLog();
};

//////////
// The TPrimitiveManager class maintains a set of primitive engine
// functions.  These functions can be called from our scripting
// language.
//
class TPrimitiveManager
{
	//////////
	// The big table of all our primitive functions.  We store
	// the function pointers as void* to avoid instantiating 
	// extra template code.  With a good C++ compiler and STL
	// library, this wouldn't be necessary.
	//
	std::map<std::string,void*> mPrimitiveMap;

public:
	//////////
	// A PrimitiveFunc implements a single primitive.
	//
	// [in] inArgs - The arguments to the primitive.
	//
	typedef void (*PrimitiveFunc)(TArgumentList &inArgs);
	
	//////////
	// Register a primitive with the primitive manager.
	//
	// [in] inName - The name of the primitive, in lowercase.
	// [in] inFunc - The function which implements this primitive.
	//
	void RegisterPrimitive(const std::string &inName, PrimitiveFunc inFunc);

	//////////
	// Does a primitive with the given name exist?
	//
	// [in] inName - The name of the primitive, in lowercase.
	// [out] return - Whether the given primitive exists.
	//
	bool DoesPrimitiveExist(const std::string &inName);

	//////////
	// Call the specified primitive.  This function throws
	// all sorts of exciting exceptions, so you should probably
	// wrap it in a try/catch block and deal with any problems
	// that arise.
	//
	// [in] inName - The name of the primitive, in lowercase.
	// [in] inArgs - The arguments to the primitive.
	//
	void CallPrimitive(const std::string &inName, TArgumentList &inArgs);
};

//////////
// The global object in charge of managing our primitive functions.
//
extern TPrimitiveManager gPrimitiveManager;

//////////
// A handy macro for declaring a 5L primitive function and registering
// it with the gPrimitiveManager, all in one fell swoop.  There are
// several bits of pre-processor wizardry going on here:
//
//   1) The 'do { ... } while (0)' makes C++ treat the multiple
//      statements in the body as a single statement.
//   2) The '#NAME' construct converts the argument token to
//      to a string literal.
//   3) The '##' construct glues two adjacent tokens together.
//
// Call it as follows:
//
//   REGISTER_5L_PRIMITIVE_WITH_NAME("log", LogMessage); // register "log"
//   REGISTER_5L_PRIMITIVE(LogMessage); // register "logmessage"
//
#define REGISTER_5L_PRIMITIVE_WITH_NAME(NAME, TOKEN) \
	do { \
		extern void DoPrim_ ## TOKEN(TArgumentList &inArgs); \
		gPrimitiveManager.RegisterPrimitive(NAME, &DoPrim_ ## TOKEN); \
	} while (0)

#define REGISTER_5L_PRIMITIVE(NAME) \
	REGISTER_5L_PRIMITIVE_WITH_NAME(MakeStringLowercase(#NAME), NAME)

//////////
// Use this macro in place of a function prototype when implementing a
// 5L primitive.  This will shield you from changes to the standard
// prototype.  (For an explanation of the macro grik, see
// REGISTER_5L_PRIMITIVE.)
//
// Use it as follows:
//
//   DEFINE_5L_PRIMITIVE(LogMessage)
//   {
//       std::string message;
//       inArgs >> message;
//       gDebugLog.Log("%s", message.c_str());
//   }
//
#define DEFINE_5L_PRIMITIVE(NAME) \
	BEGIN_NAMESPACE_FIVEL \
	extern void DoPrim_ ## NAME(TArgumentList &inArgs); \
	END_NAMESPACE_FIVEL \
	void FIVEL_NS DoPrim_ ## NAME(TArgumentList &inArgs)

//////////
// Set the return value of the current primitive.
//
// [in] inValue - The string to return.
//
inline void SetPrimitiveResult(const char *inValue)
{
	gVariableManager.SetString("_result", inValue);
}

//////////
// Set the return value of the current primitive.
//
// [in] inValue - The integer to return.
//
inline void SetPrimitiveResult(int32 inValue)
{
	gVariableManager.SetLong("_result", inValue);
}

//////////
// Set the return value of the current primitive.
//
// [in] inValue - The floating point value to return.
//
inline void SetPrimitiveResult(double inValue)
{
	gVariableManager.SetDouble("_result", inValue);
}

//////////
// Set the return value of the current primitive.
//
// [in] inValue - The boolean value to return.
//
inline void SetPrimitiveResult(bool inValue)
{
	gVariableManager.SetLong("_result", inValue ? 1 : 0);
}


END_NAMESPACE_FIVEL

#endif // TPrimitives_H
