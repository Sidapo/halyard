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

#ifndef ElementsPane_H
#define ElementsPane_H

#include "CustomTreeCtrl.h"

class StageFrame;
class Node;
typedef shared_ptr<Node> NodePtr;

/// This pane displays a tree of elements, and the nodes which contain
/// those elements.
class ElementsPane : public CustomTreeCtrl {
    DECLARE_EVENT_TABLE();

    typedef std::map<wxString,wxTreeItemId> ItemMap;

    /// Maps Node names to wxTreeItemId objects.
    ItemMap mItemMap;

    /// Examine inNode for any interesting dynamic state, and update inItem
    /// accordingly.
    void UpdateItemForDynamicNodeState(wxTreeItemId inItem, NodePtr inNode);

public:
    /// Create a new ElementsPane.
    ElementsPane(StageFrame *inStageFrame);

    /// Show or hide this window.  Overridden so that we can turn this pane
    /// off when it isn't needed.
    virtual bool Show(bool inShow = true);

    /// Register a node with this pane.
    void RegisterNode(NodePtr inNode);

    /// Unregister a node from this pane.
    void UnregisterNode(NodePtr inNode);

    /// Call this function to update the tree item corresponding to inNode.
    /// Right now, this needs to happen whenever nodes are shown or hidden.
    void NotifyNodeStateChanged(NodePtr inNode);
};

#endif // ElementsPane_H

