// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#ifndef BrowserElementIE_H
#define BrowserElementIE_H

#include "BrowserElement.h"
#include "TInterpreter.h"


//////////
// A browser element based on embedding Internet Explorer via ActiveX.  This
// could be trivially generalized to support any other browser controls
// which implement IE's ActiveX interfaces.
//
// PORTING - This obviously isn't going to work anywhere but Windows.
//
class BrowserElementIE : public BrowserElement {
    friend class CustomIEHtmlWindow;

    CustomIEHtmlWindow *mHtmlWindow;

public:
    BrowserElementIE(Stage *inStage, const wxString &inName,
                     const wxRect &inBounds,
                     FIVEL_NS TCallback *inDispatch);

    virtual void LoadFile(const wxString &inFileName);
    virtual void LoadUrl(const wxString &inUrl);
    virtual wxString GetCurrentPageUrl();
    virtual wxString GetCurrentPageTitle();
    virtual bool CanGoBack();
    virtual bool GoBack();
    virtual bool CanGoForward();
    virtual bool GoForward();
    virtual bool CanRefresh();
    virtual bool Refresh();
    virtual bool CanStop();
    virtual bool Stop();
};

#endif // BrowserElementIE_H
