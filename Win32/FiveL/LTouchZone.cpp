//////////////////////////////////////////////////////////////////////////////
//
//   (c) Copyright 1999, Trustees of Dartmouth College, All rights reserved.
//        Interactive Media Lab, Dartmouth Medical School
//
//			$Author$
//          $Date$
//          $Revision$
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// LTouchZone.cpp : 
//

#include "stdafx.h"

#include "LTouchZone.h"
#include "Globals.h"

#define BUTTONXSHIFT 2
#define BUTTONYSHIFT 3
#define ACTIVATED 8
#define NOT_ACTIV 7
#define BUTT_TXT_MAX    256

//
//  Initialize the touch zone. If no location point is given, use
//  the picture's.
//
LTouchZone::LTouchZone(TRect &r, TString &cmd, CursorType inCursor, 
					   LPicture *pict, TPoint &loc) : TObject()
{
    itsBounds = r;
    itsCommand = cmd;
    itsPict = pict;
 	cursor = inCursor;

    if ((loc.X() == 0) and (loc.Y() == 0) and (pict != NULL))
		itsPictLoc = pict->GetOrigin();
    else
        itsPictLoc = loc;
}

// used for buttpcx
LTouchZone::LTouchZone(TRect &r, TString &cmd, CursorType inCursor, 
					   LPicture *pict, TPoint &loc, const char *text) : TObject()
{
    itsBounds = r;
    itsCommand = cmd;
    itsPict = pict;
	itsText = text;
	cursor = inCursor;
 
    if ((loc.X() == 0) and (loc.Y() == 0) and (pict != NULL))
		itsPictLoc = pict->GetOrigin();
    else
        itsPictLoc = loc;
}

// used for buttpcx
LTouchZone::LTouchZone(TRect &r, TString &cmd, CursorType inCursor, 
					   LPicture *pict, TPoint &loc, const char *text, 
					   TString &header) : TObject()
{
    itsBounds = r;
    itsCommand = cmd;
    itsPict = pict;
    itsText = text;
    headerText = header;
	cursor = inCursor;

    if ((loc.X() == 0) and (loc.Y() == 0) and (pict != NULL))
		itsPictLoc = pict->GetOrigin();
    else
        itsPictLoc = loc;
}

// used for buttpcx
LTouchZone::LTouchZone(TRect &r, TString &cmd, CursorType inCursor, 
					   LPicture *pict, TPoint &loc, const char *text, TString &header, 
					   TString &secCmd) : TObject()
{
    itsBounds = r;
    itsCommand = cmd;
    itsPict = pict;
    itsText = text;
    headerText = header;
    secondCommand = secCmd;
	cursor = inCursor;

    if ((loc.X() == 0) and (loc.Y() == 0) and (pict != NULL))
		itsPictLoc = pict->GetOrigin();
    else
        itsPictLoc = loc;
}

LTouchZone::LTouchZone(TRect &r, TString &cmd, CursorType inCursor, 
					   LPicture *pict, TPoint &loc, TString &secCmd) : TObject()
{
    itsBounds = r;
    itsCommand = cmd;
    itsPict = NULL;
    secondCommand = secCmd;
	cursor = inCursor;
}


LTouchZone::~LTouchZone()
{
}    

char LTouchZone::FirstChar()
{
	if (not itsText.IsEmpty())
		return (itsText(0));
	return (0);
}

/***********************************************************************
 * Function: LTouchZone::DoCommand
 *
 *  Parameter (null)
 * Return:
 *
 * Comments:
 *  Execute the touch zone's single or double command.
 *  Hilite the associated picture if there is one, and same with TEXT, if any.
 *  Hilite picture always matted and always drawn directly to
 *  the screen.
 ***********************************************************************/
