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

// Pause in milliseconds after button is highlighted
#define HILITE_PAUSE 200

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
    itsPict = pict;
    secondCommand = secCmd;
	cursor = inCursor;

    if ((loc.X() == 0) and (loc.Y() == 0) and (pict != NULL))
		itsPictLoc = pict->GetOrigin();
    else
        itsPictLoc = loc;
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
	
			// brief pause needed to see hilited button (esp on fast machines)
			::Sleep(HILITE_PAUSE);

	        itsPict->Draw(itsPictLoc, true);
	        temp = itsText;
	        gHeaderManager.DoText(headerText, itsBounds, (const char *) temp,0,0);
        }
        else
		{
			// Douglas wants touchzones to stay highlighted, just like on the Mac.
			LPicture *hilitePict = itsPict->GetHilitePicture();
			if (hilitePict != NULL)
			{
				hilitePict->Draw(itsPictLoc, true);
				gView->Draw();

				// brief pause needed to see hilited button (esp on fast machines)
				::Sleep(HILITE_PAUSE);
			}
		}
    }

    if (not secondCommand.IsEmpty())
    {
		gDebugLog.Log("TouchZone hit: second command <%s>", secondCommand.GetString());
    	gCardManager.OneCommand(secondCommand);
    }
    
	gDebugLog.Log("TouchZone hit: command <%s>", itsCommand.GetString());
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
 Revision 1.2.2.3  2002/07/03 13:37:06  emk
 3.2.0.7, Wednesday, July 3, 2002, 2:53 PM

     Bug #980 revisted, again: The LTouchZone class supported
     touchzones with a second command OR a picture.  This has
     been fixed--you can now use both.

 Revision 1.2.2.2  2002/07/02 13:56:04  zeb
 3.2.0.5 - Changed touchzone highlighting to work like the Mac (bug #980).
 When you click on the touchzone, it draws the highlighted
 version of the graphic, waits briefly, and then executes the
 specified commands.  This involved a fix to LTouchZone.  We
 also fixed the argument parsing of (touch ...) to work like
 the Macintosh.

 Changes by Zeb and Eric.

 Revision 1.2.2.1  2002/03/13 15:06:56  emk
 Merged changed from 3.1.1 -> 3.2.1 into the 3.2.0.1 codebase,
 because we want these in the stable engine.  Highlights:

   1) FiveL.prefs file support.
   2) Removal of -D command line flag.

 Revision 1.3  2002/02/19 12:35:12  tvw
 Bugs #494 and #495 are addressed in this update.

 (1) 5L.prefs configuration file introduced
 (2) 5L_d.exe will no longer be part of CVS codebase, 5L.prefs allows for
     running in different modes.
 (3) Dozens of compile-time switches were removed in favor of
     having a single executable and parameters in the 5L.prefs file.
 (4) CryptStream was updated to support encrypting/decrypting any file.
 (5) Clear file streaming is no longer supported by CryptStream

 For more details, refer to ReleaseNotes.txt

 Revision 1.2  2002/01/23 20:39:20  tvw
 A group of changes to support a new stable build.

 (1) Only a single instance of the FiveL executable may run.

 (2) New command-line option "-D" used to lookup the installation directory in the system registry.
     Note: Underscores will be parsed as spaces(" ").
     Ex: FiveL -D HIV_Prevention_Counseling

 (3) Slow down the flash on buttpcx so it can be seen on
     fast machines.  A 200 mS pause was added.

 (4) Several bugfixes to prevent possible crashes when error
     conditions occur.

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
