// -*- Mode: C++; tab-width: 4; -*-

// Needed for RegisterMacPrimitives.
#include "TCommon.h"
#include "TPrimitives.h"
#include "TCommonPrimitives.h"
#include "TMacPrimitives.h"

// Needed to implement the primitives.
#include <stdlib.h>
#include <UInternetConfig.h>
#include "TLogger.h"
#include "TRect.h"
#include "TPoint.h"
#include "CMac5LApp.h"
#include "TVariable.h"
#include "CResource.h"
#include "CHeader.h"
#include "CFiles.h"
#include "CPicture.h"
#include "CPlayerView.h"
#include "CMoviePlayer.h"
#include "CPlayerText.h"
#include "CPlayerBox.h"
#include "CPlayerLine.h"
#include "CPlayerOval.h"
#include "CPlayerPict.h"
#include "CPlayerView.h"
#include "CPlayerInput.h"
#include "CTouchZone.h"
#include "CModule.h"
#include "TDateUtil.h"
#include "GraphicsTools.h"
#include "TStyleSheet.h"
#include "TException.h"
#include "TMac5LInterpreter.h"
#include "TPrimitives.h"
#include "gamma.h"

USING_NAMESPACE_FIVEL


//=========================================================================
//  RegisterMacPrimitives
//=========================================================================
//  Install our Macintosh-specific primitive functions.
//
//  Notes: Eliminated "loadpick" alias, "rvar", "rnode".
//  Disabled "pause".

void FIVEL_NS RegisterMacPrimitives()
{
    REGISTER_5L_PRIMITIVE(Add);
    // new audio commands
    REGISTER_5L_PRIMITIVE(Audio);
    REGISTER_5L_PRIMITIVE(AudioKill);
    REGISTER_5L_PRIMITIVE(AudioPlay);
    REGISTER_5L_PRIMITIVE(AudioVolume);
    REGISTER_5L_PRIMITIVE(AudioWait);
    // end of new audio commands
    REGISTER_5L_PRIMITIVE(Background);
    REGISTER_5L_PRIMITIVE(Beep);
    REGISTER_5L_PRIMITIVE(Blippo);
    REGISTER_5L_PRIMITIVE(Blueramp);
    REGISTER_5L_PRIMITIVE(Box);
    REGISTER_5L_PRIMITIVE(Browse);
    REGISTER_5L_PRIMITIVE(Buttpcx);
    REGISTER_5L_PRIMITIVE(CheckVol);
    REGISTER_5L_PRIMITIVE(Close);
    REGISTER_5L_PRIMITIVE(CTouch);
    REGISTER_5L_PRIMITIVE(Cursor);
#ifdef DEBUG
	REGISTER_5L_PRIMITIVE(Debug);
#endif
    REGISTER_5L_PRIMITIVE(Div);
    REGISTER_5L_PRIMITIVE(EjectDisc);
    REGISTER_5L_PRIMITIVE(Fade);
    REGISTER_5L_PRIMITIVE(Highlight);
    REGISTER_5L_PRIMITIVE(Hidemouse);
    REGISTER_5L_PRIMITIVE(Input);
    REGISTER_5L_PRIMITIVE(Jump);
    REGISTER_5L_PRIMITIVE(Key);
    REGISTER_5L_PRIMITIVE(Keybind);
    REGISTER_5L_PRIMITIVE(Kill);
    REGISTER_5L_PRIMITIVE(Line);
    REGISTER_5L_PRIMITIVE(Loadpal);
    REGISTER_5L_PRIMITIVE(Loadpic);
    REGISTER_5L_PRIMITIVE(Lock);
    REGISTER_5L_PRIMITIVE(Lookup);
    REGISTER_5L_PRIMITIVE(Micro);
    REGISTER_5L_PRIMITIVE(Nap);
    REGISTER_5L_PRIMITIVE(Open);
    REGISTER_5L_PRIMITIVE(Origin);
    REGISTER_5L_PRIMITIVE_WITH_NAME("circle", Oval);
	REGISTER_5L_PRIMITIVE(Oval);
	//REGISTER_5L_PRIMITIVE_WITH_NAME("pause", QTPause);
	REGISTER_5L_PRIMITIVE(Play);
    REGISTER_5L_PRIMITIVE(PlayQTFile);
    REGISTER_5L_PRIMITIVE(PlayQTLoop);
    REGISTER_5L_PRIMITIVE(PlayQTRect);
    REGISTER_5L_PRIMITIVE_WITH_NAME("preload", PreloadQTFile);
	REGISTER_5L_PRIMITIVE(Print);
	//(QTPause)
    REGISTER_5L_PRIMITIVE(Read);
#ifdef DEBUG
	REGISTER_5L_PRIMITIVE(ReDoScript);
#endif
	//REGISTER_5L_PRIMITIVE(Refresh);
	REGISTER_5L_PRIMITIVE(ResetOrigin);
    REGISTER_5L_PRIMITIVE(Resume);
    REGISTER_5L_PRIMITIVE(Rewrite);
    REGISTER_5L_PRIMITIVE(Screen);
    REGISTER_5L_PRIMITIVE(Search);
    REGISTER_5L_PRIMITIVE(Set);
    REGISTER_5L_PRIMITIVE(Showmouse);
    REGISTER_5L_PRIMITIVE(Still);
    REGISTER_5L_PRIMITIVE(Sub);
    REGISTER_5L_PRIMITIVE(Text);
    REGISTER_5L_PRIMITIVE(TextAA);
    REGISTER_5L_PRIMITIVE(Timeout);
    REGISTER_5L_PRIMITIVE(Touch);
    REGISTER_5L_PRIMITIVE(Unblippo);
    REGISTER_5L_PRIMITIVE(Unlock);
    REGISTER_5L_PRIMITIVE_WITH_NAME("video", PlayQTFile);
	REGISTER_5L_PRIMITIVE(Wait);
    REGISTER_5L_PRIMITIVE(Write);
}


//=========================================================================
//  Implementation of Macintosh Primitives
//=========================================================================

