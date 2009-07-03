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

#include <boost/foreach.hpp>

#include "Element.h"
#include "Card.h"
#include "HalyardApp.h"
#include "Stage.h"
#include "StageFrame.h"
#include "ElementsPane.h"

using namespace Halyard;


//=========================================================================
//  Constructor and destructor
//=========================================================================

Element::Element(const wxString &inName, Halyard::TCallbackPtr inDispatcher)
    : Node(inName, inDispatcher), mHasLegacyZOrderAndVisibility(false),
      mIsVisible(true), mIsShown(true)
{
    // TODO - We initialially set mIsVisible to true, because subclasses of
    // Widget are created in a visible state.  We may want to think about
    // this a bit more.
    //
    // mIsVisible will be recalculated in Register.  We can't do anything
    // about it here, because our virtual methods aren't fully set up until
    // all our subclass constructors have finished running.
}


//=========================================================================
//  Inherited from Node
//=========================================================================

NodePtr Element::GetParentForPurposeOfZOrderAndVisibility() {
    if (HasLegacyZOrderAndVisibility()) {
        GroupMemberPtr group_member(wxGetApp().GetStage()
                                    ->GetCurrentGroupMember());
        ASSERT(group_member);
        return group_member;
    }
    return GetParent();
}


//=========================================================================
//  Legacy Z-order and visibility support
//=========================================================================

void Element::UseLegacyZOrderAndVisibility() {
    ASSERT(!mHasLegacyZOrderAndVisibility);
    mHasLegacyZOrderAndVisibility = true;
    ElementPtr as_shared(shared_from_this(), dynamic_cast_tag());
    ASSERT(as_shared);
    wxGetApp().GetStage()->RegisterLegacyZOrderAndVisibility(as_shared);
    RecursivelyCalculateVisibility();
}


//=========================================================================
//  Visibility
//=========================================================================

void Element::CalculateVisibility() {
    bool previously_visible = IsVisible();
    mIsVisible = (GetParentForPurposeOfZOrderAndVisibility()->IsVisible() &&
                  GetIsShown());
    ASSERT(IsVisible() == mIsVisible);

    // Hide or show any associated wxWindow objects, Quake overlays, etc.
    if (previously_visible != IsVisible())
        NotifyVisibilityChanged();

    // Notify the ElementsPane and Stage that nodes have changed.  Yes, we
    // notify the stage repeatedly as we recursively walk our children, but
    // that's OK--doing so is cheap.
    wxGetApp().GetStageFrame()->GetElementsPane()->
        NotifyNodeStateChanged(shared_from_this());
    wxGetApp().GetStage()->NotifyNodesChanged();
}

void Element::RecursivelyCalculateVisibility() {
    // Calculate our own visibility.
    CalculateVisibility();

    // Notify our children.  If GetIsShown() returns false, then we don't
    // have to do anything, because that child will remain invisible for
    // its own reasons.  But if GetIsShown() returns true for a child, then
    // we will need to update its visibility, because its visibility
    // depends on ours.
    BOOST_FOREACH(ElementPtr elem, GetElements()) {
        if (IsChildForPurposeOfZOrderAndVisibility(elem) && elem->GetIsShown())
            elem->RecursivelyCalculateVisibility();
    }
}

bool Element::IsVisible() {
    return HasVisibleRepresentation() && mIsVisible;
}

bool Element::GetIsShown() {
    return HasVisibleRepresentation() && mIsShown;
}

void Element::SetIsShown(bool inIsShown) {
    if (GetIsShown() != inIsShown) {
        // If we're trying to show a permanently invisible element, raise
        // an exception.
        if (!HasVisibleRepresentation() && inIsShown)
            OperationNotSupported("show");

        // Update mIsShown.
        mIsShown = inIsShown;

        // Now that we've updated IsShown, we need to make sure that we
        // update the visibility of our child elements correctly.
        RecursivelyCalculateVisibility();
    }
}


//=========================================================================
//  Compositing
//=========================================================================

void Element::RecursivelyInvalidateCompositing() {
    InvalidateCompositing();
    BOOST_FOREACH(ElementPtr elem, GetElements())
        if (IsChildForPurposeOfZOrderAndVisibility(elem))
            elem->RecursivelyInvalidateCompositing();
}


//=========================================================================
//  Registration and unregistration
//=========================================================================

void Element::Register() {
    ElementPtr as_shared(shared_from_this(), dynamic_cast_tag());
    ASSERT(as_shared);
    GetParent()->RegisterChildElement(as_shared);
    Node::Register();      // Do this after our parent knows about us.

    // Fix mIsVisible now that we're hooked up (and have access to our
    // virtual methds), but before we have any child elements (so we don't
    // mess up their visibility calculations).
    ASSERT(GetElements().empty());
    CalculateVisibility();
}

void Element::Unregister() {
    Node::Unregister(); // Do this while our parent still knows about us.
    ElementPtr as_shared(shared_from_this(), dynamic_cast_tag());
    ASSERT(as_shared);
    GetParent()->UnregisterChildElement(as_shared);
    if (HasLegacyZOrderAndVisibility())
        wxGetApp().GetStage()->UnregisterLegacyZOrderAndVisibility(as_shared);
}


//=========================================================================
//  Other member functions
//=========================================================================

void Element::MoveTo(const wxPoint &inPoint) {
    OperationNotSupported("move");
}

