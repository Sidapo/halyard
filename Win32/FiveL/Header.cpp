// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-
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
// Header.cpp : Old, Windows-specific text drawing code.  This file is
//              obsolete, and is only kept for backwards-compatibility
//				with older 5L programs.  All new programs and features
//				should use TStyleSheet instead.
//

#include "stdafx.h"
#include "TCommon.h"
#include "LUtil.h"
#include "Header.h"
#include "Globals.h"
#include "TEncoding.h"
#include "TCommonPrimitives.h"

HeaderManager gHeaderManager;

//  Build the header.  Colors are allegedly checked to match InfoWindows
//  standards (EGA + MIC restrictions...), whatever that means.
//
Header::Header(TArgumentList &inArgs)
{
    itsAlign = AlignLeft;
    itsColor = itsHighlightColor = itsShadowColor = itsShadHighColor = 0;
    itsShadow = 0;
    itsOffset = 0;

    TString     align, fontname;

    // HNAME FONTNAME...
    //
    inArgs >> mName >> fontname;
	mName = MakeStringLowercase(mName);
	fontname.MakeLower();
    
    itsFont = gFontManager.GetFont(fontname); //creates a resource if dne, and loads font
    //  ...ALIGNMENT COLOR HIGHCOLOR...
    inArgs >> align >> itsColor >> itsHighlightColor; 

    align.MakeLower();
	if (align.Equal("center"))
        itsAlign = AlignCenter;
    else if (align.Equal("right"))
        itsAlign = AlignRight;
    else
        itsAlign = AlignLeft;

    //  ...SHADOW SHADCOLOR
    if (inArgs.HasMoreArguments()) 
    {
        inArgs >> itsShadow >> itsShadowColor;
    }
    
    //@@@9-19-96 to handle font differences bet. Win. & Mac.
    if (inArgs.HasMoreArguments()) 
    {
        inArgs >> itsOffset;
    }
    
    if (inArgs.HasMoreArguments()) 
    {
        inArgs >> itsShadHighColor;
    }
}

/***********************************************************************
 * Function: Header::Prep
 *
 *  Parameter (null)
 * Return:
 *
 * Comments:
 *      Prepare header for text printing
 ***********************************************************************/
void Header::Prep()
{
    itsFont->Set();
    itsFont->SetColor(itsColor);
}

/***********************************************************************
 * Function: HeightHeader
 *
 *  Parameter (null)
 * Return:
 *  Returns text font height (protected in the TXHEADER Font structure.)
 * Comments:
 *
 ***********************************************************************/
int Header::HeightHeader()
{
    return itsFont->Height();
}

char    *vval = new char[10];
//for return value

/***********************************************************************
 * Function: Draw
 *
 *  Parameter bounds    (where to stick it in)
 *  Parameter inText    (what to stick in)
 *  Parameter color     (what color)
 *  Parameter shadow    (what shadow displacement)
 * Return:
 *
 * Comments:
 *      Bigtime major important text drawing method...
 *  The text should be space-delimited. Special characters may
 *  be embedded.  Does on-the-fly text formatting to fit text in "bounds"
 *      Also maintains the _incr_y 5L GLOBAL to reflect the last coord.
 *  of printed text (for bullet drawing)
 ***********************************************************************/

// Helper function.
// TODO - Refactor back into TEncoding (somehow) once logging code is merged between
// Macintosh and Windows engines.
static void LogEncodingErrors (const std::string &inBadString,
							   size_t inBadPos, const char *inErrMsg)
{
	gLog.Caution("ENCODING WARNING: %s at position %d in string <<%s>>.",
		inErrMsg, inBadPos, inBadString.c_str());
}

