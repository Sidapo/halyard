// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#include "TCommon.h"
#include "TPrimitives.h"
#include "TException.h"
#include "TLogger.h"
#include "TTemplateUtils.h"

USING_NAMESPACE_FIVEL

TPrimitiveManager FIVEL_NS gPrimitiveManager;


//=========================================================================
// TArgumentList Methods
//=========================================================================

void TArgumentList::BeginLog(const std::string &inFunctionName)
{
	ASSERT(mDebugString.empty());
	mDebugString = inFunctionName + std::string(":");
}

void TArgumentList::LogParameter(const std::string &inParameterValue)
{
	if (!mDebugString.empty())
		mDebugString += std::string(" ") + inParameterValue;
}

std::string TArgumentList::EndLog()
{
	ASSERT(!mDebugString.empty());
	std::string result = mDebugString;
	mDebugString = "";
	return result;
}

TArgumentList &FIVEL_NS operator>>(TArgumentList &args, TString &out)
{
    out = TString(args.GetStringArg().c_str());
	args.LogParameter(MakeQuotedString(out.GetString()));
    return args;
}

TArgumentList &FIVEL_NS operator>>(TArgumentList &args, std::string &out)
{
    out = args.GetStringArg();
	args.LogParameter(MakeQuotedString(out));
    return args;
}

TArgumentList &FIVEL_NS operator>>(TArgumentList &args, int16 &out)
{
    int32 temp;
    temp = args.GetInt32Arg();
    if (temp < MIN_INT16 || MAX_INT16 < temp)
		throw TException(__FILE__, __LINE__,
						 "Can't represent value as 16-bit integer");
    out = temp;
	args.LogParameter(TString::IntToString(out).GetString());
    return args;
}

TArgumentList &FIVEL_NS operator>>(TArgumentList &args, int32 &out)
{
    out = args.GetInt32Arg();
	args.LogParameter(TString::IntToString(out).GetString());
    return args;
}

TArgumentList &FIVEL_NS operator>>(TArgumentList &args, bool &out)
{
    out = args.GetInt32Arg() ? true : false;
	args.LogParameter(out ? "#t" : "#f");
	return args;
}

TArgumentList &FIVEL_NS operator>>(TArgumentList &args, uint32 &out)
{
    out = args.GetUInt32Arg();
	args.LogParameter(TString::UIntToString(out).GetString());
    return args;
}

TArgumentList &FIVEL_NS operator>>(TArgumentList &args, double &out)
{
    out = args.GetDoubleArg();
	args.LogParameter(TString::DoubleToString(out).GetString());
    return args;
}

TArgumentList &FIVEL_NS operator>>(TArgumentList &args, TRect &out)
{
    out = args.GetRectArg();
	args.LogParameter(std::string("(rect ") +
					  TString::IntToString(out.Left()).GetString() +
					  std::string(" ") +
					  TString::IntToString(out.Top()).GetString() +
					  std::string(" ") +
					  TString::IntToString(out.Right()).GetString() +
					  std::string(" ") +
					  TString::IntToString(out.Bottom()).GetString() +
					  std::string(")"));
    return args;
}

TArgumentList &FIVEL_NS operator>>(TArgumentList &args, TPoint &out)
{
    out = args.GetPointArg();
	args.LogParameter(std::string("(pt ") +
					  TString::IntToString(out.X()).GetString() +
					  std::string(" ") +
					  TString::IntToString(out.Y()).GetString() +
					  std::string(")"));
    return args;
}

TArgumentList &FIVEL_NS operator>>(TArgumentList &args,
								   GraphicsTools::Color &out)
{
    out = args.GetColorArg();
	args.LogParameter(std::string("(rbga ") +
					  TString::IntToString(out.red).GetString() +
					  std::string(" ") +
					  TString::IntToString(out.green).GetString() +
					  std::string(" ") +
					  TString::IntToString(out.blue).GetString() +
					  std::string(" ") +
					  TString::IntToString(out.alpha).GetString() +
					  std::string(")"));
    return args;
}

TArgumentList &FIVEL_NS operator>>(TArgumentList &args, TCallback* &out)
{
    out = args.GetCallbackArg();
	args.LogParameter(out->PrintableRepresentation());
    return args;
}

TArgumentList &FIVEL_NS operator>>(TArgumentList &args, TArgumentList* &out)
{
	out = args.GetListArg();
	args.LogParameter("#<list>");
	return args;
}


//=========================================================================
// TPrimitiveManager Methods
//=========================================================================

void TPrimitiveManager::RegisterPrimitive(const std::string &inName,
										  PrimitiveFunc inFunc)
{
    ASSERT(inName != "");
    ASSERT(inFunc != NULL);

    // Erase any existing primitive with this name.
    std::map<std::string,void*>::iterator existing =
	mPrimitiveMap.find(inName);
    if (existing != mPrimitiveMap.end())
    {
		gDebugLog.Log("Replacing primitive <%s>", inName.c_str());
		mPrimitiveMap.erase(existing);
    }
    
    // Insert the new entry.
    mPrimitiveMap.insert(std::pair<std::string,void*>(inName, inFunc));
}

bool TPrimitiveManager::DoesPrimitiveExist(const std::string &inName)
{
    ASSERT(inName != "");

    std::map<std::string,void*>::iterator found = mPrimitiveMap.find(inName);
    if (found != mPrimitiveMap.end())
		return true;
    else
		return false;
}

void TPrimitiveManager::CallPrimitive(const std::string &inName,
									  TArgumentList &inArgs)
{
    ASSERT(inName != "");
    
    // Find the primitive.
    std::map<std::string,void*>::iterator found = mPrimitiveMap.find(inName);
    if (found == mPrimitiveMap.end())
		throw TException(__FILE__, __LINE__,
						 "Tried to call non-existant primitive");
    PrimitiveFunc primitive = static_cast<PrimitiveFunc>(found->second);

	// Ask the TArgumentList to log all the parameters it returns.
	inArgs.BeginLog(inName);

	// Clear the result value.
	gVariableManager.MakeNull("_result");
    
    // Call it.
    (*primitive)(inArgs);

	// Extract the logged arguments and write them to the debug log.
	std::string call_info = inArgs.EndLog();
	if (gVariableManager.IsNull("_result"))
		gDebugLog.Log(">>> %s", call_info.c_str());
	else
		gDebugLog.Log(">>> %s -> %s",
					  call_info.c_str(),
					  gVariableManager.GetString("_result"));
}