/*----------------------------------------------
    (ADD VARIABLE AMOUNT)

    Adds the given amount to the given variable.
    
    cbo - This originally was written to use floats.
------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Add)
{
	TString		theVarName;
	int32		theAmount;
	int32		theOrigValue;
	int32		theResValue;

    inArgs >> theVarName >> theAmount;
    
	// cbo_fix - we don't have fcvt like Windows does
   // sum = gVariableManager.GetDouble(vname);
	theOrigValue = gVariableManager.GetLong(theVarName);
    theResValue = theOrigValue + theAmount;

    //gVariableManager.SetDouble(vname, sum);
    gVariableManager.SetLong(theVarName, theResValue);

	gDebugLog.Log("add: %s <%ld> + <%ld> = <%ld>", 
		(const char *) theVarName, theOrigValue, theAmount, theResValue);
}

//
//	DoAudio - 
//
DEFINE_5L_PRIMITIVE(Audio)
{
	TString		audio_file;
	TString		flag;
	int32		offset = 0;
	int32		volume = 100;
	int32		fade_time = 0;
	bool		do_loop = false;
	bool		do_kill = false;
	
	inArgs >> audio_file;
	
	if (inArgs.HasMoreArguments())
		inArgs >> offset;
	if (inArgs.HasMoreArguments())
		inArgs >> volume;
	if (inArgs.HasMoreArguments())
		inArgs >> fade_time;
		
	while (inArgs.HasMoreArguments())
	{
		inArgs >> flag;
		flag.MakeLower();
		if (flag.Equal("kill"))
			do_kill = true;
		else if (flag.Equal("loop"))
			do_loop = true;
	}
	
	if (do_loop)
		gMovieManager.PlayLoop(audio_file.GetString(), fade_time);
	else
		gMovieManager.Play(audio_file.GetString(), offset, true, NULL);
	gDebugLog.Log("Audio: %s  ", audio_file.GetString()); 
}	

//
//	DoAudioKill - Kill audio clips. If fade_time is > 0, use it to fade the volume
//		out over that many tenths of a second. If loop is present, kill loops too.
//
//		(audiokill [fade_time] [loop])
//
//			default value:
//				fade_time		0
//				loop			no
//
DEFINE_5L_PRIMITIVE(AudioKill)
{
	TString		loop_flag;
	int32		fade_time = 0;
	bool		do_kill_loops = FALSE;
	
	if (inArgs.HasMoreArguments())
		inArgs >> fade_time;
		
	if (inArgs.HasMoreArguments())
	{
		inArgs >> loop_flag;
		loop_flag.MakeLower();
		
		if (loop_flag == (char *) "loop")
			do_kill_loops = true;
		else
			gLog.Caution("Bad flag to audiokill command <%s>", loop_flag.GetString());
	}
}

//
//	DoAudioPlay - Play the audio clip.
//
//		(audioplay clip_name [offset] [volume] [fade_time] [kill] [loop])
//
//			default values:
//				offset		0
//				volume		100
//				fade_time	0
//				kill		no
//				loop		no
//
DEFINE_5L_PRIMITIVE(AudioPlay)
{
	TString		audio_file;
	TString		flag;
	int32		the_offset = 0;
	int32		the_volume = 100;
	int32		the_fade_time = 0;
	bool		do_loop = false;
	bool		do_kill = false;
	
	inArgs >> audio_file;
	
	if (inArgs.HasMoreArguments())
		inArgs >> the_offset;
	if (inArgs.HasMoreArguments())
		inArgs >> the_volume;
	if (inArgs.HasMoreArguments())
		inArgs >> the_fade_time;
	
	while (inArgs.HasMoreArguments())
	{
		inArgs >> flag;
		flag.MakeLower();
		
		if (flag == (char *) "kill")
			do_kill = true;
		else if (flag == (char *) "loop")
			do_loop = true;
		else
			gLog.Caution("Bad flag to audioplay command <%s>", flag.GetString());
	}
}

//
//	DoAudioVolume - Set the volume for all playing audio clips. The volume
//		is given as a number between 0 (off) and 100 (full volume). The 
//		optional fade_time parameter is used to fade down or up to the volume
//		over that many tenths of a second.
//
//		(audiovolume volume [fade_time])
//
//			default values:
//				fade_time	0
//
DEFINE_5L_PRIMITIVE(AudioVolume)
{
	int32		the_volume;
	int32		the_fade_time = 0;
	
	inArgs >> the_volume;
	
	if (inArgs.HasMoreArguments())
		inArgs >> the_fade_time;
		
}

//
//	DoAudioWait - Wait for a particular frame of the playing audio clip.
//
//		(audiowait frame)
//
DEFINE_5L_PRIMITIVE(AudioWait)
{
	int32		the_frame;
	
	inArgs >> the_frame;
	
}	
	
	

/*---------------------------------------------------------------
    (BACKGROUND picfile x1 y1 x2 y2)

	Draw the pic in the bounding rectangle (x1 y1 x2 y2)
-----------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Background)
{
    TString     TempCmd, picname;
    TRect     	loc;
    Rect		macLoc;
    
    inArgs >> picname >> loc;
    
    gOrigin.AdjustRect(&loc);
    macLoc = loc.GetRect();

	gDebugLog.Log("background: <%s>, <L T R B> %d %d %d %d", 
		picname.GetString(), macLoc.left, macLoc.top, macLoc.right, macLoc.bottom);

    gPlayerView->SetBackPic(picname, macLoc);
}

/*---------------------------------------------------------------
    (BEEP <FREQ> <DURATION>)

    The computer beeps. Optional parameters control the frequency
    (pitch) of the beep and its duration, in tenths of seconds.
-----------------------------------------------------------------*/
 DEFINE_5L_PRIMITIVE(Beep)
{
    int16     freq = 1500;        //  Hz
    int16     duration = 80;      //  Milliseconds

    if (inArgs.HasMoreArguments()) 
		inArgs >> freq;
    
    if (inArgs.HasMoreArguments()) 
	{
        inArgs >> duration;
        duration *= 100;
    }

	gDebugLog.Log("beep: freq <%d>, duration <%d>", freq, duration);
    
 	// cbo_fix - we can do better than this
	SysBeep(30);
}
/*---------------------------------------------------------------
    (BLIPPO)

    Copy the current display screen to the offscreen buffer while
    still allowing drawing on screen. Use unblippo to restore the
    saved screen.
-----------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Blippo)
{
	gDebugLog.Log("blippo: ");

	gPlayerView->Blippo();
}

/*----------------------------------------------------------------
    (BLUERAMP one two three four)
    Puts the stupid blueramp inside the given rectangle.
 ----------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Blueramp)
{
#ifdef CBO_FIX
    TRect bounds;

    inArgs >> bounds;
    Blueramp(bounds);
#endif
}

/*----------------------------------------------------------------
    (BOX LEFT TOP RIGHT BOTTOM FILL COLOR <THICKNESS>)

    Draws a box of the given color at the coordinates specified.
    If FILL == "FILL", the box is filled in. Otherwise,
    only the outline of the box is drawn.
------------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Box)
{
	CPlayerBox	*boxPtr;
    TRect		bounds;
    Rect		macBounds;
    int16		color, lineThickness = 1;
    TString		fill;
    Boolean		theFill = false;


    inArgs >> bounds >> fill >> color;
    if (inArgs.HasMoreArguments()) 
    	inArgs >> lineThickness;

    gOrigin.AdjustRect(&bounds);
    macBounds = bounds.GetRect();
    
    fill.MakeLower();
    if (fill == (char *) "fill")
    	theFill = true;

	if (theFill)
		gDebugLog.Log("filled box: <L T R B> %d %d %d %d, color <%d>, thickness: <%d>", 
			bounds.Left(), bounds.Top(), bounds.Right(), bounds.Bottom(), color, lineThickness);
	else
		gDebugLog.Log("outline box: <L T R B> %d %d %d %d, color <%d>, thickness: <%d>", 
			bounds.Left(), bounds.Top(), bounds.Right(), bounds.Bottom(), color, lineThickness);
    
    boxPtr = new CPlayerBox(macBounds, theFill, lineThickness, color);
	if (boxPtr != nil)
		delete boxPtr;
}


/*---------------------------------------------------------------
   DoBrowse launches a browser and goes to the given URL.
   It opens the default browser according to the settings in 
   InternetConfig.
-----------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Browse)
{
	
	TString theURL;
	
	inArgs >> theURL;
	
	long startSel = 0;
   	long endSel = theURL.Length();
  	
    if (PP::UInternetConfig::PP_ICAvailable())	
    {
    		gDebugLog.Log("Launching default web browser");
    
    	// Theoretically our suspend event should show the menu bar without problems. 
    	// However, this doesn't work. Internet explorer's menu bar will be screwed up 
    	// if Internet Explorer was not previously running. 
    	// Therefore we need to show the menu bar here, before we call PP_ICLaunchURL. 
    	::ShowMenuBar();
        PP::UInternetConfig::PP_ICLaunchURL("\p", (char *) theURL.GetString(), endSel, &startSel, &endSel);
    	
    }
    else
    {
    		gDebugLog.Log("Problems with accessing InternetConfig during browse command.");
	}
}




/*---------------------------------------------------------------
    (BUTTPCX PCXFILE X Y header text Command <second Command>)
    Puts a button with text "text" on the screen and activates a corresponding
    touchzone.  It takes care of the picture and hilited picture display.
    OPTIONAL:  set command to be executed prior to the jump.
-----------------------------------------------------------------*/

