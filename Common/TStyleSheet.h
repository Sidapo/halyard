// -*- Mode: C++; tab-width: 4; -*-

#ifndef TStyleSheet_H
#define TStyleSheet_H

#include <string>

#include "TStream.h"
#include "TIndex.h"

#include "GraphicsTools.h"
#include "Typography.h"


BEGIN_NAMESPACE_FIVEL

class TStyleSheet : public TIndex {
private:
	// (defstyle STYLENAME FONTNAME SIZE JUSTIFICATION COLOR HIGHCOLOR
	//           [LEADING [SHADOWOFFSET SHADOWCOLOR [SHADOWHIGHCOLOR]]])
	std::string          mStyleName;
	std::string          mFontName;
	int                  mSize;
	Typography::Justification mJustification;
	GraphicsTools::Color mColor;
	GraphicsTools::Color mHighlightColor;
	Typography::Distance mLeading;
	Typography::Distance mShadowOffset;
	GraphicsTools::Color mShadowColor;
	GraphicsTools::Color mHighlightShadowColor;

public:
	//////////
	// Create a new style sheet from a TIndexFile and a pair
	// of offsets.
	//
	TStyleSheet(TIndexFile *inFile, const char *inName,
				int32 inStart, int32 inEnd);
		
	//////////
	// Convert a 5L-format string into a StyledText object, using the
	// data stored in this style.
	//
	Typography::StyledText MakeStyledText(const std::string& inText);
	
	//////////
	// Draw text onto the specified image.
	//
	// [in] inText - The text to draw, with standard 5L formatting.
	// [in] inPosition - The upper-left corner of the text box.
	// [in] inLineLength - The maximum number of pixels available for
	//                     a line.  This is (I hope) a hard limit,
	//                     and no pixels should ever be drawn beyond it.
	// [in] inImage -      The image into which we should draw.
	//                     This must not be deallocated until the
	//                     TextRendering engine is destroyed.
	//
	void Draw(const std::string& inText,
			  GraphicsTools::Point inPosition,
			  GraphicsTools::Distance inLineLength,
			  GraphicsTools::Image *inImage);
};

class TStyleSheetManager : public TIndexManager {   
public:
	virtual void MakeNewIndex(TIndexFile *inFile, const char *inName,
							  int32 inStart, int32 inEnd);
};

END_NAMESPACE_FIVEL

#endif // TStyleSheet_H