void Header::Draw(TRect &bounds, char *inText, int color, int Shadow)
{
	HDC		hDC;
	HFONT	hOldFont;
	TRect	dirty_rect;
    TPoint  loc(bounds.Left(), bounds.Top());
    int     maxWidth = bounds.Right() - bounds.Left();
    int     pixWidth;
	int		text_width = 0;
	int		incr_x;
    long    lineStart;
    long    index = 0;
    bool	first_line = true;		// only use offset for first line

    ASSERT(itsFont);
    Prep();
    fHilite = fUnderline = false;

	// Encode our string for Windows.
	TEncoding<char> encoding("windows-1252", &LogEncodingErrors);
	std::string encodedString = encoding.TransformString(inText);
	const char *text = encodedString.c_str();
    long tLen = encodedString.length();

	incr_x = bounds.Left();
    
    // set our dirty rectangle
	dirty_rect = bounds;
    
	//use slow one
	{
		//First, set up device context:
		hDC = gView->GetDC();
	 	   	
	    hOldFont = (HFONT) ::SelectObject(hDC, itsFont->GetFontHandle());
	    
	    ::SetBkMode(hDC, TRANSPARENT);

	    //  Loop until all the text is drawn.
	    while (index < tLen) 
	    {
	        //  How much should we draw on this line?
	        //
	        lineStart = index;
	        pixWidth = GetLineLength(text, &index, tLen, maxWidth);
	
	        //  Set starting point for drawing based on alignment.
	        //
	        switch (itsAlign) 
	        {
	            case AlignLeft:
	                loc.SetX(bounds.Left());
	                break;
	            case AlignRight:
	                loc.SetX(bounds.Right() - pixWidth);
	                break;
	            case AlignCenter:
	                loc.SetX((bounds.Left() + bounds.Right() - pixWidth) >> 1);
	                break;
	        }
	
	        //  Draw the text.
	        //
	        
	        //itsFont->SetColor(color);
	        // Color support for text command removed. Color set in header.
	        
	        text_width = DrawLine(loc, text, lineStart, index);
			incr_x = max(incr_x, text_width);
	
	        //  Bump the y coordinate for the next line.
	        loc.OffsetY(itsFont->Height());

	        
	        //  Don't bother drawing text once we hit the bottom of the screen.
	        //
	        if (loc.Y() > 480)  
				goto end;
	    }
	    
		// reset stuff
		::SetBkMode(hDC,OPAQUE);
		::SelectObject(hDC, hOldFont);
	}

end:
	::UpdateSpecialVariablesForText(TPoint(incr_x, loc.Y()));
    
    // make sure the dirty rect extends down to cover characters below the
    // text line - use g as an example (should really see if there are chars
    // that are below the line - and use the right one)
    dirty_rect.SetBottom(loc.Y() + (itsFont->CharHeight('g') - itsFont->Height()) + 2);
	dirty_rect.SetRight(incr_x);

    gView->DirtyRect(&dirty_rect);
}

/*  SPECIAL CHARACTERS
    \t      Tab.
    \n      Newline. Hard return.
	\w		Newline (windows only)
    \*      Literals. Examples include \(, \), \$, \;, \`, \', \\
    ^       Hilite toggle.
    |       Underline toggle.
    `,'     Single smart quotes. (ASCII 212, 213)
    ``, ''  Double smart quotes. (ASCII 210, 211)

    Smart quotes work only if we're using a Mac based font that has
    smart quotes at the proper ASCII locations.
*/

/***********************************************************************
 * Function: Header::GetLineLength
 *
 *  Parameter s         (to print)
 *  Parameter index     (current position in string -- GETS MODIFIED!)
 *  Parameter tLen      (final position)
 *  Parameter maxWidth  (determined by the box & font size)
 * Return:
 *        How many chars should be printed.
 * Comments:
 *   Determine how many characters should be drawn on the line
 *  and the pixel width of that line.
 *  ALWAYS progress at least one character into the string!
 *  This routine must progress or we may loop forever...
 *  Does some weird stuff with SmartQuotes, check out the case '`': etc..
 ***********************************************************************/
