// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#if !defined (_TSchemeInterpreter_h_)
#define _TSchemeInterpreter_h_

#include "TCommon.h"
#include "TInterpreter.h"
#include "TPrimitives.h"
#include "scheme.h"

BEGIN_NAMESPACE_FIVEL


//////////
// A smart-pointer class which can point to a Scheme object and prevent
// it from being garbage-collected.  You must use this class to point to
// a Scheme_Object stored anywhere except the stack (which the Scheme GC
// automatically scans for us).  So global and heap objects *must* use
// this class to refer to anything in the Scheme heap.
//
template <class Type>
class TSchemePtr
{
	Type *mPtr;
	
	void Set(Type *inPtr)
	{
		if (mPtr == inPtr)
			return;
		if (mPtr)
			scheme_gc_ptr_ok(mPtr);
		mPtr = inPtr;
		if (inPtr)
			scheme_dont_gc_ptr(mPtr);
	}

public:
	TSchemePtr() : mPtr(NULL) {}
	TSchemePtr(Type *inPtr) : mPtr(NULL) { Set(inPtr); }
	TSchemePtr(const TSchemePtr &inSchemePtr) : mPtr(NULL) { Set(inSchemePtr.mPtr); }
	operator Type*() { return mPtr; }
	TSchemePtr<Type> &operator=(Type *inPtr) { Set(inPtr); }
	TSchemePtr<Type> &operator=(const TSchemePtr &inPtr) { Set(inSchemePtr.mPtr); }
};


//////////
// The interface to our Scheme interpreter.
//
class TSchemeInterpreter : public TInterpreter
{
	friend class TSchemeCallback;
	friend class TSchemeArgumentList;

	static Scheme_Env *sGlobalEnv;

	static SystemIdleProc sSystemIdleProc;

	static Scheme_Object *Call5LPrim(void *inData, int inArgc,
									 Scheme_Object **inArgv);

	static Scheme_Object *CallScheme(const char *inFuncName,
									 int inArgc, Scheme_Object **inArgv);

	static Scheme_Object *CallSchemeSimple(const char *inFuncName);

public:
	TSchemeInterpreter();
	virtual ~TSchemeInterpreter();

	void DoIdle() { ASSERT(sSystemIdleProc); (*sSystemIdleProc)(); }

	// For documentation of these virtual methods, see TInterpreter.h.
	virtual void Run(SystemIdleProc inIdleProc);
	virtual void KillInterpreter();
	virtual void Pause(void);
	virtual void WakeUp(void);
	virtual bool Paused(void);
	virtual void Timeout(const char *inName, int32 inTime);
	virtual void Nap(int32 inTime);
	virtual bool Napping(void);
	virtual void KillNap(void);
	virtual void KillCurrentCard(void);
	virtual void JumpToCardByName(const char *inName);
	virtual std::string CurCardName(void);
	virtual std::string PrevCardName(void);
};


//////////
// A C++ wrapper for a zero-argument Scheme callback function (a "thunk").
//
class TSchemeCallback : public TCallback
{
	//////////
	// Our callback object.  Note that we need to use a TSchemePtr to
	// prevent mCallback from being garbage-collected, because we're
	// a heap-based object.
	//
	TSchemePtr<Scheme_Object> mCallback;

public:
	TSchemeCallback(Scheme_Object *inCallback) : mCallback(inCallback) {}

	// For documentation of these virtual methods, see TInterpreter.h.
	virtual void Run();
	virtual std::string PrintableRepresentation() { return "#<thunk>"; }
};


//////////
// A TInterpreterManager for our Scheme interpreter.  This handles
// reloading scripts and other fun stuff that involves creating
// and destroying interpreters.
//
class TSchemeInterpreterManager : public TInterpreterManager
{
public:
	TSchemeInterpreterManager(TInterpreter::SystemIdleProc inIdleProc);

protected:
	virtual TInterpreter *MakeInterpreter();
};


//////////
// A list of Scheme values, for use by the argument-parsing system.  This
// is an abstract class with several subclasses, because mzscheme has several
// different kinds of lists which we might want to process.
//
class TSchemeArgumentList : public TArgumentList
{
	void TypeCheckFail();
	void TypeCheck(Scheme_Type inType, Scheme_Object *inValue);
	void TypeCheckStruct(const char *inPredicate, Scheme_Object *inVal);
	int32 GetInt32Member(const char *inName, Scheme_Object *inVal);

public:
	TSchemeArgumentList() {}

protected:
	//////////
	// Fetch the next argument.
	//
	virtual Scheme_Object *GetNextArg() = 0;
	
	// For documentation of these virtual methods, see TPrimitives.h.
	virtual std::string GetStringArg();
	virtual int32 GetInt32Arg();
	virtual uint32 GetUInt32Arg();
	virtual bool GetBoolArg();
	virtual double GetDoubleArg();
	virtual TPoint GetPointArg();
	virtual TRect GetRectArg();
	virtual GraphicsTools::Color GetColorArg();
	//virtual int32 GetPercentArg();
	virtual TCallback *GetCallbackArg();
	virtual TArgumentList *GetListArg();
};


//////////
// A list of Scheme_Object values stored as an argc,argv pair.  This is
// the format which mzscheme uses to pass function arguments on the stack.
//
class TSchemeArgvList : public TSchemeArgumentList
{
	//////////
	// The number of arguments in mArgv.
	//
	int mArgc;

	//////////
	// The arguments passed to a primitive.  We don't need to use a
	// TSchemePtr here, because mzscheme protects this object from
	// collection.
	//
	Scheme_Object **mArgv;

	//////////
	// The number of arguments processed so far.
	//
	int mArgsReturned;

public:
	TSchemeArgvList(int inArgc, Scheme_Object **inArgv)
		: mArgc(inArgc), mArgv(inArgv), mArgsReturned(0) {}

	virtual bool HasMoreArguments() { return mArgsReturned < mArgc; }

protected:
	Scheme_Object *GetNextArg()
	{
		if (!HasMoreArguments())
			throw TException(__FILE__, __LINE__, "Not enough arguments");
		return mArgv[mArgsReturned++];
	}
};

END_NAMESPACE_FIVEL

#endif // TSchemeInterpreter
