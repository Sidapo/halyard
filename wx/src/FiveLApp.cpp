// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-
// @BEGIN_LICENSE
//
// Tamale - Multimedia authoring and playback system
// Copyright 1993-2004 Trustees of Dartmouth College
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// @END_LICENSE

#include "TamaleHeaders.h"
#include <wx/image.h>
#include <wx/fs_inet.h>
#include <wx/xrc/xmlres.h>
#include <wx/evtloop.h>
#include <wx/sysopt.h>
#include <wx/stdpaths.h>

#include "TStartup.h"
#include "TDeveloperPrefs.h"

#include "AppConfig.h"
#include "FancyCrashReporter.h"
#if CONFIG_HAVE_QUICKTIME
#	include "TQTMovie.h"
#   include "TQTPrimitives.h"
#endif // CONFIG_HAVE_QUICKTIME
#include "AppGlobals.h"
#include "FiveLApp.h"
#include "Log5L.h"
#include "GuiUtil.h"
#include "StageFrame.h"
#include "dlg/StartupDlg.h"
#include "TWxPrimitives.h"
#if CONFIG_HAVE_QUAKE2
#	include "TQuake2Primitives.h"
#endif // CONFIG_HAVE_QUAKE2
#include "AudioStream.h"
#include "Downloader.h"
#include "Stage.h"
#include "CommandLine.h"

// Provided by auto-generated resources.cpp file.
extern void InitXmlResource();

USING_NAMESPACE_FIVEL


//=========================================================================
//  FiveLApp Methods
//=========================================================================

IMPLEMENT_APP(FiveLApp)

BEGIN_EVENT_TABLE(FiveLApp, wxApp)
    EVT_ACTIVATE_APP(FiveLApp::OnActivateApp)
END_EVENT_TABLE()

FiveLApp::FiveLApp()
    : mHaveOwnEventLoop(false), mLogsAreInitialized(false), mStageFrame(NULL),
      mShouldLaunchUpdateInstaller(false)
{
    // Do nothing here but set up instance variables.  Real work should
    // happen in FiveLApp::OnInit, below.
}

void FiveLApp::Heartbeat() {
    // This should internally call the Windows function ::PeekMessage,
    // which is sufficient to mark our application as alive.
    (void) Pending();
}

void FiveLApp::LaunchUpdateInstallerBeforeExiting() {
    mShouldLaunchUpdateInstaller = true;
}

void FiveLApp::LaunchUpdateInstaller() {
    // PORTABILITY - We need to make the UpdateInstaller work elsewhere.
    // XXX - Extract application exe name from wxWidgets?
    FileSystem::Path base(FileSystem::GetBaseDirectory());
    FileSystem::Path updater(base.AddComponent("UpdateInstaller"));
	FileSystem::Path tamale(base.AddComponent("Tamale"));
	std::vector<std::string> clItems;
	clItems.push_back(updater.ToNativePathString());
	clItems.push_back(base.ToNativePathString());
	clItems.push_back(tamale.ToNativePathString());
	clItems.push_back(base.ToNativePathString());
	CommandLine cl(clItems);
    wxExecute(cl.WindowsQuotedString().c_str(), wxEXEC_ASYNC);
}

