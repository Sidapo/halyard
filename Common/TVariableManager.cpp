// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#include "CommonHeaders.h"
#include "TestCase.h"
#include "TInterpreter.h"
#include "TVariableManager.h"

USING_NAMESPACE_FIVEL

TVariableManager FIVEL_NS gVariableManager;

REGISTER_TEST_CASE_FILE(TVariableManager);


//=========================================================================
//  TVariableManager Methods
//=========================================================================

TValue TVariableManager::Get(const std::string &inName) {
	TValueMap::iterator iter = mMap.find(MakeStringLowercase(inName));
	if (iter != mMap.end()) {
		return iter->second; 
    } else {
        std::string err = "Tried to read uninitialized variable: " + inName;
		THROW(err.c_str());
    }
}

void TVariableManager::Set(const std::string &inName, const TValue &inValue) {
	std::pair<std::string, TValue> p(MakeStringLowercase(inName), inValue); 
	
	std::pair<TValueMap::iterator, bool> result;
	result = mMap.insert(p);

    // If variable already exists, replace it with inValue
    // and notify caller that a replacement has been made.
	if (!result.second) {
		mMap.erase(result.first);
		mMap.insert(p);
	}
}

bool TVariableManager::VariableExists(const std::string &inName) {
	TValueMap::iterator iter = mMap.find(MakeStringLowercase(inName));
	return iter != mMap.end();
}
		
TValue::Type TVariableManager::GetType(const char *inName) {
	return Get(inName).GetType();
}

void TVariableManager::MakeNull(const char *inName) {
	Set(inName, TValue(TNull()));
}

bool TVariableManager::IsNull(const char *inName) {
    return (Get(inName).GetType() == TValue::TYPE_NULL);
}


//=========================================================================
//  Tests
//=========================================================================

#if BUILD_TEST_CASES

BEGIN_TEST_CASE(TestTVariableManager, TestCase) {
	TVariableManager manager;

	manager.Set("foo", "bar");
	CHECK_EQ(std::string(manager.Get("foo")), 
			 "bar");
	//manager.SetString("foo", "bar");
	CHECK_EQ(manager.GetType("foo"), TValue::TYPE_STRING); 

	// check extracting a non existent variable
	//CHECK_THROWN(std::exception,
	//			   std::string(manager.GetString("bar")));

	manager.Set("CASETEST", "foo");
	CHECK_EQ(std::string(manager.Get("casetest")), "foo");

	manager.Set("hello", TSymbol("world"));
	CHECK_EQ(TSymbol(manager.Get("hello")).GetName(),
			 "world");
	CHECK_EQ(manager.GetType("hello"), TValue::TYPE_SYMBOL); 

	manager.Set("longVar", MAX_INT32);
	CHECK_EQ(int32(manager.Get("longVar")), MAX_INT32);
	CHECK_EQ(manager.GetType("longVar"), TValue::TYPE_LONG); 

	manager.Set("ulongVar", MAX_UINT32);
	CHECK_EQ(uint32(manager.Get("ulongVar")), MAX_UINT32);
	CHECK_EQ(manager.GetType("ulongVar"), TValue::TYPE_ULONG); 

	manager.Set("doubleVar", 10.0);
	CHECK_EQ(double(manager.Get("doubleVar")), 10.0);
	CHECK_EQ(manager.GetType("doubleVar"), TValue::TYPE_DOUBLE);

	manager.Set("boolVar", true);
	CHECK_EQ(bool(manager.Get("boolVar")), true);
	manager.Set("boolVar", false);
	CHECK_EQ(bool(manager.Get("boolVar")), false);
	CHECK_EQ(manager.GetType("boolVar"), TValue::TYPE_BOOLEAN);

	manager.Set("pointVar", TPoint(0, 0));
	CHECK_EQ(TPoint(manager.Get("pointVar")), TPoint(0, 0));
	CHECK_EQ(manager.GetType("pointVar"), TValue::TYPE_POINT);

	manager.Set("rectVar", TRect(0, 1, 2, 3));
	CHECK_EQ(TRect(manager.Get("rectVar")),TRect(0, 1, 2, 3));
	CHECK_EQ(manager.GetType("rectVar"), TValue::TYPE_RECT);

	manager.Set("colorVar", 
	    GraphicsTools::Color(0, 1, 2, 3));
	CHECK_EQ(GraphicsTools::Color(manager.Get("colorVar")), 
			 GraphicsTools::Color(0, 1, 2, 3));
	CHECK_EQ(manager.GetType("colorVar"), TValue::TYPE_COLOR);
	
    // check resetting TValues
	CHECK_EQ(manager.IsNull("longVar"), false);
	manager.MakeNull("longVar");
	CHECK_EQ(manager.IsNull("longVar"), true);

} END_TEST_CASE(TestTVariableManager);


#endif // BUILD_TEST_CASES
