/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL v2.1+) — ported from Android's android.text.method.Touch.
 * NOTE: the buffer-meta part of the "selecting" check (MetaKeyKeyListener.getMetaState)
 * is not ported (no MetaKeyKeyListener) — only the event SHIFT bit is used here.
 *********************************************************************************/
#include <text/method/touch.h>
#include <widget/textview.h>
#include <text/layout.h>
#include <text/spannablestring.h>   // Spannable, make_span_filter
#include <view/motionevent.h>
#include <view/viewconfiguration.h>
#include <view/keyevent.h>
#include <algorithm>

namespace cdroid{

// getSpans returns const ParcelableSpan*; DragState is mutated during a drag, so
// cast away const on the returned (borrowed, NoCopySpan) pointer.
static Touch::DragState* asDrag(const ParcelableSpan* p) {
    return const_cast<Touch::DragState*>(dynamic_cast<const Touch::DragState*>(p));
}

// DragState is a NoCopySpan (borrowed): the Spannable neither frees nor clones it,
// so detach + delete manually at every removal site (mirrors what Android's GC
// reclaims when the span becomes unreachable). Forgetting this leaks one DragState
// per touch down/up cycle.
static void clearDragSpans(Spannable& buffer, const std::vector<const ParcelableSpan*>& spans) {
    for (auto p : spans) {
        buffer.removeSpan(p);
        delete asDrag(p);
    }
}

void Touch::scrollTo(TextView& widget, Layout& layout, int x, int y) {
    const int horizontalPadding = widget.getTotalPaddingLeft() + widget.getTotalPaddingRight();
    const int availableWidth = widget.getWidth() - horizontalPadding;

    const int top = layout.getLineForVertical(y);
    const auto a = layout.getParagraphAlignment(top);
    const bool ltr = layout.getParagraphDirection(top) > 0;

    int left, right;
    if (widget.getHorizontallyScrolling()) {
        const int verticalPadding = widget.getTotalPaddingTop() + widget.getTotalPaddingBottom();
        const int bottom = layout.getLineForVertical(y + widget.getHeight() - verticalPadding);
        left = INT_MAX;
        right = 0;
        for (int i = top; i <= bottom; i++) {
            left = (int)std::min((float)left, layout.getLineLeft(i));
            right = (int)std::max((float)right, layout.getLineRight(i));
        }
    } else {
        left = 0;
        right = availableWidth;
    }

    const int actualWidth = right - left;
    if (actualWidth < availableWidth) {
        if (a == Layout::Alignment::ALIGN_CENTER) {
            x = left - ((availableWidth - actualWidth) / 2);
        } else if ((ltr && (a == Layout::Alignment::ALIGN_OPPOSITE)) ||
                   (!ltr && (a == Layout::Alignment::ALIGN_NORMAL)) ||
                   (a == Layout::Alignment::ALIGN_RIGHT)) {
            x = left - (availableWidth - actualWidth);
        } else {
            x = left;
        }
    } else {
        x = std::min(x, right - availableWidth);
        x = std::max(x, left);
    }

    widget.scrollTo(x, y);
}

bool Touch::onTouchEvent(TextView& widget, Spannable& buffer, MotionEvent& event) {
    const int action = event.getActionMasked();
    auto dragSpans = buffer.getSpans(0, buffer.length(), make_span_filter<DragState>());

    switch (action) {
    case MotionEvent::ACTION_DOWN: {
        clearDragSpans(buffer, dragSpans);
        buffer.setSpan(new DragState(event.getX(), event.getY(),
                                     widget.getScrollX(), widget.getScrollY()),
                       0, 0, Spanned::SPAN_MARK_MARK);
        return true;
    }
    case MotionEvent::ACTION_UP: {
        // Capture mUsed before clearing: clearDragSpans deletes the span, after
        // which dragSpans[0] would dangle.
        const bool used = !dragSpans.empty() && asDrag(dragSpans[0])->mUsed;
        clearDragSpans(buffer, dragSpans);
        return used;
    }
    case MotionEvent::ACTION_MOVE: {
        if (!dragSpans.empty()) {
            DragState* d = asDrag(dragSpans[0]);
            if (!d->mFarEnough) {
                const int slop = ViewConfiguration::get(widget.getContext()).getScaledTouchSlop();
                if (std::abs(event.getX() - d->mX) >= slop ||
                    std::abs(event.getY() - d->mY) >= slop) {
                    d->mFarEnough = true;
                }
            }
            if (d->mFarEnough) {
                d->mUsed = true;
                // Android also checks MetaKeyKeyListener buffer meta here (SELECTING);
                // CDROID has no MetaKeyKeyListener — use the event SHIFT bit only.
                const bool cap = (event.getMetaState() & KeyEvent::META_SHIFT_ON) != 0;
                float dx, dy;
                if (cap) { dx = event.getX() - d->mX; dy = event.getY() - d->mY; }
                else     { dx = d->mX - event.getX(); dy = d->mY - event.getY(); }
                d->mX = event.getX();
                d->mY = event.getY();

                int nx = widget.getScrollX() + (int)dx;
                int ny = widget.getScrollY() + (int)dy;
                const int padding = widget.getTotalPaddingTop() + widget.getTotalPaddingBottom();
                Layout* layout = widget.getLayout();
                if (layout) {
                    ny = std::min(ny, layout->getHeight() - (widget.getHeight() - padding));
                    ny = std::max(ny, 0);
                    const int oldX = widget.getScrollX();
                    const int oldY = widget.getScrollY();
                    scrollTo(widget, *layout, nx, ny);
                    if (oldX != widget.getScrollX() || oldY != widget.getScrollY()) {
                        widget.cancelLongPress();
                    }
                }
                return true;
            }
        }
        break;
    }
    default: break;
    }
    return false;
}

int Touch::getInitialScrollX(TextView& /*widget*/, Spannable& buffer) {
    auto ds = buffer.getSpans(0, buffer.length(), make_span_filter<DragState>());
    return ds.empty() ? -1 : asDrag(ds[0])->mScrollX;
}

int Touch::getInitialScrollY(TextView& /*widget*/, Spannable& buffer) {
    auto ds = buffer.getSpans(0, buffer.length(), make_span_filter<DragState>());
    return ds.empty() ? -1 : asDrag(ds[0])->mScrollY;
}

}/*endof namespace*/