DEFINE_5L_PRIMITIVE(Buttpcx)
{
    TRect       bounds;
    TPoint      buttLoc;
	const char		*theHeadName = nil;
    CPicture    *thePicture = nil;
    TString     theHeaderName, picname, Text;
	TCallback	*callback;
    TString		cursorType;
    CursorType	cursor = HAND_CURSOR;
    CursorType	tmpCursor = UNKNOWN_CURSOR;

    inArgs >> picname >> buttLoc >> theHeaderName >> Text >> callback;

	if (inArgs.HasMoreArguments())
	{
		inArgs >> cursorType;
		
		tmpCursor = gCursorManager.FindCursor(cursorType);
		if (tmpCursor != UNKNOWN_CURSOR)
	    	cursor = tmpCursor;
	    else
	    	gLog.Caution("Unknown cursor type: %s", cursorType.GetString());
	}
	
    gOrigin.AdjustPoint(&buttLoc);
	
	if (not picname.IsEmpty())
	{
		thePicture = gPictureManager.GetPicture(picname);
		
		if (thePicture != NULL)
		{
			bounds = thePicture->GetBounds();
			bounds.Offset(buttLoc);
		}
	}
	
	if (not theHeaderName.IsEmpty())
		theHeadName = theHeaderName.GetString();

	gDebugLog.Log("buttpcx: <%s>, <L T R B> %d %d %d %d, header <%s>, text <%s>",
			(const char *) picname,
			bounds.Left(), bounds.Top(), bounds.Right(), bounds.Bottom(),
			(const char *) theHeadName, (const char *) Text);
	
	UpdateSpecialVariablesForGraphic(bounds);

	if (thePicture != NULL)
		new CTouchZone(bounds, callback, thePicture, buttLoc, (const char *) Text, cursor, 
				(const char *) theHeadName);
	else
		gDebugLog.Log("no picture <%s>, no touch zone", (const char *) picname);

	
	// cbo_test - try this
	//gPlayerView->AdjustMyCursor();
}

//
//	DoCheckVol - Check if the volume is mounted and return the path to it.
//		(checkvol vol_name var_to_get_path [card_to_jump_if_no_volume])
//
DEFINE_5L_PRIMITIVE(CheckVol)
{
	TString		vol_name;
	TString		real_path_var;
	TString		no_volume;
	
	inArgs >> vol_name >> real_path_var;
	
	if (inArgs.HasMoreArguments())
		inArgs >> no_volume;
		
	gDebugLog.Log("checkvol: <%s>, put path into <%s>",
		vol_name.GetString(), real_path_var.GetString());

	gVariableManager.SetLong(real_path_var.GetString(), 0);
	
	if (gModMan->VolMounted(vol_name))
	{
		if (not vol_name.EndsWith(':'))
			vol_name += ":";
			
		gVariableManager.SetString(real_path_var.GetString(), vol_name);
	}
	else if (not no_volume.IsEmpty())
	{
		gDebugLog.Log("checkvol: failed, jumping to <%s>",
			no_volume.GetString());

		gPlayerView->ProcessEvents(false);	// stop processing keys and touch zones
		TInterpreter::GetInstance()->JumpToCardByName(no_volume.GetString());
    	gPlayerView->Draw(nil);				// refresh the screen so see everything before jumping
	}
}
	

/*----------------------------------------------------------------------
    (CLOSE FILENAME)

    Close the given text file.
------------------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Close)
{
    TString     filename;

    inArgs >> filename;

	gDebugLog.Log("close: file <%s>", filename.GetString());

    gFileManager.Close(filename);
}

/*-------------------------------------------------------------
    (CTOUCH)

    Clear all current touch zones. Typically called immediately
    before setting new touch zones.
---------------------------------------------------------------*/
 DEFINE_5L_PRIMITIVE(CTouch)
{
    int16	left, top;

    if (inArgs.HasMoreArguments())  
    {
        inArgs >> left >> top;

		gDebugLog.Log("ctouch: at left <%d>, top <%d>", left, top);   

        gPlayerView->CTouch(left, top);
    }
    else 
    {
		gDebugLog.Log("ctouch: all");
    	gPlayerView->CTouch();
    }

	// cbo_test - put this back in because we took it out in wait and nap
	gPlayerView->AdjustMyCursor();   	
}

//
//	DoCursor - Change the cursor.
//
DEFINE_5L_PRIMITIVE(Cursor)
{
	CursorType	theCursor = ARROW_CURSOR;
	CursorType	tmpCursor;
	TString		cursorStr;
	bool		forceShow = false;
	
	if (inArgs.HasMoreArguments())
	{
		inArgs >> cursorStr;
		
		tmpCursor = gCursorManager.FindCursor(cursorStr);
		if (tmpCursor != UNKNOWN_CURSOR)
		{
			theCursor = tmpCursor;
			forceShow = true;
			
			gDebugLog.Log("Changing cursor to %s", cursorStr.GetString());
		}
		else
		{
			gLog.Caution("Unknown cursor type: %s", cursorStr.GetString());
			gDebugLog.Log("Unknown cursor type: %s", cursorStr.GetString());
		}
	}
	
	gCursorManager.ChangeCursor(theCursor);
	gCursorManager.ForceShow(forceShow);
}

//
//	DoDebug - Drop into the debugger
//
#ifdef DEBUG
DEFINE_5L_PRIMITIVE(Debug)
{
	// drop into debugger
	BreakToSourceDebugger_();
}
#endif

