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

#ifndef ScriptEditor_H
#define ScriptEditor_H

#include "TInterpreter.h"
#include "EventDelegator.h"

class DocNotebook;

class ScriptEditor : public wxFrame {
    static ScriptEditor *sFrame;
    static void MaybeCreateFrame();

    DocNotebook *mNotebook;
    EventDelegator mDelegator;
    bool mProcessingActivateEvent;

public:
    static void EditScripts();
    static bool SaveAllForReloadScript();
    static bool ProcessEventIfExists(wxEvent &event);

    ScriptEditor();
    ~ScriptEditor();

    virtual bool ProcessEvent(wxEvent& event);

private:
    void OpenDocument(const wxString &path);

    void OnActivate(wxActivateEvent &event);
    void OnClose(wxCloseEvent &event);
    void OnNew(wxCommandEvent &event);
    void OnOpen(wxCommandEvent &event);
    void OnCloseWindow(wxCommandEvent &event);
    void DisableUiItem(wxUpdateUIEvent &event);

    DECLARE_EVENT_TABLE();
};

#endif // ScriptEditor_H
