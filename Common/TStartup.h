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

#if !defined (_TStartup_h_)
#define _TStartup_h_

#include "TInterpreter.h"

// On the Macintosh, we need to manually set the stack base so the
// Boehm GC doesn't get confused (it doesn't know how to detect it
// correctly).
#ifdef FIVEL_PLATFORM_MACINTOSH
#   include <scheme.h>
#   define FIVEL_SET_STACK_BASE() \
	    do { \
            int dummy; \
	        scheme_set_stack_base(&dummy, 0); \
        }  while (0)
#else // !FIVEL_PLATFORM_MACINTOSH
#   define FIVEL_SET_STACK_BASE() ((void) 0)
#endif // !FIVEL_PLATFORM_MACINTOSH

BEGIN_NAMESPACE_FIVEL

//////////
/// Initialize the various modules of the Common/ library.
/// If you want to call FileSystem::SetBaseDirectory, do it
/// before calling this function.
///
extern void InitializeCommonCode();

//////////
/// Create a Scheme interpreter manager.
///
extern TInterpreterManager *
GetSchemeInterpreterManager(TInterpreter::SystemIdleProc inIdleProc);

//////////
/// If the current program looks like a Scheme script, attempt
/// to start a Scheme interpreter.  Otherwise, return NULL.  This function
/// can be used to support two languages in a single engine.
///
extern TInterpreterManager *
MaybeGetSchemeInterpreterManager(TInterpreter::SystemIdleProc inIdleProc);

END_NAMESPACE_FIVEL

#endif // TStartup
