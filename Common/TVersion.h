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
// TVersion.h : Version strings.
//

#define VERSION_MAJOR_NUM	3
#define VERSION_MINOR_NUM	5
#define VERSION_REV_BIG		10
#define VERSION_REV_SMALL	0

#define VERSION_STRING		"5L 3.5.10 (Development)"
#define SHORT_NAME			"5L"


/*
 $Log$
 Revision 1.42  2002/11/06 18:29:46  emk
 3.5.10 - 6 Nov 2002 - emk

 wx5L:

 Working on making wx5L portable.  The Windows wx5L build is slightly
 broken; I'll fix it once the portable code is happy.

   * Added FIVEL_NO_MOVIES define to supress use of QuickTime in wx5L builds.
   * Refactored MovieWindow into two classes: MovieWindow, which does
     nothing, and MovieWindowQT which does all the work.  Refer to
     MovieWindowNative to get the right one on a given platform.
   * Wrote scripts to convert *.ico and *.bmp files to *.xpm files.
   * Added AppGraphics.h and AppGraphics.cpp, which import xpm files.
   * Checked in some other utility scripts, and updated 5L-stats.

 Regular 5L:

 Added type information to 5L variables, and replaced (var ...) with a more
 powerful form of (define ...).  These changes should make Scheme more
 pleasant for content authors.

   * TVariable now stores type information.
   * Added SetTyped primitive, and replaced VariableExists with
     VariableInitialized.
   * Added support for "symbol" arguments to primitives.  These correspond
     to Scheme symbols, and should eventually be used when a primitive
     argument refers to a variable name (or one a small, fixed set of strings).
   * Fixed bugs in TVariable's unsigned integer handling.
   * Removed TYPE argument from call-5l-prim, engine-var, etc.
   * Renamed DEFINE-PERSISTENT-VARIABLE to DEFINE/P.
   * Updated fake-engine.ss to mimic new engine, and fixed bug in tool.ss.

 Revision 1.41  2002/10/31 22:58:48  emk
 Edited a comment which prevented GCC from compiling this file.

 Revision 1.40  2002/10/31 18:25:11  emk
 3.5.9 - 31 Oct 2002 - emk

   * New wxWindows front-end now officially added to the build tree.
     This is still massively incomplete, but it's on its way.
   * Lots of compilation flag tweaks, to get all the various Win32 projects
     on the same page re: threads, RTTI, exceptions, etc.
   * CryptTool removed as first step in preparing for open souce release.
     This will simplify our legal issues a bit.
   * Linux build fixes.
   * Fixed bug where MacOS X 5L always played the same video from the CD.
   * Converted all *.rsrc and *.PPob files to *.r files, and included
     instructions on converting them back and forth.  This will allow us
     to play a bit more nicely with CVS, and eliminate all but a few
     files with resource forks from our CVS repository.
   * Source tree re-organization:
     - Common/libs -> libs
     - Common/freetype2 -> libs/freetype2
     - Rename Common/Runtime/5L/ *.ss to have lowercase names
     - Linux, Mac, Win32 build fixes

 Revision 1.39  2002/10/15 18:32:34  emk
 3.5.8 - 15 Oct 2002 - emk

 Engine:

   * The Windows engine now reloads scripts in the same fashion as the
     Mac engine--if a load fails, you get a chance to retry it.
   * Log files get flushed after every line.
   * The LOG and SCHEMEIDLE primitives are no longer logged, to reduce
     clutter in Debug.log.
   * Fixed tons of bugs in places where the Windows engine assumed it
     had a TInterpreter object, but didn't (i.e., lots of "sInstance"
     assertions are gone).
   * Added support for measuring text without drawing it.
   * Added support for checking whether an engine variable is initialized.
   * Made sure LCursor initializes mForceShow.

 Runtime:

   * Support for compiling *.ss files to *.zo files automatically
     (incomplete--it only handles modules loaded with REQUIRE right now).
   * Support for deferring code until the engine is in a safe place to
     run it.  This means that you can call VIDEO, INPUT, etc., from
     within a callback.
   * The syntax of 'for' loops has been simplified.
   * Explicit support for using stylesheets with the INPUT command.
   * Added 'card-exists?' predicate.
   * Fixed bug in 'member?'.
   * Added support for using (var ...) declarations within 'lambda', 'define'
     and 'let'.
   * Added support for initializing engine variables.
   * Made call-5L-prim handle return values of non-string types gracefully.
   * Fixed bugs causing centered and left-aligned text to be measured
     incorrectly.
   * Added a hook system based on Emacs.

 Revision 1.38  2002/10/09 17:34:33  emk
 3.5.7 - 9 Oct 2002 - emk

   * Scheme: Changed 'for-each-item' to 'foreach', and added 'for'.
   * Added extract-docs.pl, which generates HTML manuals.
   * Added many new test cases for the new 5L language.
   * Fixed minor bugs in CryptStream*.*, as discovered by valgrind.
   * All primitives which used to take palette indices now take RGB colors.
   * Old 5L: Added DEFPALETTE command for declaring palettes without
     BMP files.  This provides backwards compatibility for old code.
   * Removed Windows cursor-clipping code because it was occassionally
     immobilizing the cursor completely.

 Revision 1.37  2002/10/03 19:26:10  emk
 Rebuilt Windows engine with MzScheme 202 and cursor fix from 3.4.2.

 Revision 1.36  2002/10/03 02:46:29  emk
 3.5.5 - 1 Oct 2002 - emk

 Preliminary MzScheme support for Macintosh engine.  DO NOT BUILD THE
 NON-CARBON POWERPC BUILD!  WHEN RUN UNDER OS X'S OS 9 EMULATOR, IT WILL
 CORRUPT YOUR OS 9 SYSTEM FILE.

 For legal reasons, this Macintosh engine SHOULD NOT BE DISTRIBUTED outside
 of IML until we do our open source release.  It's statically-linked against
 the LGPL'd MzSchemeLib, which would require us to jump through some
 annoying hoops if we wanted to ship proprietary binaries.

 User-visible changes:

   * This engine supports both legacy 5L and Scheme.
   * No PowerPC build.
   * No more start scripts.  Move everything into your main script.
   * No more switching between scripts.
   * CD ejection and fades are gone, thanks to the fact this build is
     Carbon-only.

 Engine changes:

   * Created CodeWarrior project for MzSchemeLib, to avoid the pay-per-view
     cage match between MPW StdCLib and Metrowerks Standard Library over
     who's going to link with what.
   * Fixed MzScheme's call to PBGetCatInfo to work under Carbon.
   * Added code to explicitly set Boehm GC stack base on the Macintosh.
     Boehm can't figure this out without help, and will crash on startup.
   * Namespace fixes to make things compile on the Mac.
   * Made a mess of the MzSchemeLib MPW build infrastructure trying to get
     things to work.  I'm checking in my hacked-up version because (1) it
     marginally works and (2) we might need it again someday.
   * Removed a number of unused files with licensing problems, and sorted
     the rest of the problematic files into categories.
   * Changed event-handling model to give the TInterpreterManager object control,
     instead of the traditional PowerPlant::LApplication::Run method.  This
     requires us to roll our own version of Run and do some tricky event
     handling in CMac5LApp::MacIdleProc.  But it also means the Mac engine
     now supports arbitrary interpreters.  This change has lots of
     consequences, most of them complicated--and it may affect stability.
   * CPlayerView is now more careful about checking whether a TInterpreter
     object actually exists before calling methods on it.
   * Renamed MakeNewIndex methods to ProcessTopLevelForm.
   * Moved all old5l-related files into Mac/Source/lang/old5l.
   * Moved interpreter-managing code into TMac5LInterpreter.{h,cpp}.
   * Moved file-related primitives into TMac5LPrimitives.cpp.
   * Shift-scripts are gone.
   * Ripped out support for switching scripts.
   * Reimplemented script reloading, exiting, etc., using the
     TInterpreterManager interface.
   * Lots of changes to the Macintosh project files.
   * Updated Windows interpreter to newest MzScheme release.
   * Fixed the Linux build.

 Headers (and therefore text input) are only available to the old 5L
 language.  This will change soon.

 Revision 1.35  2002/08/22 00:12:05  emk
 3.5.4 - 21 Aug 2002 - emk

 Engine:

   * Moved many source files from Common to Common/lang/old5L, and from
     Win32/FiveL to Win32/FiveL/lang/old5l, including the index system, the
     parser and stream classes, the crypto classes and the file I/O classes.
   * Broke the dependencies between Header and TIndex, in a fashion similar
     to what I did for TStyleSheet in 3.5.1.  This means we can call
     INPUT from Scheme, which more-or-less completes the Scheme primitives.
   * Made sure that header and stylesheet names were case insensitive.

 Revision 1.34  2002/08/19 22:31:48  emk
 3.5.3 - 18 Aug 2002 - emk

 Changes to support the rapidly-growing test suites.

 Engine:

   * Renamed _error variable to _errorcode to avoid conflicting with the
     _error variable used by the 'video' command (which we'll probably
     need to clean up sometime, but that's life).  This will affect
     scripts which call the BROWSE command, but not much else.

 Runtime:

   * Implemented DEFINE-ENGINE-VARIABLE, which provides transparent
     binding between Scheme and 5L variables.
   * Added *TEXT-X*, *TEXT-Y*, *GRAPHIC-X* and *GRAPHIC-Y* variables,
     which are bound to _INCR_X, INCR_Y, _Graphic_X and _Graphic_Y.
   * Added WHILE and FOR-EACH-ITEM loops.
   * Added HAVE-5L-PRIM? and REFRESH functions.
   * Modified JUMP function to use JUMP primitive when available, so video
     gets killed when jumping between cards.
   * Refresh the screen at the end of each card.

 Revision 1.33  2002/08/19 17:27:28  emk
 3.5.2 - 18 Aug 2002 - emk

   * Modified the TSchemeInterpreter class to make intelligent use of
     namespaces and modules.
   * Fully modularized the runtime, and made the lispish language extensions
     available.  The "lispish" code currently belongs to me (and is released
     under the LGPL), but I'll be happy to relicense it to the Trustees.
   * Got redoscript to work--we no longer attempt to restart the entire
     Scheme interpreter; we merely throw away a sandbox.
   * Moved the code for resetting stylesheets out of the TWin5LInterpreter
     and into the main Win32 engine.

 Revision 1.32  2002/08/17 01:41:55  emk
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

 Revision 1.31  2002/08/16 16:26:22  emk
 3.5.0 - 16 Aug 2002 - emk, zeb

 Preliminary Scheme support for Windows.  The Macintosh build is broken
 until Brian updates the event loop to use a TInterpreterManager object
 and figures out how to get the mzscheme libraries building.

 Ported 3.4.1 changes forward to Windows.

 Revision 1.30.2.1  2002/08/14 20:24:49  emk
 Language bugfixes/enhancements/changes for HIV Prevention Counseling.  I
 removed some deeply-buried bugs in TStream and elsewhere, so please test
 this build thoroughly.

   * New entities: &shy;, &nbsp;, and &radic;.  I've also added
     &check; and &cross;, but we don't have the necessary font support yet.
   * TStream now handles whitespace rationally.  String literals are
     parsed verbatim, and the old "randomly munge whitespace" behavior
     has been fixed.  Most of the other changes are necessary consequences
     of this change.
   * Verbatim CR, LF and TAB characters in strings will be passed through.
     This may affect screen layout.
   * The (get ...) primitive has been backported from 3.5.
   * The '&' syntax has been removed.  Instead of '&foo$bar', you should
     now write '$(get foo$bar)'.
   * Entities don't need to be escaped any more: \&amp; -> &amp;.

 Thanks to this cleanup, it was possible to implement several much-wanted
 features without too much work:

   * New primitives: WHEN, UNLESS and WHILE.
   * BODY has been renamed to BEGIN, and longer prematurely evaluates all
     the variables in nested expressions.
   * Debug log improvements.

 Revision 1.30  2002/07/26 20:10:22  emk
 (Updating the *rest* of the version numbers.  Ooops.)

 Revision 1.29  2002/07/26 20:07:39  emk
 3.4.0 - 26 July 2002 - emk

   * Updated version numbers for official stable release!

 Revision 1.28  2002/07/26 20:00:15  zeb
 3.3.21 - 26 July 2002 - zeb

   * Added FileSystem::ExistenceCheck, which we use to check for the
     existence of various files during the startup process (bug #937).

 Revision 1.27  2002/07/26 17:55:10  emk
 3.3.20 - 26 July 2002 - emk

 A QA binge, thanks to RedHat's memprof, Bruce Perens' Electric Fence,
 and Rational's Purify.

   * Linux build fixes so I can run memprof and Electric Fence.
   * Fixed a bug in TStream::GetStringArg when called on an empty stream.
     This is probably why we were seeing weird results when CHeader called
     TStream::more() too many times.
   * Fixed a buffer-overflow bug in TLogger when logging large messages.
   * Squashed a bunch of memory leaks in CryptStream.cpp.
   * Made new CryptStream auto_ptr code work under Windows.
   * PURIFY: Fixed memory leak in TBTree::Add of duplicate node.  We now
     notify the user if there are duplicate cards, macros, etc.
   * PURIFY: Fixed memory leak in TBTree destructor.
   * PURIFY: Fixed memory leak in ConfigManager destructor.
   * PURIFY: Fixed memory leaks when deleting DIBs.
   * PURIFY: Made sure we deleted offscreen GWorld when exiting.
   * PURIFY: Fixed memory leak in LBrowser.
   * PURIFY: Fixed memory leak in LFileBundle.
   * PURIFY: Fixed uninitialized memory reads when View methods were
     called before View::Init.
   * PURIFY: Made View::Draw a no-op before View::Init is called.
     (It seems that Windows causes us to call Draw too early.)
   * Added TOUCHCOUNT, TOUCHCOORDS and TOUCHACTIVATE commands so Douglas
     can build an automatic test monkey.  These are Win32-only, because
     the Mac touchzone system needs an overhaul and I don't want to
     mess with it right now (#1076).
   * Added StValueRestorer<> template class which can save and restore
     the values of variables in an exception-safe fashion.
   * Began code audit for exception safety (bug #1074).

 Revision 1.26  2002/07/24 17:41:02  emk
 3.3.19 - 24 July 2002 - emk

   * Cleaned up Win32 5L.log (bug #1057).
   * We now print the glyph cache size every 100K (bug #969).

 Revision 1.25  2002/07/24 01:16:43  emk
 3.3.18 - 23 July 2002 - emk

   * Forward-ported QtComponentVersion to the Mac (bug #1054).
   * Fixed redoscript/keybind race condition (bug #1036).
   * Added a top-level try/catch block on the Mac (bug #955).
   * Added error checking in Mac BROWSE (bug #1070).
   * Enlarged our growzone a bit, and made sure it's working.
     This will help us determine if we have memory usage problems.

 Revision 1.24  2002/07/23 21:53:41  emk
 3.3.17 - 23 July 2002 - emk

   * Fixed RETURN in macros (bug #1053).
   * Fixed typography exception when missing buttpcx graphic (bug #1039).
   * Made Win32 BROWSE return an error if it fails (bug #793).
   * Forward-ported QtComponentVersion to Win32 (bug #1054).
   * Performance tuned Win32 textaa (bug #933).

 Revision 1.23  2002/07/19 21:17:47  emk
 * Fixed "b" parameter to defstyle (bug #1052).
 * Fixed Mac engine to log missing macros to Debug.log (bug #1060).

 Revision 1.22  2002/07/17 16:06:48  brian
 3.3.15 - 17 July 2002 - brian

   * Fixed excessive error messages for missing button graphics (#1039).
   * This bug might still occur on Windows; it needs to be looked into.

 Revision 1.21  2002/07/15 18:11:31  brian
 3.3.14 - 15 July 2002 - brian, emk

   * Fixed GWorld flashing (bug #930).
   * Windows 3.3.13 fixes ported to the Macintosh.

 (emk, checking in from Brian's account after finishing some
 bug-stomping for him)

 Revision 1.20  2002/07/15 15:56:32  zeb
 3.3.13 - 15 July 2002 - zeb, emk
   * Language change: (IF cond true_cmd false_cmd) now takes arbitrary
     expressions for 'cond'.  The following new primitives have
     been added: AND, OR, NOT, contains, =, <>, <, >, <=, >=.
   * Added a new (LOG filename msg) command, which allows the programmer
     to write to "5L", "debug" and "MissingMedia" logs.
   * Major logging improvements: All primitives are now automatically
     logged in a standard format (bug #1003).
   * Adjusting of coordinates using origin is now logged.
   * Callbacks are now logged in a much more useful fashion.
   * Old arithmetic primitives now return a value (add, sub, div).
   * Added MakeQuotedString to TTemplateUtils and wrote a matching test suite.

 Revision 1.19  2002/07/10 15:21:33  brian
 3.3.12 - Improved debug.log for text and textaa commands (bug #979).

 Revision 1.18  2002/07/08 16:43:46  emk
 3.3.11 - Bugfixes from 3.2.0.5 through 3.2.0.7.

   * Ported Win32 QuickTime 6/VP3 bugfix forward from 3.2.0.x.
   * Ported Win32 QuickTime 6 gamma bugfix forward from 3.2.0.x.
   * Ported Win32 line drawing bugfix forward from 3.2.0.x.
   * Fixed Win32 (touch ...) command to highlight touchzones more like the
     Macintosh.  (It now redraws the unhighlighted graphic at the end of the
     highlight sequence.)

 Revision 1.17  2002/07/08 14:25:33  zeb
 3.3.10 - 7 July 2002

   * Added a new Release-Notes.txt file.
   * Fixed a bug which caused LFileBundle to die with an assertion
     failure when importing an empty file.
   * New feature: Modulo primitive.  5L now supports Modulo(x,y).
     Syntax is (% x y).

 Revision 1.16  2002/06/24 15:17:11  emk
 3.3.9 - Highly experimental engine which makes _INCR_X,
 _INCR_Y, _Graphic_X and _Graphic_Y relative to the current
 origin.   This will break macros in existing code!

 Revision 1.15  2002/06/21 15:41:58  emk
 3.3.8 - 5L language improvements, including nested expressions,
 return values and new primitives.

   * Expressions can now be nested: '(set x $(+ 2 $(* 3 5)))' will
     set 'x' to 17.  Nested expressions should be indented as follows:

       (set x $(+ $really_big_variable_1
                  $really_big_variable_2))

     ...that is, arguments should _stack in a column_.  I will be
     extremely anal about this if I'm reading your code.

   * '(return ...)' now takes an optional argument, which will be
     returned from the macro.  So you can define your own functions, too.

   * New primitives: +, -, *, /, truncate, float+, float-, float*,
     float/, strlen, substr, findsubstr, length, nth, haskey, getval.

 A note on Lisp naming conventions--when you create a new "data structure"
 type, you should generally name functions as follows:

   # Define a type 'pt' with members 'x' and 'y' using lists.
   (macrodef pt (return ($1 $2)))
   (macrodef pt-x (return $(nth 0 $1)))
   (macrodef pt-y (return $(nth 1 $1)))

   # Alternative implementation of a type 'pt2' using associative lists.
   # (You couldn't pass 'pt2' to a built-in command, but it's a nice small
   # example.)
   (macrodef pt2 (return (x $1 y $2)))      # (pt2 10 20) => (x 10 y 20)
   (macrodef pt2-x (return $(getval $1 x))) # (pt2-x ...) => 10
   (macrodef pt2-y (return $(getval $1 y))) # (pt2-y ...) => 20

 The function 'pt' is called the "constructor", and the functions 'pt-x' and
 'pt-y' are called "accessors".

 Revision 1.14  2002/06/20 21:01:48  emk
 3.3.7 - Debug log updates, and error message if 5L is run without
 Mac5L.config or other support files.

 Revision 1.13  2002/06/20 19:49:57  emk
 3.3.6 - Fixed problems with BUTTPCX (and other?) highlighting.

 Revision 1.12  2002/06/20 16:31:00  emk
 3.3.5 - Merged the 'FiveL_3_3_4_refactor_lang_1' branch back into the trunk.
 This branch contained the following enhancements:

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

 Revision 1.11.2.11  2002/06/20 15:58:44  emk
 3.3.4.12 - Last build of the 'FiveL_3_3_4_refactor_lang_1' branch
 before merging.

 Revision 1.11.2.10  2002/06/19 22:50:55  emk
 3.3.4.11 - Refactored Mac code to move primitives from CCard.{h,cpp} to
 TMacPrimitives.{h,cpp}, and break most of the remaining dependencies on
 the 5L interpreter.

 Language changes: LOADPICK, RVAR and RNODE are gone.  I've also disabled
 the Mac PAUSE command until Douglas tells me how it should work.

 Testing: Please beat *very* hard on this build, and pay special attention
 to WAIT, NAP, TIMEOUT, and similar commands.

 Next up: I plan to merge this branch into HEAD tomorrow.

 Revision 1.11.2.9  2002/06/19 18:28:47  emk
 3.3.4.10 - Debug log message improvements on the Mac.

 Revision 1.11.2.8  2002/06/19 14:07:13  emk
 3.3.4.9: Fixed _Origin_X, _Origin_Y on Windows, and added a "default style"
 parameter to "defstyle" on both platforms.

 The new syntax:

   (defstyle sample (Nimbus Roman No9 L) 12 r left 0xF0F0F000 0xFFFF0000)

 ...where r = "regular", b = "bold", i = "italic" and bi = "bold italic".

 Revision 1.11.2.7  2002/06/18 21:56:38  emk
 3.3.4.8 - Added (BODY ...) command on Mac, fixed arguments of BUTTPCX, TOUCH,
 and KEYBIND to match Win32 engine, and refactored Mac engine to more-or-less
 respect the TInterpreter interface.

 Things to test: REDOSCRIPT, redo-REDOSCRIPT (feed REDOSCRIPT a bogus script,
 try to fix it, then run REDOSCRIPT again), TOUCH, BUTTPCX, ORIGIN.

 Some low-level details:

   - Added a KillCurrentCard method to the TInterpreter interface.  This
     works a lot like Pause, but it cannot be resumed.
   - Added a rough-cut TMac5LInterpreter class (with some methods stubbed
     out, because they are not needed on the Mac--we should look at
     this API in detail and formalize it sometime after 3.4).
   - Modified CTouchZone to take TCallback objects.
   - Modified CPlayerView's keybinding support to take TCallback objects
     (and to use a std::map instead of a PowerPlant list class).
   - Began to separate special forms (IF, BODY, EXIT, RETURN) from other
     commands.
   - Moved ReadSpecialVariable_* methods out of CCard and into CMac5LApp.
   - Made sure CMac5LApp::mReDoReDo got initialized to false.
   - Merged OpenScript and OpenScriptAgain into one function.

 Revision 1.11.2.6  2002/06/15 01:06:32  emk
 3.3.4.7 - Carbonization of Mac build, support for running non-Carbonized build
 in MacOS X's OS 9 emulator, and basic support for 5L.prefs on the Mac.  The
 Carbon build isn't yet ready for prime time--see BugHunt for details--but it
 is good enough to use for engine development.

 * Language changes

   - CHECKDISC is gone; use CHECKVOL instead.
   - EJECT is disabled in the Carbon build, because Carbon has no way to
     identify CD drives reliably.  EJECT still works in the regular build.
   - Gamma fades are ignored in the Carbon build.
   - KEYBINDs must now be accessed with the Command key only--not Option.

 * Things to test

 Please be hugely brutal to 5L; this is a big update.

   - 8-bit systems, palettes, ORIGIN, EJECT on the non-Carbon build.

 * Internal changes

   - TException class (and all subclasses) now take a __FILE__ and __LINE__
     parameter.  This is ugly, but it allows me to debug 5L exceptions even
     without a working debugger (under the OS 9 emulator, for example).
   - FileSystem::Path::(DoesExist|IsRegularFile|IsDirectory) now rely on
     native MacOS File Manager calls instead of the broken MSL stat()
     function (which fails in the OS 9 emulator).
   - The ImlUnit test harness flushes its output more often.
   - Many data structure accessors (and such functions as c2pstr) have been
     replaced by their Carbon equivalents.
   - We now use PowerPlant accessors to get at the QuickDraw globals.
   - We now use PowerPlant calls in place of ValidRect and InvalRect.
   - Some very nasty code which set the palettes of our offscreen GWorlds
     has been removed (offscreen GWorlds have CLUTs, not palettes!).
     The various drawing commands now use gPaletteManager to map indexes
     to RGBColor values, and RGBForeColor to set the color--no more calls
     to ::PmForeColor on offscreen GWorlds, thank you!
   - The CMenuUtil code (which used low-memory system globals to hide
     and show the menu bar) has been removed entirely and replaced by
     calls to HideMenuBar and ShowMenuBar (which are present in 8.5 and
     Carbon).  This is much simpler, nicer, more portable and safer.
   - A bunch of code which had been disabled with #ifdefs has been
     removed entirely.  This mostly related to palettes and an obsolete
     version of the fade code which used GWorlds.
   - Code which used ROM-based KCHR resources to map option keys back to
     their unmodified key caps has been removed.  This means KEYBINDs
     can only be accessed using the Command key.
   - We assume Carbon systems always support the HFS file system (duh).
   - We use PowerPlant glue to access either StandardFile or Navigation
     Services, under OS 8/9 and Carbon, respectively.
   - Some old subroutines in CModuleManager appeared to have been
     snarfed from More Files, an old Mac utility library.  These have
     been moved into MoreFiles.{h,cpp}.

 * Known Carbon Problems

 Fades, ejecting CD-ROMs and playing QuickTime movies are all broken in
 the Carbon build.  Douglas has found a problem with ORIGIN.  It looks
 like we should continue to ship the OS 9 build for use with MacOS X,
 at least for next few months.

 Revision 1.11.2.5  2002/06/12 19:42:36  emk
 3.3.4.6 - Fixed bug where the origin didn't get restored after each macro
 call.  (This bug was introduced in 3.3.4.5.)

 Revision 1.11.2.4  2002/06/12 19:02:51  emk
 3.3.4.5 - Moved Do* commands from Card.{h,cpp} to TWinPrimitives.{h,cpp},
 and broke the remaining dependencies between these primitive commands and
 the current 5L interpreter.  The TInterpreter and TPrimitives interfaces
 are now quite mature.

 *** Please beat very, very hard on this build.  I don't anticipate
 further changes to the Windows engine for a while. ***

 REMOVED COMMANDS: kill (use still), loadpick (use loadpic)
 NEEDS TESTING: origin w/macros, other uses of origin.  5L now
   sets the origin to 0,0 whenever it begins a new card, which
   should produce behavior identical to the old system, unless
   I've overlooked something.
 NEEDS TESTING: make sure all the commands are available, and
   have the right names.  I've checked this a dozen times
   by eye, but I might have overlooked something.

 The only remaining dependencies between the interpreter and the rest of 5L
 are in the Header and TStyleSheet classes.  I'm postponing this last bit
 of cleanup until after 3.4.  Up next: Repeat the 3.3.4.{1-5} changes for
 the Macintosh.

 Revision 1.11.2.3  2002/06/11 18:15:31  emk
 3.3.4.4 - Partial separation of primitives from interpreter, and
 various 5L language enhancements related to callbacks.

   - Finished fleshing out TArgumentList, added support for callbacks.
   - Made all built-in primitives access their arguments through the
     TArgument interface.
   - Implemented a BODY command.
   - Changed how the TOUCH, BUTTPCX and KEYBIND commands parse their
     callback arguments.  See below for details; you'll have to change
     some code.  This was necessary to move callback parsing into
     TStream's implementation of the TArgumentList interface.

 5L Language Changes
 -------------------

   * (KEYBIND ...) now takes an arbitrary command instead of a card name.
     As with TOUCH and BUTTPCX, variables are evaluated when the
     keybind is installed, not when it is invoked.  Examples:

       (keybind f (jump foo))
       (keybind a (add x 10))

   * You can now run a series of zero or more commands using (BODY cmd...).
     This should work with IF, TOUCH, BUTTPCX and KEYBIND.  Example:

       (body
         (set x 10)
         (set y 20))

     Commands such as WAIT, JUMP, NAP, etc., will not do what you expect
     unless they're the last statement in a BODY.  This is caused by the
     low-level design of the interpreter, and is non-trivial to fix.

     RETURN is also not BODY-friendly.

     When you pass a body to IF, TOUCH, BUTTPCX or KEYBIND, all the
     variables in the body will be evaluated *before* any code is run!

   * The arguments to BUTTPCX and TOUCH have been rationalized after
     consultation with Douglas.  The commands now work as follows:

       (TOUCH rect cmd [cursor [picture [point]]])
       (BUTTPCX picture point header label cmd [cursor])

     Note that the second callback has disappeared from both TOUCH and
     BUTTPCX; use BODY instead.

 Revision 1.11.2.2  2002/06/10 17:52:48  emk
 3.3.4.3 - Added a TArgumentList class in TPrimitives.  This class provides
 an abstract interface to argument list parsing, and replaces parts of
 TStream.  This will allow us to begin breaking dependencies between
 the primitives and the nasty parsing gunk in TStream.

 Revision 1.11.2.1  2002/06/05 20:42:29  emk
 3.3.4.2 - Broke Win5L dependencies on TIndex file by moving various pieces
 of code into TWin5LInterpreter.  Windows 5L now accesses the interpreter
 through a well-defined API.  Changes:

   * Removed many direct and indirect #includes of TIndex.h.
   * Added a TInterpreter method ReloadScript, which can be called by the
     higher-level ReDoScript command.
   * Checked in some files which should have been included in the 3.3.4.1
     checkin--these files contain the initial refactorings of Card and Macro
     callsites to go through the TInterpreter interface.

 Up next: Refactor various Do* methods out of Card and into a procedural
 database.

 Revision 1.11  2002/05/29 13:58:10  emk
 3.3.4 - Fixed various crash-on-exit problems (including those in TBTree,
 TIndex and TLogger::FatalError), and reverted the Win32 _INCR_Y code
 to the behavior that shipped with Genetics.

 Revision 1.10  2002/05/15 11:05:18  emk
 3.3.3 - Merged in changes from FiveL_3_3_2_emk_typography_merge branch.
 Synopsis: The Common code is now up to 20Kloc, anti-aliased typography
 is available, and several subsystems have been refactored.  For more
 detailed descriptions, see the CVS branch.

 The merged Mac code hasn't been built yet; I'll take care of that next.

 Revision 1.9.2.9  2002/05/01 11:34:02  emk
 Added support for passing a "(pcent ...)" argument to "defstyle" to
 specify leading as a percentage of the base font size (and cleaned up
 a few minor test suite issues).

 Revision 1.9.2.8  2002/05/01 07:10:49  emk
 3.3.2.7 - Fixed assertion failure on "Special Variables" screen, fixed
 missing bullets, and added "\&Delta;" and "\&delta;" entities for
 use with the (textaa ...) command only.

 Revision 1.9.2.7  2002/05/01 03:27:02  emk
 3.3.2.6 - First Windows engine with (textaa ...) command.

 - Implemented a primitive, slow Image::DrawPixMap command that uses
 ::GetPixel and ::SetPixel to do alpha blending (shudder).  Strangely
 enough, it's about as fast as the somewhat optimized Mac routines.
 Anyone got a good GDI book?

 - Fixed several assertion failures.

 Known problems:

 - Occasional assertion failure on exit.  The reference-counting on
 TIndexFile claims it's getting dereferenced too many times.  This is
 an old bug; all the TBTree and TBNode classes are pretty dodgy.

 - Assertion failure on "Special Variables" screen in 5Ltest.  This is
 caused by overlong lines.

 Revision 1.9.2.6  2002/04/30 07:57:24  emk
 3.3.2.5 - Port Win32 code to use the 20Kloc of Common code that now
 exists.  The (defstyle ...) command should work, but (textaa ...) isn't
 available yet.

 Next up: Implement the (textaa ...) command and the low-level
 GraphicsTools::Image::DrawBitMap.

 Revision 1.9.2.5  2002/04/29 06:29:58  emk
 3.3.2.4 - Contains first set of performance tweaks, and fixes problem with assertion failures after reload and on CME screens.

 Revision 1.9.2.4  2002/04/26 11:31:01  emk
 3.3.2.3 - Fixed highlight shadow color to default to regular shadow color (instead of highlight color, which is obviously wrong).

 Revision 1.9.2.3  2002/04/26 08:51:21  emk
 3.3.2.2 - First experimental engine with (textaa ...) and (defstyle ...) commands.

 Changes:

   - Ported new TEncoding template class to the Mac.

   - Updated TStyleSheet to provide bug-for-bug compatibility with the way backslashed escaped sequences are processed.

 Revision 1.9.2.2  2002/04/23 11:29:47  emk
 Prepended "VERSION_" to the version-related preprocessor defines, because this way is (1) nicer and (2) matches the Mac engine's preprocessor defines.

 Revision 1.9.2.1  2002/04/19 11:20:13  emk
 Start of the heavy typography merging work.  I'm doing this on a branch
 so I don't cause problems for any of the other developers.

 Alpha-blend text colors.

 Merged Mac and Windows versions of several files into the Common directory.
 Not all of these work on Mac and/or Windows yet, but they're getting there.
 Primary sources for the merged code are:

   Win/FiveL/LVersion.h -> Common/TVersion.h
   Win/FiveL/LStream.h -> Common/TStream.h
   Mac/Source/CStream.cp -> Common/TStream.cpp
   Mac/Source/CStreamTests.cp -> Common/TStreamTests.cpp

 TStream changes:

   * The TStream code now uses a callback to variable values.  This will
     probably go away once Variable and CVariable get merged.
   * Input operators for std::string and GraphicTools::Color.

 Isolated Windows-specific code in TLogger.*, in preparation for a big merge.

   * Added a portable function to set up logging.
   * Fixed the logging code to use the portable FileSystem library.
   * Made FatalError actually quit the application.

 Turned off the FiveL namespace on FIVEL_PLATFORM_OTHER, so we can debug
 with GDB, which has a few minor but painful namespace issues.

 TString changes:

   * Made sure we can convert from std::string to a TString.
   * Added some more assertions.
   * Fixed bug in various operator= methods which would allow the string's
     internal data pointer to be NULL.
   * Changed operator[] and operator() arguments to be 'int' instead of
     'int32' to avoid nasty compiler warnings.

 Typography::Style changes:

   * Added a "ShadowOffset" field that specifies the offset of the
     drop shadow.
   * Added an operator== for testing.
   * Added a ToggleFaceStyle method for toggling specified face style bits.

 Typography::StyledText changes:

   * Added a method to append a single character.

 Other Typography changes:

   * Made FaceStyle an int, not an enum, so we can do bit math with it.
   * Added assertions to made sure you can't extract a StyledText iterator
     until you've called EndConstruction.

 Revision 1.9  2002/04/19 10:21:52  hyjin
 Added support for a movie controller in 5L applications, and deleted some buggy pre-roll code that appeared to be causing crashes.  We're not a hundred percent sure all the crashing problems are fixed, but things seem to be working very well.  Please test this extensively!

 Set global variable _bShowMC to see the movie controller (case insensitive).

 Changes by Yijin, reviewed by Eric Kidd.

 Revision 1.8  2002/04/19 06:02:57  emk
 Merged in MD5-replacement changes from 3.2.0.4.

 This code will become 3.3.1.  PLEASE USE THIS VERSION IN PREFERENCE TO THE 3.3.0
 OR OLDER BINARIES, WHICH THE FSF's LEGAL COUNCIL HAS ASKED US NOT TO SHIP.

 Revision 1.7  2002/03/29 11:14:42  emk
 Final Win32 fixes for 3.3.0 development release.

 Revision 1.6  2002/02/27 13:21:12  tvw
 Bug #613 - Changed calculation of _INCR_Y to include descenders
 (part or letter that goes below baseline).

 Revision 1.5  2002/02/19 12:35:12  tvw
 Bugs #494 and #495 are addressed in this update.

 (1) 5L.prefs configuration file introduced
 (2) 5L_d.exe will no longer be part of CVS codebase, 5L.prefs allows for
     running in different modes.
 (3) Dozens of compile-time switches were removed in favor of
     having a single executable and parameters in the 5L.prefs file.
 (4) CryptStream was updated to support encrypting/decrypting any file.
 (5) Clear file streaming is no longer supported by CryptStream

 For more details, refer to ReleaseNotes.txt

 Revision 1.4  2002/02/19 11:41:38  tvw
 Merged from branch FiveL_3_1_1_Stabilization

 Revision 1.3.2.1  2002/02/19 10:20:23  tvw
 Stable build v3.2.0 (based on v3.1.1)

 Revision 1.3  2002/01/24 19:22:41  tvw
 Fixed bug (#531) in -D command-line option causing
 system registry read error.

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

 Revision 1.14  2000/08/08 19:03:41  chuck
 no message

 Revision 1.13  2000/05/11 12:54:54  chuck
 v 2.01 b2

 Revision 1.12  2000/04/07 17:05:16  chuck
 v 2.01 build 1

 Revision 1.11  2000/02/02 15:15:32  chuck
 no message

 Revision 1.10  2000/01/04 13:32:56  chuck
 New cursors

 Revision 1.9  1999/12/16 17:17:36  chuck
 no message

 Revision 1.8  1999/11/16 13:46:32  chuck
 no message

 Revision 1.7  1999/11/04 14:18:50  chuck
 2.00 Build 10

 Revision 1.6  1999/11/02 17:16:37  chuck
 2.00 Build 8

 Revision 1.5  1999/10/27 19:42:40  chuck
 Better cursor management

 Revision 1.4  1999/10/22 20:29:09  chuck
 New cursor management.

 Revision 1.3  1999/09/28 15:14:08  chuck
 no message

 Revision 1.2  1999/09/24 19:57:19  chuck
 Initial revision

*/
