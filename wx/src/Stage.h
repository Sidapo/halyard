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

#ifndef Stage_H
#define Stage_H

#include "TInterpreter.h"
#include "AppGlobals.h"
#include "DirtyList.h"
#include "DrawingContextStack.h"
#include "CairoDrawing.h"

class StageFrame;
class Element;
typedef shared_ptr<Element> ElementPtr;
class MediaElement;
typedef shared_ptr<MediaElement> MediaElementPtr;
class EventDispatcher;
class ImageCache;
class CursorManager;
class TransitionManager;
class DrawingArea;

BEGIN_NAMESPACE_HALYARD
class Cursor;
END_NAMESPACE_HALYARD

/// The window where all actual script output and interaction occurs.
class Stage : public wxWindow, public Halyard::TReloadNotified {
    /// A list of Elements.
    typedef std::deque<ElementPtr> ElementCollection;

    /// The StageFrame associated with the stage.  We need to poke at it
    /// occassionally to implement various features.
    StageFrame *mFrame;

    /// This timer sends us periodic events, which we pass to Element
    /// objects and our script as "idle" events.  We can't use wxIdleEvent
    /// for this, because it offers no way to control the frequency with
    /// which events are sent.
    wxTimer mTimer;

    /// The size of our drawing stage.
    wxSize mStageSize;

    /// The last card the stage was known to be on.
    std::string mLastCard;

    /// Before drawing graphics to the stage, we need to composite several
    /// layers into a single image.  We use this buffer for that purpose.
    wxBitmap mCompositingPixmap;

    /// Rectangles which need to be recomposited.
    DirtyList mRectsToComposite;
    
    /// Rectangles which need to be redrawn to the screen during a refresh.
    DirtyList mRectsToRefresh;

    /// This drawing area contains the background graphics for the stage.
    /// Other layers are composited with this layer.
    std::auto_ptr<DrawingArea> mBackgroundDrawingArea;

    /// The current stack of drawing contexts.
    std::auto_ptr<DrawingContextStack> mDrawingContextStack;

    /// A bitmap for use during various fade effects.
    wxBitmap mOffscreenFadePixmap;

    /// We try to rate-limit our idle events to prevent performance
    /// problems with the Scheme garbage collector (the event dispatching
    /// system allocates some memory to process events).
    wxLongLong mLastIdleEvent;

    /// This object does all of our event-dispatching for us.
    EventDispatcher *mEventDispatcher;

    /// Our image cache.  This allows us to speed up image loading
    /// tremendously in some common cases.
    ImageCache *mImageCache;

    /// Our cursor manager.  This maps strings to cursors, and allows the
    /// application to register new cursors.
    CursorManager *mCursorManager;

    /// Our transition manager.  This maps transition names to transitions,
    /// and applies them.
    TransitionManager *mTransitionManager;

    /// The cursor that we're (nominally) displaying for the stage right
    /// now, if we're actually displaying a cursor.  See the notes in
    /// Cursor.h about cursor-pointer ownership.
    Halyard::Cursor *mDesiredCursor;

    /// The cursor we're *actually* displaying right now.  See the notes in
    /// Cursor.h about cursor-pointer ownership.
    Halyard::Cursor *mActualCursor;

    /// Our currently active elements.
    ElementCollection mElements;

    /// The element which most recently contained the mouse.
    ///
    /// Invariant: This variable is always NULL, or points to a valid
    /// lightweight element.  Be careful when deleting elements!
    ElementPtr mCurrentElement;

    /// The element which most recently contained the mouse, _including_
    /// those elements which don't otherwise support mouse interaction.
    ///
    /// Invariant: This variable is always NULL, or points to a valid
    /// lightweight element.
    ElementPtr mCurrentElementNamedInStatusBar;

    /// The element which has a "grab" on the mouse.
    ElementPtr mGrabbedElement;

    /// The movie we're waiting on, or NULL if we're not waiting on anything.
    MediaElementPtr mWaitElement;

    /// The movie frame we're waiting on.
    MovieFrame mWaitFrame;

    /// Have the elements on the stage changed since the last time we
    /// processed events?
    bool mElementsHaveChanged;

    /// Should we hide the cursor until the user moves the mouse?
    bool mShouldHideCursorUntilMouseMoved;

    /// Are we displaying the XY co-ordinates of the cursor?
    bool mIsDisplayingXy;

    /// Are we displaying the grid over the stage?
    bool mIsDisplayingGrid;

