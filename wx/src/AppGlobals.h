// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#ifndef AppGlobals_H
#define AppGlobals_H

//////////
// We use this trace mask to debug flicker and other stage drawing problems.
//
#define TRACE_STAGE_DRAWING "STAGE DRAWING"

//////////
// Enumerations for menu items, toolbar buttons, and other command-
// generating widgets.
//
enum {
    FIVEL_EXIT = 1,
    FIVEL_RELOAD_SCRIPT = 100,
	FIVEL_JUMP_CARD,
    FIVEL_ABOUT = 200,

    FIVEL_SHOW_LOG = 300,
	FIVEL_SHOW_LISTENER,
	FIVEL_SHOW_TIMECODER,
    FIVEL_FULL_SCREEN,
    FIVEL_DISPLAY_XY,
	FIVEL_DISPLAY_GRID,
	FIVEL_DISPLAY_BORDERS,

	FIVEL_TEXT_ENTRY = 1000,
	FIVEL_LOCATION_BOX,
	FIVEL_LISTENER_TEXT_ENTRY
};

// Define this symbol to 1 to help debug redraw problems by making each
// suspicious widget a different, alarming color.
#define FIVEL_USE_UGLY_WINDOW_COLORS 0

// Choose an appropriate set of window colors.
#if FIVEL_USE_UGLY_WINDOW_COLORS
#  define STAGE_FRAME_COLOR      (*wxRED)
#  define STAGE_BACKGROUND_COLOR (*wxCYAN)
#  define STAGE_COLOR            (*wxGREEN)
#  define MOVIE_WINDOW_COLOR     (wxColour(0, 255, 255))
#else // !FIVEL_USE_UGLY_WINDOW_COLORS
#  define STAGE_FRAME_COLOR      (*wxBLACK)
#  define STAGE_BACKGROUND_COLOR (*wxBLACK)
#  define STAGE_COLOR            (*wxBLACK)
#  define MOVIE_WINDOW_COLOR     (*wxBLACK)
#endif // !FIVEL_USE_UGLY_WINDOW_COLORS

#endif // AppGlobals_H
