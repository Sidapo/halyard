// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#ifndef Widget_H
#define Widget_H

#include "Stage.h"
#include "Element.h"

//////////
// A widget represents a full-fledged wxWindow object hanging around on
// our stage.  It does its own event processing.
//
class Widget : public Element
{
	//////////
	// The wxWindow implementing this widget.
	//
	wxWindow *mWindow;
	
protected:
	//////////
	// A constructor for Widget subclasses which need to do complicated
	// widget creation.  Before the subclass exits its constructor, it
	// must call InitializeWidgetWindow (below).
	//
	// [in] inStage - The stage to which this widget is attached.
	// [in] inName - The name of this widget.
	//
	Widget(Stage *inStage, const wxString &inName);

	//////////
	// See the constructor without am inWindow argument for details.
	//
	// [in] inWindow - The wxWindow implementing this widget.
	//                 Set the parent of this window to the Stage--
	//                 this will get event-handling and destruction
	//                 hooked up correctly.
	//
	void InitializeWidgetWindow(wxWindow *inWindow);

public:
	//////////
	// Create a new Widget, and add it to the stage.
	//
	// [in] inStage - The stage to which this widget is attached.
	// [in] inName - The name of this widget.
	// [in] inWindow - The wxWindow implementing this widget.
	//                 Set the parent of this window to the Stage--
	//                 this will get event-handling and destruction
	//                 hooked up correctly.
	//
	Widget(Stage *inStage, const wxString &inName, wxWindow *inWindow);

	//////////
	// Destroy the widget.
	//
	~Widget();

	//////////
	// Get the bounding rectangle for the widget.
	//
	virtual wxRect GetRect();

	//////////
	// Show or hide the widget.
	//
	virtual void Show(bool inShow);

	//////////
	// Return true if the stage object is shown on the screen.
	//
	virtual bool IsShown();

	//////////
	// Draw an outline around the widget.
	//
	virtual void DrawElementBorder(wxDC &inDC);
};

#endif // Widget_H
