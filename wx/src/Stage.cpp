// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#include <wx/wx.h>
#include <wx/dcraw.h>
#include <wx/treectrl.h>
#include <wx/laywin.h>
#include <wx/config.h>
#include <wx/filename.h>

#include "TCommon.h"
#include "TInterpreter.h"
#include "TLogger.h"
#include "TVariable.h"
#include "TStyleSheet.h"
#include "doc/Document.h"
#include "doc/TamaleProgram.h"

// XXX - Huh?  Who included by TStyleSheet.h (on Win32) is defining DrawText?
#undef DrawText

#include "AppConfig.h"
#include "AppGlobals.h"
#include "AppGraphics.h"
#include "FiveLApp.h"
#include "Stage.h"
#include "Element.h"
#include "MovieElement.h"
#include "Listener.h"
#include "Timecoder.h"
#include "LocationBox.h"
#include "dlg/ProgramPropDlg.h"

#if CONFIG_HAVE_QUAKE2
#	include "Quake2Engine.h"
#endif // CONFIG_HAVE_QUAKE2

USING_NAMESPACE_FIVEL


//=========================================================================
//  StageBackground
//=========================================================================
//  This class implements the wxWindow *behind* the stage.  It has few
//  duties other than (1) being black and (2) keeping the stage centered.

class StageBackground : public wxSashLayoutWindow
{
	DECLARE_EVENT_TABLE();

	StageFrame *mStageFrame;
	bool mHaveStage;

	void OnSize(wxSizeEvent& event);

public:
	StageBackground(StageFrame *inStageFrame);
	void UpdateColor();
	void CenterStage(Stage *inStage);
};

BEGIN_EVENT_TABLE(StageBackground, wxSashLayoutWindow)
    EVT_SIZE(StageBackground::OnSize)
END_EVENT_TABLE()

StageBackground::StageBackground(StageFrame *inStageFrame)
	: wxSashLayoutWindow(inStageFrame, -1, wxDefaultPosition, wxDefaultSize,
						 wxNO_BORDER),
	  mStageFrame(inStageFrame), mHaveStage(false)
{
	if (mStageFrame->IsFullScreen())
		SetBackgroundColour(STAGE_BACKGROUND_COLOR);
	else
		SetBackgroundColour(STAGE_BACKGROUND_COLOR_NEUTRAL);
}

void StageBackground::OnSize(wxSizeEvent &WXUNUSED(event))
{
    if (GetAutoLayout())
        Layout();
}

void StageBackground::UpdateColor()
{
	// We're called immediately *before* toggling ShowFullScreen,
	// so we need to set our background color to what we want it
	// to be *after* the call.  So this test is backwards...
	if (!mStageFrame->IsFullScreen())
		SetBackgroundColour(STAGE_BACKGROUND_COLOR);
	else
		SetBackgroundColour(STAGE_BACKGROUND_COLOR_NEUTRAL);
}

void StageBackground::CenterStage(Stage *inStage)
{
	ASSERT(!mHaveStage);
	mHaveStage = true;

	// Align our stage within the StageBackground using a pair of
    // sizers.  I arrived at these parameters using trial and error.
    wxBoxSizer *v_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *h_sizer = new wxBoxSizer(wxHORIZONTAL);
    v_sizer->Add(h_sizer, 1 /* stretch */, wxALIGN_CENTER, 0);
    h_sizer->Add(inStage, 0 /* no stretch */, wxALIGN_CENTER, 0);
    SetSizer(v_sizer);
}


//=========================================================================
//  ProgramTree
//=========================================================================

#include <map>

class ProgramTreeCtrl : public wxTreeCtrl
{
	DECLARE_EVENT_TABLE()

public:
	ProgramTreeCtrl(wxWindow *inParent, int id);

private:
	void OnMouseDClick(wxMouseEvent& event);
};	

class ProgramTree : public wxSashLayoutWindow
{
	DECLARE_EVENT_TABLE()

	typedef std::map<std::string,wxTreeItemId> ItemMap;

	wxTreeCtrl *mTree;

	wxTreeItemId mRootID;
	wxTreeItemId mCardsID;
	wxTreeItemId mBackgroundsID;

	ItemMap mCardMap;

	bool mHaveLastHighlightedItem;
	wxTreeItemId mLastHighlightedItem;

	enum {
		MINIMUM_WIDTH = 150
	};

public:
	ProgramTree(StageFrame *inStageFrame, int inID);

    //////////
    // Regiter a newly-loaded card with the program tree.
    //
    void RegisterCard(const wxString &inName);

	//////////
	// Set the name of the currently executing program.
	//
	void SetProgramName(const wxString &inName);

	//////////
	// Set the default width which will be used when laying out this window.
	//
	void SetDefaultWidth(int inWidth);

	//////////
	// Notify the program tree that script is being reloaded.
	//
    void NotifyScriptReload();

    //////////
    // Notify the program tree that the interpreter has moved to a new card.
    //
    void NotifyEnterCard();
};

BEGIN_EVENT_TABLE(ProgramTreeCtrl, wxTreeCtrl)
    EVT_LEFT_DCLICK(ProgramTreeCtrl::OnMouseDClick)
END_EVENT_TABLE()

ProgramTreeCtrl::ProgramTreeCtrl(wxWindow *inParent, int id)
	: wxTreeCtrl(inParent, id)
{
}

void ProgramTreeCtrl::OnMouseDClick(wxMouseEvent& event)
{
    wxTreeItemId id = HitTest(event.GetPosition());
    if (id)
        ::wxLogError("Clicked on: " + GetItemText(id));
}

BEGIN_EVENT_TABLE(ProgramTree, wxSashLayoutWindow)
END_EVENT_TABLE()

ProgramTree::ProgramTree(StageFrame *inStageFrame, int inID)
	: wxSashLayoutWindow(inStageFrame, inID),
	  mHaveLastHighlightedItem(false)
{
	// Set up our tree control.
	mTree = new ProgramTreeCtrl(this, FIVEL_PROGRAM_TREE_CTRL);
	mRootID = mTree->AddRoot("Program");
	mCardsID = mTree->AppendItem(mRootID, "Cards");
	mBackgroundsID = mTree->AppendItem(mRootID, "Backgrounds");

	// Set our minimum sash width.
	SetMinimumSizeX(MINIMUM_WIDTH);
    SetDefaultWidth(MINIMUM_WIDTH);
}

