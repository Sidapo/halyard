// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#include <wx/wx.h>

#include "Zone.h"
#include "TInterpreter.h"

Zone::Zone(Stage *inStage, const wxString &inName, const wxRect &inBounds,
		   TCallback *inAction)
    : Element(inStage, inName), mBounds(inBounds), mAction(inAction)
{
    ASSERT(mAction);
}

Zone::~Zone()
{
	// XXX - This callback may still be active!
    delete mAction;
}

bool Zone::IsPointInElement(const wxPoint &inPoint)
{
    return mBounds.Inside(inPoint);
}

void Zone::Click()
{
    mAction->Run();
}