    /// Are we displaying borders for the interactive elements.
    bool mIsDisplayingBorders;
    
    /// Should the next compile include error tracing?
    bool mIsErrortraceCompileEnabled;

    /// Is the stage being destroyed yet?
    bool mIsBeingDestroyed;

    /// The last stage position copied with a right-click.
    std::vector<wxPoint> mCopiedPoints;

    /// Get the compositing pixmap associated with this stage.
    wxBitmap &GetCompositingPixmap();

    /// Validate the entire stage--i.e., mark it as having been redrawn.
    void ValidateStage();

    /// Set up clipping regions on a DC to make sure that we don't try
    /// to overdraw any heavyweight elements.
    void ClipElementsThatDrawThemselves(wxDC &inDC);

    /// Repaint the stage.
    void PaintStage(wxDC &inDC, const wxRegion &inDirtyRegion);

    /// Draw a border for the specified element.
    void DrawElementBorder(wxDC &inDC, ElementPtr inElement);

    /// Return true if our game engine is current displayed.
    bool GameEngineIsDisplayed();

    /// Focus our game engine.
    void GameEngineSetFocus();

    /// End an active Wait().
    void EndWait();

    /// Put the interpreter to sleep.
    void InterpreterSleep();

    /// Wake the interpreter up at the next opportunity.
    void InterpreterSetShouldWakeUp();

    /// Find an element by name, and return an iterator.
    ElementCollection::iterator
    FindElementByName(ElementCollection &inCollection,
                      const wxString &inName);

    /// Detach an element from the stage and destroy it.
    void DestroyElement(ElementPtr inElement);

    /// We've entered an element; update things appropriately.
    void EnterElement(ElementPtr inElement, const wxPoint &inPosition);

    /// We've left an element; update things appropriately.
    void LeaveElement(ElementPtr inElement, const wxPoint &inPosition);

    /// Get the current mouse position, relative to the origin of the
    /// stage.
    wxPoint CurrentMousePosition();

    /// Actually update the current cursor to match mDesiredCursor
    /// and the result of ShouldShowCursor().  You generally want to
    /// call UpdateCurrentElementAndCursor instead, which will actually
    /// detect when mDesiredCursor needs to be changed.
    void UpdateDisplayedCursor();

    /// Called when the currently displayed cursor is being destroyed,
    /// and needs to be replaced by something reasonable.  See also
    /// UpdateDisplayedCursor for related code.
    void ReplaceDisplayedCursorWithDefault();

    /// Figure out which element we're inside, and figure out what cursor
    /// we should be displaying now.
    void UpdateCurrentElementAndCursor(const wxPoint &inPosition);
    void UpdateCurrentElementAndCursor();

public:
    /// Create a new stage.  Should only be called by StageFrame.
    ///
    /// \param inParent  The immediate parent of this stage.
    /// \param inFrame  The StageFrame in which this stage appears.
    ///                Probably not the same as inParent.
    /// \param inStageSize  The size of the stage.
    Stage(wxWindow *inParent, StageFrame *inFrame, wxSize inStageSize);

    /// We need to clean up any resources which aren't directly managed
    /// by wxWindows.
    ~Stage();

    /// Get the size of our stage.
    wxSize GetStageSize() const { return mStageSize; }

    /// Load a special-purpose graphic associated with our currently running
    /// Halyard script. Such graphics currently include script icons and
    /// the splash screens.
    CairoSurfacePtr GetBrandingImage(const std::string &inName);

    /// If we can, show a splash screen for the loading program.  This is
    /// called shortly before showing the stage, and well before loading
    /// the actual script.
    void MaybeShowSplashScreen();

    /// Draw the specified splash-screen graphic, if available.
    void MaybeDrawSplashGraphic(const std::string &inName);

    /// Draw a progress bar on the Stage showing how much of the script
    /// has been loaded.
    void DrawLoadProgress();

    /// Similar to Refresh, but does nothing if we're not showing a splash
    /// screen.
    void RefreshSplashScreen();

    /// Raise an element to the top of our current list of elements.
    /// Takes O(N) time, and is used primarily by hackish z-order fixes.
    void RaiseToTop(ElementPtr inElem);

    /// Should the script be allowed to idle right now?
    bool IsIdleAllowed() const;

    /// Return true if and only if the script is fully initialized.
    /// Will briefly return false after the script is reloaded.
    bool IsScriptInitialized();

    /// Place the stage into edit mode or take it out again.  May
    /// only be called if IsScriptInitialized() returns true.
    void SetEditMode(bool inWantEditMode = true);

