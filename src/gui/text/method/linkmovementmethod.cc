/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <vector>
#include <widget/textview.h>
#include <text/selection.h>
#include <text/spannablestring.h>
#include <text/style/clickablespan.h>
//#include <text/style/textlinkspan.h>
#include <text/method/linkmovementmethod.h>

namespace cdroid{

class TextLinkSpan:public:ClickableSpan{};

bool LinkMovementMethod::handleMovementKey(TextView& widget, Spannable& buffer, int keyCode,
        int movementMetaState, KeyEvent& event) {
    switch (keyCode) {
    case KeyEvent::KEYCODE_DPAD_CENTER:
    case KeyEvent::KEYCODE_ENTER:
        if (KeyEvent::metaStateHasNoModifiers(movementMetaState)) {
            if (event.getAction() == KeyEvent::ACTION_DOWN &&
                    event.getRepeatCount() == 0 && action(CLICK, widget, buffer)) {
                return true;
            }
        }
        break;
    }
    return ScrollingMovementMethod::handleMovementKey(widget, buffer, keyCode, movementMetaState, event);
}

bool LinkMovementMethod::up(TextView& widget, Spannable& buffer) {
    if (action(UP, widget, buffer)) {
        return true;
    }

    return ScrollingMovementMethod::up(widget, buffer);
}

bool LinkMovementMethod::down(TextView& widget, Spannable& buffer) {
    if (action(DOWN, widget, buffer)) {
        return true;
    }

    return ScrollingMovementMethod::down(widget, buffer);
}

bool LinkMovementMethod::left(TextView& widget, Spannable& buffer) {
    if (action(UP, widget, buffer)) {
        return true;
    }

    return ScrollingMovementMethod::left(widget, buffer);
}

bool LinkMovementMethod::right(TextView& widget, Spannable& buffer) {
    if (action(DOWN, widget, buffer)) {
        return true;
    }

    return ScrollingMovementMethod::right(widget, buffer);
}

bool LinkMovementMethod::action(int what, TextView& widget, Spannable& buffer) {
    Layout* layout = widget.getLayout();
    if (widget.isOffsetMappingAvailable()) {
        // The text in the layout is transformed and has OffsetMapping, don't do anything.
        return false;
    }

    int padding = widget.getTotalPaddingTop() +
                  widget.getTotalPaddingBottom();
    int areaTop = widget.getScrollY();
    int areaBot = areaTop + widget.getHeight() - padding;

    int lineTop = layout->getLineForVertical(areaTop);
    int lineBot = layout->getLineForVertical(areaBot);

    int first = layout->getLineStart(lineTop);
    int last = layout->getLineEnd(lineBot);

    auto candidates = buffer.getSpans(first, last, make_span_filter<ClickableSpan>());

    int a = Selection::getSelectionStart(&buffer);
    int b = Selection::getSelectionEnd(&buffer);

    int selStart = std::min(a, b);
    int selEnd = std::max(a, b);

    if (selStart < 0) {
        if (buffer.getSpanStart(&FROM_BELOW) >= 0) {
            selStart = selEnd = buffer.length();
        }
    }

    if (selStart > last)
        selStart = selEnd = INT_MAX;
    if (selEnd < first)
        selStart = selEnd = -1;

    switch (what) {
        case CLICK:
            if (selStart == selEnd) {
                return false;
            }

            auto links = buffer.getSpans(selStart, selEnd, make_span_filter<ClickableSpan>());

            if (links.size() != 1) {
                return false;
            }

            const ClickableSpan* link = dynamic_cast<const ClickableSpan*>(links[0]);
            if (dynamic_cast<const TextLinkSpan*>(link)) {
                dynamic_cast<const TextLinkSpan*>(link)->onClick(widget, TextLinkSpan::INVOCATION_METHOD_KEYBOARD);
            } else {
                link->onClick(widget);
            }
            break;

        case UP:
            int bestStart, bestEnd;

            bestStart = -1;
            bestEnd = -1;

            for (int i = 0; i < candidates.size(); i++) {
                int end = buffer.getSpanEnd(candidates[i]);

                if (end < selEnd || selStart == selEnd) {
                    if (end > bestEnd) {
                        bestStart = buffer.getSpanStart(candidates[i]);
                        bestEnd = end;
                    }
                }
            }

            if (bestStart >= 0) {
                Selection::setSelection(&buffer, bestEnd, bestStart);
                return true;
            }

            break;

        case DOWN:
            bestStart = INT_MAX;
            bestEnd = INT_MAX;

            for (int i = 0; i < candidates.size(); i++) {
                int start = buffer.getSpanStart(candidates[i]);

                if (start > selStart || selStart == selEnd) {
                    if (start < bestStart) {
                        bestStart = start;
                        bestEnd = buffer.getSpanEnd(candidates[i]);
                    }
                }
            }

            if (bestEnd < INT_MAX) {
                Selection::setSelection(&buffer, bestStart, bestEnd);
                return true;
            }

            break;
    }

    return false;
}

bool LinkMovementMethod::onTouchEvent(TextView& widget, Spannable& buffer,MotionEvent& event) {
    const int action = event.getAction();

    if (action == MotionEvent::ACTION_UP || action == MotionEvent::ACTION_DOWN) {
        int x = (int) event.getX();
        int y = (int) event.getY();

        x -= widget.getTotalPaddingLeft();
        y -= widget.getTotalPaddingTop();

        x += widget.getScrollX();
        y += widget.getScrollY();

        Layout* layout = widget.getLayout();
        std::vector<ParcelableSpan*> links;
        if (y < 0 || y > layout->getHeight()) {
            //links = null;
        } else {
            int line = layout->getLineForVertical(y);
            if (x < layout->getLineLeft(line) || x > layout->getLineRight(line)) {
                //links = null;
            } else {
                int off = layout->getOffsetForHorizontal(line, x);
                links = buffer.getSpans(off, off, make_span_filter<ClickableSpan>());
            }
        }

        if (!links.empty()) {
            ClickableSpan* link = dynamic_cast<ClickableSpan*>(links[0]);
            if (action == MotionEvent::ACTION_UP) {
                if (dynamic_cast<TextLinkSpan*>(link)) {
                    dynamic_cast<TextLinkSpan*>(link)->onClick(
                            widget, TextLinkSpan::NVOCATION_METHOD_TOUCH);
                } else {
                    link->onClick(widget);
                }
            } else if (action == MotionEvent::ACTION_DOWN) {
                if (widget.getContext().getApplicationInfo().targetSdkVersion
                        >= Build.VERSION_CODES.P) {
                    // Selection change will reposition the toolbar. Hide it for a few ms for a
                    // smoother transition.
                    widget.hideFloatingToolbar(HIDE_FLOATING_TOOLBAR_DELAY_MS);
                }
                Selection::setSelection(&buffer, buffer.getSpanStart(link), buffer.getSpanEnd(link));
            }
            return true;
        } else {
            Selection::removeSelection(&buffer);
        }
    }

    return ScrollingMovementMethod::onTouchEvent(widget, buffer, event);
}

void LinkMovementMethod::initialize(TextView& widget, Spannable& text) {
    Selection::removeSelection(&text);
    text.removeSpan(&FROM_BELOW);
}

void LinkMovementMethod::onTakeFocus(TextView& view, Spannable& text, int dir) {
    Selection::removeSelection(&text);

    if ((dir & View::FOCUS_BACKWARD) != 0) {
        text.setSpan(&FROM_BELOW, 0, 0, Spannable::SPAN_POINT_POINT);
    } else {
        text.removeSpan(&FROM_BELOW);
    }
}

MovementMethod* LinkMovementMethod::getInstance() {
    if (sInstance == nullptr)
        sInstance = new LinkMovementMethod();

    return sInstance;
}

//private static LinkMovementMethod sInstance;
NoCopySpan LinkMovementMethod::FROM_BELOW;
}