void FiveLApp::IdleProc(bool inBlock)
{
	if (wxGetApp().HaveStage() && !wxGetApp().GetStage()->IsIdleAllowed())
		THROW("Tried to call (idle) at an unsafe time");

	// Constraints:
	//   1) Cannot leave until all pending events are dispatched,
	//      or else GUI responsiveness will drop dramatically.
	//   2) Cannot make a *blocking* call to Dispatch() until
	//      all outstanding idles have been processed, or
	//      else idle event processing will be delayed indefinitely.
	//   3) Must not process idles if events are pending, or else
	//      the application may freeze.
	//   4) If we're not in blocking mode, must place an upper limit
	//      on how many idles we process before returning to
	//      Scheme, or an infinitely repeating idler (such as wxQuake2)
	//      will prevent us from ever returning.

	// Run the idle loop, getting events from the user and giving time to
	// process idles. Block will be true when we want to block and only 
	// process user events, never going back to scheme until an event tells us
	// to.

	if (inBlock)
	{
		while (!wxGetApp().Pending() && wxGetApp().ProcessIdle())
			;
		wxGetApp().Dispatch();
	}

	// We don't allow more than five consecutive idles before returning to
	// Scheme.  Returning to Scheme is very fast, and we may need to
	// process a jump command.
	int max_idles = 5;

	// Dispatch all queued events and return from the IdleProc.  Note that
	// the '||' operator short-circuts, and will prevent us from calling
	// ProcessIdle until the pending queue is drained.  See the ProcessIdle
	// documentation for an explanation of when and why we might need to
	// call it multiple times.
	while (wxGetApp().Pending() || (wxGetApp().ProcessIdle() && --max_idles))
	{
		if (wxGetApp().Pending())
			wxGetApp().Dispatch();
	}
}

void FiveLApp::ErrorDialog(const char* inTitle, const char *inMessage) {
    wxMessageDialog dlg(NULL, inMessage, inTitle, wxOK|wxICON_ERROR);
    dlg.ShowModal();
}

void FiveLApp::ReportFatalException(std::exception &e) {
    if (mLogsAreInitialized) {
        TException::ReportFatalException(e);
    } else {
        ErrorDialog("Unexpected Error", e.what());
        exit(1);
    }
}

void FiveLApp::ReportFatalException() {
    if (mLogsAreInitialized) {
        TException::ReportFatalException();
    } else {
        ErrorDialog("Unexpected Error", "An unknown error occurred.");
        exit(1);
    }
}

void FiveLApp::OnUnhandledException() {
    ReportFatalException();
}

void FiveLApp::OnFatalException() {
    // We're dead.
    CrashReporter::GetInstance()->CrashNow();
}

#ifdef __WXDEBUG__

void FiveLApp::OnAssert(const wxChar *file, int line, const wxChar *cond,
                        const wxChar *msg)
{
    if (!CrashReporter::HaveInstance()) {
        // We don't have a crash reporter yet, so let wxWidgets handle
        // this error.
        wxApp::OnAssert(file, line, cond, msg);
    } else {
        // We have a crash report, so don't even bother to pop up a dialog
        // before crashing.  We *do* append this to our application log
        // just in case the assertion actually occurred in the crash
        // reporter--at least we'll record it somewhere.
        wxString message;
        message << "Assertion: ";
        if (msg)
            message << msg << ": ";
        message << cond << " (at " << file << ":" << line << ")";
        gLog.Log("wxWidgets %s", message.mb_str());
        CrashReporter::GetInstance()->CrashNow(message.mb_str());
    }
}

#endif // __WXDEBUG__

