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

#ifndef CardGroup_H
#define CardGroup_H

#include "GroupMember.h"

/// A CardGroup represents a group or a card.
class CardGroup : public GroupMember {
    GroupMemberPtr mMember;

public:
    CardGroup(const wxString &inName,
              Halyard::TCallbackPtr inDispatcher = Halyard::TCallbackPtr());

    Type GetType() { return CARD_GROUP; }

    virtual void RecursivelyCompositeInto(CairoContext &inCr,
                                          bool inIsCompositingDragLayer = false,
                                          bool inAncestorIsInDragLayer = false);
    virtual NodePtr FindNodeAt(const wxPoint &inPoint,
                               bool inMustWantCursor = true);
    virtual void RecursivelyReregisterWithElementsPane(ElementsPane *inPane);

    /// Register the child GroupMember of this CardGroup.  Since C++ nodes
    /// represent running nodes, a CardGroup may only have one member at a
    /// time.
    void RegisterMember(GroupMemberPtr inMember);

    /// Unregister the child GroupMember of this CardGroup.
    void UnregisterMember(GroupMemberPtr inMember);
};
typedef shared_ptr<CardGroup> CardGroupPtr;

#endif // CardGroup_H
