// -*- Mode: C++; tab-width: 4; -*-

#ifndef GraphicsTools_H
#define GraphicsTools_H

#include "TCommon.h"

#include <assert.h>

//////////
// Portable graphics primitives.  Over time, it might be good to refactor
// more code into this library.
//
namespace GraphicsTools {

	//////////
	// A distance in pixels.
	//
	typedef signed long Distance;

	//////////
	// An 8-bit color channel.
	//
	typedef unsigned char Channel;

	//////////
	// A point.
	//
	struct Point {
		Distance x;
		Distance y;

		Point(Distance inX, Distance inY) : x(inX), y(inY) {}
	};

	//////////
	// A 24-bit color with an 8-bit alpha channel.
	//
	struct Color {
		Channel red;
		Channel green;
		Channel blue;
		Channel alpha;

		Color() {}
		Color(Channel inRed, Channel inGreen,
			  Channel inBlue, Channel inAlpha = 0)
			: red(inRed), green(inGreen), blue(inBlue), alpha(inAlpha) {}
	};

	//////////
	// An RGBA pixmap, used as a portable output buffer by a number
	// of different graphics-related subsystems (i.e., Typography).
	// This class exposes a lot of 'const' member variables to
	// support high-speed drawing routines.
	//
	class Pixmap {
	private:
		Pixmap &operator=(const Pixmap &inPixmap)
		    { ASSERT(false); return *this; }
		Pixmap(const Pixmap &inPixmap)
			{ ASSERT(false); }

	public:
		//////////
		// The display width of the pixmap, in pixels. 
		//
		const Distance width;

		//////////
		// The height of the pixmap, in pixels. 
		//
		const Distance height;

		//////////
		// The memory width of the pixmap, in pixels. 
		//
		const Distance pitch;

		//////////
		// The actual pixmap.  To access the pixel at a given x,y
		// co-ordinate, use 'pixmap->pixels[x + bitmap->pitch * y]'.
		// If you want some error-checking, it might be better
		// to call 'At' (below).
		//
		Color *const pixels;		

	public:
		//////////
		// Create a pixmap.
		//
		Pixmap(Distance inWidth, Distance inHeight);

		//////////
		// Destroy a pixmap.
		//
		~Pixmap();

		//////////
		// Clear the pixmap to the specified color.
		//
		void Clear(Color inColor = Color(255,255,255,255));

		//////////
		// Access the pixel at (inX,inY).  You can use this function
		// on the left-hand-side of an assignment.
		// 
		Color &At(int inX, int inY)
		{
			ASSERT(inX >= 0 && inX < width);
			ASSERT(inY >= 0 && inY < height);
			return pixels[inX + pitch * inY];
		}

		//////////
		// Access the pixel at 'inPoint'.  You can use this function
		// on the left-hand-side of an assignement.
		Color &At(Point inPoint)
		{
			return At(inPoint.x, inPoint.y);
		}

	};

	//////////
	// An abstract class representing a drawing area.  No pixel-by-pixel
	// access is available; for that, create pixmaps and call DrawPixmap.
	//
	class Image {
	public:
		Image() {}
		virtual ~Image() {}

		//////////
		// Draw a pixmap at the specified point.
		//
		virtual void DrawPixmap(Point inPoint, Pixmap &inPixmap) = 0;
	};
}

#endif // GraphicsTools_H
