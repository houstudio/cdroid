#include "lrcview.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include <core/systemclock.h>
#include <porting/cdlog.h>

using namespace cdroid;

namespace remusic {

// ---- LrcRow.createRows: "[01:23.45][01:25.00]text" may carry several tags ----
std::vector<LrcRow> LrcRow::createRows(const std::string& line) {
    std::vector<LrcRow> rows;
    size_t pos = 0;
    std::vector<int> times;
    // leading tag run
    while (pos + 1 < line.size() && line[pos] == '[') {
        const size_t close = line.find(']', pos);
        if (close == std::string::npos) break;
        const std::string tag = line.substr(pos + 1, close - pos - 1);
        // mm:ss.xx / mm:ss.xxx / mm:ss
        int mm = -1, ss = -1, ms = 0;
        float frac = 0.f;
        if (sscanf(tag.c_str(), "%d:%d%f", &mm, &ss, &frac) >= 2 && mm >= 0 && ss >= 0) {
            ms = (int)(frac * 1000.f + 0.5f);
            times.push_back(mm * 60000 + ss * 1000 + ms);
            pos = close + 1;
        } else {
            break;   // metadata tag like [ti:...] — skip the line
        }
    }
    if (times.empty()) return rows;
    std::string content = line.substr(pos);
    if (!content.empty() && content.back() == '\r') content.pop_back();
    for (int t : times) {
        LrcRow r;
        r.time = t;
        r.content = content;
        rows.push_back(r);
    }
    return rows;
}

std::vector<LrcRow> parseLrc(const std::string& text) {
    std::vector<LrcRow> rows;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        auto r = LrcRow::createRows(line);
        rows.insert(rows.end(), r.begin(), r.end());
    }
    std::sort(rows.begin(), rows.end(), [](const LrcRow& a, const LrcRow& b) {
        return a.time < b.time;
    });
    for (size_t i = 0; i + 1 < rows.size(); i++)
        rows[i].totalTime = rows[i + 1].time - rows[i].time;
    return rows;
}

// ---- LrcView ----
LrcView::LrcView(Context* ctx, const AttributeSet* attrs) : View(ctx, attrs) {
    setLayoutParams(new ViewGroup::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
}

void LrcView::setLrcRows(const std::vector<LrcRow>& rows) {
    mRows = rows;
    mCurrentRow = -1;
    mScrollY = 0;
    mTargetScrollY = 0;
    invalidate();
}

int LrcView::rowForTime(int timeMs) const {
    // last row whose time <= timeMs
    int lo = 0, hi = (int)mRows.size() - 1, ans = -1;
    while (lo <= hi) {
        const int mid = (lo + hi) / 2;
        if (mRows[mid].time <= timeMs) { ans = mid; lo = mid + 1; }
        else hi = mid - 1;
    }
    return ans;
}

int LrcView::centerYForRow(int row) const {
    return getHeight() / 2 + row * mLineHeight - (int)mScrollY;
}

void LrcView::onSizeChanged(int w, int h, int oldw, int oldh) {
    View::onSizeChanged(w, h, oldw, oldh);
    mLineHeight = std::max(48, (int)(mTextSize * 1.9f));
}

void LrcView::seekTo(int timeMs, bool /*fromUser*/) {
    const int row = rowForTime(timeMs);
    if (row != mCurrentRow) {
        mCurrentRow = row;
        mTargetScrollY = std::max(0, row) * mLineHeight;
        invalidate();
    }
}

void LrcView::smoothScrollTo(int /*target*/) {
    // simple exponential approach driven from seekTo's invalidate
}

void LrcView::onDraw(Canvas& canvas) {
    View::onDraw(canvas);
    if (mRows.empty()) {
        // the original's empty-state hint
        return;
    }
    // ease toward the target centering offset
    mScrollY += (mTargetScrollY - mScrollY) * 0.25f;
    const int w = getWidth();
    for (int i = 0; i < (int)mRows.size(); i++) {
        const int cy = centerYForRow(i);
        if (cy < -mLineHeight || cy > getHeight() + mLineHeight) continue;
        if (i == mCurrentRow) {
            canvas.set_color(mCurColor);
            canvas.set_font_size(mTextSize + 2);
        } else {
            canvas.set_color(mNormColor);
            canvas.set_font_size(mTextSize);
        }
        Rect line;
        line.set(0, cy - mLineHeight / 2, w, mLineHeight);
        canvas.draw_text(line, mRows[i].content,
                Gravity::CENTER_HORIZONTAL | Gravity::CENTER_VERTICAL);
    }
    if (mTargetScrollY != mScrollY) postInvalidate();
}

bool LrcView::onTouchEvent(MotionEvent& event) {
    switch (event.getActionMasked()) {
    case MotionEvent::ACTION_DOWN:
        mDragging = true;
        mDownY = event.getY();
        return true;
    case MotionEvent::ACTION_MOVE: {
        const float dy = event.getY() - mDownY;
        mDownY = event.getY();
        mTargetScrollY = std::max(0.f, mTargetScrollY - dy);
        mDragRow = (int)(mTargetScrollY / mLineHeight + 0.5f);
        invalidate();
        return true;
    }
    case MotionEvent::ACTION_UP:
    case MotionEvent::ACTION_CANCEL:
        if (mDragging && mDragRow >= 0 && mDragRow < (int)mRows.size() && mOnSeek)
            mOnSeek(mRows[mDragRow].time);
        mDragging = false;
        return true;
    }
    return View::onTouchEvent(event);
}

} // namespace remusic

// XML tag com.wm.remusic.lrc.LrcView — the inflater registry keys on the
// short class name.
static const int sLrcViewRegistered = (LayoutInflater::registerInflater("LrcView", 0,
        [](Context* ctx, const AttributeSet& attr) -> View* {
    return new remusic::LrcView(ctx, &attr);
}), 0);