    /// Is the stage in edit mode?  May only be called if
    /// IsScriptInitialized() returns true.
    bool IsInEditMode();

    /// Hide the cursor until the user moves the mouse.  Overrides all
    /// other cursor display logic.  Typically called by scripts at the
    /// start of movie playback to keep the cursor from obscuring movie
    /// details, even when there's interactive stuff elsewhere on the
    /// screen.
    void HideCursorUntilMouseMoved();

    /// Should we display a cursor?
    bool ShouldShowCursor();

    /// Should we send events to our event dispatcher?
    bool ShouldSendEvents();

    /// Return true if and only if it's safe to call TryJumpTo().
    bool CanJump();

    /// Jump to the specified card.  May only be called if
    /// IsScriptInitialized() returns true.  If the specified
    /// card does not exist, displays an error to the user.
    void TryJumpTo(const wxString &inName);

    /// Return the EventDispatcher associated with this stage.
    EventDispatcher *GetEventDispatcher() { return mEventDispatcher; }

    /// Return the image cache associated with this stage.
    ImageCache *GetImageCache() { return mImageCache; }

    /// Return the cursor manager associated with this stage.
    CursorManager *GetCursorManager() { return mCursorManager; }

    /// Notify the stage that the interpreter has moved to a new card.
    void NotifyEnterCard(const wxString &inName);

    /// Notify the stage that the interpreter is leaving an old card.
    void NotifyExitCard();

    /// Notify the stage that the script is being reloaded.
    void NotifyReloadScriptStarting();

    /// Notify the stage that the script has been reloaded.
    void NotifyReloadScriptSucceeded();

    /// Let the stage know that the list of active elements has changed.
    void NotifyElementsChanged();

    /// Update the TStateDB system clock.
    void UpdateClockKeysInStateDB();

    /// Redirect all further drawing calls to the specified element until
    /// further notice.
    void PushDrawingContext(ElementPtr inElement)
        { mDrawingContextStack->PushDrawingContext(inElement); }

    /// Pop the top element off the current drawing stack, and make sure
    /// it matches the specified element.
    void PopDrawingContext(ElementPtr inElement)
        { mDrawingContextStack->PopDrawingContext(inElement); }
    
    /// Send an idle message to any elements on the stage.  (This is one
    /// of many functions called by OnTimer, and you probably won't need
    /// to call it directly.)
    ///
    /// \todo Make this function and On* functions private or protected.
    void IdleElements();

    /// Do our idle-time processing.
    void OnTimer(wxTimerEvent &inEvent);

    /// Trap mouse movement events so we can do various useful things.
    void OnMouseMove(wxMouseEvent &inEvent);

    /// Intercept the erase event to prevent flicker.
    void OnEraseBackground(wxEraseEvent &inEvent);

    /// Redraw the stage.
    void OnPaint(wxPaintEvent &inEvent);

    /// Handle a character event.
    void OnChar(wxKeyEvent &inEvent);

    /// Handle a mouse-down event.
    void OnLeftDown(wxMouseEvent &inEvent);

    /// Handle a mouse double-click event.
    void OnLeftDClick(wxMouseEvent &inEvent);

    /// Handle a mouse-up event.
    void OnLeftUp(wxMouseEvent &inEvent);

    /// Handle a mouse-down event.
    void OnRightDown(wxMouseEvent &inEvent);

    /// Handle unexpected loss of mouse grab.
    void OnMouseCaptureChanged(wxMouseCaptureChangedEvent &inEvent);

    /// Are we currently displaying the XY co-ordinates of the cursor?
    bool IsDisplayingXy() { return mIsDisplayingXy; }

    /// Toggle the display of the cursor's XY co-ordinates.
    void ToggleDisplayXy() { mIsDisplayingXy = !mIsDisplayingXy; }

    /// Are we currently displaying the grid?
    bool IsDisplayingGrid() { return mIsDisplayingGrid; }

    /// Toggle the display of the grid.
    void ToggleDisplayGrid()
        { InvalidateScreen(); mIsDisplayingGrid = !mIsDisplayingGrid; }

    /// Are we currently displaying the borders?
    bool IsDisplayingBorders() { return mIsDisplayingBorders; }

    /// Toggle the display of the borders.
    void ToggleDisplayBorders()
        { InvalidateScreen(); mIsDisplayingBorders = !mIsDisplayingBorders; }