/*--------------------------------------------------------
        (DIV X Y)

        X <- X/Y,  X will be truncated to int16.
 ---------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Div)
{
    TString		theVarName;
    double		theAmount;
    int32		theOrigValue;
    int32		theResValue;

    inArgs >> theVarName >> theAmount;


	theOrigValue = gVariableManager.GetLong(theVarName);

    if (theAmount == 0.0)
    {
		gLog.Caution("Division by zero: %s <%ld> / <%f>", 
			(const char *) theVarName, theOrigValue, theAmount);
			
		theResValue = 0;
    }
    else
		theResValue = (int32) (theOrigValue / theAmount);
   
    gVariableManager.SetLong(theVarName, theResValue);
    
	gDebugLog.Log("div: %s <%ld> by <%f> = <%ld>", 
		(const char *) theVarName, theOrigValue, theAmount, theResValue);
}

//
//	(ejectdisc) - Eject whatever CD is in the drive.
//
DEFINE_5L_PRIMITIVE(EjectDisc)
{
#if CALL_NOT_IN_CARBON
	gModMan->EjectCD();
#endif // CALL_NOT_IN_CARBON

	gDebugLog.Log("Ejecting disk");
}

/*---------------------------------------------------------------
    (FADE DIR <STEPS>)

    Either FADE IN or FADE OUT. This ramps the palette in or out.
-----------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Fade)
{
    TString     direction;
    int16       steps = 1;

    inArgs >> direction;
    if (inArgs.HasMoreArguments()) 
    	inArgs >> steps;

	gDebugLog.Log("fade: %s, steps <%d>", direction.GetString(), steps);

	// cbo_hack - try making the fades a bit faster
//	if (steps >= 10)
//		steps -= (steps / 3);
	
    direction.MakeLower();
    if (direction == (char *) "in")
    {
    	gPlayerView->Draw(nil);		// refresh the screen first
		DoGFade(true, steps, true);
	}
    else if (direction == (char *) "out")
		DoGFade(false, steps, true);
    else
        gLog.Caution("Fade in or out, but don't fade %s", (const char *) direction);
}

/*---------------------------------------------------------
    (HIGHLIGHT PICNAME)

    Will highlight a picture already displayed on screen by
    drawing PICNAMEH.PCX, pausing, and then redrawing
    PICNAME.PCX. Both of these pictures are matted.
-----------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Highlight)
{
#ifdef CBO_FIX
    TString     picName;
    Picture     *thePicture;
    TPoint       pt;

    inArgs >> picName;

    thePicture = gPictureManager.GetPicture(picName);
    thePicture->GetLoc(&pt);
    thePicture->Hilite(pt);
#endif
}

/*----------------------------------------------------------------
    (HIDEMOUSE)
    As expected, hides the mouse. WARNING!  unknown behv. when TouchScreen!!
 ---------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Hidemouse)
{
	gCursorManager.HideCursor();
	gDebugLog.Log("Hiding cursor");
}

/*---------------------------------------------------------------------
    (INPUT STYLE VARIABLE MASK X Y <REQUIRE>)

    Allow user input from the keyboard. The text they input is saved
    in the given variable. It appears onscreen in the given header
    style, with upper left corner X, Y. The mask is an input mask which
    controls the kind of characters the user may input. (See GX Text
    for more details about the mask.)

    If REQUIRE = true then the entire mask must be filled. Otherwise
    any input, however short, is accepted.
-----------------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Input)
{
    TString     theVarName, mask, style, required;
    TRect		bounds;
    Rect		macBounds;
    int16       fRequire = false;

    inArgs >> style >> theVarName >> mask >> bounds;

    if (inArgs.HasMoreArguments()) 
    {
        inArgs >> required;
        required.MakeLower();
        if (required == (char *) "true") 
        	fRequire = true;
    }

   	gOrigin.AdjustRect(&bounds);
   	macBounds = bounds.GetRect();

	gDebugLog.Log("input: into <%s>, style <%s>, mask <%s>, <L T R B> %d %d %d %d, completion required <%s>",
			theVarName.GetString(), style.GetString(), mask.GetString(), 
			bounds.Left(), bounds.Top(), bounds.Right(), bounds.Bottom(),
			required.GetString());
   	
	TInterpreter::GetInstance()->Pause();
   	gPlayerView->ProcessEvents(false);
   	
	DoCPlayerInput(theVarName, style, mask, macBounds, fRequire);
	
	gPlayerView->Draw(nil);
}

/*-------------------------
    (JUMP JUMPCARD)

    Jump to the given card.
---------------------------*/
DEFINE_5L_PRIMITIVE(Jump)
{
    TString     jumpCard;

    inArgs >> jumpCard;

	gDebugLog.Log("jump: to <%s>", (const char *) jumpCard);
    
	gPlayerView->ProcessEvents(false);	// stop processing keys and touch zones
	
    TInterpreter::GetInstance()->JumpToCardByName(jumpCard);
    
    gPlayerView->Draw(nil);				// refresh the screen so see everything before jumping
}

