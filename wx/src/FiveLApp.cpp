#include <wx/wx.h>
#include <wx/image.h>
#include <wx/fs_inet.h>

#include "TStartup.h"
#include "TDeveloperPrefs.h"

#include "TQTMovie.h"

#include "AppGlobals.h"
#include "FiveLApp.h"
#include "Log5L.h"
#include "Stage.h"
#include "TWxPrimitives.h"

USING_NAMESPACE_FIVEL

IMPLEMENT_APP(FiveLApp)

FiveLApp::FiveLApp()
    : mHaveOwnEventLoop(false), mStageFrame(NULL)
{
    // Do nothing here but set up instance variables.  Real work should
    // happen in FiveLApp::OnInit, below.
}

void FiveLApp::IdleProc()
{
    // Dispatch all queued events and return from the IdleProc.  Note that
    // the '||' operator short-circuts, and will prevent us from calling
    // ProcessIdle until the pending queue is drained.  See the ProcessIdle
    // documentation for an explanation of when and why we might need to
    // call it multiple times.
    while (wxGetApp().Pending() || wxGetApp().ProcessIdle())
    {
	if (wxGetApp().Pending())
	    wxGetApp().Dispatch();
    }
}

bool FiveLApp::OnInit()
{
    // Get the 5L runtime going.
    ::InitializeCommonCode();
    ::RegisterWxPrimitives();

    // Send copies of all wxWindows logging messages to our traditional 5L
    // logs.  This gives us a single copy of everything.
    // TODO - How do we clean up these resources?
    wxLog::SetActiveTarget(new wxLogGui());
    new wxLogChain(new Log5L());

    // Configure some useful trace masks for debugging the application.
    // Comment these out to disable a particular kind of tracing.
    wxLog::AddTraceMask(TRACE_STAGE_DRAWING);
    //wxLog::AddTraceMask(wxTRACE_Messages);
    //wxLog::SetTraceMask(wxTraceMessages);

    // Start up QuickTime.
    TQTMovie::InitializeMovies();

    // Initialize some optional wxWindows features.
    ::wxInitAllImageHandlers();
    wxFileSystem::AddHandler(new wxInternetFSHandler);

    // Create and display our stage frame.
    mStageFrame = new StageFrame("wx5L", wxSize(640, 480));
    mStageFrame->Show();
    SetTopWindow(mStageFrame);
    return TRUE;
}

int FiveLApp::OnExit()
{
    // Shut down QuickTime.  wxWindows guarantees to have destroyed
    // all windows and frames by this point.
    TQTMovie::ShutDownMovies();

    // XXX - Undocumented return value, trying 0 for now.
    return 0;
}

int FiveLApp::MainLoop()
{
    // Try to create a SchemeInterpreterManager.
    TInterpreterManager *manager =
        ::MaybeGetSchemeInterpreterManager(&FiveLApp::IdleProc);

    if (manager)
    {
        // WARNING - No Scheme function may ever be called above this
        // point on the stack!
        FIVEL_SET_STACK_BASE();

        // Run our own event loop.
        SetExitOnFrameDelete(FALSE);
        mHaveOwnEventLoop = true;
	IdleProc();
        manager->Run();
        delete manager;
        manager = NULL;
    }
    else
    {
	wxLogError("Could not find a \"Runtime\" directory.");

        // Run the built-in main loop instead.
        return wxApp::MainLoop();
    }

    // XXX - Is this good enough?  This return value is platform-specific,
    // and it's not always 0.
    return 0;
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
