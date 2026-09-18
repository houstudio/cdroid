#include <widgetEx/appbar/collapsingtexthelper.h>
#include <core/canvas.h>
#include <view/view.h>
#include <algorithm>
#include <cmath>

namespace cdroid {

CollapsingTextHelper::CollapsingTextHelper(View* view) : mView(view) {
    mTextPaint.setTextSize(mExpandedTextSize);
}

void CollapsingTextHelper::setText(const std::u16string& text) {
    if (mText == text) return;
    mText = text;
    recalculate();
}

const std::u16string& CollapsingTextHelper::getText() const { return mText; }

void CollapsingTextHelper::setExpandedBounds(int left, int top, int right, int bottom) {
    mExpandedBounds.set(left, top, right, bottom);
    recalculate();
}

void CollapsingTextHelper::setCollapsedBounds(int left, int top, int right, int bottom) {
    mCollapsedBounds.set(left, top, right, bottom);
    recalculate();
}

void CollapsingTextHelper::setExpandedTextSize(float size) {
    if (size <= 0.0f || mExpandedTextSize == size) return;
    mExpandedTextSize = size;
    recalculate();
}

void CollapsingTextHelper::setCollapsedTextSize(float size) {
    if (size <= 0.0f || mCollapsedTextSize == size) return;
    mCollapsedTextSize = size;
    recalculate();
}

void CollapsingTextHelper::setExpandedTextColor(int color) {
    mExpandedTextColor = color;
    recalculate();
}

void CollapsingTextHelper::setCollapsedTextColor(int color) {
    mCollapsedTextColor = color;
    recalculate();
}

void CollapsingTextHelper::setExpandedFraction(float fraction) {
    fraction = std::max(0.0f, std::min(1.0f, fraction));
    if (mExpandedFraction == fraction) return;
    mExpandedFraction = fraction;
    recalculate();
}

float CollapsingTextHelper::getExpandedFraction() const { return mExpandedFraction; }

void CollapsingTextHelper::setDrawEnabled(bool enabled) { mDrawEnabled = enabled; }

void CollapsingTextHelper::recalculate() {
    const float fraction = mExpandedFraction;
    mCurrentTextSize = mExpandedTextSize + (mCollapsedTextSize - mExpandedTextSize) * fraction;
    mCurrentX = mExpandedBounds.left + (mCollapsedBounds.left - mExpandedBounds.left) * fraction;
    const float expandedBaseline = mExpandedBounds.bottom() - mTextPaint.descent();
    const float collapsedBaseline = mCollapsedBounds.bottom() - mTextPaint.descent();
    mCurrentY = expandedBaseline + (collapsedBaseline - expandedBaseline) * fraction;
    mTextPaint.setTextSize(mCurrentTextSize);
    const auto blend = [fraction](int from, int to, int shift) {
        const int a = (from >> shift) & 0xFF;
        const int b = (to >> shift) & 0xFF;
        return static_cast<int>(a + (b - a) * fraction);
    };
    mTextPaint.setColor((blend(mExpandedTextColor, mCollapsedTextColor, 24) << 24)
            | (blend(mExpandedTextColor, mCollapsedTextColor, 16) << 16)
            | (blend(mExpandedTextColor, mCollapsedTextColor, 8) << 8)
            | blend(mExpandedTextColor, mCollapsedTextColor, 0));
}

void CollapsingTextHelper::draw(Canvas& canvas) {
    if (!mDrawEnabled || mText.empty()) return;
    canvas.save();
        canvas.rectangle(mCollapsedBounds.left, mCollapsedBounds.top,
            mCollapsedBounds.width, mCollapsedBounds.height);
    canvas.clip();
    mTextPaint.drawTextRun(canvas, mText.data(), 0, static_cast<int>(mText.size()),
            0, static_cast<int>(mText.size()), mCurrentX, mCurrentY, false);
    canvas.restore();
}

}