void ProgramTree::RegisterCard(const wxString &inName)
{
	// Check to make sure we don't already have a card by this name.
	wxASSERT(mCardMap.find(inName.mb_str()) == mCardMap.end());

	// Insert the card into our tree.
	wxTreeItemId id = mTree->AppendItem(mCardsID, inName);

	// Record the card in our map.
	mCardMap.insert(ItemMap::value_type(inName.mb_str(), id));
}

void ProgramTree::SetProgramName(const wxString &inName)
{
	mTree->SetItemText(mRootID, "Program '" + inName + "'");
}

void ProgramTree::SetDefaultWidth(int inWidth)
{
	SetDefaultSize(wxSize(inWidth, 0 /* unused */));
}

void ProgramTree::NotifyScriptReload()
{
	mCardMap.clear();
	mTree->SetItemText(mRootID, "Program");
	mTree->CollapseAndReset(mCardsID);
	mTree->CollapseAndReset(mBackgroundsID);
}

void ProgramTree::NotifyEnterCard()
{
	// Look up the ID corresponding to this card.
	ASSERT(TInterpreter::HaveInstance());
	std::string card = TInterpreter::GetInstance()->CurCardName();
	ItemMap::iterator found = mCardMap.find(card);
	wxASSERT(found != mCardMap.end());

	// Move the highlighting to the appropriate card.
	if (mHaveLastHighlightedItem)
		mTree->SetItemBold(mLastHighlightedItem, FALSE);
	mTree->SetItemBold(found->second);
	mHaveLastHighlightedItem = true;
	mLastHighlightedItem = found->second;
	mTree->EnsureVisible(found->second);
}


//=========================================================================
//  StageFrame Methods
//=========================================================================

BEGIN_EVENT_TABLE(StageFrame, wxFrame)
    EVT_MENU(FIVEL_EXIT, StageFrame::OnExit)

    EVT_UPDATE_UI(FIVEL_NEW_PROGRAM, StageFrame::UpdateUiNewProgram)
    EVT_MENU(FIVEL_NEW_PROGRAM, StageFrame::OnNewProgram)
    EVT_UPDATE_UI(FIVEL_OPEN_PROGRAM, StageFrame::UpdateUiOpenProgram)
    EVT_MENU(FIVEL_OPEN_PROGRAM, StageFrame::OnOpenProgram)
    EVT_UPDATE_UI(FIVEL_SAVE_PROGRAM, StageFrame::UpdateUiSaveProgram)
    EVT_MENU(FIVEL_SAVE_PROGRAM, StageFrame::OnSaveProgram)
    EVT_MENU(FIVEL_RELOAD_SCRIPT, StageFrame::OnReloadScript)

    EVT_MENU(FIVEL_ABOUT, StageFrame::OnAbout)
    EVT_MENU(FIVEL_SHOW_LOG, StageFrame::OnShowLog)
    EVT_MENU(FIVEL_SHOW_LISTENER, StageFrame::OnShowListener)
    EVT_MENU(FIVEL_SHOW_TIMECODER, StageFrame::OnShowTimecoder)
    EVT_UPDATE_UI(FIVEL_FULL_SCREEN, StageFrame::UpdateUiFullScreen)
    EVT_MENU(FIVEL_FULL_SCREEN, StageFrame::OnFullScreen)
    EVT_UPDATE_UI(FIVEL_DISPLAY_XY, StageFrame::UpdateUiDisplayXy)
    EVT_MENU(FIVEL_DISPLAY_XY, StageFrame::OnDisplayXy)
    EVT_UPDATE_UI(FIVEL_DISPLAY_GRID, StageFrame::UpdateUiDisplayGrid)
    EVT_MENU(FIVEL_DISPLAY_GRID, StageFrame::OnDisplayGrid)
    EVT_UPDATE_UI(FIVEL_DISPLAY_BORDERS, StageFrame::UpdateUiDisplayBorders)
    EVT_MENU(FIVEL_DISPLAY_BORDERS, StageFrame::OnDisplayBorders)
    EVT_UPDATE_UI(FIVEL_PROPERTIES, StageFrame::UpdateUiProperties)
    EVT_MENU(FIVEL_PROPERTIES, StageFrame::OnProperties)
    EVT_UPDATE_UI(FIVEL_JUMP_CARD, StageFrame::UpdateUiJumpCard)
    EVT_MENU(FIVEL_JUMP_CARD, StageFrame::OnJumpCard)
    EVT_SASH_DRAGGED(FIVEL_PROGRAM_TREE, StageFrame::OnSashDrag)
	EVT_SIZE(StageFrame::OnSize)
    EVT_CLOSE(StageFrame::OnClose)
END_EVENT_TABLE()