/*--------------------------------------------------------
    (KEY COLOR)

    Switch the overlay mode and set the keycolor to COLOR.
----------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Key)
{
#ifdef CBO_FIX
    int16     newKeyColor;

    inArgs >> newKeyColor;
    //CheckColorIW(&newKeyColor);
    gVideoManager->overlay(newKeyColor);

#endif
}

/*-----------------------------------------------------------
    (KEYBIND CHAR <LINKCARD>)

    Bind the given character to the linkcard so that pressing
    ALT-CHAR jumps to the linkcard. If no linkcard is given,
    then remove the binding associated with CHAR.
-------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Keybind)
{
    TString 	keyEquiv; 
	char		theChar;
	TCallback	*theCallback = NULL;
	
    inArgs >> keyEquiv;

    if (inArgs.HasMoreArguments())
        inArgs >> theCallback;
        
    keyEquiv.MakeLower();
    if (keyEquiv == (const char *) "esc")
    	theChar = 0x1B;					// the Escape key
    else
    	theChar = keyEquiv(0);

	gDebugLog.Log("keybind: key <%c>", keyEquiv(0));

	gPlayerView->AddKeyBinding(theChar, theCallback);
}

//
//	DoKill - Kill whatever movie is playing.
//
DEFINE_5L_PRIMITIVE(Kill)
{
	if (gMovieManager.Playing())
	{
		gMovieManager.Kill();
		gDebugLog.Log("kill: the movie be dead");
	}
	else
		gDebugLog.Log("kill: nothing to kill");	
}

/*--------------------------------------------------------------
    (LINE X1 Y1 X2 Y2 COLOR <THICKNESS>)

    Draw a line of given color and thickness (default is 1) from
    x1, y1 to x2, y2.
----------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Line)
{
	CPlayerLine	*linePtr;
    TPoint  	a, b;
    int16   	color, thickness = 1;
    Rect		theRect;
 	inArgs >> a >> b >> color;
    
    if (inArgs.HasMoreArguments())
    	inArgs >> thickness;

    gOrigin.AdjustPoint(&a);
    gOrigin.AdjustPoint(&b);
    
    //
    // make sure horizontal and vertical lines don't have
    //			funny numbers when thickness > 1
    if ((b.X() == (a.X() + thickness)) or (b.X() == (a.X() - thickness)))
    	b.SetX(a.X());
    if ((b.Y() == (a.Y() + thickness)) or (b.Y() == (a.Y() - thickness)))
    	b.SetY(a.Y());
    
    ::SetRect(&theRect, a.X(), a.Y(), b.X(), b.Y());
    linePtr = new CPlayerLine(theRect, thickness, color);
	if (linePtr != nil)
		delete linePtr;
		
	if (a.X()==b.X())
		gDebugLog.Log("Vertical line: <L T R B> %d %d %d %d color %d thickness: %d width: %d height: %d", 
					  a.X(), a.Y(),b.X(), b.Y(), color, thickness, abs(b.X()-a.X()), abs(b.Y()-a.Y())); 
	else if (a.Y()==b.Y())
		gDebugLog.Log("Horizontal line: <L T R B> %d %d %d %d color %d thickness: %d width: %d height: %d", 
					  a.X(), a.Y(),b.X(), b.Y(), color, thickness, abs(b.X()-a.X()), abs(b.Y()-a.Y())); 
	else
		gDebugLog.Log("Line: <L T R B> %d %d %d %d color %d thickness: %d width: %d height: %d", 
					  a.X(), a.Y(),b.X(), b.Y(), color, thickness, abs(b.X()-a.X()), abs(b.Y()-a.Y())); 
}

/*-------------------------------------------------------------
    (LOADPAL PICTURE)

    Set the screen palette to the palette of the given picture.
---------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Loadpal)
{
	CPalette	*thePal = nil;
    TString 	palname;
    TString		flag;
    bool		noload = false;
    bool		lock = false;
    bool		unlock = false;

	inArgs >> palname;

	palname.MakeLower();

	gDebugLog.Log("loadpal: <%s>", palname.GetString());

	while (inArgs.HasMoreArguments())
	{
		inArgs >> flag;
		flag.MakeLower();
		
		if (flag == (char *) "noload")
			noload = true;
		else if (flag == (char *) "lock")
			lock = true;
		else if (flag == (char *) "unlock")
			unlock = true;
		else
			gLog.Caution("Bad flag to loadpal command <%s>", flag.GetString());
	}
	
	thePal = gPaletteManager.GetPalette(palname);
	
	if (thePal != nil)
	{
		if (lock)
			thePal->Lock(true);
		else if (unlock)
			thePal->Lock(false);
			
		if (not noload)
			gPaletteManager.SetPalette(thePal, true);
	}
	else
		gDebugLog.Log("Couldn't find palette <%s>", palname.GetString());
}

/*---------------------------------------------------------------------
    (LOADPIC PICTURE X Y <FLAGS...>)
-----------------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Loadpic)
{
	CPlayerPict	*pictPtr;
    TString     TempCmd, picname, flag;
    CPicture    *thePicture = NULL;
    CPalette	*thePalette = NULL;
    TPoint	    loc;
    bool       	matte = false;
    bool		noshow = false;
    bool		lock = false;
    bool		unlock = false;
    bool		do_pal = false;
    
    inArgs >> picname >> loc;
	picname.MakeLower();
	
    gOrigin.AdjustPoint(&loc);	

	TString theFlags = "";
	
   while (inArgs.HasMoreArguments()) 
    {
    	if (theFlags == "")
    		theFlags = "flags: "; // contains the name of all flags in script
								  //- these will be printed to debuglog 
    	
        inArgs >> flag;
        flag.MakeLower();
        
        
        if (flag == (char *) "noshow")
        {
            noshow = true;
            theFlags += "noshow ";
        }
        else if (flag == (char *) "pal")
        {
        	do_pal = true;
        	theFlags += "pal ";
        }
        else if (flag == (char *) "matte")
        { 
        	matte = true;  
        	theFlags += "matte ";
        }
        else if (flag == (char *) "lock")
        {
        	lock = true;
        	theFlags += "lock ";
        }
        else if (flag == (char *) "unlock")
        {
        	unlock = true;
        	theFlags += "unlock ";
        }
        else
			gLog.Caution("Unknown flag to loadpic command <%s>", flag.GetString());
    }

	thePicture = gPictureManager.GetPicture(picname);
	
	if (thePicture != NULL)
	{
		// GetBounds returns rect with (0,0,width, height).  We need to
		// offset this before calling UpdateGraphicsForSpecialVariables.
		TRect bounds = thePicture->GetBounds();	
		bounds.Offset(loc);
		UpdateSpecialVariablesForGraphic(bounds);
		Rect sides = bounds.GetRect();
	
		gDebugLog.Log("loadpic: <%s>, <L T R B> %d %d %d %d %s",
					  picname.GetString(), loc.X(), loc.Y(),
					  sides.right, sides.bottom, theFlags.GetString());
	}

	if (thePicture != nil)
	{
		if (lock)
			thePicture->Lock(true);
		else if (unlock)
			thePicture->Lock(false);
	}
	
	if (do_pal and (thePicture != NULL))
	{
		thePalette = gPaletteManager.GetPalette(picname);
		
		if (thePalette != nil)
			gPaletteManager.SetPalette(thePalette, true);
	}
    
	if ((not noshow) and (thePicture != NULL))
	{
    	pictPtr = new CPlayerPict(thePicture, loc, matte);
		if (pictPtr != NULL)
			delete pictPtr;
	}
}

/*--------------------------------------------------------
    (LOCK <CLEAR>)

    CLEAR   If given, clear the offscreen buffer. Otherwise,
            copy the current screen contents offscreen.

    Lock the screen so that all drawing takes place in the
    offscreen buffer.
----------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Lock)
{
    TString     clear;
    bool		doClear = false;

    if (inArgs.HasMoreArguments())
    {
        inArgs >> clear;
        clear.MakeLower();
    }

    if (clear == (char *)"clear")
		doClear = true;
    else 
		doClear = false;

	gDebugLog.Log("lock: Clear: (0=false) <%d>", doClear);
		
	gPlayerView->Lock(doClear);
}

/*------------------------------------------------------------------
    (LOOKUP FILENAME FIELD1 <FIELD2> ... <FIELDN>)

    FILENAME    The text file to lookup the record in.

    FIELD1..N   The fields which must match (case not important) the
                record.

    Assumes the text file is a tab delimited, return marked database
    and tries to find the record that matches the first N fields. If
    it succeeds, the file pointer is positioned after the first N
    fields and the scriptor can start doing (read..until tab) calls.
    If it fails, the pointer is at end of file.
--------------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Lookup)
{
    TString     searchString, param, filename;
    int16       numFields = 0;

    inArgs >> filename;

    //  Append all the fields together into a search string that looks
    //  like "field1 TAB field2 TAB ... fieldN"
    //
    while (inArgs.HasMoreArguments()) 
    {
        inArgs >> param;
        if (numFields > 0)
            searchString += '\t';
        numFields++;
        searchString += param;
    }

	gDebugLog.Log("lookup: file <%s>, search <%s>", filename.GetString(), searchString.GetString());
    
    gFileManager.Lookup(filename, searchString, numFields);
}

/*-------------------------------------------------------------------
    (MICRO EFFECT)

    Switch to graphics only (micro) mode. The effect defines how this
    transition is accomplished.
---------------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Micro)
{
#ifdef DONT_DO_THIS
	TString     strEffect;
    FXType      theEffect;

    inArgs >> strEffect;

    theEffect = StringToEffect(strEffect);
	// Not quite sure what the spec says to do here. I think all we need to do is
	// the same as an unlock(), but may want to gfade down & up if from movie.
	
	gPlayerView->Micro(theEffect);
#endif
}

/*------------------------------------------------
    (NAP TIME)

    Pause execution for TIME tenths of seconds.
    The user can abort a int32 nap via the ESC key.
--------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Nap)
{
    int32    tenths;

    inArgs >> tenths;
	
	TInterpreter::GetInstance()->Nap(tenths);

	gDebugLog.Log("nap: %d", tenths);
	//gDebugLog.Log("Refreshing Card (DoNap)");
	
	// cbo_test - took this out to prevent flashing
	gPlayerView->AdjustMyCursor();
	
	gPlayerView->Draw(nil);
	gPlayerView->ProcessEvents(true);
}

/*----------------------------------------------------------------
    (OPEN FILENAME KIND)

    Open a text file. KIND specifies the kind of access one
    will have to the file. It may be APPEND (write only, appending
    to the end of the file), NEW (write only, overwriting
    anything in the file), or READONLY.
------------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Open)
{
    TString     filename, kind;
    char		*slashPtr;
    FileKind    fKind = fReadOnly;

    inArgs >> filename >> kind;
    kind.MakeLower();

    if (kind == (char *)"append") 
    	fKind = fWriteAppend;
    else if (kind == (char *)"new") 
    	fKind = fWriteNew;
    else if (kind == (char *)"readonly") 
    	fKind = fReadOnly;
    else
        gLog.Caution("Unknown open file kind: %s", (const char *)kind);
    
    // Filenames can look DOS like.     
    slashPtr = strstr(filename.GetString(), "\\");
    if (slashPtr != NULL)
    	*slashPtr = ':';

	gDebugLog.Log("open with %s: file <%s>", kind.GetString(),filename.GetString());

    gFileManager.Open(filename, fKind);
}

/*------------------------------------------------------------
    (ORIGIN DX DY)

    Move the local coordinates for this particular card (or
    macro) by the delta values given. This change is an offset
    from whatever the current coordinates are. There is no
    way to set the absolute coordinates for a macro or card!
--------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Origin)
{
    TPoint   delta;

    inArgs >> delta;

    gOrigin.OffsetOrigin(delta);
    
	TPoint origin = gOrigin.GetOrigin();
    gDebugLog.Log("Origin set to <X Y> %d %d", origin.X(), origin.Y());
}

/*----------------------------------------------------------------
    (OVAL LEFT TOP RIGHT BOTTOM FILL COLOR <THICKNESS>)

    Draws an oval of the given color at the coordinates specified.
    If FILL == "FILL", the oval is filled in. Otherwise,
    only the outline of the oval is drawn.
------------------------------------------------------------------*/

