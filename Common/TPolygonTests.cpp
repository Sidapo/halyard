// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#include "TCommon.h"
#include "TPoint.h"
#include "TRect.h"
#include "TPolygon.h"
#include "ImlUnit.h"

USING_NAMESPACE_FIVEL

extern void test_TPolygon (void);

void test_TPolygon (void)
{
  // Setup our polygons
	std::vector<TPoint> convex_pts, concave_pts, intersect_pts, rect_pts,
		empty_pts, single_pts, line_pts, intersect2_pts;
	
	convex_pts.push_back(TPoint(1, 2));
	convex_pts.push_back(TPoint(1, 3));
	convex_pts.push_back(TPoint(2, 4));
	convex_pts.push_back(TPoint(3, 4));
	convex_pts.push_back(TPoint(4, 3));
	convex_pts.push_back(TPoint(4, 2));
	convex_pts.push_back(TPoint(3, 1));
	convex_pts.push_back(TPoint(2, 1));
	
	concave_pts.push_back(TPoint(1, 2));
	concave_pts.push_back(TPoint(2, 3));
	concave_pts.push_back(TPoint(4, 3));
	concave_pts.push_back(TPoint(3, 2));
	concave_pts.push_back(TPoint(4, 1));
	concave_pts.push_back(TPoint(2, 1));
	
	intersect_pts.push_back(TPoint(2, 1));
	intersect_pts.push_back(TPoint(2, 5));
	intersect_pts.push_back(TPoint(9, 1));
	intersect_pts.push_back(TPoint(0, 3));
	intersect_pts.push_back(TPoint(9, 5));

	rect_pts.push_back(TPoint(10, 10));
	rect_pts.push_back(TPoint(20, 10));
	rect_pts.push_back(TPoint(20, 20));
	rect_pts.push_back(TPoint(10, 20));
	
	single_pts.push_back(TPoint(0, 0));
	
	line_pts.push_back(TPoint(0, 0));
	line_pts.push_back(TPoint(2, 2));

	intersect2_pts.push_back(TPoint(3, 2));
	intersect2_pts.push_back(TPoint(3, 6));
	intersect2_pts.push_back(TPoint(10, 2));
	intersect2_pts.push_back(TPoint(1, 4));
	intersect2_pts.push_back(TPoint(10, 6));

	TPolygon convex(convex_pts), 
		concave(concave_pts), 
		intersect(intersect_pts),
		rect(rect_pts),
		empty(empty_pts),
		single(single_pts),
		line(line_pts),
		intersect2(intersect2_pts);

	intersect2.Offset(TPoint(-1, -1));

	TEST(convex.Bounds() == TRect(1, 1, 4, 4));
	TEST(concave.Bounds() == TRect(1, 1, 3, 4));
	TEST(intersect.Bounds() == TRect(1, 0, 5, 9));
	TEST(rect.Bounds() == TRect(10, 10, 20, 20));
	TEST(convex.Vertices() == convex_pts);
	TEST(concave.Vertices() == concave_pts);
	TEST(intersect.Vertices() == intersect_pts);
	TEST(rect.Vertices() == rect_pts);

	TEST(intersect2.Bounds() == intersect.Bounds());
	TEST(intersect2 == intersect);

	TEST(convex.Contains(TPoint(2, 2)));
	TEST(!convex.Contains(TPoint(0, 2)));
	TEST(!convex.Contains(TPoint(5, 2)));

	TEST(concave.Contains(TPoint(2, 2)));
	TEST(!concave.Contains(TPoint(1, 1)));
	TEST(!concave.Contains(TPoint(1, 3)));
	TEST(!concave.Contains(TPoint(4, 2)));
	
  	TEST(intersect.Contains(TPoint(6, 2)));
	TEST(intersect.Contains(TPoint(0, 3)));
	TEST(!intersect.Contains(TPoint(4, 3)));
	TEST(!intersect.Contains(TPoint(1, 2)));

	TEST(rect.Contains(TPoint(15, 15)));
	TEST(rect.Contains(TPoint(11, 10)));
	TEST(rect.Contains(TPoint(10, 10)));
	TEST(rect.Contains(TPoint(19, 19)));
	TEST(!rect.Contains(TPoint(10, 9)));
	TEST(!rect.Contains(TPoint(9, 10)));
	TEST(!rect.Contains(TPoint(20, 20)));

	TEST(!empty.Contains(TPoint(0, 0)));
	TEST(!empty.Contains(TPoint(1, 0)));
	TEST(!single.Contains(TPoint(0, 0)));
	TEST(!single.Contains(TPoint(1, 0)));
	TEST(!line.Contains(TPoint(0, 0)));
	TEST(!line.Contains(TPoint(1, 1)));
	TEST(!line.Contains(TPoint(2, 1)));
}