StageFrame::StageFrame(const wxChar *inTitle, wxSize inSize)
    : wxFrame((wxFrame*) NULL, -1, inTitle,
              LoadFramePosition(), wxDefaultSize,
			  wxDEFAULT_FRAME_STYLE),
	  mDocument(NULL),
	  mHaveLoadedFrameLayout(false)
{
    // Set up useful logging.
    mLogWindow = new wxLogWindow(this, "Application Log", FALSE);

	// We create our tool windows on demand.
	for (int i = 0; i < TOOL_COUNT; i++)
		mToolWindows[i] = NULL;

	// Get an appropriate icon for this window.
    SetIcon(wxICON(ic_5L));

    // Make our background black.  This should theoretically be handled
    // by 'background->SetBackgroundColour' below, but Windows takes a
    // fraction of a second to show that object.
    SetBackgroundColour(STAGE_FRAME_COLOR);

	// Create a sash window holding a tree widget.
	mProgramTree = new ProgramTree(this, FIVEL_PROGRAM_TREE);
	mProgramTree->SetOrientation(wxLAYOUT_VERTICAL);
	mProgramTree->SetAlignment(wxLAYOUT_LEFT);
	mProgramTree->SetSashVisible(wxSASH_RIGHT, TRUE);

    // Create a background panel to surround our stage with.  This keeps
    // life simple.
    mBackground = new StageBackground(this);

    // Create a stage object to scribble on, and center it.
    mStage = new Stage(mBackground, this, inSize);
	mBackground->CenterStage(mStage);
	mStage->Hide();

    // Set up our File menu.
    mFileMenu = new wxMenu();
    mFileMenu->Append(FIVEL_NEW_PROGRAM, "&New Program...\tCtrl+N",
                      "Create a new Tamale program.");
    mFileMenu->Append(FIVEL_OPEN_PROGRAM, "&Open Program...\tCtrl+O",
                      "Open an existing Tamale program.");
    mFileMenu->Append(FIVEL_SAVE_PROGRAM, "&Save Program\tCtrl+S",
                      "Save the current Tamale program.");
    mFileMenu->AppendSeparator();
    mFileMenu->Append(FIVEL_RELOAD_SCRIPT, "&Reload Script\tCtrl+R",
                      "Reload the currently executing 5L script.");
    mFileMenu->AppendSeparator();
    mFileMenu->Append(FIVEL_EXIT, "E&xit\tCtrl+Q", "Exit the application.");

    // Set up our Card menu.
    mCardMenu = new wxMenu();
    mCardMenu->Append(FIVEL_JUMP_CARD, "&Jump to Card...\tCtrl+J",
                      "Jump to a specified card by name.");

    // Set up our View menu.  Only include the "Full Screen" item on
	// platforms where it's likely to work.
    mViewMenu = new wxMenu();
#if CONFIG_ENABLE_FULL_SCREEN
    mViewMenu->AppendCheckItem(FIVEL_FULL_SCREEN,
                               "&Full Screen\tCtrl+F",
                               "Use a full screen window.");
    mViewMenu->AppendSeparator();
#endif // CONFIG_ENABLE_FULL_SCREEN
    mViewMenu->AppendCheckItem(FIVEL_DISPLAY_XY, "Display Cursor &XY",
                               "Display the cursor's XY position.");
    mViewMenu->AppendCheckItem(FIVEL_DISPLAY_GRID, "Display &Grid\tCtrl+G",
                               "Display a grid over the card.");
    mViewMenu->AppendCheckItem(FIVEL_DISPLAY_BORDERS,
                               "Display &Borders\tCtrl+B",
                               "Display the borders of interactive elements.");
    mViewMenu->AppendSeparator();
	mViewMenu->Append(FIVEL_PROPERTIES,
					  "&Properties...\tAlt+Enter",
					  "Edit the properties of the selected object.");

    // Set up our Window menu.
    mWindowMenu = new wxMenu();
    mWindowMenu->Append(FIVEL_SHOW_LISTENER, "Show &Listener\tCtrl+L",
                        "Show interactive script listener.");
    mWindowMenu->Append(FIVEL_SHOW_TIMECODER, "Show &Timecoder\tCtrl+T",
                        "Show the movie timecoding utility.");
    mWindowMenu->Append(FIVEL_SHOW_LOG, "Show &Debug Log",
                        "Show application debug log window.");

    // Set up our Help menu.
    mHelpMenu = new wxMenu();
    mHelpMenu->Append(FIVEL_ABOUT, "&About",
                      "About the 5L multimedia system.");

    // Set up our menu bar.
    mMenuBar = new wxMenuBar();
    mMenuBar->Append(mFileMenu, "&File");
    mMenuBar->Append(mCardMenu, "&Card");
    mMenuBar->Append(mViewMenu, "&View");
    mMenuBar->Append(mWindowMenu, "&Window");
    mMenuBar->Append(mHelpMenu, "&Help");
    SetMenuBar(mMenuBar);

    // Add a tool bar.
    CreateToolBar();
    wxToolBar *tb = GetToolBar();
    tb->AddTool(FIVEL_RELOAD_SCRIPT, "Reload", wxBITMAP(tb_reload),
                "Reload Script");
	mLocationBox = new LocationBox(tb);
	tb->AddControl(mLocationBox);
    tb->AddSeparator();
    tb->AddCheckTool(FIVEL_DISPLAY_XY, "Display XY", wxBITMAP(tb_xy),
                     wxNullBitmap, "Display Cursor XY");
    tb->AddCheckTool(FIVEL_DISPLAY_GRID, "Display Grid", wxBITMAP(tb_grid),
                     wxNullBitmap, "Display Grid");
    tb->AddCheckTool(FIVEL_DISPLAY_BORDERS,
                     "Display Borders", wxBITMAP(tb_borders),
                     wxNullBitmap, "Display Borders");
    tb->Realize();
        
    // Add a status bar with 1 field.  The 0 disables the resize thumb
    // at the lower right.
    CreateStatusBar(1, 0);

    // Resize the "client area" of the window (the part that's left over
    // after menus, status bars, etc.) to hold the stage and the program
	// tree.
    SetClientSize(wxSize(inSize.GetWidth() + mProgramTree->GetMinimumSizeX(),
						 inSize.GetHeight()));

	// Don't allow the window to get any smaller.
	// XXX - This probably isn't a reliable way to do this.
	wxSize min_size = GetSize();
	SetSizeHints(min_size.GetWidth(), min_size.GetHeight());

	// Re-load our saved frame layout.  We can't do this until after
	// our setup is completed.
	LoadFrameLayout();
}

wxPoint StageFrame::LoadFramePosition()
{
	// TODO - Sanity-check position.
	long pos_x, pos_y;
	wxConfigBase *config = wxConfigBase::Get();
	if (config->Read("/Layout/Default/StageFrame/Left", &pos_x) &&
		config->Read("/Layout/Default/StageFrame/Top", &pos_y))
		return wxPoint(pos_x, pos_y);
	else
		return wxDefaultPosition;
}