DEFINE_5L_PRIMITIVE(Oval)
{
	CPlayerOval	*ovalPtr;
    TRect		bounds;
    Rect		macBounds;
    int16		color, lineThickness = 1;
    TString		fill;
    Boolean		theFill = false;


    inArgs >> bounds >> fill >> color;
    if (inArgs.HasMoreArguments()) 
    	inArgs >> lineThickness;

    gOrigin.AdjustRect(&bounds);
    macBounds = bounds.GetRect();
    
    fill.MakeLower();
    if (fill == (char *) "fill")
    	theFill = true;
    	
    if (theFill)
		gDebugLog.Log("filled oval: <L T R B> %d %d %d %d, color <%d>, thickness: <%d>", 
			bounds.Left(), bounds.Top(), bounds.Right(), bounds.Bottom(), color, lineThickness);
	else
		gDebugLog.Log("outline oval:<L T R B> %d %d %d %d, color <%d>, thickness: <%d>", 
			bounds.Left(), bounds.Top(), bounds.Right(), bounds.Bottom(), color, lineThickness);
    
    ovalPtr = new CPlayerOval(macBounds, theFill, lineThickness, color);
	if (ovalPtr != nil)
		delete ovalPtr;
}


DEFINE_5L_PRIMITIVE(Pause)
{
#ifdef CBO_FIX
    int32    tenths = 0L;

    inArgs >> tenths;
    
    gDebugLog.Log("pause: %d", tenths);


    gVideoManager->pause(tenths);
#endif
}

/*-----------------------------------------------------------
    (PLAY FRAME1 FRAME2 TRACK <SPEED>)

    Begin playing the segment from FRAME1..FRAME2 with the
    given audio track (use 0 for none and 3 for stereo).
    An optional speed in fps can be given; the particular
    videodisk player will choose as close a speed as possible
    (since not all players support the same range of fps's).
-------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Play)
{
#ifdef CBO_FIX
    int32        frame1, frame2, speed = 30;
    AudioState  track;
    int16 trackNum;

    inArgs >> frame1 >> frame2 >> trackNum;
    if (inArgs.HasMoreArguments()) {
        speed = 30;  // fps = 30fps
    }

    switch (trackNum) {
        case 0:
            track = audNONE;
            break;
        case 1:
            track = audONE;
            break;
        case 2:
            track = audTWO;
            break;
        case 3:
            track = audSTEREO;
            break;
        case 4:
            track = audONE;
            break;
        default:
            gLog.Caution("Illegal audio track parameter %ld.", trackNum);
    }
    gVideoManager->audio(track);
    gVideoManager->clip(frame1, frame2, 30); //Override speed!!

#endif
}

/*-----------------------------------------------------------
    (playqtfile file [frame] [pal] [origin])
    
    Play a QuickTime file. Frame is the frame offset to be
    used with subsequent wait commands and corresponds to
    a laser disc frame (nothing to do with QuickTime). Pal
    is the palette to use (name.pic).

-------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(PlayQTFile)
{
	TString			theQTFile;
	TString			thePal;
	TPoint			movieOrigin(0, 0);
	const char		*thePalStr = NULL;
	int32			theOffset = 0;
	bool			audioOnly = false;
	TString 		theFlags = "Flags: ";
	inArgs >> theQTFile;

	if (inArgs.HasMoreArguments())
	{
		inArgs >> theOffset;
		theFlags += " offset: ";
		theFlags += theOffset;
	}
	if (inArgs.HasMoreArguments())
	{
		inArgs >> thePal;
		theFlags += " pal: ";
		theFlags += thePal.GetString();
	}	
	if (inArgs.HasMoreArguments())
	{
		inArgs >> movieOrigin;
		theFlags += " origin: ";
		theFlags += movieOrigin.X();
		theFlags += movieOrigin.Y();
		
		gOrigin.AdjustPoint(&movieOrigin);
		gMovieManager.SetOrigin(movieOrigin);
	}
	
	if (theQTFile.Contains(".a2", false))
		audioOnly = true;
    		
	if (not thePal.IsEmpty())
	{
		thePal.MakeLower();
		thePalStr = thePal.GetString();
	}
	
	gDebugLog.Log("playqtfile: <%s>, %s", theQTFile, theFlags.GetString());
				
	gMovieManager.Play(theQTFile.GetString(), theOffset, 
		audioOnly, thePalStr);
}

//
//	PlayQTLoop - Loop the given audio file.
//
//		(playqtloop file [fade])
//
DEFINE_5L_PRIMITIVE(PlayQTLoop)
{
	TString		theQTFile;
	int32		theFadeTime = 0;
	bool		audioOnly = false;
	
	inArgs >> theQTFile;
	
	if (inArgs.HasMoreArguments())
		inArgs >> theFadeTime;
	
    if (strstr(theQTFile.GetString(), ".a2")) 
		audioOnly = true;

	if (not audioOnly)
		gLog.Caution("playqtloop can only be used with audio files!");
	else
	{
		gDebugLog.Log("playqtloop: <%s> <%ld>", theQTFile.GetString(), theFadeTime);
		gMovieManager.PlayLoop(theQTFile.GetString(), theFadeTime);
	}
}	

//
//	PlayQTRect - Give a point which will be used to create a rect for the
//			next PlayQTFile command.
//
//		(playqtrect X Y>
//
DEFINE_5L_PRIMITIVE(PlayQTRect)
{
	TString		theQTFile;
	TString		thePal;
	TPoint		thePT;
	
	inArgs >> thePT;
	
	gOrigin.AdjustPoint(&thePT);
		
	gDebugLog.Log("playqtrect: X <%d>, Y <%d>", thePT.X(), thePT.Y());

	gMovieManager.SetOrigin(thePT);
}
	
DEFINE_5L_PRIMITIVE(PreloadQTFile)
{
	TString 	theQTFile;
	TString		syncFlag;
	int32		tenths = 0;
	bool		audioOnly = false;
	bool		doSync = false;

    inArgs >> theQTFile;
    
    if (inArgs.HasMoreArguments())
    	inArgs >> tenths;
    	
    if (inArgs.HasMoreArguments())
    {
    	inArgs >> syncFlag;
    	
    	if (syncFlag.Equal("sync", false))
    		doSync = true;
		else
			gDebugLog.Log("perload: bad flag <%s>, looking for sync", syncFlag.GetString());
	}
    
    if (strstr(theQTFile.GetString(), ".a2"))
    	audioOnly = true;

	gDebugLog.Log("preload: <%s>, tenths <%d>, %s", 
		theQTFile.GetString(), tenths, (doSync ? "sync" : "async"));
	
	if (tenths > 0)
	{
		TInterpreter::GetInstance()->Nap(tenths);

		// cbo_test - took this out to prevent flashing
		gPlayerView->AdjustMyCursor();
		
		gPlayerView->Draw(nil);
		gPlayerView->ProcessEvents(true);
	}
    
    // don't actually preroll	
	//gMovieManager.Preroll(theQTFile.GetString(), audioOnly, doSync);    
}

/*-----------------------------------------------------------------
    (PRINT JUSTIFICATION TEXT)

    Prints the given text with either CENTER or LEFT justification.

    !!! This is currently disabled. Don't bother updating printer
        support until we get a project that needs it.
-------------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Print)
{
#ifdef CBO_FIX
    TString     just;
    TString     text;

    inArgs >> just >> text;

    just.MakeLower();
    if (just == (char *)"center") 
    {
        // Call centered print routine.
    } 
    else 
    {
        // Call default print routine.
    }

#endif
}

/*-----------------------------------------------------------------
    (READ FILENAME VARIABLE <UNTIL DELIM>)

    Read data from a text file and put it into the variable.
    Normally this will read the next word as defined by whitespace.
    Use the UNTIL DELIM construct to read until some other
    delimiter.

    Valid delimiters are TAB, RETURN, EOF, or any single character.
-------------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Read)
{
    TString         filename, vname, untilstr, delimstr;
    unsigned char   delim;
    TString         res;

    inArgs >> filename >> vname;

    if (inArgs.HasMoreArguments()) 
    {
        inArgs >> untilstr >> delimstr;
        delimstr.MakeLower();
        
        if (delimstr == (char *)"tab") 
        	delim = '\t';
        else if (delimstr == (char *)"return") 
        	delim = NEWLINE_CHAR;
        else if (delimstr == (char *)"eof") 
        	delim = 0;
        else 
        	delim = delimstr(0);

        gFileManager.ReadUntil(filename, res, delim);
    } 
    else 
    	gFileManager.Read(filename, res);

	gDebugLog.Log("read: var <%s>, value <%s>", vname.GetString(), res.GetString());
    	
    gVariableManager.SetString(vname.GetString(), res.GetString());
}

#ifdef DEBUG
//
//	DoReDoScript
//
DEFINE_5L_PRIMITIVE(ReDoScript)
{
	TString		theCard;
	
	inArgs >> theCard;
	
	gDebugLog.Log("redoscript: <%s>", (const char *) theCard);
	TInterpreter::GetInstance()->DoReDoScript(theCard);	
}
#endif

//
//	DoRefresh - Refresh the screen.
//
DEFINE_5L_PRIMITIVE(Refresh)
{
	//gPlayerView->Refresh();
}

//
//	DoResetOrigin - Reset the origin or set it to something new.
//
DEFINE_5L_PRIMITIVE(ResetOrigin)
{
	TPoint		newOrigin(0, 0);
	
	if (inArgs.HasMoreArguments())
		inArgs >> newOrigin;
	
	gDebugLog.Log("ResetOrigin to 0 0");	
	gOrigin.SetOrigin(newOrigin);
}

/*---------------------------------------------------------------
    (RESUME)

    If the user touched a touch zone and paused a playing segment
    (audio or video) then this command will resume playback.
-----------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Resume)
{
	gDebugLog.Log("resume");

	gPlayerView->DoResume(false);
}

/*----------------------------------------------------------------------
    (REWRITE FILENAME FIELD1 <FIELD2> ... <FIELDN>)

    FILENAME    The text file which contains the record to be rewritten.

    FIELD1..N   The fields which define the record.

    Given a file that is open for appending, this command will look up
    the given record, as specified by the fields. It will move that
    record to the end of the file and position the file pointer at the
    end of the file (appending) so that the specific data may be
    written.
------------------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Rewrite)
{
    TString		searchString, param, filename;
    int16		numFields = 0;

    inArgs >> filename;

    //  Append all the fields together into a search string that looks
    //  like "field1 TAB field2 TAB ... fieldN"
    //
    while (inArgs.HasMoreArguments()) 
    {
        inArgs >> param;
        if (numFields > 0)
            searchString += '\t';
        numFields++;
        searchString += param;
    }

	gDebugLog.Log("rewrite: file <%s>, look for <%s>", filename.GetString(), searchString.GetString());

    gFileManager.Rewrite(filename, searchString, numFields);
}

/*---------------------------------------------------------------
    (RNODE PICTURE.PCX || FONT.GFT)       10MAR94
    DISABLED!!
----------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Rnode)
{
#ifdef CBO_FIX
    TString NodeKey;

    inArgs >> NodeKey;
   //   gHeaderManager.Remove(NodPt);
   // ZapNode(NodeKey);

#endif
}

//
//	DoQTPause - 
//
//		(pause time)
//
/*
DEFINE_5L_PRIMITIVE(QTPause)
{
	int32	tenths;
	
	inArgs >> tenths;
	
	if (gMovieManager.Playing())
	{
		gDebugLog.Log("pause: %ld milliseconds", tenths);
		gPlayerView->DoPause(false);
		
		TInterpreter::GetInstance()->Nap(tenths);
		mResumeMovie = true;		// resume movie when we wake up
				
		// cbo_test - took this out to prevent flashing
		gPlayerView->AdjustMyCursor();
		gPlayerView->Draw(nil);
		gPlayerView->ProcessEvents(true);
	}
	else
		gDebugLog.Log("pause: nothing playing");
	
}	
*/