    /// Should the next compile include error tracing?
    bool IsErrortraceCompileEnabled() { return mIsErrortraceCompileEnabled; }

    /// Toggle whether the next compile should include error tracing.
    void ToggleErrortraceCompile()
        { mIsErrortraceCompileEnabled = !mIsErrortraceCompileEnabled; }

    /// Invalidate the entire stage.
    void InvalidateStage();

    /// Invalidate just the screen, not the offscreen compositing for the stage.
    void InvalidateScreen();

    /// Invalidate the specified rectangle.
    void InvalidateRect(const wxRect &inRect);

    /// Get the currently selected drawing area for this stage.
    DrawingArea *GetCurrentDrawingArea();

    /// Get the background drawing area for this stage.
    DrawingArea *GetBackgroundDrawingArea()
        { return mBackgroundDrawingArea.get(); }

    /// Save a screenshot to the specified file
    ///
    /// \param inFilename  The name of the file to save to.
    void Screenshot(const wxString &inFilename);

    /// Copy a string to the clipboard.
    ///
    /// \param inString  The string to copy.
    void CopyStringToClipboard(const wxString &inString);

    /// Suspend the interpreter until the named movie reaches the specified
    /// frame.
    ///
    /// \param inElementName  The name of the MediaElement to wait on.
    /// \param inUntilFrame  The frame to wait until.
    /// \return  true if the wait request was valid, false if the
    ///                named element doesn't exist or isn't a movie.
    bool Wait(const wxString &inElementName, MovieFrame inUntilFrame);

    /// Refresh the screen using the specified effect.
    ///
    /// \param inTransition  The name of the transition to use, or "none".
    /// \param inMilliseconds  The desired duration of the transition.
    void RefreshStage(const std::string &inTransition,
                      int inMilliseconds);

    /// Add a Element to this Stage.  This should only be called
    /// by the Element class.
    void AddElement(ElementPtr inElement);

    /// Find an element by name.
    ///
    /// \param inElementName  The name to search for.
    /// \return  A pointer to the element, or NULL.
    ElementPtr FindElement(const wxString &inElementName);

    /// Find the lightweight Element containing the specified point, if
    /// any.
    ///
    /// \param inPoint  The point to check.
    /// \param inMustWantCursor  Only find elements where WantsCursor is true.
    /// \return  A pointer to the Element, or NULL.
    ElementPtr FindLightWeightElement(const wxPoint &inPoint,
                                      bool inMustWantCursor = true);

    /// Find the appropriate event dispatcher for the given point.
    /// We normally call this function when we want to find a
    /// lightweight element to handle some kind of mouse event.
    ///
    /// \param inPoint  The point to check.
    /// \return  The event dispatcher which should handle
    ///                this event.
    EventDispatcher *FindEventDispatcher(const wxPoint &inPoint);

    /// Delete a Element by name.
    ///
    /// \param inName  The name of the Element to delete.
    /// \return  Returns true if that Element existed.
    bool DeleteElementByName(const wxString &inName);

    /// Delete all Elements owned the Stage.
    void DeleteElements();

    /// Return true if a movie is playing.
    bool IsMediaPlaying();

    /// End all media (audio & video) elements which are playing.
    void EndMediaElements();

    /// "Grab" the mouse on behalf of the specified element.  This means
    /// that all mouse events will be sent to that element until further
    /// notice, regardless of where the event occurred.  Grabs are used to
    /// implement standard buttons without busy-looping during mouse down.
    void MouseGrab(ElementPtr inElement);

    /// Ungrab the mouse.  'inElement' should match the previous grab.
    void MouseUngrab(ElementPtr inElement);

    /// Is the mouse grabbed right now?
    bool MouseIsGrabbed() { return mGrabbedElement ? true : false; }

    /// Is the mouse grabbed by the specified element?
    bool MouseIsGrabbedBy(ElementPtr inElement)
        { return mGrabbedElement == inElement; }

    /// Should we send mouse events to the specified element?  This is
    /// normally true, unless a grab is in effect, in which case only
    /// the grabbed element should receive mouse events.
    bool ShouldSendMouseEventsToElement(ElementPtr inElement);

    /// Get the number of accessible children of the Stage.
    int GetAccessibleChildCount() { return mElements.size(); }

    /// Get the specified accessible child of the Stage.  Uses a zero-based
    /// index, unlike the corresponding wxWidgets API.
    ElementPtr GetAccessibleChild(size_t i) { return mElements[i]; }

    DECLARE_EVENT_TABLE();
};

#endif // Stage_H
