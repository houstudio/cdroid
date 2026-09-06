// Port of the com.wm.remusic.lrc package — LrcRow / DefaultLrcParser /
// LrcView (the karaoke lyrics view). Self-contained: parses "[mm:ss.xx] text"
// lines, highlights the row for the current playback position, centers it,
// and lets a drag scrub the song (onSeekListener), like the original's
// scroll-to-seek.
#ifndef __REMUSIC_LRCVIEW_H__
#define __REMUSIC_LRCVIEW_H__

#include <string>
#include <vector>

#include <cdroid.h>
#include <widget/textview.h>

namespace remusic {

class LrcRow {
public:
    std::string timeStr;
    int time = 0;          // ms
    std::string content;
    int totalTime = 5000;  // ms this row stays

    static std::vector<LrcRow> createRows(const std::string& line);
};

std::vector<LrcRow> parseLrc(const std::string& text);

class LrcView : public cdroid::View {
public:
    LrcView(cdroid::Context* ctx, const cdroid::AttributeSet* attrs = nullptr);

    void setLrcRows(const std::vector<LrcRow>& rows);
    void seekTo(int timeMs, bool fromUser);
    void setOnSeekListener(std::function<void(int)> fn) { mOnSeek = std::move(fn); }
    /** Day theme wants dark lyrics on the light page; night keeps the
     *  light-on-dark pair. */
    void setColors(uint32_t current, uint32_t normal) { mCurColor = current; mNormColor = normal; invalidate(); }

protected:
    void onDraw(cdroid::Canvas& canvas) override;
    bool onTouchEvent(cdroid::MotionEvent& event) override;
    void onSizeChanged(int w, int h, int oldw, int oldh) override;

private:
    int rowForTime(int timeMs) const;
    int centerYForRow(int row) const;
    void smoothScrollTo(int target);

    std::vector<LrcRow> mRows;
    int mCurrentRow = -1;
    uint32_t mCurColor = 0xFF3333FF;
    uint32_t mNormColor = 0xFFAAAAAA;
    float mScrollY = 0;         // px, row-area offset (grows downward)
    float mTargetScrollY = 0;
    int mLineHeight = 64;
    int mTextSize = 34;
    bool mDragging = false;
    float mDownY = 0;
    int mDragRow = -1;
    std::function<void(int)> mOnSeek;
};

} // namespace remusic
#endif
