// -*- Mode: C++; tab-width: 4; -*-

#include "CommonHeaders.h"

#include <iostream>

#include "ImlUnit.h"

// We need to call some module initialization functions.
#include "TStartup.h"

// We declare some testing-related primitives for the interpreter.
#include "TPrimitives.h"

// XXX - Hack to make REGISTER_5L_PRIMITIVE work correctly.  It needs to be
// called from a function in the FiveL:: namespace, which is silly.
BEGIN_NAMESPACE_FIVEL
extern void RegisterTestPrimitives();
END_NAMESPACE_FIVEL

USING_NAMESPACE_FIVEL

// XXX - For now, we'll declare the per-file test entry points here, to
// avoid creating extra header files just for one function.  These will
// most likely be replaced with static constructor tricks as I continue to
// C++-ify the testing API.
extern void test_TTemplateUtils(void);
extern void test_TString (void);
extern void test_TBTree (void);
extern void test_TEncoding (void);
extern void test_FileSystem (void);
extern void test_Model(void);
extern void test_Typography (void);
extern void test_TStyleSheet (void);
extern void test_TSchemeInterpreter (void);
extern void test_TVectorDiff (void);
extern void test_TPolygon (void);

DEFINE_5L_PRIMITIVE(test)
{
	std::string info;
	bool result;
	inArgs >> info >> result;
	TEST_WITH_LABEL(info.c_str(), result);
}

void FIVEL_NS RegisterTestPrimitives()
{
	REGISTER_5L_PRIMITIVE(test);
}

int main (int argc, char **argv)
{
	FIVEL_SET_STACK_BASE();

	try
	{
		FIVEL_NS InitializeCommonCode();
		RegisterTestPrimitives();
		test_TTemplateUtils();
		test_TString();
		test_TBTree();
		test_TEncoding();
		test_FileSystem();
		test_Model();
		test_Typography();
		test_TStyleSheet();
		test_TSchemeInterpreter();
		test_TVectorDiff();
		test_TPolygon();
	}
	catch (std::exception &error)
	{
		std::cerr << std::endl << "Exception: " << error.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cerr << std::endl << "An unknown exception occurred!"
				  << std::endl;
		return 1;
	}

	return tests_finished();
}