bool FiveLApp::OnInit() {
    // All code in this routine should be protected by an
    // exception-trapping block of some sort, because wxWidgets has weak
    // internal exception handling and doesn't tend to report exception
    // strings properly.
    //
    // Partway down this routine, we'll set mLogsAreInitialized to true,
    // allowing a more advanced error reporting system to take over.
    BEGIN_EXCEPTION_TRAPPER();

    // Name our application.
    SetAppName("Tamale");
    
    // Turn off default use of wxCLIP_CHILDREN.  While wxCLIP_CHILDREN
    // is a very nice feature, we currently rely on Stage repaints
    // overwriting MovieWindowQT objects to make them "transparent"
    // until the movie starts playing.  Yes, this is hackish behavior,
    // but right now, we're tuned for it.
    wxSystemOptions::SetOption("msw.window.no-clip-children", 1);

    // Figure out where we can store log files and such.
    wxString dir(wxStandardPaths::Get().GetUserDataDir());
    if (!::wxDirExists(dir) && !::wxMkdir(dir)) {
        // We need to use this error dialog routine because we haven't
        // initialized our logging subsystem yet, and we can't call
        // gLog.FatalError(...).
        ErrorDialog("Cannot Create Directory", "Unable to create " + dir);
        return false;
    }
    FileSystem::SetAppDataDirectory(dir.mb_str());

    // Get the 5L runtime going.
    ::InitializeCommonCode(new FancyCrashReporter());
    mLogsAreInitialized = true;
    ::RegisterWxPrimitives();
#if CONFIG_HAVE_QUAKE2
    ::RegisterQuake2Primitives();
#endif // CONFIG_HAVE_QUAKE2
    
    // Send all wxWindows logging messages to our traditional 5L
    // logs.  This gives us a single copy of everything.
    // TODO - How do we clean up these resources?
    wxLog::SetActiveTarget(new Log5L());
    
    // Configure some useful trace masks for debugging the application.
    // Comment these out to disable a particular kind of tracing.
    //wxLog::AddTraceMask(TRACE_STAGE_DRAWING);
    //wxLog::AddTraceMask(wxTRACE_Messages);
    //wxLog::SetTraceMask(wxTraceMessages);
    
#if CONFIG_HAVE_QUICKTIME
    // Start up QuickTime.
    try {
        TQTMovie::InitializeMovies();
    } catch (std::exception &) {
        // We don't call gLog.FatalError here, because this isn't
        // really an error, it's a configuration problem, and we don't
        // really want crash reports about it.
        ErrorDialog("QuickTime Unavailable",
                    "Could not set up QuickTime. Please "
                    "make sure QuickTime is properly installed.");
        return false;
    }
    RegisterQuickTimePrimitives();
#endif // CONFIG_HAVE_QUICKTIME
    
    // Start up our audio synthesis layer.
    AudioStream::InitializeStreams();
    
    // Initialize some optional wxWindows features.
    ::wxInitAllImageHandlers();
    wxFileSystem::AddHandler(new wxInternetFSHandler);
    
    // Load our resources (they're linked into our application).
    // TODO - Only initialize the handlers we need; this will greatly
    // reduce application size.
    wxXmlResource::Get()->InitAllHandlers();
    InitXmlResource();
    
    // Parse our command-line arguments.
	size_t ac; char **av;
    for (ac = argc-1, av = argv+1; ac > 0; --ac, ++av) {   
        if (ac >= 2 && av[0] == std::string("-e")) {
            TInterpreterManager::SetInitialCommand(av[1]);
            --ac, ++av;
        } else {
            TInterpreterManager::SetRuntimeMode(true);
            mArgScript = av[0];
        }
    }

    // Make sure we restore the taskbar, etc., before exiting with
    // an error message.
    TLogger::RegisterExitPrepFunction(&ShowSystemWindows);

    // Create and display our stage frame.
    //mStageFrame = new StageFrame(wxSize(640, 480));
    mStageFrame = new StageFrame(wxSize(800, 600));
    mStageFrame->Show();
    // Enable this to go to full-screen mode *almost* immediately.
    // TODO - You'll see the standard window for a small fraction of a
    // second.  Fixing this will require tweaking wxWindows.
    // TODO - Well, the behavior looks slightly better with the current
    // version of wxWidgets.  We'll probably need to tweak this some more.
    if (TInterpreterManager::IsInRuntimeMode())
        mStageFrame->ShowFullScreen(TRUE);
    SetTopWindow(mStageFrame);

    END_EXCEPTION_TRAPPER(ReportFatalException);
    
    return true;
}

