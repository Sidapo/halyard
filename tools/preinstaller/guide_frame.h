// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-
// @BEGIN_LICENSE
//
// Halyard - Multimedia authoring and playback system
// Copyright 1993-2008 Trustees of Dartmouth College
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

// This code was originally based on the example code at
// <http://wxwidgets.org/docs/tutorials/hello.htm>.

#ifndef GUIDE_FRAME_H
#define GUIDE_FRAME_H 1

class GuideFrame : public wxFrame {
public: 
    GuideFrame(bool shouldWarnAboutProLicense);
    
private:
    // Represents a button that can have a check-mark next to it.
    struct CheckableButton {
        wxStaticBitmap *bitmap;
        wxButton *button;
        wxStaticText *message;

        CheckableButton() : bitmap(NULL), button(NULL), message(NULL) {}
        CheckableButton(wxStaticBitmap *inBitmap, wxButton *inButton,
                        wxStaticText *inMessage)
            : bitmap(inBitmap), button(inButton), message(inMessage) {}
    };

    /// True if we need to warn the user about a possible loss of QuickTime
    /// Pro license.
    bool mShouldWarnAboutProLicense;

    /// True if our application is in the foreground.
    bool mInForeground;

    // True if we should be checking the QuickTime when we're in the
    // foreground.
    bool mShouldCheckQuickTimeVersionWhenInForeground;

    wxTimer mTimer;

    wxFont mTitleFont;
    wxFont mStepHeadingFont;

    wxBitmap mBlankBitmap;
    wxBitmap mArrowBitmap;
    wxBitmap mCheckBitmap;

    wxWindow *mBackground;
    wxSizer *mMainColumn;
    CheckableButton mQTButton;
    CheckableButton mAppButton;

    void CreateStepHeading(int number, const wxString &heading);
    CheckableButton CreateStepButton(int id, const wxString &name);

    bool ConfirmLossOfQuickTimeProOK();
    void OnInstallQuickTime(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnActivate(wxActivateEvent& event);
    void CheckQuickTimeVersion();
    void OnInstallApplication(wxCommandEvent& event);
    void OnClose(wxCloseEvent &inEvent);
    void ForceImmediateRedraw();

    DECLARE_EVENT_TABLE()
};

#endif // !defined(GUIDE_FRAME_H)