void LTouchZone::DoCommand()
{
    TString     header;
    TString     temp;
    TRect        bounds1;
    LPicture     *itsPictHilite;
    int         dl, fontHeight;

    bounds1.Set(itsBounds);

    if (itsPict != NULL)  
    {
        if (not itsText.IsEmpty())  
        {
	        temp = "^";
	        temp += itsText;              //Color Change.
	        temp += "^";
	
	        itsPictHilite = itsPict->GetHilitePicture();  //Hilited button...
	        itsPictHilite->Draw(itsPictLoc, true);
	
	        dl = itsBounds.Bottom() - itsBounds.Top();      //...and text...
	        fontHeight = gHeaderManager.Height((const char *) headerText);
	        dl -= fontHeight;
	        dl /= 2;
			itsBounds.OffsetTop(dl);
			itsBounds.OffsetBottom(dl);
	        gHeaderManager.DoText(headerText, itsBounds, (const char *) temp,0,0);
	        
	        gView->Draw();		// blast the hilited stuff to the screen
	
	        //nap(300,0);             //And redraw+text.
	        itsPict->Draw(itsPictLoc, true);
	        temp = itsText;
	        gHeaderManager.DoText(headerText, itsBounds, (const char *) temp,0,0);
        }
        else 
        	itsPict->Hilite(itsPictLoc, true); 
    }

    if (not secondCommand.IsEmpty())
    {
#ifdef _DEBUG
		gDebugLog.Log("TouchZone hit: second command <%s>", secondCommand.GetString());
#endif 
    	gCardManager.OneCommand(secondCommand);
    }
    
#ifdef _DEBUG
	gDebugLog.Log("TouchZone hit: command <%s>", itsCommand.GetString());
#endif
    gCardManager.OneCommand(itsCommand);
    
    gView->Draw();		// the command executed might have changed something
}

//
//	LTouchZoneManager methods.
//

//
//	LTouchZoneManager - Construct the touch zone list.
//
LTouchZoneManager::LTouchZoneManager() : TArray()
{
}

//
//	RemoveAll
//
void LTouchZoneManager::RemoveAll(void)
{
	Clear();
}

//
//	Clear - Clear all touchzones.
//
void LTouchZoneManager::Clear(void)
{
	DeleteAll();
}

//
//	Clear - Clear the touch zone at left and top (if any). Only clear
//		the first one you come to.
//
void LTouchZoneManager::Clear(int left, int top)
{ 
	LTouchZone	*theZone;
    int32 		i;
    bool		done = false;
    
    i = 0;
    while ((not done) and (i < NumItems()))
    {
    	theZone = (LTouchZone *) Item(i);
    	
    	if (theZone != NULL)
    	{
			if ((theZone->GetBounds().Left() == left) and
				(theZone->GetBounds().Top() == top))
     		{
    			DeleteIndex(i);
    			done = true;
    		}
    	}
    	
    	i++;
    }
}

//
//	GetTouchZone - Return the touch zone where the given point hit (if any).
//		Search the array from the end so that later defined zones are
//		on top.
//
LTouchZone *LTouchZoneManager::GetTouchZone(TPoint &where)
{
	LTouchZone	*theZone;    
	int32		i;
    

 	for (i = NumItems() - 1; i >= 0; i--)
    {
    	theZone = (LTouchZone *) Item(i);
        
        if (theZone->Hit(where))
            return (theZone);
    }

    return (NULL);
}

//
//	GetTouchZone - Return a LTouchZone if:
//		1. the enter key was pressed and there is only one zone
//		2. the first key of the text was pressed		
//
LTouchZone *LTouchZoneManager::GetTouchZone(WPARAM wParam)
{
	int32		i;
	LTouchZone	*theZone;
	
	if ((NumItems() == 1) and (wParam == 13))
		return ((LTouchZone *) Item(0));
	else
	{
		for (i = 0; i < NumItems(); i++)
		{
			theZone = (LTouchZone *) Item(i);
			
			if ((theZone->HasText()) 
			and (tolower((char) wParam) == tolower(theZone->FirstChar())))
			{
				return (theZone);		
            }
		}
	}
	
	return (NULL);
}

/*
 $Log$
 Revision 1.1  2001/09/24 15:11:01  tvw
 FiveL v3.00 Build 10

 First commit of /iml/FiveL/Release branch.

 There are now seperate branches for development and release
 codebases.

 Development - /iml/FiveL/Dev
 Release - /iml/FiveL/Release

 Revision 1.4  2000/04/07 17:05:16  chuck
 v 2.01 build 1

 Revision 1.3  1999/10/22 20:29:09  chuck
 New cursor management.

 Revision 1.2  1999/09/24 19:57:19  chuck
 Initial revision

*/
