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

#include "TamaleHeaders.h"
#include "Overlay.h"
#include "CommonWxConv.h"
#include "TPoint.h"
#include "Stage.h"

USING_NAMESPACE_FIVEL

Overlay::Overlay(Stage *inStage, const wxString &inName,
		 const wxRect &inBounds, FIVEL_NS TCallbackPtr inDispatch,
		 wxCursor &inCursor, bool inHasAlpha)
    : LightweightElement(inStage, inName, inDispatch, inCursor),
      mDrawingArea(inStage, inBounds, inHasAlpha)
{
}

void Overlay::Show(bool inShow) {
    mDrawingArea.Show(inShow);
    LightweightElement::Show(inShow);
}

bool Overlay::IsPointInElement(const wxPoint &inPoint) {
	wxRect bounds = mDrawingArea.GetBounds();
    if (!bounds.Inside(inPoint)) {
		// Outside our bounding box.
		return false; 
	} else if (!mDrawingArea.HasAlpha()) {
		// We're opaque, so we only need to check the bounding box.
		return true;
	} else {
		// We have an alpha channel.  We only consider points to be inside
		// the overlay if they contain graphical data.
		GraphicsTools::Color c =
			mDrawingArea.GetPixel(inPoint.x - bounds.x, inPoint.y - bounds.y);
		return !c.IsCompletelyTransparent();
	}
}

void Overlay::MoveTo(const wxPoint &inPoint) {
    mDrawingArea.MoveTo(inPoint);
}

void Overlay::DrawElementBorder(wxDC &inDC) {
    inDC.DrawRectangle(mDrawingArea.GetBounds());
}

void Overlay::CompositeInto(wxDC &inDC, const wxRect &inClipRect) {
    mDrawingArea.CompositeInto(inDC, inClipRect);
}
