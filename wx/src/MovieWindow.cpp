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
#include "AppGlobals.h"
#include "FiveLApp.h"
#include "MovieWindow.h"

MovieWindow::MovieWindow(wxWindow *inParent, wxWindowID inID,
						 const wxPoint &inPos,
						 const wxSize &inSize,
						 long inWindowStyle,
						 MovieWindowStyle inMovieWindowStyle,
						 const wxString &inName)
    : wxWindow(inParent, inID, inPos, inSize, inWindowStyle, inName),
      mMovieWindowStyle(inMovieWindowStyle)
{
    // Set a more-appropriate default background color for a movie.
    SetBackgroundColour(MOVIE_WINDOW_COLOR);

	// If this is an audio-only movie, hide the widget.
	if (mMovieWindowStyle & MOVIE_AUDIO_ONLY)
		Hide();

	wxLogTrace(TRACE_STAGE_DRAWING, "Created movie window.");
}

MovieWindow::~MovieWindow()
{
	wxLogTrace(TRACE_STAGE_DRAWING, "Destroyed movie window.");
}

void MovieWindow::SetMovie(const wxString &inName)
{
	wxLogWarning("Movies not supported; skipping \"%s\".",
				 (const char *) inName);
}

MovieFrame MovieWindow::GetFrame()
{
	return 0;
}

bool MovieWindow::IsDone()
{
	return true;
}

void MovieWindow::Pause()
{
	return;
}

void MovieWindow::Resume()
{
	return;
}

void MovieWindow::SetVolume(const std::string &inChannel, double inVolume)
{
    return;
}