void StageFrame::LoadFrameLayout()
{
	// Get our default values.
	wxSize sz = GetClientSize();
	long is_maximized = IsMaximized();
	long sz_client_width = sz.GetWidth();
	long sz_client_height = sz.GetHeight();
	long program_tree_width = mProgramTree->GetMinimumSizeX();

	// Load values from our config file.
	wxConfigBase *config = wxConfigBase::Get();
	config->Read("/Layout/Default/StageFrame/IsMaximized", &is_maximized);
	config->Read("/Layout/Default/StageFrame/ClientWidth", &sz_client_width);
	config->Read("/Layout/Default/StageFrame/ClientHeight", &sz_client_height);
	config->Read("/Layout/Default/StageFrame/ProgramTreeWidth",
				 &program_tree_width);

	// Restore our non-maximized layout first.  We restore the
	// ProgramTree width before anything else, so it will get appropriately
	// adjusted by the frame resize events.
	// NOTE - We'll only make the window larger, never smaller, because
	// we assume that GetClientSize is currently the minimum allowable.
	mProgramTree->SetDefaultWidth(program_tree_width);
	wxSize new_size = GetClientSize();
	if (sz_client_width >= new_size.GetWidth() &&
		sz_client_height >= new_size.GetHeight())
		new_size = wxSize(sz_client_width, sz_client_height);
	SetClientSize(new_size);

	// If necessary, maximize our window.
	if (is_maximized)
		Maximize(TRUE);

	// It's now safe to resave these values.
	mHaveLoadedFrameLayout = true;
}

void StageFrame::MaybeSaveFrameLayout()
{
	// Don't save the frame layout if we haven't loaded it yet, or if we're
	// in full-screen mode (which has an automatically-chosen layout).
	if (!mHaveLoadedFrameLayout || IsFullScreen())
		return;

	wxConfigBase *config = wxConfigBase::Get();
	config->Write("/Layout/Default/StageFrame/IsMaximized",
				  IsMaximized() ? 1 : 0);
	if (!IsMaximized())
	{
		// Only save the window position if we aren't maximized.
		wxPoint pos = GetPosition();
		wxSize sz = GetClientSize();
		config->Write("/Layout/Default/StageFrame/Left", pos.x);
		config->Write("/Layout/Default/StageFrame/Top", pos.y);
		config->Write("/Layout/Default/StageFrame/ClientWidth", sz.GetWidth());
		config->Write("/Layout/Default/StageFrame/ClientHeight",
					  sz.GetHeight());
	}
	config->Write("/Layout/Default/StageFrame/ProgramTreeWidth",
				  mProgramTree->GetSize().GetWidth());
}

bool StageFrame::ShowFullScreen(bool show, long style)
{
	mProgramTree->Show(!show);
	mBackground->UpdateColor();
	bool result = wxFrame::ShowFullScreen(show, style);
	return result;
}

void StageFrame::NewDocument()
{
	wxASSERT(mDocument == NULL);
	wxDirDialog dlg(this, "Add Tamale data to an existing program folder:");

	wxConfigBase *config = wxConfigBase::Get();
	wxString recent;
	if (config->Read("/Recent/DocPath", &recent))
		dlg.SetPath(recent);

	if (dlg.ShowModal() == wxID_OK)
	{
		wxString file = dlg.GetPath();
		mDocument = new Document(file.mb_str());
		config->Write("/Recent/DocPath", file);

		ProgramPropDlg prop_dlg(this, mDocument->GetTamaleProgram());
		prop_dlg.ShowModal();

		mStage->Show();
	}
}

void StageFrame::OpenDocument()
{
	wxASSERT(mDocument == NULL);
	wxDirDialog dlg(this, "Open a Tamale program folder:");

	wxConfigBase *config = wxConfigBase::Get();
	wxString recent;
	if (config->Read("/Recent/DocPath", &recent))
		dlg.SetPath(recent);

	if (dlg.ShowModal() == wxID_OK)
	{
		wxString file = dlg.GetPath();
		mDocument = new Document(file.mb_str(), Document::OPEN);
		config->Write("/Recent/DocPath", file);
		mStage->Show();
	}
}

void StageFrame::OnExit(wxCommandEvent &inEvent)
{
    Close(FALSE);
}

void StageFrame::UpdateUiNewProgram(wxUpdateUIEvent &inEvent)
{
	inEvent.Enable(mDocument == NULL);
}

void StageFrame::OnNewProgram(wxCommandEvent &inEvent)
{
	BEGIN_EXCEPTION_TRAPPER()
		NewDocument();
	END_EXCEPTION_TRAPPER()
}

void StageFrame::UpdateUiOpenProgram(wxUpdateUIEvent &inEvent)
{
	inEvent.Enable(mDocument == NULL);
}

void StageFrame::OnOpenProgram(wxCommandEvent &inEvent)
{
	BEGIN_EXCEPTION_TRAPPER()
		OpenDocument();
	END_EXCEPTION_TRAPPER()
}

void StageFrame::UpdateUiSaveProgram(wxUpdateUIEvent &inEvent)
{
	inEvent.Enable(mDocument != NULL && mDocument->IsDirty());
}

void StageFrame::OnSaveProgram(wxCommandEvent &inEvent)
{
	mDocument->Save();
}

void StageFrame::OnReloadScript(wxCommandEvent &inEvent)
{
    if (TInterpreterManager::HaveInstance())
    {
        TInterpreterManager *manager = TInterpreterManager::GetInstance();
        if (manager->FailedToLoad())
        {
            // The previous attempt to load a script failed, so we need
            // to try loading it again.  We call NotifyScriptReload to
			// purge any remaining data from a partially completed load.
            mStage->NotifyScriptReload();
            manager->RequestRetryLoadScript();
        }
        else
        {
            // We have a running script.  Go ahead and reload it.
            ASSERT(TInterpreter::HaveInstance());
            TInterpreter *interp = TInterpreter::GetInstance();
            manager->RequestReloadScript(interp->CurCardName().c_str());

            // Tell our stage that the script is going away.
            mStage->NotifyScriptReload();
        }
    }
            
    SetStatusText("Script reloaded.");
}

void StageFrame::OnAbout(wxCommandEvent &inEvent)
{
    wxMessageDialog about(this,
                          "wx5L Prototype\n"
                          "Copyright 2002 The Trustees of Dartmouth College",
                          "About wx5L", wxOK);
    about.ShowModal();
}

void StageFrame::OnShowLog(wxCommandEvent &inEvent)
{
    mLogWindow->Show(TRUE);
}

void StageFrame::OnShowListener(wxCommandEvent &inEvent)
{
	if (!mToolWindows[TOOL_LISTENER])
		mToolWindows[TOOL_LISTENER] = new Listener(this);
	if (!mToolWindows[TOOL_LISTENER]->IsShown())
		mToolWindows[TOOL_LISTENER]->Show();
	mToolWindows[TOOL_LISTENER]->Raise();
}

