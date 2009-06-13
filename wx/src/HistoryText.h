// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
// @BEGIN_LICENSE
//
// Halyard - Multimedia authoring and playback system
// Copyright 1993-2009 Trustees of Dartmouth College
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

#ifndef HistoryText_H
#define HistoryText_H

/// A HistoryText control is a wxTextCtrl. It implements a command history 
/// along the lines of Bash. Each time you hit "RETURN", what you're 
/// currently typing is added to the history, which can be navigated using 
/// the arrow keys or CTRL-P and CTRL-N.
class HistoryTextCtrl : public wxTextCtrl {
    std::vector<wxString> mHistoryItems;
    size_t mHistoryCurrent;

public:
    HistoryTextCtrl(wxWindow* parent, 
                    wxWindowID id, 
                    const wxString& value = wxT(""), 
                    const wxPoint& pos = wxDefaultPosition, 
                    const wxSize& size = wxDefaultSize, 
                    long style = 0, 
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = wxTextCtrlNameStr);
        
    void OnKeyDown(wxKeyEvent &inEvent);
    void OnTextEnter(wxCommandEvent &inEvent);

    void SaveCurrHist();
    void DisplayCurrHist();
    void HistPrev();
    void HistNext();
    
    DECLARE_EVENT_TABLE();
};

#endif // HistoryText_H