int Header::GetLineLength(const char *s, long *index, long tLen, int maxWidth)
{
    long            pos = *index;
    long            wordBreak = *index;
    unsigned char   ch;
    int             pixWidth = 0;
    int             pixBreak = 0;
    int             pixLast = 0;
    int             done = (*index >= tLen);

    //  Skip any leading spaces. They should not figure into the width.
    //
    while (s[pos] == ' ')
        pos++;

    while (!done) 
	{
        ch = s[pos];
        switch (ch) 
		{

            case 0:             //  End of string.
                done = true;

            case '^':           //  Hilite char. Ignore.
            case '|':           //  Underline char. Ignore.
                pos++;  
                ch = 0;         //  Set to 0 so we don't count its width
                break;

            case '\\':          //  Special char marker.

                switch (ch = s[++pos]) 
				{
                    case 0:
					case 'w':	// newline (windows only)
                    case 'n':   //  End of string or line.

                        //  If last char was a space don't count it!
                        //
                        if (pos > 1 && s[pos - 2] == ' ')
                            pixWidth = pixBreak;

                        done = true;
                        ch = 0;
                        pos++;

                    default:
                        break;
                }
                break;

            case '\'':          //  Close quote.
                if (s[pos + 1] == ch) 
				{
                    pos++;
                    ch = 211;		// 0xD3
                } 
				else 
					ch = 213;		// 0xD5
                break;

            case '`':           //  Open quote.
                if (s[pos + 1] == ch) 
				{
                    pos++;
                    ch = 210;		// 0xD2
                } 
				else 
					ch = 212;		// 0xD4
                break;

            case ' ':           //  Space.

                pixBreak = pixWidth;
                wordBreak = pos;
				break;

            default:
                break;
        }

        //  Figure spacing if the character exists.
        //
        if (ch != 0) 
		{

            pixLast = pixWidth;
            pixWidth += itsFont->CharWidth(ch);

            if (pixWidth > maxWidth) 
			{
                //  Always return one character in narrow rect.
                if (pos == *index)
                    pos++;
                //  If we have less than one word...
                else if (wordBreak == *index)
                    pixWidth = pixLast;
                //  Use the last word break.
                else 
				{
                    pixWidth = pixBreak;
                    pos = wordBreak + 1;    // plus the space
                }
                done = true;

            } 
			else 
				pos++;
        }
    }

    *index = pos;
    return pixWidth;

    return 0;

}

/***********************************************************************
 * Function: Header::DrawLine
 *
 *  Parameter loc  (where to start)
 *  Parameter s    (what to print)
 *  Parameter a    (start at a in "s")
 *  Parameter b    (..and end in b)
 * Return:
 *		The actual width (in pixels) of the text that was output.
 *
 * Comments:
 *   Draw the number of characters given, starting at s.
 ***********************************************************************/
