// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#if !defined (_TPolygon_h_)
#define _TPolygon_h_

#include "TCommon.h"

#include "TObject.h"
#include "TPoint.h"
#include "TRect.h"

#include <vector>

BEGIN_NAMESPACE_FIVEL

//////////
// A class representing a closed polygon.
//
class TPolygon : public TObject
{
	std::vector<TPoint> mVertices;
	TRect mBounds;

public:
	//////////
	// An iterator for traversing the edges of the polygon.
	// XXX - not yet implemented! Should be implemented!
	//
// 	class edge_iterator
// 	{
// 		int mPosition;
// 		TPolygon mPolygon;

// 	public:
// 		edge_iterator(TPolygon inPoly) : mPosition(0), mPolygon(inPoly) {}
// 		TPolygon &operator*()

	//////////
	// Create a new polygon.
	//
	// [in] inVertices - The vertices of the polygon.
	// 
	TPolygon(std::vector<TPoint> &inVertices);
	TPolygon() : mVertices(), mBounds() {}

	//////////
	// Determine if a point is in the polygon.
	// 
	// [in] inPt - The point to check.
	// 
	bool Contains(TPoint &inPt);
	
	//////////
	// Offset the polygon.
	// 
	// [in] inPt - Point to be used as the offset.
	// 
	void Offset(const TPoint &inPt);

	//////////
	// Get a rectangle which completely contains the polygon.
	// 
	const TRect Bounds() const { return mBounds; }
	
	//////////
	// Get the points which define the polygon.
	// 
	const std::vector<TPoint> Vertices() const { return mVertices; }

	//////////
	// Test if two polygons are equal.
	// 
	bool operator==(const TPolygon &inPoly) const;
};

END_NAMESPACE_FIVEL

#endif // _TPolygon_h_
