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

#ifndef Zone_H
#define Zone_H

#include "LightweightElement.h"
#include "TPolygon.h"


//////////
/// A zone is the simplest form of lightweight element.
///
class Zone : public LightweightElement
{
	TPolygon mPolygon;
	
public:
	Zone(Stage *inStage, const wxString &inName, const TPolygon &inPoly,
		 FIVEL_NS TCallbackPtr inDispatch, wxCursor &inCursor);

	virtual bool IsPointInElement(const wxPoint &inPoint);

	virtual void DrawElementBorder(wxDC &inDC);
};

#endif // Zone_H
