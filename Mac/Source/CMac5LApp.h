// ===========================================================================
//	CMac5LApp.h					�1994 Metrowerks Inc. All rights reserved.
// ===========================================================================

#pragma once

#include "KHeader.h"
#include "KLogger.h"
#include "CPalette.h"

class	CBackWindow;

class	CMac5LApp : public LApplication
{
	public:
							CMac5LApp();
		virtual 			~CMac5LApp();
	
		void				QuitScript(void);
		void				StartUp(void);

		bool				OpenScript(FSSpec *scriptSpec);	
#ifdef DEBUG
		void				ReDoScript(const char *curCard);
		bool				OpenScriptAgain(FSSpec *scriptSpec, const char *jumpCard);
		void				ReDoReDoScript(void);

		Boolean				ObeyCommand(CommandT inCommand, void *ioParam);
		void				FindCommandStatus(
								CommandT 			inCommand,
								Boolean&			outEnabled,
								Boolean&			outUsesMark,
								UInt16&				outMark,
								Str255				outName);
#endif
		void				CheckMemory(void);
		void				MaxMemory(void);
		void				DumpMemory(void);
		

//		void				SetPalette(PaletteHandle inPalHand, bool inActivateNow);
		
//		void				NewColorTable(CPalette *inPal, bool inGraphics);
		
//		void				DoNewPalette(CTabHandle inCTab);
//		void				RestorePalette(void);
//		CTabHandle			GetCTab(void) { return (mGraphicsPal->GetCTab()); }
//		void				CheckPalette(void);
//		bool				HaveNewPal(void)
//					{ return (mHaveNewPal); }
		
		Boolean				AttemptQuitSelf(Int32 inSaveOption);
		virtual void		HandleAppleEvent(
								const AppleEvent	&inAppleEvent,
								AppleEvent			&outAEReply,
								AEDesc				&outResult,
								Int32				inAENumber);
		
		void				DoExit(int16 inSide);
									
	protected:
		CBackWindow			*mDisplayWindow;
		
		void				CleanUp(void);
		void				SetGlobals(void);
		void				DoAEOpenDoc(
								const AppleEvent 	&inAppleEvent,
								AppleEvent			&outARReply,
								long 				inAENumber);
		
	private:
		Rect				mScreenRect;
		int					mBitDepth;
		bool				mScriptRunning;
//		bool				mHaveNewPal;
//		CPalette			*mGraphicsPal;
//		CPalette			*mMoviePal;
//		PaletteHandle		mCurPal;
		KString				mReDoCard;		// card to jump to when redo redoscript
		bool				mReDoReDo;		// waiting for a redo redoscript?
	
		bool				GetScriptFile(FSSpec *scriptSpec);
};

// global application object
extern CMac5LApp *gTheApp;

extern KLogger gLog;
extern KLogger gMissingMediaLog;
#ifdef DEBUG
extern KLogger gDebugLog;
#endif

extern Handle clickSound;
extern WindowPtr gWindow;