/*---------------------------------------------------------------
    (SCREEN COLOR)

    A fast way to fill the entire screen with a particular color.
-----------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Screen)
{
    int16 color;

    inArgs >> color;

	gDebugLog.Log("screen: <%d>", color);

    gPlayerView->ColorCard(color);
}

/*-----------------------------------
    (SEARCH FRAME [FLAG])

    Search to the given frame number.
        If FLAG present (anything) then wait for completion.
-------------------------------------*/
DEFINE_5L_PRIMITIVE(Search)
{
#ifdef CBO_FIX
    int32    frame;

    inArgs >> frame;

    gVideoManager->search(frame);


#endif
}

/*---------------------------------------
    (SET VARIABLE NEWVALUE)

    Sets the variable to the given value.
-----------------------------------------*/
DEFINE_5L_PRIMITIVE(Set)
{
    TString     	vname;
    TString			value;
    TString			flag;
    uint32			date;
    int32			date_type;
	
    inArgs >> vname >> value;
    
    if (inArgs.HasMoreArguments())
    {
    	inArgs >> flag;
    	
    	flag.MakeLower();
    	
    	if (flag == (char *) "longdate")
    		date_type = DT_LONGDATE;
    	else if (flag == (char *) "date")
    		date_type = DT_DATE;
    	else if (flag == (char *) "time")
    		date_type = DT_TIME;
    	else if (flag == (char *) "year")
    		date_type = DT_YEAR;
    	else if (flag == (char *) "month")
    		date_type = DT_MONTH;
    	else if (flag == (char *) "longmonth")
    		date_type = DT_LONGMONTH;
    	else if (flag == (char *) "day")
    		date_type = DT_DAY;
    	else if (flag == (char *) "longday")
    		date_type = DT_LONGDAY;
    	else
    		gLog.Caution("Bad flag to set command <%s>", flag.GetString());

		date = (uint32) value;
		
    	gVariableManager.SetDate(vname.GetString(), date, date_type);

		gDebugLog.Log("set date: <%s> to <%s>", (const char *) vname, gVariableManager.GetString(vname.GetString()));  	
    }
    else
    {
		gDebugLog.Log("set: <%s> to <%s>", (const char *) vname, (const char *) value);

    	gVariableManager.SetString(vname.GetString(), value.GetString());
    }
}


