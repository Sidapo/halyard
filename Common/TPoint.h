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

#if !defined (_TPoint_h_)
#define _TPoint_h_

BEGIN_NAMESPACE_FIVEL

//////////
/// A class to represent a point in 2D space.
///
/// \author Chuck Officer
/// \author ...and others
///
class TPoint 
{ 
    public:
		//////////
		// Constructor.
		//
		// [in_optional] inX - set the X-coordinate (default 0)
		// [in_optional] inY - set the y-coordinate (default 0)
		//
		TPoint(int32 inX = 0, int32 inY = 0);

		//////////
		// Copy Constructor.
		//
		// [in] inPt - a TPoint to copy from
		//
		TPoint(const TPoint &inPt);

		//////////
		// Set values for the point.
		//
		// [in] inX - set the X-coordinate
		// [in] inY - set the y-coordinate
		//
		void			Set(int32 inX, int32 inY);

		//////////
		// Set values for the point using another TPoint.
		//
		// [in] inPt - TPoint to copy values from
		//
		void			Set(const TPoint &inPt);

		//////////
		// Offset the point using another TPoint.
		//
		// [in] inPt - TPoint used to offset values for this point
		//
		void			Offset(TPoint &inPt);

		//////////
		// Set the X-coordinate.
		//
		// [in] inX - the X-coordinate
		//
		inline void		SetX(int32 inX) { m_X = inX; }
		
		//////////
		// Set the Y-coordinate.
		//
		// [in] inY - the Y-coordinate
		//
		inline void		SetY(int32 inY) { m_Y = inY; }

		//////////
		// Offset the X-coordinate.
		//
		// [in] inXOffset - offset for the X-coordinate
		//
		inline void		OffsetX(int32 inXOffset) { m_X += inXOffset; }
		
		//////////
		// Offset the Y-coordinate.
		//
		// [in] inYOffset - offset for the Y-coordinate
		//
		inline void		OffsetY(int32 inYOffset) { m_Y += inYOffset; }
		
		//////////
		// Get the X-coordinate
		//
		// [out] return - the X-coordinate
		//
		inline int32	X(void) const { return (m_X); }
		
		//////////
		// Get the Y-coordinate
		//
		// [out] return - the Y-coordinate
		//
		inline int32	Y(void) const { return (m_Y); }

		//////////
		// Set values for this point using another TPoint.  Same as Set().
		//
		// [in] inPt - TPoint to copy values from (r-value)
		// [out] return - l-value with coordinates set to r-value
		//
		TPoint			&operator=(const TPoint &inPt);
		
		//////////
		// Equality check.
		//
		// [in] inPt - a TPoint to check against for equality
		// [out] return - true if the two points are equal, false otherwise
		//
		bool			operator==(const TPoint &inPt) const;

	protected:
        //////////
		// X-Coordinate
		//
		int32		m_X;
		
		//////////
		// Y-Coordinate
		//
		int32		m_Y;
};

inline std::ostream &operator<<(std::ostream &out, const TPoint &p) {
	out << "(point " << p.X() << " " << p.Y() << ")";
	return out;
}

END_NAMESPACE_FIVEL

#endif // _TPoint_h_
