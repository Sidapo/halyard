// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#include "AppGraphics.h"

// Include the XPMs, making sure to turn off their 'static' declarations.
// The XPM files can be regenerated using the 'make-xpms' script in this
// directory, which should work correctly on a Linux system with NetPBM
// installed.
#if CONFIG_USE_XPMS
#	define static	
#	include "ic_5L.xpm"
#	include "ic_listener.xpm"
#	include "ic_timecoder.xpm"
#	include "tb_borders.xpm"
#	include "tb_grid.xpm"
#	include "tb_reload.xpm"
#	include "tb_xy.xpm"
#endif // CONFIG_USE_XPMS