int Header::DrawLine(TPoint &loc, const char *s, long a, long b)
{
	HDC				hDC;
	COLORREF		theColor;
    TPoint			pt(loc);
    TPoint   		shad;
    unsigned char	ch;
	int				incr_x = loc.X();
    
    hDC = gView->GetDC();
 
	// See if we don't have anything special to do.
	//
	if (not (strstr(s, "^")		// no hilite
		or (strstr(s, "|"))		// no underline
		or (strstr(s, "\\"))	// no special characters
		or (strstr(s, "@"))))	// no bold
	{
	    //  Skip leading spaces. They should not be drawn.
	    //
		
	    while (s[a] == ' ')
	        a++;
	    
	    const char* tptr = &s[a];
	    
	    //find line end

        itsFont->SetUnderline(fUnderline);
	
        //  Shadow first.
        //
        if (itsShadow) 
        {
            shad.Set(pt.X() + itsShadow, pt.Y() + itsShadow);
            itsFont->SetColor(fHilite ? itsShadHighColor : itsShadowColor);
			
			theColor = gView->GetColor(itsFont->Color()); 
			
			::SetTextColor(hDC, theColor);
			::TabbedTextOut(hDC, shad.X(), shad.Y() + itsOffset, tptr, (int) (b-a-1), 0, NULL, 0);
        }
	
        //  Main text.
        //
        itsFont->SetColor(fHilite ? itsHighlightColor : itsColor);
        
        theColor = gView->GetColor(itsFont->Color());
        
		::SetTextColor(hDC, theColor);

		// set incr_x
		SIZE	textSize;

		if (::GetTextExtentPoint32(hDC, tptr, (int) (b-a-1), &textSize))
			incr_x += textSize.cx;

		::TabbedTextOut(hDC, pt.X(), pt.Y() + itsOffset, tptr, (int) (b-a-1), 0, NULL, 0);
	}
	else //slow way:
	{
	    //  Skip leading spaces. They should not be drawn.
	    //

	    pt.OffsetY(itsOffset); //@@@9-19-96 added offset

	    while (s[a] == ' ') 		// skip whitespace at start
	        a++;
	
	    while (a < b) 
		{
	
	        switch (ch = s[a]) 
	        {
	            case 0:             //  End of string.
	                return (incr_x);
	
	            case '^':           //  Hilite char.
	                a++;
	                fHilite = !fHilite;
	                ch = 0;             //  Set to 0 so we don't draw it.
	                
	                if (fHilite)				// just starting style text
	                {
	                	while (s[a] == ' ') 	// skip spaces
	                		a++; 
	                }
	                
	                break;
	
	            case '|':
	                a++;
	                fUnderline = !fUnderline;
	                ch = 0; 
	                
	                if (fUnderline)				// just starting style text
	                {
	                	while (s[a] == ' ')		// skip spaces
	                		a++;                              
	                }
	                
	                break;

	            case '@':
	                a++;
	                itsFont->SetBold(not itsFont->Bold());
	                ch = 0;
	                
	                if (itsFont->Bold())			// just starting style text
	                {
	                	while (s[a] == ' ') 	// skip spaces
	                		a++;
	                }
	                
	                break;
	
	            case '\\':          //  Special char marker.
	
	                switch (ch = s[++a]) 
					{
	                    case 0:
						case 'w':			// newline (windows only)
	                    case 'n':           //  End of string or line.
	                        return (incr_x);
	                    case 't':
	                    	ch = 9; //tab
							break;
						case 'm':			// newline (mac only)
							ch = 0;			// ignore on Windows
							break;
	                    default:
	                        break;
	                }
	                break;
	
	/*            case '\'':          //  Close quote.
	                if (s[a + 1] == ch) {
	                    a++;
	                    ch = 146;
	                } else ch = 180;
	                break;
	            case '`':           //  Open quote.
	                if (s[a + 1] == ch) {
	                    a++;
	                    ch = 210;
	                } else ch = 212;
	                break;
	*/
	           case '\'':          //  apostrophe.
	                 ch = 146;
	                 break;
	            default:
	                break;
	        }
	
	        //  Draw if the character exists.
	        //
	        if (ch != 0) 
	        {
	            //create new font, bold/underline if necessary: 
	            itsFont->SetUnderline(fUnderline); //this sets bold, too!!!
	             
	
	            //  Shadow first.
	            //
	            if (itsShadow) 
	            {
	                shad.Set(pt.X() + itsShadow, pt.Y() + itsShadow);
	                itsFont->SetColor(fHilite ? itsShadHighColor : itsShadowColor);
	                itsFont->Draw(shad, ch);
	            }
	
	            //  Main text.
	            //
	            itsFont->SetColor(fHilite ? itsHighlightColor : itsColor);
	            itsFont->Draw(pt, ch);
	
	            //  Move location.
	            //
	            if (ch == 9) 
					pt.OffsetX(3*itsFont->CharWidth(ch)); //tab widths
	            else 
					pt.OffsetX(itsFont->CharWidth(ch));
	            
				incr_x = pt.X();

	            a++;
	        }
	    }
	} 
	return (incr_x);
}

/*****************************

    HEADER MANAGER METHODS

*****************************/

Header *HeaderManager::Find(const std::string &inName)
{
	std::string name = MakeStringLowercase(inName);
    std::map<std::string,Header*>::iterator found =
		mHeaderMap.find(name);
    if (found != mHeaderMap.end())
		return found->second;
    else
		return NULL;
}

void HeaderManager::AddHeader(TArgumentList &inArgs)
{
	// Create the header and get the name.
	std::auto_ptr<Header> header =
		std::auto_ptr<Header>(new Header(inArgs));
	std::string name = header->GetName();

	// Check for an exiting header with the same name.
	if (Find(name))
	{
		gLog.Error("Can't redefine style header <%s>.", name.c_str());
		return;
	}

	// Insert the new header in our map.
	mHeaderMap.insert(std::pair<std::string,Header*>(name,
													 header.release()));
}

