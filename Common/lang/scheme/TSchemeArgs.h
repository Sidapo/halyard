// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-
// @BEGIN_LICENSE
//
// Halyard - Multimedia authoring and playback system
// Copyright 1993-2008 Trustees of Dartmouth College
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

#ifndef TSchemeArgs_H
#define TSchemeArgs_H

#include "scheme.h"

BEGIN_NAMESPACE_HALYARD

//////////
/// This is a wrapper for building Scheme_Object * argc/argv pairs in a
/// convenient and GC-safe fashion.  We automatically handle initialization
/// with NULL, and registration of the correct size with TSchemeReg.  Note
/// that passing a TSchemeArgs object from one function to another is
/// generally useless--this class is intended for use within a single
/// function.
///
///   void foo() {
///       // First, declare our argument array.
///       TSchemeArgs<2> args;
///
///       // Next, register our argument array with TSchemeReg.
///       TSchemeReg<0,1> reg;
///       reg.args(args);
///       reg.done();
///
///       // Set up our arguments and call Scheme.
///       args[0] = ...;
///       args[1] = ...;
///       TSchemeInterpreter::CallScheme("my_func", args.size(), args.ptr());
///   }
///
template <size_t LENGTH>
class TSchemeArgs : boost::noncopyable {
    Scheme_Object *mArgs[LENGTH];

public:
    /// The type of elements contained in this array.
    typedef Scheme_Object *elem_type;

    TSchemeArgs() throw () {
        // Initialize all pointers to NULL for ease of use with .
        for (size_t i = 0; i < LENGTH; i++)
            mArgs[i] = NULL;
    }

    /// The number of items in this array;
    size_t size() const throw () { return LENGTH; }

    /// Access the specified argument.
    elem_type &operator[](size_t i) throw () {
        ASSERT(0 <= i && i <= LENGTH);
        return mArgs[i];
    }

    /// Get a pointer to the underlying array (for passing to Scheme).
    elem_type *get() throw () { return mArgs; }
};

END_NAMESPACE_HALYARD

#endif // TSchemeArgs_H