void StageFrame::OnShowTimecoder(wxCommandEvent &inEvent)
{
	if (!mToolWindows[TOOL_TIMECODER])
		mToolWindows[TOOL_TIMECODER] = new Timecoder(this);
	if (!mToolWindows[TOOL_TIMECODER]->IsShown())
		mToolWindows[TOOL_TIMECODER]->Show();
	mToolWindows[TOOL_TIMECODER]->Raise();	
}

void StageFrame::UpdateUiFullScreen(wxUpdateUIEvent &inEvent)
{
    inEvent.Check(IsFullScreen());
}

void StageFrame::OnFullScreen(wxCommandEvent &inEvent)
{
    if (IsFullScreen())
        ShowFullScreen(FALSE);
    else
    {
        ShowFullScreen(TRUE);
        // TODO - WXBUG - We need to raise above the Windows dock more
        // quickly.
    }
}

void StageFrame::UpdateUiDisplayXy(wxUpdateUIEvent &inEvent)
{
    inEvent.Check(mStage->IsDisplayingXy());
}

void StageFrame::OnDisplayXy(wxCommandEvent &inEvent)
{
    mStage->ToggleDisplayXy();
}

void StageFrame::UpdateUiDisplayGrid(wxUpdateUIEvent &inEvent)
{
    inEvent.Check(mStage->IsDisplayingGrid());
}

void StageFrame::OnDisplayGrid(wxCommandEvent &inEvent)
{
    mStage->ToggleDisplayGrid();
}

void StageFrame::UpdateUiDisplayBorders(wxUpdateUIEvent &inEvent)
{
    inEvent.Check(mStage->IsDisplayingBorders());
}

void StageFrame::OnDisplayBorders(wxCommandEvent &inEvent)
{
    mStage->ToggleDisplayBorders();
}

void StageFrame::UpdateUiProperties(wxUpdateUIEvent &inEvent)
{
	inEvent.Enable(mDocument != NULL);
}

void StageFrame::OnProperties(wxCommandEvent &inEvent)
{
	ProgramPropDlg prop_dlg(this, mDocument->GetTamaleProgram());
	prop_dlg.ShowModal();
}

void StageFrame::UpdateUiJumpCard(wxUpdateUIEvent &inEvent)
{
    inEvent.Enable(TInterpreter::HaveInstance());
}

void StageFrame::OnJumpCard(wxCommandEvent &inEvent)
{
    if (TInterpreter::HaveInstance())
    {
		if (!IsFullScreen())
			mLocationBox->Prompt();
		else
		{
			wxTextEntryDialog dialog(this, "Jump to Card", "Card:");
			if (dialog.ShowModal() == wxID_OK)
				mLocationBox->TryJump(dialog.GetValue());
		}
	}
}