int FiveLApp::OnExit() {
    BEGIN_EXCEPTION_TRAPPER();

    // Make sure we put back the taskbar, etc.
    ShowSystemWindows();

	// Shut down our audio synthesis layer.
	AudioStream::ShutDownStreams();
    
    // Shut down QuickTime.  wxWindows guarantees to have destroyed
    // all windows and frames by this point.
#if CONFIG_HAVE_QUICKTIME
    TQTMovie::ShutDownMovies();
#endif // CONFIG_HAVE_QUICKTIME
    
    END_EXCEPTION_TRAPPER(ReportFatalException);

    // Launch the UpdateInstaller if we've been asked to do so.
    if (mShouldLaunchUpdateInstaller)
        LaunchUpdateInstaller();

    // XXX - Undocumented return value, trying 0 for now.
    return 0;
}

namespace {
    // HACK - Do the song and dance required to get a custom event loop running
    // with the latest build of wxWidgets.  This is a gross hack, and requires
    // patching wxWidgets itself.  Hopefully a better interface will be
    // included in a future version.
    class StEventLoopSetup {
        wxEventLoop **m_evtloop_var;
        
    public:
        StEventLoopSetup(wxEventLoop **evtloop_var)
            : m_evtloop_var(evtloop_var)
        {
            *m_evtloop_var = new wxEventLoop;
            (*m_evtloop_var)->StartRunning(); // Call our patch.
        }

        ~StEventLoopSetup() {
            (*m_evtloop_var)->StopRunning(); // Call our patch.
            delete *m_evtloop_var;
            *m_evtloop_var = NULL;
        }
    };
};

int FiveLApp::MainLoop() {
	// WARNING - No Scheme function may ever be called above this
    // point on the stack!
    FIVEL_SET_STACK_BASE();

    bool error = false;

    BEGIN_EXCEPTION_TRAPPER();

    // Create a wxEventLoop object to process events.  This became
    // necessary in wxWindows 2.5.x or so.
    StEventLoopSetup setup(&m_mainLoop);
        
    // Create a SchemeInterpreterManager.
    TInterpreterManager *manager =
		GetSchemeInterpreterManager(&FiveLApp::IdleProc);

	Downloader *downloader = new Downloader();

    // Check to see if we have any command-line arguments.
    if (TInterpreterManager::IsInRuntimeMode()) {
        // Open the specified document directly.
        mStageFrame->OpenDocument(mArgScript);
    } else {
        // Prompt for a program to open.  We can't do this until the
        // TInterpreterManager is loaded.  (We need to do this inside
        // a block to make sure the dialog is destroyed quickly.)
		StartupDlg startup_dialog(mStageFrame);
		startup_dialog.ShowModal();
	}

	// Run our own event loop.
	SetExitOnFrameDelete(FALSE);
	mHaveOwnEventLoop = true;
	IdleProc(false);
	manager->Run();
    error = manager->ExitedWithError();
	delete downloader;
	downloader = NULL;
	delete manager;
	manager = NULL;

	// We could run the built-in event loop using the following call,
	// if there were some reason the above code would not work (perhaps
	// because of a starup error):
	//    return wxApp::MainLoop();

    END_EXCEPTION_TRAPPER(ReportFatalException);

    // Return our application's exit code.  This function is documented as
    // returning zero, but the exit code is really passed onto OnRun, the
    // return value of which is documented to be the application's exit
    // code.
    return error ? 1 : 0;
}

void FiveLApp::ExitMainLoop()
{
    if (mHaveOwnEventLoop)
    {
        // Ask the TInterpreterManager to begin an orderly shutdown.
        ASSERT(TInterpreterManager::HaveInstance());
        TInterpreterManager::GetInstance()->RequestQuitApplication();
    }
    else
    {
        // Handle things normally.
        wxApp::ExitMainLoop();
    }
}

Stage *FiveLApp::GetStage()
{
    return GetStageFrame()->GetStage();
}

void FiveLApp::OnActivateApp(wxActivateEvent &event) {
    // If we're being deactivated, and we have a full-screen window,
    // iconize it so the user can see the desktop.
    if (!event.GetActive() && mStageFrame && mStageFrame->IsFullScreen())
        mStageFrame->Iconize();
}
