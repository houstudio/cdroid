/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __ACCESSIBILITY_VIEW_COMMAND_H__
#define __ACCESSIBILITY_VIEW_COMMAND_H__
#include <string>
namespace cdroid{

class View;
class Bundle;

/** Functional interface used to create a custom accessibility action.
    (androidx.core.view.accessibility.AccessibilityViewCommand — the command half
    of ViewCompat.add/replaceAccessibilityAction, collapsed onto View in CDROID.)
    Commands are borrowed: the view's action list keeps the raw pointer and the
    caller owns the instance, mirroring the androidx reference semantics. */
class AccessibilityViewCommand {
public:
    /** Object containing arguments passed into an AccessibilityViewCommand. */
    class CommandArguments {
    public:
        virtual ~CommandArguments() = default;
        void setBundle(Bundle* bundle);
    protected:
        Bundle* mBundle = nullptr;
    };

    virtual ~AccessibilityViewCommand() = default;

    /** Performs the action.
        @return true if the action was handled, false otherwise.
        @param view The view to act on.
        @param arguments Optional action arguments (may be null). */
    virtual bool perform(View& view, CommandArguments* arguments) = 0;

    /** Arguments for AccessibilityNodeInfo::AccessibilityAction
        ACTION_NEXT_AT_MOVEMENT_GRANULARITY / ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY. */
    class MoveAtGranularityArguments : public CommandArguments {
    public:
        int getGranularity() const;
        bool getExtendSelection() const;
    };

    /** Arguments for ACTION_NEXT_HTML_ELEMENT / ACTION_PREVIOUS_HTML_ELEMENT. */
    class MoveHtmlArguments : public CommandArguments {
    public:
        std::string getHTMLElement() const;
    };

    /** Arguments for ACTION_SET_SELECTION. */
    class SetSelectionArguments : public CommandArguments {
    public:
        int getStart() const;
        int getEnd() const;
    };

    /** Arguments for ACTION_SET_TEXT. */
    class SetTextArguments : public CommandArguments {
    public:
        std::string getText() const;
    };

    /** Arguments for ACTION_SCROLL_TO_POSITION. */
    class ScrollToPositionArguments : public CommandArguments {
    public:
        int getRow() const;
        int getColumn() const;
    };

    /** Arguments for ACTION_SET_PROGRESS. */
    class SetProgressArguments : public CommandArguments {
    public:
        float getProgress() const;
    };

    /** Arguments for ACTION_MOVE_WINDOW. */
    class MoveWindowArguments : public CommandArguments {
    public:
        int getX() const;
        int getY() const;
    };
};

/** androidx AccessibilityActionCompat carries a per-action
    CommandArguments class chosen by the standard action id (the singleton
    constants are built with MoveAtGranularityArguments.class, SetProgressArguments
   .class, ...). Collapsed: derive the typed arguments from the action id —
    returns nullptr for actions that take no typed arguments. */
AccessibilityViewCommand::CommandArguments* createCommandArguments(int actionId);

}/*endof namespace cdroid*/
#endif/*__ACCESSIBILITY_VIEW_COMMAND_H__*/
