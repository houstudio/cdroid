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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  0211-130 1 USA
*/
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <content/typedarray.h>
#include <drawable/colorstatelist.h>
#include <text/String.h>
#include <text/staticlayout.h>
#include <view/motionevent.h>
#include <view/accessibility/accessibilityevent.h>
#include <view/accessibility/accessibilitynodeinfo.h>
#include <widget/internal_R.h>
#include <widgetEx/widgetex_styleable.h>
#include <widgetEx/wear/curvedtextview.h>

namespace cdroid{
using namespace cdroid::internal;

// androidx.wear.widget.CurvedTextView, line-aligned with
// wear/src/main/java/androidx/wear/widget/CurvedTextView.java.
constexpr float CurvedTextView::MIN_SWEEP_DEGREE;
constexpr float CurvedTextView::MAX_SWEEP_DEGREE;
constexpr int CurvedTextView::FONT_WEIGHT_MAX;

DECLARE_WIDGET2(CurvedTextView, "androidx.wear.widget.CurvedTextView");

CurvedTextView::CurvedTextView(Context* context):CurvedTextView(context,nullptr){}

CurvedTextView::CurvedTextView(Context* context,const AttributeSet* attrs)
    :CurvedTextView(context,attrs,(int)internal::R::attr::textViewStyle){}

CurvedTextView::CurvedTextView(Context* context,const AttributeSet* attrs,int defStyle)
    :CurvedTextView(context,attrs,defStyle,0){}

// java:120-191 — the attribute-reading constructor.
CurvedTextView::CurvedTextView(Context* context,const AttributeSet* attrs,int defStyle,int defStyleRes)
    :View(context,attrs,defStyle,defStyleRes){
    mPaint.setAntiAlias(true);

    TextAppearanceAttributes attributes;
    attributes.mTextColor = ColorStateList::valueOf(DEFAULT_TEXT_COLOR);

    // a.recycle() calls omitted — the CDROID TypedArray is a unique_ptr.
    auto a = context->getTheme().obtainStyledAttributes(attrs,
            internal::R::styleable::WearTextViewAppearance, defStyle, defStyleRes);

    std::unique_ptr<TypedArray> appearance;
    const uint32_t ap = a->getResourceId(
            internal::R::styleable::WearTextViewAppearance_textAppearance, (uint32_t)-1);

    if (ap != (uint32_t)-1) {
        appearance = context->obtainStyledAttributes((int)ap,
                internal::R::styleable::WearTextAppearance);
    }
    if (appearance != nullptr) {
        readTextAppearance(*appearance, attributes, true);
    }

    a = context->obtainStyledAttributes(attrs, internal::R::styleable::CurvedTextView,
            defStyle, defStyleRes);
    // overrride the value in the appearance with explicitly specified attribute values
    readTextAppearance(*a, attributes, false);

    // read the other supported TextView attributes
    if (a->hasValue(internal::R::styleable::CurvedTextView_text)) {
        mText = a->getString(internal::R::styleable::CurvedTextView_text);
    }

    int textEllipsize = a->getInt(internal::R::styleable::CurvedTextView_ellipsize, 0);
    switch (textEllipsize) {
        case 1:
            mEllipsize = TextUtils::TruncateAt::START;
            break;
        case 2:
            mEllipsize = TextUtils::TruncateAt::MIDDLE;
            break;
        case 3:
            mEllipsize = TextUtils::TruncateAt::END;
            break;
        default:
            mEllipsize = TextUtils::TruncateAt::NONE;  // null
    }

    // read the custom CurvedTextView attributes
    mMaxSweepDegrees = a->getFloat(internal::R::styleable::CurvedTextView_maxSweepDegrees,
            MAX_SWEEP_DEGREE);
    mMaxSweepDegrees = std::min(mMaxSweepDegrees, MAX_SWEEP_DEGREE);
    mMinSweepDegrees = a->getFloat(internal::R::styleable::CurvedTextView_minSweepDegrees,
            MIN_SWEEP_DEGREE);
    if (mMinSweepDegrees > mMaxSweepDegrees) {
        throw std::invalid_argument("MinSweepDegrees cannot be bigger than MaxSweepDegrees");
    }
    mAnchorType = a->getInt(internal::R::styleable::CurvedTextView_anchorPosition, UNSET_ANCHOR_TYPE);
    mAnchorAngleDegrees = a->getFloat(
            internal::R::styleable::CurvedTextView_anchorAngleDegrees, UNSET_ANCHOR_DEGREE);
    mAnchorAngleDegrees = std::fmod(mAnchorAngleDegrees, 360.f);
    mClockwise = a->getBoolean(internal::R::styleable::CurvedTextView_clockwise, DEFAULT_CLOCKWISE);

    applyTextAppearance(attributes);

    mPaint.setTextSize(mTextSize);
}

// java:193-204
float CurvedTextView::getSweepAngleDegrees(){
    return mBackgroundSweepDegrees;
}

// java:199-204 — We need to be careful because this is also set by onMeasure below.
void CurvedTextView::setSweepAngleDegrees(float angleDegrees){
    mBackgroundSweepDegrees = angleDegrees;
}

// java:206-210 — Paint::getFontMetrics() is not ported; ascent()/descent() are
// the same FontMetrics fields.
int CurvedTextView::getThickness(){
    return (int) std::round(mPaint.descent() - mPaint.ascent());
}

// java:212-228
void CurvedTextView::checkInvalidAttributeAsChild(){
    if (mAnchorType != UNSET_ANCHOR_TYPE) {
        throw std::invalid_argument(
                "CurvedTextView shall not set anchorType value when added intoArcLayout");
    }

    if (mAnchorAngleDegrees != UNSET_ANCHOR_DEGREE) {
        throw std::invalid_argument(
                "CurvedTextView shall not set anchorAngleDegrees value when added into ArcLayout");
    }
}

// java:230-249 — See ArcLayout.Widget#isPointInsideClickArea(float, float)
bool CurvedTextView::isPointInsideClickArea(float x,float y){
    float radius2 =
            std::min(getWidth(), getHeight()) / 2.f
                    - (mClockwise ? getPaddingTop() : getPaddingBottom());
    float radius1 = radius2 - mPaint.descent() + mPaint.ascent();

    float dx = x - getWidth() / 2;
    float dy = y - getHeight() / 2;

    float r2 = dx * dx + dy * dy;
    if (r2 < radius1 * radius1 || r2 > radius2 * radius2) {
        return false;
    }

    // Since we are symmetrical on the Y-axis, we can constrain the angle to the x>=0 quadrants.
    float angle = (float) (std::atan2(std::abs(dx), -dy) * 180.0 / M_PI);
    return angle < mBackgroundSweepDegrees / 2;
}

// java:251-255
void CurvedTextView::onSizeChanged(int w,int h,int oldw,int oldh){
    View::onSizeChanged(w, h, oldw, oldh);
    doUpdate();
}

// java:257-267 — CDROID has a single (current) API level, and Configuration
// has no fontWeightAdjustment, so the API 31 re-resolution branch reduces to
// re-applying the original typeface.
void CurvedTextView::onConfigurationChanged(Configuration& newConfig){
    View::onConfigurationChanged(newConfig);
    if (mFontWeight >= 0) {
        resolveStyleAndSetTypeface(mOriginalTypeface, mTextStyle, mFontWeight);
    } else {
        setTypeface(mOriginalTypeface, mTextStyle);
    }
}

// java:269-284
void CurvedTextView::onMeasure(int widthMeasureSpec,int heightMeasureSpec){
    View::onMeasure(widthMeasureSpec, heightMeasureSpec);

    const std::u16string u16text = TextUtils::utf8_utf16(mText);
    mPaint.getTextBounds(u16text, 0, (int) u16text.length(), mBounds);

    // Note that ascent is negative.
    mPathRadius =
            std::min(getMeasuredWidth(), getMeasuredHeight()) / 2.f
                    + (mClockwise
                            ? mPaint.ascent() - getPaddingTop()
                            : -mPaint.descent() - getPaddingBottom());
    mTextSweepDegrees =
            std::min(getWidthSelf() / mPathRadius / (float) M_PI * 180.f, MAX_SWEEP_DEGREE);
    mBackgroundSweepDegrees = std::max(std::min(mMaxSweepDegrees, mTextSweepDegrees),
            mMinSweepDegrees);
}

// java:286-288
float CurvedTextView::getWidthSelf(){
    return (float) mBounds.width + getPaddingLeft() + getPaddingRight();
}

// java:290-316 — the StaticLayout ellipsize probe; the source String and the
// layout are freed here (Java relies on GC).
std::string CurvedTextView::ellipsize(int ellipsizedWidth){
    const std::u16string u16text = TextUtils::utf8_utf16(mText);
    String* source = new String(u16text);
    StaticLayout::Builder* layoutBuilder =
            StaticLayout::Builder::obtain(source, 0, (int) source->length(), &mPaint,
                    ellipsizedWidth);
    layoutBuilder->setEllipsize(mEllipsize);
    layoutBuilder->setMaxLines(1);
    StaticLayout* layout = layoutBuilder->build();   // build() recycles the builder

    // Cut text that it's too big even if no ellipsize mode is provided.
    if (mEllipsize == TextUtils::TruncateAt::NONE) {  // null
        const std::string result = TextUtils::utf16_utf8(u16text.substr(0, layout->getLineEnd(0)));
        delete layout;
        delete source;
        return result;
    }

    int ellipsisCount = layout->getEllipsisCount(0);
    if (ellipsisCount == 0) {
        delete layout;
        delete source;
        return mText;
    }

    int ellipsisStart = layout->getEllipsisStart(0);
    std::u16string textToDrawArray = u16text;
    textToDrawArray[ellipsisStart] = u'\u2026'; // ellipsis "..."
    for (int i = ellipsisStart + 1; i < ellipsisStart + ellipsisCount; i++) {
        if (i >= 0 && i < (int) u16text.length()) {
            textToDrawArray[i] = u'\uFEFF'; // 0-width space
        }
    }
    delete layout;
    delete source;
    return TextUtils::utf16_utf8(textToDrawArray);
}

// java:318-447
void CurvedTextView::updatePathsIfNeeded(bool withBackground){
    // The dirty flag is not set when properties we inherit from View are modified
    if (!mDirty && getTextAlignment() == mLastUsedTextAlignment) {
        return;
    }

    mDirty = false;
    mLastUsedTextAlignment = getTextAlignment();

    if (mTextSweepDegrees <= mMaxSweepDegrees) {
        mTextToDraw = mText;
    } else {
        mTextToDraw = ellipsize(
                (int) (mMaxSweepDegrees / 180.f * M_PI * mPathRadius)
                        - getPaddingLeft()
                        - getPaddingRight());
        mTextSweepDegrees = mMaxSweepDegrees;
    }

    float clockwiseFactor = mClockwise ? 1.f : -1.f;

    float alignmentFactor = 0.5f;
    switch (getTextAlignment()) {
        case TEXT_ALIGNMENT_TEXT_START:
        case TEXT_ALIGNMENT_VIEW_START:
            alignmentFactor = 0.f;
            break;
        case TEXT_ALIGNMENT_TEXT_END:
        case TEXT_ALIGNMENT_VIEW_END:
            alignmentFactor = 1.f;
            break;
        default:
            alignmentFactor = 0.5f; // TEXT_ALIGNMENT_CENTER
    }

    float anchorTypeFactor;
    switch (mAnchorType) {
        case ArcLayout::ANCHOR_START:
            anchorTypeFactor = 0.5f;
            break;
        case ArcLayout::ANCHOR_END:
            anchorTypeFactor = -0.5f;
            break;
        case ArcLayout::ANCHOR_CENTER: // Center is the default.
        default:
            anchorTypeFactor = 0.f;
    }

    mLocalRotateAngle =
            (mAnchorAngleDegrees == UNSET_ANCHOR_DEGREE ? 0.f : mAnchorAngleDegrees)
                    + clockwiseFactor * anchorTypeFactor * mBackgroundSweepDegrees;

    // Always draw the curved text on top center, then rotate the canvas to the right position
    float backgroundStartAngle =
            -clockwiseFactor * 0.5f * mBackgroundSweepDegrees + ANCHOR_DEGREE_OFFSET;

    float textStartAngle =
            backgroundStartAngle
                    + clockwiseFactor
                            * (float)
                                    (alignmentFactor
                                                    * (mBackgroundSweepDegrees
                                                            - mTextSweepDegrees)
                                            + getPaddingLeft() / mPathRadius / M_PI * 180);

    float centerX = getWidth() / 2.f;
    float centerY = getHeight() / 2.f;
    mPath.reset();
    // Path.addArc(oval l,t,r,b, ...) == arcTo(box l,t,w,h, forceMoveTo=true).
    mPath.arcTo(centerX - mPathRadius, centerY - mPathRadius,
            2.f * mPathRadius, 2.f * mPathRadius,
            textStartAngle, clockwiseFactor * mTextSweepDegrees, true);

    if (withBackground) {
        mBgPath.reset();
        // NOTE: Ensure that if the code to compute these radius* change, containsPoint() is
        // also updated.
        float radius1 = mPathRadius - clockwiseFactor * mPaint.descent();
        float radius2 = mPathRadius - clockwiseFactor * mPaint.ascent();
        mBgPath.arcTo(centerX - radius2, centerY - radius2, 2.f * radius2, 2.f * radius2,
                backgroundStartAngle, clockwiseFactor * mBackgroundSweepDegrees, false);
        mBgPath.arcTo(centerX - radius1, centerY - radius1, 2.f * radius1, 2.f * radius1,
                backgroundStartAngle + clockwiseFactor * mBackgroundSweepDegrees,
                -clockwiseFactor * mBackgroundSweepDegrees, false);
        mBgPath.close();

        float angle = backgroundStartAngle;
        float x0 = (float) (centerX + radius2 * std::cos(angle * M_PI / 180));
        float x1 = (float) (centerX + radius1 * std::cos(angle * M_PI / 180));
        float y0 = (float) (centerY + radius2 * std::sin(angle * M_PI / 180));
        float y1 = (float) (centerY + radius1 * std::sin(angle * M_PI / 180));
        angle = backgroundStartAngle + clockwiseFactor * mBackgroundSweepDegrees;
        float x2 = (float) (centerX + radius2 * std::cos(angle * M_PI / 180));
        float x3 = (float) (centerX + radius1 * std::cos(angle * M_PI / 180));
        // Background axis-aligned bounding box calculation. Note that, we always center the
        // text on the top-center of the view.
        // top: always will be centerY - outerRadius
        // bottom: the max y of end points of the outer and inner arc contains the text
        // left: if over -90 degrees, centerX - outerRadius, otherwise the min x of start,
        // end points of the outer and inner arc contains the text
        // right: if over 90 degrees, centerX + outerRadius, otherwise the max x of start,
        // end points of the outer and inner arc contains the text
        float outerRadius = std::max(radius1, radius2);
        // CDROID Rect is (left, top, width, height); the upstream bottom/right
        // assignments translate to height/width.
        mBgBounds.top = (int) (centerY - outerRadius);
        mBgBounds.height = (int) std::max(y0, y1) - mBgBounds.top;
        mBgBounds.left =
                mBackgroundSweepDegrees >= 180.0f
                        ? (int) (centerX - outerRadius)
                        : (int) std::min(x0, std::min(x1, std::min(x2, x3)));
        mBgBounds.width =
                mBackgroundSweepDegrees >= 180.0f
                        ? (int) (centerX + outerRadius) - mBgBounds.left
                        : (int) std::max(x0, std::max(x1, std::max(x2, x3))) - mBgBounds.left;
    }
}

// java:449-483 — event.getAction() == CDROID MotionEvent::getActionMasked().
bool CurvedTextView::onTouchEvent(MotionEvent& event){
    if (!mHandlingTouch && event.getActionMasked() != MotionEvent::ACTION_DOWN) {
        return false;
    }

    float x0 = event.getX() - getWidth() / 2;
    float y0 = event.getY() - getHeight() / 2;

    double rotAngle = -mLocalRotateAngle * M_PI / 180.0;

    float tempX = (float) ((x0 * std::cos(rotAngle) - y0 * std::sin(rotAngle)) + getWidth() / 2);
    y0 = (float) ((x0 * std::sin(rotAngle) + y0 * std::cos(rotAngle)) + getHeight() / 2);
    x0 = tempX;

    // Should we start handling the touch events?
    if (!mHandlingTouch && isPointInsideClickArea(x0, y0)) {
        mHandlingTouch = true;
    }

    // We just started or are in the middle of handling events, forward to View to handle.
    if (mHandlingTouch) {
        if (event.getActionMasked() == MotionEvent::ACTION_UP
                || event.getActionMasked() == MotionEvent::ACTION_CANCEL) {
            // We should end handling events now
            mHandlingTouch = false;
        }
        event.offsetLocation(x0 - event.getX(), y0 - event.getY());
        return View::onTouchEvent(event);
    }

    return false;
}

// java:485-500
void CurvedTextView::draw(Canvas& canvas){
    canvas.save();

    bool withBackground = getBackground() != nullptr;
    updatePathsIfNeeded(withBackground);
    // canvas.rotate(degrees, px, py) — pivot rotate via translate/rotate/translate.
    const float pivotX = getWidth() / 2.f;
    const float pivotY = getHeight() / 2.f;
    canvas.translate(pivotX, pivotY);
    canvas.rotate_degrees(mLocalRotateAngle);
    canvas.translate(-pivotX, -pivotY);

    if (withBackground) {
        mBgPath.append_to_context(&canvas);
        canvas.clip();   // clipPath(mBgPath)
        getBackground()->setBounds(mBgBounds);
    }
    View::draw(canvas);

    canvas.restore();
}

// java:502-507 — Canvas::drawTextOnPath is Paint::drawTextOnPath in CDROID.
void CurvedTextView::onDraw(Canvas& canvas){
    mPaint.setColor(mTextColor);
    mPaint.setStyle(Paint::Style::FILL);
    mPaint.drawTextOnPath(canvas, mTextToDraw, mPath, 0.f, 0.f);
}

// java:509-541 — @Nullable String familyName == empty std::string.
void CurvedTextView::setTypefaceFromAttrs(const std::string& familyName,int typefaceIndex,
        int style,int weight){
    // typeface is ignored when font family is set
    if (mTypeface == nullptr && !familyName.empty()) {
        // Lookup normal Typeface from system font map.
        Typeface* normalTypeface = Typeface::create(familyName, Typeface::NORMAL);
        resolveStyleAndSetTypeface(normalTypeface, style, weight);
    } else if (mTypeface != nullptr) {
        resolveStyleAndSetTypeface(mTypeface, style, weight);
    } else { // both typeface and familyName is null.
        switch (typefaceIndex) {
            case 1:
                resolveStyleAndSetTypeface(Typeface::SANS_SERIF, style, weight);
                break;
            case 2:
                resolveStyleAndSetTypeface(Typeface::SERIF, style, weight);
                break;
            case 3:
                resolveStyleAndSetTypeface(Typeface::MONOSPACE, style, weight);
                break;
            default:
                resolveStyleAndSetTypeface(nullptr, style, weight);
        }
    }
}

// java:543-553 — the SDK_INT >= 28 branch is always taken at CDROID's API level.
void CurvedTextView::resolveStyleAndSetTypeface(Typeface* tf,int style,int weight){
    mOriginalTypeface = tf;
    mTextStyle = style;
    mFontWeight = weight;
    if (weight >= 0) {
        mTypeface = getAdjustedTypeface(tf, style, weight);
        mPaint.setTypeface(mTypeface);
    } else {
        setTypeface(tf, style);
    }
}

// java:555-592 — the API 31 Configuration.fontWeightAdjustment branch is
// omitted: CDROID's Configuration has no fontWeightAdjustment (the adjustment
// is always 0 upstream, making that branch a no-op). The Api28Impl.createTypeface
// shim is inlined — C++ needs no per-API-level nested classes.
Typeface* CurvedTextView::getAdjustedTypeface(Typeface* tf,int style,int weight){
    if (weight >= 0) {
        int clampedWeight = std::min(FONT_WEIGHT_MAX, weight);
        bool italic = (style & Typeface::ITALIC) != 0;
        return Typeface::create(tf, clampedWeight, italic);
    }
    if (style > 0) {
        if (tf == nullptr) {
            return Typeface::defaultFromStyle(style);
        } else {
            return Typeface::create(tf, style);
        }
    }
    return tf;
}

// java:594-630 — sets the Typeface taking into account the given attributes;
// hasAdjustment stays false (no Configuration.fontWeightAdjustment in CDROID).
void CurvedTextView::setTypeface(Typeface* tf,int style){
    mOriginalTypeface = tf;
    mTextStyle = style;
    mFontWeight = -1;

    mTypeface = getAdjustedTypeface(tf, style, -1);
    mPaint.setTypeface(mTypeface);

    if (style > 0) {
        int typefaceStyle = mTypeface != nullptr ? mTypeface->getStyle() : 0;
        int need = style & ~typefaceStyle;
        mPaint.setFakeBoldText((need & Typeface::BOLD) != 0);
        mPaint.setTextSkewX(((need & Typeface::ITALIC) != 0) ? ITALIC_SKEW_X : 0.f);
    } else {
        mPaint.setFakeBoldText(false);
        mPaint.setTextSkewX(0.f);
    }
    doUpdate();
}

// java:648-672 — sets the textColor, size, style, font etc from the specified
// TextAppearanceAttributes. Paint::setFontVariationSettings (upstream
// Api26Impl.paintSetFontVariationSettings) is not ported — CDROID Paint has no
// variation-settings support; the value is kept for the getter.
void CurvedTextView::applyTextAppearance(const TextAppearanceAttributes& attributes){
    if (attributes.mTextColor != nullptr) {
        mTextColor = attributes.mTextColor->getDefaultColor();
    }

    if (attributes.mTextSize != -1.f) {
        mTextSize = attributes.mTextSize;
    }

    setTypefaceFromAttrs(
            attributes.mFontFamily,
            attributes.mTypefaceIndex,
            attributes.mTextStyle,
            attributes.mFontWeight);

    mPaint.setLetterSpacing(attributes.mLetterSpacing);
    mLetterSpacing = attributes.mLetterSpacing;
    mPaint.setFontFeatureSettings(attributes.mFontFeatureSettings);
    mFontFeatureSettings = attributes.mFontFeatureSettings;
    mFontVariationSettings = attributes.mFontVariationSettings;
}

// java:674-752 — read the Text Appearance attributes from a given TypedArray;
// values already set in `attributes` are overridden.
void CurvedTextView::readTextAppearance(const TypedArray& appearance,
        TextAppearanceAttributes& attributes,bool isTextAppearance){
    int attrIndex =
            isTextAppearance
                    ? internal::R::styleable::WearTextAppearance_textColor
                    : internal::R::styleable::CurvedTextView_textColor;
    if (appearance.hasValue(attrIndex)) {
        attributes.mTextColor = appearance.getColorStateList(attrIndex);
    }

    attributes.mTextSize =
            appearance.getDimensionPixelSize(
                    isTextAppearance
                            ? internal::R::styleable::WearTextAppearance_textSize
                            : internal::R::styleable::CurvedTextView_textSize,
                    (int) attributes.mTextSize);

    attributes.mTextStyle =
            appearance.getInt(
                    isTextAppearance
                            ? internal::R::styleable::WearTextAppearance_textStyle
                            : internal::R::styleable::CurvedTextView_textStyle,
                    attributes.mTextStyle);

    // make sure that the typeface attribute is read before fontFamily attribute
    attributes.mTypefaceIndex =
            appearance.getInt(
                    isTextAppearance
                            ? internal::R::styleable::WearTextAppearance_typeface
                            : internal::R::styleable::CurvedTextView_typeface,
                    attributes.mTypefaceIndex);
    if (attributes.mTypefaceIndex != -1 && !attributes.mFontFamilyExplicit) {
        attributes.mFontFamily.clear();   // null
    }

    attrIndex =
            isTextAppearance
                    ? internal::R::styleable::WearTextAppearance_fontFamily
                    : internal::R::styleable::CurvedTextView_fontFamily;
    if (appearance.hasValue(attrIndex)) {
        attributes.mFontFamily = appearance.getString(attrIndex);
        attributes.mFontFamilyExplicit = !isTextAppearance;
    }

    attributes.mFontWeight =
            appearance.getInt(
                    isTextAppearance
                            ? internal::R::styleable::WearTextAppearance_textFontWeight
                            : internal::R::styleable::CurvedTextView_textFontWeight,
                    attributes.mFontWeight);

    attributes.mLetterSpacing =
            appearance.getFloat(
                    isTextAppearance
                            ? internal::R::styleable::WearTextAppearance_letterSpacing
                            : internal::R::styleable::CurvedTextView_letterSpacing,
                    attributes.mLetterSpacing);

    attrIndex =
            isTextAppearance
                    ? internal::R::styleable::WearTextAppearance_fontFeatureSettings
                    : internal::R::styleable::CurvedTextView_fontFeatureSettings;
    if (appearance.hasValue(attrIndex)) {
        attributes.mFontFeatureSettings = appearance.getString(attrIndex);
    }

    attrIndex =
            isTextAppearance
                    ? internal::R::styleable::WearTextAppearance_fontVariationSettings
                    : internal::R::styleable::CurvedTextView_fontVariationSettings;
    if (appearance.hasValue(attrIndex)) {
        attributes.mFontVariationSettings = appearance.getString(attrIndex);
    }
}

// java:754-758
void CurvedTextView::doUpdate(){
    mDirty = true;
    requestLayout();
    postInvalidate();
}

// java:760-763
void CurvedTextView::doRedraw(){
    mDirty = true;
    postInvalidate();
}

// java:765-779
int CurvedTextView::getAnchorType() const{
    return mAnchorType;
}

void CurvedTextView::setAnchorType(int value){
    mAnchorType = value;
    doUpdate();
}

// java:781-792
float CurvedTextView::getAnchorAngleDegrees() const{
    return mAnchorAngleDegrees;
}

void CurvedTextView::setAnchorAngleDegrees(float value){
    mAnchorAngleDegrees = value;
    doRedraw();
}

// java:794-812
void CurvedTextView::setSweepRangeDegrees(float minSweep,float maxSweep){
    if (minSweep > maxSweep) {
        throw std::invalid_argument("MaxSweepDegrees cannot be smaller than MinSweepDegrees");
    }
    mMinSweepDegrees = std::min(std::max(minSweep, MIN_SWEEP_DEGREE), MAX_SWEEP_DEGREE);
    mMaxSweepDegrees = std::min(maxSweep, MAX_SWEEP_DEGREE);
    doUpdate();
}

// java:814-824
float CurvedTextView::getMinSweepDegrees() const{
    return mMinSweepDegrees;
}

float CurvedTextView::getMaxSweepDegrees() const{
    return mMaxSweepDegrees;
}

// java:826-847
const std::string& CurvedTextView::getText() const{
    return mText;
}

void CurvedTextView::setText(const std::string& value){
    mText = value;
    doUpdate();
}

float CurvedTextView::getTextSize() const{
    return mTextSize;
}

void CurvedTextView::setTextSize(float value){
    mTextSize = value;
    mPaint.setTextSize(mTextSize);
    doUpdate();
}

// java:849-869
Typeface* CurvedTextView::getTypeface() const{
    return mTypeface;
}

void CurvedTextView::setTypeface(Typeface* value){
    mOriginalTypeface = value;
    mTextStyle = DEFAULT_TEXT_STYLE;
    mFontWeight = -1;

    mTypeface = getAdjustedTypeface(value, DEFAULT_TEXT_STYLE, -1);
    mPaint.setTypeface(mTypeface);

    mPaint.setFakeBoldText(false);
    mPaint.setTextSkewX(0.f);
    doUpdate();
}

// java:871-880
bool CurvedTextView::isClockwise() const{
    return mClockwise;
}

void CurvedTextView::setClockwise(bool value){
    mClockwise = value;
    doUpdate();
}

// java:882-892
int CurvedTextView::getTextColor() const{
    return mTextColor;
}

void CurvedTextView::setTextColor(int value){
    mTextColor = value;
    doRedraw();
}

// java:894-908 — TruncateAt::NONE is the null sentinel.
TextUtils::TruncateAt CurvedTextView::getEllipsize() const{
    return mEllipsize;
}

void CurvedTextView::setEllipsize(TextUtils::TruncateAt value){
    mEllipsize = value;
    doRedraw();
}

// java:910-929
float CurvedTextView::getLetterSpacing() const{
    return mLetterSpacing;
}

void CurvedTextView::setLetterSpacing(float value){
    mLetterSpacing = value;
    doUpdate();
}

// java:931-952
const std::string& CurvedTextView::getFontFeatureSettings() const{
    return mFontFeatureSettings;
}

void CurvedTextView::setFontFeatureSettings(const std::string& value){
    mPaint.setFontFeatureSettings(value);
    mFontFeatureSettings = value;
    doUpdate();
}

// java:954-971 — Paint::setFontVariationSettings is not ported (CDROID Paint
// has no variation-settings support); the value is stored so the getter
// round-trips.
const std::string& CurvedTextView::getFontVariationSettings() const{
    return mFontVariationSettings;
}

void CurvedTextView::setFontVariationSettings(const std::string& value){
    mFontVariationSettings = value;
    doUpdate();
}

// java:973-983
void CurvedTextView::onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info){
    View::onInitializeAccessibilityNodeInfo(info);
    info.setText(mText);
}

void CurvedTextView::onPopulateAccessibilityEvent(AccessibilityEvent& event){
    View::onPopulateAccessibilityEvent(event);
    event.getText().push_back(mText);
}

}/*endof namespace*/
