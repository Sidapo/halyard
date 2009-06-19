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

#include "AppHeaders.h"
#include "CardGroup.h"

using namespace Halyard;


//=========================================================================
//  CardGroup Methods
//=========================================================================

CardGroup::CardGroup(const wxString &inName, Halyard::TCallbackPtr inDispatcher)
    : GroupMember(inName, inDispatcher)
{
}

void CardGroup::RecursivelyCompositeInto(CairoContext &inCr,
                                         bool inIsCompositingDragLayer,
                                          bool inAncestorIsInDragLayer)
{
    // Since there are only CardGroup objects in our ancestory, we should
    // never be in the drag layer.
    ASSERT(!inAncestorIsInDragLayer);
    ASSERT(!IsInDragLayer());

    // First, composite this node and all our elements, and then
    // recursively composite mMember.  This order is intentional--we assume
    // that elements are very tightly tied to their parents, and we don't
    // want our elements to float over mMember's elements.
    GroupMember::RecursivelyCompositeInto(inCr, inIsCompositingDragLayer,
                                          inAncestorIsInDragLayer);
    if (mMember)
        mMember->RecursivelyCompositeInto(inCr, inIsCompositingDragLayer,
                                          inAncestorIsInDragLayer);
}

NodePtr CardGroup::FindNodeAt(const wxPoint &inPoint, bool inMustWantCursor) {
    // First, ask mMember to look for a node, because it's drawn on top of
    // us.
    if (mMember) {
        NodePtr found(mMember->FindNodeAt(inPoint, inMustWantCursor));
        if (found)
            return found;
    }

    // If that fails, check our elements and ourself.
    return GroupMember::FindNodeAt(inPoint, inMustWantCursor);
}

void CardGroup::RecursivelyReregisterWithElementsPane(ElementsPane *inPane) {
    GroupMember::RecursivelyReregisterWithElementsPane(inPane);
    if (mMember)
        mMember->RecursivelyReregisterWithElementsPane(inPane);
}

void CardGroup::RegisterMember(GroupMemberPtr inMember) {
    ASSERT(!mMember);
    mMember = inMember;
}

void CardGroup::UnregisterMember(GroupMemberPtr inMember) {
    ASSERT(mMember == inMember);
    mMember.reset();
}