/*---------------------------------------------------------------------
    (SHOWMOUSE)
    Shows the mouse (shouldn't be needed, maybe only in conjunction w/ hide~)
 ---------------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Showmouse)
{
	gCursorManager.ShowCursor();
}

/*----------------------------------------------
    (STILL)

    Pause the video playback.
------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Still)
{
	gDebugLog.Log("still");

	gPlayerView->DoPause(false);
}

/*----------------------------------------------
    (SUB VARIABLE AMOUNT)

    Subtract the given amount from the variable.
    
    cbo - This originally was written to use floats.
------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Sub)
{
	TString 	theVarName;
	uint32		theAmount;
	uint32		theOrigValue;
	uint32		theResValue;

    inArgs >> theVarName >> theAmount;


    theOrigValue = gVariableManager.GetLong(theVarName.GetString());
    theResValue = theOrigValue - theAmount;

    gVariableManager.SetLong(theVarName, theResValue);

	gDebugLog.Log("sub: %s <%ld> - <%ld> = <%ld>", 
		(const char *) theVarName, theOrigValue, theAmount, theResValue);
}

/*--------------------------------------------------------------
    (TEXT HEADER LEFT TOP RIGHT BOTTOM TEXTSTRING)

    Display the given textstring, using the given header style,
    within the given rect. Note that the bottom of the rectangle
    is elastic... it will actually be as much or as little as
    necessary to display all the text.
----------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Text)
{
	CPlayerText	*textPtr;
	TRect		bounds;
	TString 	header, text;

    inArgs >> header >> bounds >> text;

    gOrigin.AdjustRect(&bounds);

	gDebugLog.Log("text: header <%s>, text <%s>", header.GetString(), text.GetString());
    
    textPtr = new CPlayerText(header.GetString(), bounds, text.GetString(), 0, 0);
	if (textPtr != nil)
		delete textPtr;
}

/*--------------------------------------------------------------
    (TEXTAA STYLESHEET LEFT TOP RIGHT BOTTOM TEXTSTRING)

    Display the given textstring, using the given header style,
    within the given rect. Note that the bottom of the rectangle
    is elastic... it will actually be as much or as little as
    necessary to display all the text.
----------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(TextAA)
{
	TRect		bounds;
	std::string style, text;

    inArgs >> style >> bounds >> text;

    gOrigin.AdjustRect(&bounds);
	gDebugLog.Log("textaa: style <%s>, text <%s>",
				  style.c_str(), text.c_str());

	try
	{
		gStyleSheetManager.Draw(style, text,
								GraphicsTools::Point(bounds.Left(),
													 bounds.Top()),
								bounds.Right() - bounds.Left(),
								gPlayerView);
	}
	catch (std::exception &error)
	{
		gDebugLog.Error("ERROR: %s", error.what());
	}
	catch (...)
	{
		gDebugLog.Error("ERROR: Unknown exception");
	}
}
        
        
/*-----------------------------------------------------------
    (TIMEOUT DELAY CARD)

    If the user doesn't respond in DELAY seconds, jump to the
    given card.
-------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Timeout)
{
    TString 	cardName;
    int32     	secs = 0;

    inArgs >> secs >> cardName;

	gDebugLog.Log("timeout: delay <%ld>s, jump to <%s>",
				  secs, (const char *) cardName);
	
	TInterpreter::GetInstance()->Timeout(cardName, secs);
}

/*--------------------------------------------------------------
    (TOUCH LEFT TOP RIGHT BOTTOM CARD CURSOR <PICT <X Y>>)

    Create a touch zone bounded by the given rectangle. Touching
    this touch zone will make the program go to card CARD. If a
    PICT is specified than that picture will highlight in
    reponse to the touch. If there are more than one copy of
    a given picture on the screen, specify an X and Y coordinate
    for the one you want to highlight.
----------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Touch)
{
    TRect       bounds;
    TPoint		loc;
    CPicture	*thePicture = NULL;
	TCallback	*callback;
    TString     picname;
    TString		cursorType;
    CursorType	cursor = HAND_CURSOR;
    CursorType	tmpCursor;

    inArgs >> bounds >> callback;

	gDebugLog.Log("touch: <L T R B> %d %d %d %d", bounds.Left(), bounds.Top(), bounds.Right(), bounds.Bottom());

    gOrigin.AdjustRect(&bounds);

	// Get our cursor, if any.
	if (inArgs.HasMoreArguments())
	{
		inArgs >> cursorType;

		tmpCursor = gCursorManager.FindCursor(cursorType);
		if (tmpCursor != UNKNOWN_CURSOR)
			cursor = tmpCursor;
	}

	// Get our picture-related arguments.
	if (inArgs.HasMoreArguments())
	{
		// Get the name of the picture
		inArgs >> picname;
		
		// Get it's offset (if specified), otherwise just use the topLeft
		// of the bounds
		if (inArgs.HasMoreArguments()) 
		{
			inArgs >> loc;	
			gOrigin.AdjustPoint(&loc);
		}
		else 
			loc = bounds.TopLeft();
	}
    
    if (not picname.IsEmpty())
    	thePicture = gPictureManager.GetPicture(picname);
    
    new CTouchZone(bounds, callback, thePicture, loc, cursor);
    
   // gPlayerView->AdjustMyCursor();
}

/*----------------------------------------------------------
    (UNBLIPPO <EFFECT> <DELAY>)

    Copies the offscreen buffer to the display with a given
    special effect. An optional delay will allow the user to
    control the speed of this effect. DELAY is the length in
    tenths of seconds that the effect should take.
------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Unblippo)
{
    TString	effect;
    int32   delay = 0;
    FXType	theEffect = kFXNone;

    if (inArgs.HasMoreArguments())
    {
        inArgs >> effect;
	        theEffect = StringToEffect(effect);

        if (inArgs.HasMoreArguments())
        	inArgs >> delay;
    }

	gDebugLog.Log("unblippo: Effect: <%d>  Delay: <%ld>", theEffect, delay);

	if (gPlayerView->BlippoAvailable())
		gPlayerView->UnBlippo(theEffect, delay);
	else
        gDebugLog.Caution("Unblippo: No Blippo bits available!");
}


/*----------------------------------------------------------
    (UNLOCK <EFFECT> <DELAY>)

    Copies the offscreen buffer to the display with a given
    special effect. An optional delay will allow the user to
    control the speed of this effect. DELAY is the length in
    tenths of seconds that the effect should take.
------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Unlock)
{
    TString	effect;
    int32   delay = 0;
    FXType	theEffect = kFXNone;

    if (inArgs.HasMoreArguments()) 
    {
        inArgs >> effect;
	        theEffect = StringToEffect(effect);

        if (inArgs.HasMoreArguments())
        	inArgs >> delay;
    }

	gDebugLog.Log("unlock: Effect: <%d>  Delay: <%ld>", theEffect, delay);

	gPlayerView->UnLock(theEffect, delay);
}

/*------------------------------------------------------
    (VIDEO EFFECT)

    Switch the video only mode with the given effect.

    Note: this effect is accomplished, in most cases, by
    temporarily switching to overlay (key) mode for the
    effect.
--------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Video)
{
#ifdef CBO_FIX
    TString effect;
    Effect  theEffect;

    inArgs >> effect;
//    theEffect = StringToEffect(effect);
//    gVideoManager->video(theEffect);
	pfadeup(true, true);

#endif
}


/*-----------------------------------------------------------
    (WAIT <FRAME>)

    Wait until the videodisc player reaches the given frame
    number, or the end of the currently playing segment if
    no frame number is given.

    This is a busy wait. See LVideo.cpp for information about
    what the system does while waiting for the frame.
-------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Wait)
{
    int32    frame = 0;

    if (inArgs.HasMoreArguments()) 
		inArgs >> frame;

	if (frame)
		gDebugLog.Log("wait: %ld", frame);
	else
		gDebugLog.Log("wait: to end");
	

	// have to check if we are playing something first to be sure
	//		WakeCard will actually do something and we sould pause
	if (gMovieManager.Playing())
	{
		// if we are playing audio, blast the gworld to the screen
		if (gMovieManager.AudioPlaying())
		{
			// cbo_test - took this out because was causing flashing
			gPlayerView->AdjustMyCursor();

			gPlayerView->Draw(nil);
			
			gPlayerView->ProcessEvents(true);
		}

		gMovieManager.WakeCard(frame);
		TInterpreter::GetInstance()->Pause();
	}
#ifdef DEBUG
	else
	{
		if ((frame == 0) and (gModMan->NoVolume()))
		{
			gDebugLog.Log("wait: no volume or movie, pausing");
			
			//::SysBeep(30);
			TInterpreter::GetInstance()->Pause();
		}
		else
			gDebugLog.Log("wait: nothing to wait for");
	}
#endif
}

/*-----------------------------------------------------------
    (WRITE FILENAME DATA)

    Write the given data to the file.
-------------------------------------------------------------*/
DEFINE_5L_PRIMITIVE(Write)
{
    TString     filename, data;

    inArgs >> filename >> data;

	gDebugLog.Log("write: file <%s>, data <%s>", filename.GetString(), data.GetString());

    gFileManager.Write(filename, data);
}