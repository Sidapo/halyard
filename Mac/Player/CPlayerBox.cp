/* =================================================================================
	CPlayerBox.cp	
   ================================================================================= */

#include <iostream>
#include <Palettes.h>
#include <LStream.h>
#include <UDrawingState.h>

#include "Mac5L.h"
#include "CPlayerView.h"
#include "CPlayerBox.h"


/* ---------------------------------------------------------------------------------
		� CPlayerBox(&Rect, Fill, LIneWidth, ColorIdx)
		
		Constructor for a custom Player Pane class. Sets up a SPaneInfo struct to
		call the custom pane's constructor.
   --------------------------------------------------------------------------------- */

CPlayerBox::CPlayerBox(
	const Rect		&inBounds,		// Bouding rect (relative to PlayerView's rect)
	const Boolean 	inFill,			// TRUE if rect to be filled (solid)
	const int8		inLineWidth,	// Frame's line width, in pixels
	const int16		inColorIdx)		// Index into CLUT
{
	// Assign our data.
	mFill 		= inFill;
	mLineWidth 	= inLineWidth;
	mColorIdx 	= inColorIdx;
	mBounds 	= inBounds;
	
	// Get the offscreen gworld from the card view, and draw into it.
	CGWorld *theGWorld;
	theGWorld = gPlayerView->GetGWorld();
	theGWorld->BeginDrawing();		// Draw to offscreem GWorld
	DrawSelf();
	theGWorld->EndDrawing();
	
//	::InvalRect(&mBounds);
}

// ---------------------------------------------------------------------------------
//		� ~CPlayerBox
// ---------------------------------------------------------------------------------

CPlayerBox::~CPlayerBox()
{
}

// ---------------------------------------------------------------------------------
//		� DrawSelf
// ---------------------------------------------------------------------------------

void
CPlayerBox::DrawSelf()
{
	StColorPenState savePenState;
	StColorPenState::Normalize();
	
	// Set the pen state for color, width. Get the rgb color from
	// the color index.
	::PmForeColor(mColorIdx);
	::PenSize(mLineWidth, mLineWidth);
	
	// If 'mFill' is TRUE, we want to fill the box. Otherwise, just frame it.
	if (mFill)
	{
		::PenMode(patCopy);
		::PaintRect(&mBounds);
	}
	else
		::FrameRect( &mBounds );
}