void HeaderManager::RemoveAll()
{
	// Delete the individual headers and empty the map.
	std::map<std::string,Header*>::iterator iter =
		mHeaderMap.begin();
	for (; iter != mHeaderMap.end(); ++iter)
		delete iter->second;
	mHeaderMap.clear();
}

/***********************************************************************
 * Function: HeaderManager::DoHeader
 *
 *  Parameter headername
 * Return:
 *
 * Comments:
 *      Set the font characteristics of the header in
 *  preparation of drawing.
 ***********************************************************************/
void HeaderManager::DoHeader(const char *headername)
{
    Header  *hdr;

    hdr = Find(headername);
    
    if (hdr == NULL)
    {
    	gLog.Log("Couldn't find header <%s>.", headername);
    	return;
    }
    
    hdr->Prep();
}

/***********************************************************************
 * Function: HeaderManager::DoText
 *
 *  Parameter header        (5L header name, not TX's)
 *  Parameter bounds        (rectangle to print)
 *  Parameter text
 * Return:
 *
 * Comments:
 *      Wrapper to call Header::Draw()...
 ***********************************************************************/
void HeaderManager::DoText(const char *header, TRect &bounds, const char *text, int color, int shadow)
{
    Header  *hdr;
    
    hdr = Find(header);
    
    if (hdr == NULL)
    {
    	gLog.Log("Couldn't find header <%s>.", header);
    	return;
    }
    
    hdr->Draw(bounds, (char *)text, color, shadow);
}

/***********************************************************************
 * Function: HeaderManager::Height
 *
 *  Parameter header
 * Return:
 *
 * Comments:
 *      Just because Height is protected in Font...
 ***********************************************************************/
int HeaderManager::Height(const char* header)
{
    Header  *hdr;

    hdr = Find(header);
    
    if (hdr == NULL)
    {
    	gLog.Log("Couldn't find header <%s>.", header);
    	return(0);
    }

    hdr->Prep();
    return (hdr->HeightHeader());
}

