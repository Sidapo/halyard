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

//////////////////////////////////////////////////////////////////////////////
//
// Debug.h : 
//

#if !defined (_Debug_h_)
#define _Debug_h_

#include "crtdbg.h"

#ifdef _DEBUG
#define DEBUG

// define bit masks for the logger
#define DEBUG_MESSAGES		0x00000001
#define DEBUG_EVENTS		0x00000002
#define DEBUG_MOVIES		0x00000004
#define DEBUG_RESOURCES		0x00000008
#define DEBUG_GRAPHICS		0x00000010

#else
#undef DEBUG
#endif

#define ASSERT(exp) _ASSERTE(exp)

#endif // _Debug_h_

/*
 $Log$
 Revision 1.1  2001/09/24 15:11:00  tvw
 FiveL v3.00 Build 10

 First commit of /iml/FiveL/Release branch.

 There are now seperate branches for development and release
 codebases.

 Development - /iml/FiveL/Dev
 Release - /iml/FiveL/Release

 Revision 1.2  1999/09/24 19:57:18  chuck
 Initial revision

*/
