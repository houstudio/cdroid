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
#include <view/accessibility/accessibilityviewcommand.h>
#include <core/bundle.h>
#include <view/accessibility/accessibilitynodeinfo.h>
#include <widget/R.h>

namespace cdroid{

using Args = AccessibilityViewCommand::CommandArguments;

void AccessibilityViewCommand::CommandArguments::setBundle(Bundle* bundle) {
    mBundle = bundle;
}

int AccessibilityViewCommand::MoveAtGranularityArguments::getGranularity() const {
    return mBundle->getInt(AccessibilityNodeInfo::ACTION_ARGUMENT_MOVEMENT_GRANULARITY_INT, 0);
}

bool AccessibilityViewCommand::MoveAtGranularityArguments::getExtendSelection() const {
    return mBundle->getBoolean(AccessibilityNodeInfo::ACTION_ARGUMENT_EXTEND_SELECTION_BOOLEAN, false);
}

std::string AccessibilityViewCommand::MoveHtmlArguments::getHTMLElement() const {
    return mBundle->getString(AccessibilityNodeInfo::ACTION_ARGUMENT_HTML_ELEMENT_STRING);
}

int AccessibilityViewCommand::SetSelectionArguments::getStart() const {
    return mBundle->getInt(AccessibilityNodeInfo::ACTION_ARGUMENT_SELECTION_START_INT, -1);
}

int AccessibilityViewCommand::SetSelectionArguments::getEnd() const {
    return mBundle->getInt(AccessibilityNodeInfo::ACTION_ARGUMENT_SELECTION_END_INT, -1);
}

std::string AccessibilityViewCommand::SetTextArguments::getText() const {
    return mBundle->getString(AccessibilityNodeInfo::ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE);
}

int AccessibilityViewCommand::ScrollToPositionArguments::getRow() const {
    return mBundle->getInt(AccessibilityNodeInfo::ACTION_ARGUMENT_ROW_INT, -1);
}

int AccessibilityViewCommand::ScrollToPositionArguments::getColumn() const {
    return mBundle->getInt(AccessibilityNodeInfo::ACTION_ARGUMENT_COLUMN_INT, -1);
}

float AccessibilityViewCommand::SetProgressArguments::getProgress() const {
    return mBundle->getFloat(AccessibilityNodeInfo::ACTION_ARGUMENT_PROGRESS_VALUE, 0.f);
}

int AccessibilityViewCommand::MoveWindowArguments::getX() const {
    return mBundle->getInt(AccessibilityNodeInfo::ACTION_ARGUMENT_MOVE_WINDOW_X, 0);
}

int AccessibilityViewCommand::MoveWindowArguments::getY() const {
    return mBundle->getInt(AccessibilityNodeInfo::ACTION_ARGUMENT_MOVE_WINDOW_Y, 0);
}

Args* createCommandArguments(int actionId) {
    // androidx builds its standard AccessibilityActionCompat singletons with the
    // matching CommandArguments class; every other action (custom ids included)
    // carries none.
    switch (actionId) {
    case AccessibilityNodeInfo::ACTION_NEXT_AT_MOVEMENT_GRANULARITY:
    case AccessibilityNodeInfo::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY:
        return new AccessibilityViewCommand::MoveAtGranularityArguments();
    case AccessibilityNodeInfo::ACTION_NEXT_HTML_ELEMENT:
    case AccessibilityNodeInfo::ACTION_PREVIOUS_HTML_ELEMENT:
        return new AccessibilityViewCommand::MoveHtmlArguments();
    case AccessibilityNodeInfo::ACTION_SET_SELECTION:
        return new AccessibilityViewCommand::SetSelectionArguments();
    case AccessibilityNodeInfo::ACTION_SET_TEXT:
        return new AccessibilityViewCommand::SetTextArguments();
    // id-based actions: switch on the R ids the singletons are built from
    case R::id::accessibilityActionScrollToPosition:
        return new AccessibilityViewCommand::ScrollToPositionArguments();
    case R::id::accessibilityActionSetProgress:
        return new AccessibilityViewCommand::SetProgressArguments();
    case R::id::accessibilityActionMoveWindow:
        return new AccessibilityViewCommand::MoveWindowArguments();
    default:
        return nullptr;
    }
}

}/*endof namespace cdroid*/