void StageFrame::OnSashDrag(wxSashEvent &inEvent)
{
    if (inEvent.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
        return;
    if (inEvent.GetId() != FIVEL_PROGRAM_TREE)
		return;

	mProgramTree->SetDefaultWidth(inEvent.GetDragRect().width);

    wxLayoutAlgorithm layout;
    layout.LayoutFrame(this, mBackground);
	MaybeSaveFrameLayout();
}

void StageFrame::OnSize(wxSizeEvent &WXUNUSED(inEvent))
{
	// Make sure no sash window can be expanded to obscure parts of our
	// stage.  We need to do this whenever the window geometry changes.
	wxSize client_size = GetClientSize();
	wxSize stage_size = GetStage()->GetStageSize();
	wxCoord available_space = client_size.GetWidth() - stage_size.GetWidth();
	if (available_space > 0)
	{
		// XXX - If available_space <= 0, then our window still hasn't
		// been resized to hold the stage, and we shouldn't call
		// SetDefaultSize unless we want weird things to happen.
		if (mProgramTree->GetSize().GetWidth() > available_space)
			mProgramTree->SetDefaultWidth(available_space);
		mProgramTree->SetMaximumSizeX(available_space);
	}

	// Ask wxLayoutAlgorithm to do smart resizing, taking our various
	// subwindows and constraints into account.
    wxLayoutAlgorithm layout;
    layout.LayoutFrame(this, mBackground);
	MaybeSaveFrameLayout();
}

void StageFrame::OnClose(wxCloseEvent &inEvent)
{
	// Ask the user to save any unsaved documents.
	if (mDocument && mDocument->IsDirty())
	{
		if (!inEvent.CanVeto())
		{
			mDocument->Save();
			wxLogError("Tamale forced to exit; saved document.");
		}
		else
		{
			wxMessageDialog dlg(this, "Save current Tamale program?",
								"Tamale", wxYES_NO|wxCANCEL|wxCENTRE|
								wxICON_EXCLAMATION);
			int button = dlg.ShowModal();
			if (button == wxID_YES)
				mDocument->Save();
			else if (button == wxID_CANCEL)
			{
				inEvent.Veto();
				return;
			}
		}
	}

	// Save our layout one last time.
	MaybeSaveFrameLayout();

    // If we've got an interpreter manager, we'll need to ask it to
    // shut down the application.
    if (TInterpreterManager::HaveInstance())
        TInterpreterManager::GetInstance()->RequestQuitApplication();

    // Detach this object from the application.  This will make various
    // assertions far more useful, and keep people from reaching us
    // through the application object.
    wxGetApp().DetachStageFrame();

    // Mark this object for destruction (as soon as the event queue
    // is empty).
    Destroy();
}


//=========================================================================
//  Stage Methods
//=========================================================================

BEGIN_EVENT_TABLE(Stage, wxWindow)
	EVT_IDLE(Stage::OnIdle)
    EVT_MOTION(Stage::OnMouseMove)
    EVT_ERASE_BACKGROUND(Stage::OnEraseBackground)
    EVT_PAINT(Stage::OnPaint)
    EVT_TEXT_ENTER(FIVEL_TEXT_ENTRY, Stage::OnTextEnter)
	EVT_LEFT_DOWN(Stage::OnLeftDown)
END_EVENT_TABLE()

Stage::Stage(wxWindow *inParent, StageFrame *inFrame, wxSize inStageSize)
    : wxWindow(inParent, -1, wxDefaultPosition, inStageSize),
      mFrame(inFrame), mStageSize(inStageSize),
      mOffscreenPixmap(inStageSize.GetWidth(), inStageSize.GetHeight()),
	  mTextCtrl(NULL), mCurrentElement(NULL), mWaitElement(NULL),
      mIsDisplayingXy(false), mIsDisplayingGrid(false),
      mIsDisplayingBorders(false)

{
    SetBackgroundColour(STAGE_COLOR);
    ClearStage(*wxBLACK);
    
    mTextCtrl =
        new wxTextCtrl(this, FIVEL_TEXT_ENTRY, "", wxDefaultPosition,
                       wxDefaultSize, wxNO_BORDER | wxTE_PROCESS_ENTER);
    mTextCtrl->Hide();

	wxLogTrace(TRACE_STAGE_DRAWING, "Stage created.");
}

Stage::~Stage()
{
	DeleteElements();
	wxLogTrace(TRACE_STAGE_DRAWING, "Stage deleted.");
}

void Stage::RegisterCard(const wxString &inName)
{
	mFrame->GetProgramTree()->RegisterCard(inName);
}

void Stage::SetProgramName(const wxString &inName)
{
	mFrame->SetTitle(inName);
	mFrame->GetProgramTree()->SetProgramName(inName);
}

void Stage::NotifyEnterCard()
{
	mFrame->GetLocationBox()->NotifyEnterCard();
	mFrame->GetProgramTree()->NotifyEnterCard();
}

void Stage::NotifyExitCard()
{
    if (mTextCtrl->IsShown())
        mTextCtrl->Hide();
	DeleteElements();
}

void Stage::NotifyScriptReload()
{
	mFrame->GetProgramTree()->NotifyScriptReload();
    NotifyExitCard();
	gStyleSheetManager.RemoveAll();

	// For now, treat Quake 2 as a special case.
#if CONFIG_HAVE_QUAKE2
	if (Quake2Engine::IsInitialized())
		Quake2Engine::GetInstance()->NotifyScriptReload();
#endif // CONFIG_HAVE_QUAKE2
}

void Stage::NotifyElementsChanged()
{
	wxLogTrace(TRACE_STAGE_DRAWING, "Elements on stage have changed.");

	// Don't do anything unless there's a good chance this window still
	// exists in some sort of valid state.
	// TODO - Is IsShown a good way to tell whether a window is still good?
	if (IsShown())
	{
		// Update our element borders (if necessary) and fix our cursor.
		if (mIsDisplayingBorders)
			InvalidateStage();
		UpdateCurrentElementAndCursor();
	}
}

void Stage::UpdateCurrentElementAndCursor()
{
	// Find which element we're in.
	wxPoint pos = ScreenToClient(::wxGetMousePosition());
	Element *obj = FindLightWeightElement(pos);
	if (obj == NULL || obj != mCurrentElement)
	{
		if (mIsDisplayingXy)
			SetCursor(*wxCROSS_CURSOR);
		else
		{
			if (obj)
				SetCursor(wxCursor(wxCURSOR_HAND));
			else
				SetCursor(wxNullCursor);
		}
		mCurrentElement = obj;
	}
}

void Stage::InterpreterSleep()
{
    // Put our interpreter to sleep.
	// TODO - Keep track of who we're sleeping for.
    ASSERT(TInterpreter::HaveInstance() &&
           !TInterpreter::GetInstance()->Paused());
    TInterpreter::GetInstance()->Pause();
}

void Stage::InterpreterWakeUp()
{
	// Wake up our Scheme interpreter.
	// We can't check TInterpreter::GetInstance()->Paused() because
	// the engine might have already woken the script up on its own.
	// TODO - Keep track of who we're sleeping for.
    ASSERT(TInterpreter::HaveInstance());
    TInterpreter::GetInstance()->WakeUp();
}

Stage::ElementCollection::iterator
Stage::FindElementByName(ElementCollection &inCollection,
						 const wxString &inName)
{
	ElementCollection::iterator i = inCollection.begin();
	for (; i != inCollection.end(); i++)
		if ((*i)->GetName() == inName)
			return i;
	return inCollection.end();
}

void Stage::OnIdle(wxIdleEvent &inEvent)
{
	if (mWaitElement && mWaitElement->HasReachedFrame(mWaitFrame))
		EndWait();
}

void Stage::OnMouseMove(wxMouseEvent &inEvent)
{
	// Do any mouse-moved processing for our Elements.
	UpdateCurrentElementAndCursor();
    if (mIsDisplayingXy)
    {
        wxClientDC dc(this);

        // Get our current screen location.
        wxPoint pos = inEvent.GetPosition();
        long x = dc.DeviceToLogicalX(pos.x);
        long y = dc.DeviceToLogicalY(pos.y);

        // Get the color at that screen location.
        // XXX - May not work on non-Windows platforms, according to
        // the wxWindows documentation.
        wxRawBitmapDC offscreen_dc;
        offscreen_dc.SelectObject(mOffscreenPixmap);
        wxColour color;
        offscreen_dc.GetPixel(x, y, &color);

        // Update the status bar.
        wxString str;
        str.Printf("X: %d, Y: %d, C: %02X%02X%02X",
                   (int) x, (int) y, (int) color.Red(),
                   (int) color.Green(), color.Blue());
        mFrame->SetStatusText(str);
    }
}

void Stage::OnEraseBackground(wxEraseEvent &inEvent)
{
	wxLogTrace(TRACE_STAGE_DRAWING, "Ignoring request to erase stage.");

    // Ignore this event to prevent flicker--we don't need to erase,
    // because we redraw everything from the offscreen buffer.  We may need
    // to override more of these events elsewhere.
}

void Stage::OnPaint(wxPaintEvent &inEvent)
{
	wxLogTrace(TRACE_STAGE_DRAWING, "Painting stage.");

    // Set up our drawing contexts.
    wxPaintDC screen_dc(this);
    
    // Blit our offscreen pixmap to the screen.
    // TODO - Could we optimize drawing by only blitting dirty regions?
	mOffscreenPixmap.BlitTo(&screen_dc, 0, 0,
							mStageSize.GetWidth(), mStageSize.GetHeight(), 0, 0);

    // If necessary, draw the grid.
    if (mIsDisplayingGrid)
    {
        int width = mStageSize.GetWidth();
        int height = mStageSize.GetHeight();
        int small_spacing = 10;
        int large_spacing = small_spacing * 10;

        // Draw the minor divisions of the grid.
        screen_dc.SetPen(*wxLIGHT_GREY_PEN);
        for (int x = 0; x < width; x += small_spacing)
            if (x % large_spacing)
                screen_dc.DrawLine(x, 0, x, height);
        for (int y = 0; y < width; y += small_spacing)
            if (y % large_spacing)
                screen_dc.DrawLine(0, y, width, y);

        // Draw the major divisions of the grid.
        screen_dc.SetPen(*wxGREEN_PEN);
        for (int x2 = 0; x2 < width; x2 += large_spacing)
            screen_dc.DrawLine(x2, 0, x2, height);
        for (int y2 = 0; y2 < width; y2 += large_spacing)
            screen_dc.DrawLine(0, y2, width, y2);
    }

	// If necessary, draw the borders.
	if (mIsDisplayingBorders)
	{
		if (mTextCtrl->IsShown())
			DrawElementBorder(screen_dc, mTextCtrl->GetRect());

		ElementCollection::iterator i = mElements.begin();
		for (; i != mElements.end(); i++)
			if ((*i)->IsShown())
				DrawElementBorder(screen_dc, (*i)->GetRect());
	}
}

void Stage::DrawElementBorder(wxDC &inDC, const wxRect &inElementRect)
{
	inDC.SetPen(*wxRED_PEN);
	inDC.SetBrush(*wxTRANSPARENT_BRUSH);

	// Draw the border *outside* our rectangle.
	wxRect r = inElementRect;
	r.Inflate(1);
	inDC.DrawRectangle(r.x, r.y, r.width, r.height);
}

void Stage::OnTextEnter(wxCommandEvent &inEvent)
{
    // Get the text.
    wxString text = FinishModalTextInput();
    
    // Set up a drawing context.
    wxRawBitmapDC dc;
    dc.SelectObject(mOffscreenPixmap);
    
    // Prepare to draw the text.
    dc.SetTextForeground(mTextCtrl->GetForegroundColour());
    dc.SetTextBackground(mTextCtrl->GetBackgroundColour());
    dc.SetFont(mTextCtrl->GetFont());

    // Draw the text.
    // PORTING - These offsets are unreliable and platform-specific.
    wxPoint pos = mTextCtrl->GetPosition();
    dc.DrawText(text, pos.x + 2, pos.y);
}

void Stage::OnLeftDown(wxMouseEvent &inEvent)
{
	Element *obj = FindLightWeightElement(inEvent.GetPosition());
	if (obj)
		obj->Click();
	//else
		// TODO - For debugging Quake 2 integration.
	    //SetFocus();
}

void Stage::InvalidateStage()
{
    InvalidateRect(wxRect(0, 0,
                          mStageSize.GetWidth(), mStageSize.GetHeight()));
}

void Stage::InvalidateRect(const wxRect &inRect)
{
	wxLogTrace(TRACE_STAGE_DRAWING, "Invalidating: %d %d %d %d",
			   inRect.x, inRect.y,
			   inRect.x + inRect.width, inRect.y + inRect.height);
    Refresh(FALSE, &inRect);
}

wxColour Stage::GetColor(const GraphicsTools::Color &inColor)
{
    if (inColor.alpha)
        gDebugLog.Caution("Removing alpha channel from color");
    return wxColour(inColor.red, inColor.green, inColor.blue);
}

void Stage::ClearStage(const wxColor &inColor)
{
    wxRawBitmapDC dc;
    dc.SelectObject(mOffscreenPixmap);
    wxBrush brush(inColor, wxSOLID);
    dc.SetBackground(brush);
    dc.Clear();
    InvalidateStage();
}

void Stage::FillBox(const wxRect &inBounds, const wxColour &inColor)
{
    wxRawBitmapDC dc;
    dc.SelectObject(mOffscreenPixmap);
    wxBrush brush(inColor, wxSOLID);
    dc.SetBrush(brush);
    dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(inBounds.x, inBounds.y, inBounds.width, inBounds.height);
	InvalidateRect(inBounds);
}

void Stage::DrawPixMap(GraphicsTools::Point inPoint,
					   GraphicsTools::PixMap &inPixMap)
{
	// Mark the rectangle as dirty.
	InvalidateRect(wxRect(inPoint.x, inPoint.y,
						  inPixMap.width, inPixMap.height));

	using GraphicsTools::AlphaBlendChannel;
	using GraphicsTools::Color;
	using GraphicsTools::Distance;
	using GraphicsTools::Point;
	
	// Clip our pixmap boundaries to fit within our screen.
	int stage_width = mStageSize.GetWidth();
	int stage_height = mStageSize.GetHeight();
	Point begin = inPoint;
	begin.x = Max(0, Min(stage_width, begin.x));
	begin.y = Max(0, Min(stage_height, begin.y));
	begin = begin - inPoint;
	Point end = inPoint + Point(inPixMap.width, inPixMap.height);
	end.x = Max(0, Min(stage_width, end.x));
	end.y = Max(0, Min(stage_height, end.y));
	end = end - inPoint;
	
	// Do some sanity checks on our clipping boundaries.
	ASSERT(begin.x == end.x || // No drawing
		   (0 <= begin.x && begin.x < end.x && end.x <= inPixMap.width));
	ASSERT(begin.y == end.y || // No drawing
		   (0 <= begin.y && begin.y < end.y && end.y <= inPixMap.height));
		
	// Figure out where in memory to begin drawing the first row.
	wxRawBitmapPixelRef24 dst_base_addr = mOffscreenPixmap.GetData24();
	wxRawBitmapStride24 dst_row_size = mOffscreenPixmap.GetStride24();
	wxRawBitmapPixelRef24 dst_row_start = dst_base_addr;
	WX_RAW24_OFFSET(dst_row_start, dst_row_size,
					inPoint.x + begin.x, inPoint.y + begin.y);
	WX_RAW24_DECLARE_LIMITS(dst_limits, mOffscreenPixmap);
	
	// Figure out where in memory to get the data for the first row.
	Color *src_base_addr = inPixMap.pixels;
	Distance src_row_size = inPixMap.pitch;
	Color *src_row_start = src_base_addr + begin.y * src_row_size + begin.x;

	// Do the actual drawing.
	for (int y = begin.y; y < end.y; y++)
	{
		wxRawBitmapPixelRef24 dst_cursor = dst_row_start;
		Color *src_cursor = src_row_start;
		for (int x = begin.x; x < end.x; x++)
		{
			// Make sure we're in bounds.
			WX_RAW24_ASSERT_WITHIN_LIMITS(dst_limits, dst_cursor);
			ASSERT(src_cursor >= src_base_addr);
			ASSERT(src_cursor <
				   src_base_addr + (inPixMap.height * src_row_size));
			
			// Draw a single pixel.
			GraphicsTools::Color new_color = *src_cursor;
			WX_RAW24_RED(dst_cursor) =
				AlphaBlendChannel(WX_RAW24_RED(dst_cursor),
								  new_color.red, new_color.alpha);
			WX_RAW24_GREEN(dst_cursor) =
				AlphaBlendChannel(WX_RAW24_GREEN(dst_cursor),
								  new_color.green, new_color.alpha);
			WX_RAW24_BLUE(dst_cursor) =
				AlphaBlendChannel(WX_RAW24_BLUE(dst_cursor),
								  new_color.blue, new_color.alpha);

			WX_RAW24_OFFSET_X(dst_cursor, 1);
			src_cursor++;
		}

		WX_RAW24_OFFSET_Y(dst_row_start, dst_row_size, 1);
		src_row_start += src_row_size;
	}
}

void Stage::DrawBitmap(const wxBitmap &inBitmap, wxCoord inX, wxCoord inY,
                       bool inTransparent)
{
    wxRawBitmapDC dc;
    dc.SelectObject(mOffscreenPixmap);
    dc.DrawBitmap(inBitmap, inX, inY, inTransparent);
    InvalidateRect(wxRect(inX, inY,
                          inX + inBitmap.GetWidth(),
                          inY + inBitmap.GetHeight()));
}

void Stage::ModalTextInput(const wxRect &inBounds,
                           const int inTextSize,
                           const wxColour &inForeColor,
                           const wxColour &inBackColor)
{
    ASSERT(!mTextCtrl->IsShown());

    // Update our control to have the right properties and show it.
    mTextCtrl->SetValue("");
    wxFont font(inTextSize, wxROMAN, wxNORMAL, wxNORMAL);
    mTextCtrl->SetFont(font);
    mTextCtrl->SetForegroundColour(inForeColor);
    mTextCtrl->SetBackgroundColour(inBackColor);
    mTextCtrl->SetSize(inBounds);
    mTextCtrl->Show();
    mTextCtrl->SetFocus();
	NotifyElementsChanged();

	InterpreterSleep();
}

wxString Stage::FinishModalTextInput()
{
    ASSERT(mTextCtrl->IsShown());

	InterpreterWakeUp();

	// Store our result somewhere useful.
	gVariableManager.SetString("_modal_input_text", mTextCtrl->GetValue());

    // Hide our text control and get the text.
    mTextCtrl->Hide();
	NotifyElementsChanged();
    return mTextCtrl->GetValue();
}

bool Stage::Wait(const wxString &inElementName, MovieFrame inUntilFrame)
{
	ASSERT(mWaitElement == NULL);

	// Look for our element.
	ElementCollection::iterator i =
		FindElementByName(mElements, inElementName);

	// Make sure we can wait on this element.
	// TODO - Refactor this error-handling code to a standalone
	// routine so we don't have to keep on typing it.
	const char *name = (const char *) inElementName;
	if (i == mElements.end())
	{
		gDebugLog.Caution("wait: Element %s does not exist", name);
		return false;
	}
	MovieElement *movie = dynamic_cast<MovieElement*>(*i);
	if (movie == NULL)
	{
		gDebugLog.Caution("wait: Element %s is not a movie", name);
		return false;		
	}

	// Return immediately (if we're already past the wait point) or
	// go to sleep for a while.
	if (movie->HasReachedFrame(inUntilFrame))
		gDebugLog.Log("wait: Movie %s has already past frame %d",
					  name, inUntilFrame);
	else
	{
		mWaitElement = movie;
		mWaitFrame = inUntilFrame;
		InterpreterSleep();
	}
	return true;
}

void Stage::EndWait()
{
	gDebugLog.Log("wait: Waking up.");
	ASSERT(mWaitElement != NULL);
	mWaitElement = NULL;
	mWaitFrame = 0;
	InterpreterWakeUp();
}

void Stage::AddElement(Element *inElement)
{
	// Delete any existing Element with the same name.
	(void) DeleteElementByName(inElement->GetName());

	// Add the new Element to our list.
	mElements.push_back(inElement);
	NotifyElementsChanged();
}

Element *Stage::FindElement(const wxString &inElementName)
{
	ElementCollection::iterator i =
		FindElementByName(mElements, inElementName);
	if (i == mElements.end())
		return NULL;
	else
		return *i;
}

Element *Stage::FindLightWeightElement(const wxPoint &inPoint)
{
	// Look for the most-recently-added Element containing inPoint.
	Element *result = NULL;
	ElementCollection::iterator i = mElements.begin();
	for (; i != mElements.end(); i++)
		if ((*i)->IsLightWeight() && (*i)->IsPointInElement(inPoint))
			result = *i;
	return result;
}

void Stage::DestroyElement(Element *inElement)
{
	// Clean up any dangling references to this object.
	if (inElement == mCurrentElement)
		mCurrentElement = NULL;
	if (inElement == mWaitElement)
		EndWait();

	// Destroy the object.
	// TODO - Implemented delayed destruction so element callbacks can
	// destroy the element they're attached to.
	delete inElement;
}

bool Stage::DeleteElementByName(const wxString &inName)
{
	bool found = false;
	ElementCollection::iterator i = FindElementByName(mElements, inName);
	if (i != mElements.end())
	{
		// Completely remove from the collection first, then destroy.
		Element *elem = *i;
		mElements.erase(i);
		DestroyElement(elem);
		found = true;
	}
	NotifyElementsChanged();
	return found;
}

void Stage::DeleteElements()
{
	ElementCollection::iterator i = mElements.begin();
	for (; i != mElements.end(); i++)
		DestroyElement(*i);
	mElements.clear();
	NotifyElementsChanged();
}
