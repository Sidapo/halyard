//////////////////////////////////////////////////////////////////////////////
//
//   (c) Copyright 1999, Trustees of Dartmouth College, All rights reserved.
//        Interactive Media Lab, Dartmouth Medical School
//
//			$Author$
//          $Date$
//          $Revision$
//
//////////////////////////////////////////////////////////////////////////////

#if !defined (_TPlatform_h_)
#define _TPlatform_h_


/*-----------------------------------------------------------------

OVERVIEW
	Platform-specific configuration code.

TPLATFORM MACRO DEFINITIONS
	#define ASSERT(x)

AUTHOR
    Chuck Officer<br>

-----------------------------------------------------------------*/


//=========================================================================
//  QuickTime Configuration
//=========================================================================

#if defined (__QTML__) || defined (macintosh)
#define _5L_QUICKTIME_
#endif


//=========================================================================
//  Win32 Configuration
//=========================================================================

#if defined (WIN32)

// TODO - This symbol begins with an underscore, which is illegal.
// Refactor the code to replace it with FIVEL_PLATFORM_WIN32.
#define _5L_WIN32_

#define FIVEL_PLATFORM_WIN32 (1)

#if defined (_DEBUG)
#define DEBUG
#endif

#include <crtdbg.h>
#define ASSERT(x) _ASSERTE(x)

// For now, the Windows engine uses some non-standard string functions
// instead of using our own, equivalent code.
#define HAVE__STRLWR 1
#define HAVE__STRUPR 1
#define HAVE__STRICMP 1

// Turn off warnings about identifiers being truncated in debug information.
// Without this, working with STL templates will drive us nuts.
#pragma warning(disable: 4786)


//=========================================================================
//  Macintosh Configuration
//=========================================================================

#elif defined (macintosh)

#define FIVEL_PLATFORM_MACINTOSH (1)

#define WANT_FIVEL_NAMESPACE (1)

#define NEWLINE_CHAR	'\r'
#define RETURN_CHAR		'\n'

#ifdef Debug_Throw
#define DEBUG

#define ASSERT(expr) assert(expr)

// backwards compatible defines
//#define DEBUG_5L
//#define DEBUG_5L_SCRIPT

#else

#define ASSERT(x)

#endif	// Debug_Throw


//=========================================================================
//  Other Platform Configuration
//=========================================================================

#else

#define FIVEL_PLATFORM_OTHER (1)

#define WANT_FIVEL_NAMESPACE (1)
#define ASSERT(expr) assert(expr)

#endif


//=========================================================================
//  Namespace Configuration
//=========================================================================
//  It's possible to define most of our common classes in a
//  'FiveL' namespace.

#ifdef WANT_FIVEL_NAMESPACE
#	define BEGIN_NAMESPACE_FIVEL  namespace FiveL {
#	define END_NAMESPACE_FIVEL    }
#	define USING_NAMESPACE_FIVEL  using namespace FiveL;
#	define FIVEL_NS				  FiveL::
#else
#	define BEGIN_NAMESPACE_FIVEL
#	define END_NAMESPACE_FIVEL
#	define USING_NAMESPACE_FIVEL
#	define FIVEL_NS
#endif

#endif // _TPlatform_h_
