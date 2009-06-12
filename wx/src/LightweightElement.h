// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
// @BEGIN_LICENSE
//
// Halyard - Multimedia authoring and playback system
// Copyright 1993-2009 Trustees of Dartmouth College
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

#ifndef LightweightElement_H
#define LightweightElement_H

#include "TInterpreter.h"
#include "Element.h"
#include "TStateDB.h"

class wxAccessible;


//////////
/// A lightweight element is basically a "virtual widget" on our stage.  It
/// doesn't have an associated wxWindow object; all of its events are passed
/// directly to it by the Stage itself.
///
/// TODO - Currently, LightweightElements are the only Elements which can
/// have event handlers in Scheme.  This is because I haven't figured out
/// how to combine the wxWindows event-handling system with the Scheme
/// event-handling system we're using.
///
class LightweightElement : public Element {
    std::string mCursorName;
    bool mIsShown;
    bool mWantsCursor;
    bool mIsInDragLayer;

#if wxUSE_ACCESSIBILITY
    shared_ptr<wxAccessible> mAccessible;
#endif // wxUSE_ACCESSIBILITY
    
public:
    LightweightElement(Stage *inStage, const wxString &inName,
                       Halyard::TCallbackPtr inDispatch,
                       const std::string &inCursorName);

    virtual bool IsShown() { return mIsShown; }
    virtual void Show(bool inShow);

    virtual bool WantsCursor() const { return mWantsCursor; }
    void SetWantsCursor(bool wantsCursor) { mWantsCursor = wantsCursor; }
    virtual bool IsLightWeight() { return true; }

    virtual std::string GetCursorName() { return mCursorName; }
    virtual void SetCursorName(const std::string &inCursorName);

    virtual bool IsInDragLayer() const;
    virtual void SetInDragLayer(bool inDragLayer);

#if wxUSE_ACCESSIBILITY
    virtual wxAccessible *GetAccessible() { return mAccessible.get(); }
#endif // wxUSE_ACCESSIBILITY
};

#endif // LightweightElement_H