/*
 $Log$
 Revision 1.11  2002/10/03 18:05:40  emk
 Comment fix.

 Revision 1.10  2002/08/22 00:12:22  emk
 3.5.4 - 21 Aug 2002 - emk

 Engine:

   * Moved many source files from Common to Common/lang/old5L, and from
     Win32/FiveL to Win32/FiveL/lang/old5l, including the index system, the
     parser and stream classes, the crypto classes and the file I/O classes.
   * Broke the dependencies between Header and TIndex, in a fashion similar
     to what I did for TStyleSheet in 3.5.1.  This means we can call
     INPUT from Scheme, which more-or-less completes the Scheme primitives.
   * Made sure that header and stylesheet names were case insensitive.

 Revision 1.9  2002/08/17 01:42:12  emk
 3.5.1 - 16 Aug 2002 - emk

 Added support for defining stylesheets in Scheme.  This means that Scheme
 can draw text!  (The INPUT doesn't work yet, because this relies on the
 separate, not-yet-fixed header system.)  This involved lots of refactoring.

   * Created TTopLevelFormProcessor as an abstract superclass of
     TIndexManager, and modified TParser to use TTopLevelFormProcessor.
     This allows the legacy 5L language to contain non-TIndex tlfs.
   * Implemented a TPrimitiveTlfProcessor class, which allows
     top-level-forms to be implemented as calls to regular 5L primitives.
   * Yanked our ValueOrPercent support from TStream into the
     TArgumentList superclass, and implemented it for all TArgumentList
     subclasses.  This allows non-5L languages to specify the funky
     percentage arguments used by the DEFSTYLE command.
   * Removed all TIndex/TIndexManager support from TStyleSheet, and
     reimplemented it using an STL std::map.  This breaks the dependencies
     between stylesheets and the old 5L interpreter.
   * Implemented a DEFSTYLE primitive.

 Revision 1.8  2002/06/24 15:27:29  emk
 3.3.9 - Highly experimental engine which makes _INCR_X,
 _INCR_Y, _Graphic_X and _Graphic_Y relative to the current
 origin.   This will break macros in existing code!

 Revision 1.7  2002/06/20 16:32:54  emk
 Merged the 'FiveL_3_3_4_refactor_lang_1' branch back into the trunk.  This
 branch contained the following enhancements:

   * Most of the communication between the interpreter and the
     engine now goes through the interfaces defined in
     TInterpreter.h and TPrimitive.h.  Among other things, this
     refactoring makes will make it easier to (1) change the interpreter
     from 5L to Scheme and (2) add portable primitives that work
     the same on both platforms.
   * A new system for handling callbacks.

 I also slipped in the following, unrelated enhancements:

   * MacOS X fixes.  Classic Mac5L once again runs under OS X, and
     there is a new, not-yet-ready-for-prime-time Carbonized build.
   * Bug fixes from the "Fix for 3.4" list.

 Revision 1.6.2.1  2002/06/06 05:47:30  emk
 3.3.4.1 - Began refactoring the Win5L interpreter to live behind an
 abstract interface.

   * Strictly limited the files which include Card.h and Macro.h.
   * Added TWin5LInterpreter class.
   * Made as much code as possible use the TInterpreter interface.
   * Fixed a few miscellaneous build warnings.

 Revision 1.6  2002/05/29 13:58:17  emk
 3.3.4 - Fixed various crash-on-exit problems (including those in TBTree,
 TIndex and TLogger::FatalError), and reverted the Win32 _INCR_Y code
 to the behavior that shipped with Genetics.

 Revision 1.5  2002/05/15 11:05:33  emk
 3.3.3 - Merged in changes from FiveL_3_3_2_emk_typography_merge branch.
 Synopsis: The Common code is now up to 20Kloc, anti-aliased typography
 is available, and several subsystems have been refactored.  For more
 detailed descriptions, see the CVS branch.

 The merged Mac code hasn't been built yet; I'll take care of that next.

 Revision 1.4.2.2  2002/05/15 09:23:56  emk
 3.3.2.8 - Last typography branch checkin before merge.

 * Fixed (wait ...) bug which caused all (wait ...) commands to wait
 until the end of the movie.

 * (buttpcx ...) now uses anti-aliased text.

 * Miscellaneous other tweaks and fixes--just getting things into shape
 for the merge.

 Revision 1.4.2.1  2002/04/30 07:57:31  emk
 3.3.2.5 - Port Win32 code to use the 20Kloc of Common code that now
 exists.  The (defstyle ...) command should work, but (textaa ...) isn't
 available yet.

 Next up: Implement the (textaa ...) command and the low-level
 GraphicsTools::Image::DrawBitMap.

 Revision 1.4  2002/03/13 12:57:18  emk
 Support for 7-bit source code--smart quotes, m-dashes, ellipsis and HTML
 entities are now integrated into the Windows engine.

 Revision 1.3  2002/02/27 14:47:33  tvw
 Merged changes from 3.2.0.1 into trunk

 Revision 1.2  2002/02/27 13:21:12  tvw
 Bug #613 - Changed calculation of _INCR_Y to include descenders
 (part or letter that goes below baseline).

 Revision 1.1  2001/09/24 15:11:01  tvw
 FiveL v3.00 Build 10

 First commit of /iml/FiveL/Release branch.

 There are now seperate branches for development and release
 codebases.

 Development - /iml/FiveL/Dev
 Release - /iml/FiveL/Release

 Revision 1.8  2000/10/27 15:51:25  hyjin
 remove previous revision

 Revision 1.6  2000/04/07 17:05:15  chuck
 v 2.01 build 1

 Revision 1.5  2000/03/01 15:46:55  chuck
 no message

 Revision 1.4  1999/12/16 17:17:36  chuck
 no message

 Revision 1.3  1999/11/16 13:46:32  chuck
 no message

 Revision 1.2  1999/09/24 19:57:18  chuck
 Initial revision

*/
