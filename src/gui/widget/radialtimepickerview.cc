/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * Port of android.widget.RadialTimePickerView (android-36). Driven by
 * TimePickerClockDelegate. Drawing follows the codebase convention: text goes
 * through Paint::drawTextRun (which honors Paint::Align), shapes/colors are
 * drawn straight onto the cairo context (Canvas is-a Cairo::Context).
 *********************************************************************************/
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include <widget/radialtimepickerview.h>
#include <core/calendar.h>
#include <core/typeface.h>
#include <core/attributeset.h>
#include <core/path.h>
#include <core/color.h>
#include <utils/mathutils.h>
#include <text/paint.h>
#include <text/textutils.h>
#include <drawable/colorstatelist.h>
#include <drawable/stateset.h>
#include <animation/objectanimator.h>
#include <animation/property.h>
#include <view/accessibility/accessibilitynodeinfo.h>
#include <view/accessibility/accessibilityevent.h>
#include <view/hapticfeedbackconstants.h>
#include <view/layoutinflater.h>
#include <porting/cdlog.h>

namespace cdroid {

namespace {
#define NUM_POSITIONS 12

constexpr int HOURS_NUMBERS[]      = {12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
constexpr int HOURS_NUMBERS_24[]   = {0, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
constexpr int MINUTES_NUMBERS[]    = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55};

int SNAP_PREFER_30S_MAP[361];
float COS_30[NUM_POSITIONS] = {0.f};
float SIN_30[NUM_POSITIONS] = {0.f};
bool sStaticInited = false;

// Split up the 360 degrees of the circle among the 60 selectable values,
// giving visible (divisible-by-30) values a larger selectable area.
void preparePrefer30sMap() {
    int snappedOutputDegrees = 0;
    int count = 1;
    int expectedCount = 8;
    for (int degrees = 0; degrees < 361; degrees++) {
        SNAP_PREFER_30S_MAP[degrees] = snappedOutputDegrees;
        if (count == expectedCount) {
            snappedOutputDegrees += 6;
            if (snappedOutputDegrees == 360) {
                expectedCount = 7;
            } else if (snappedOutputDegrees % 30 == 0) {
                expectedCount = 14;
            } else {
                expectedCount = 4;
            }
            count = 1;
        } else {
            count++;
        }
    }
}

void staticInit() {
    if (sStaticInited) return;
    sStaticInited = true;
    preparePrefer30sMap();
    const double increment = 2.0 * M_PI / NUM_POSITIONS;
    double angle = M_PI / 2.0;
    for (int i = 0; i < NUM_POSITIONS; i++) {
        COS_30[i] = (float) std::cos(angle);
        SIN_30[i] = (float) std::sin(angle);
        angle += increment;
    }
}

// Faithful port of android.util.MathUtils.lerpDeg.
float lerpDeg(float start, float end, float amount) {
    const float minAngle = std::fmod((end - start) + 180.f, 360.f) - 180.f;
    return minAngle * amount + start;
}

std::string fmtInt(const char* spec, int value) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), spec, value);
    return std::string(buf);
}
} // namespace

// FloatProperty bound to mHoursToMinutes; drives the hours<->minutes crossfade.
class Hours2Minutes : public FloatProperty {
public:
    Hours2Minutes() : FloatProperty("hoursToMinutes") {}
    void set(void* object, const AnimateValue& value) const override {
        const float v = GET_VARIANT(value, float);
        auto* self = (RadialTimePickerView*) object;
        self->mHoursToMinutes = v;
        self->invalidate();
    }
    AnimateValue get(void* object) const override {
        return ((RadialTimePickerView*) object)->mHoursToMinutes;
    }
};
namespace { static Hours2Minutes HOURS_TO_MINUTES; }

int RadialTimePickerView::snapPrefer30s(int degrees) {
    return SNAP_PREFER_30S_MAP[degrees];
}

int RadialTimePickerView::snapOnly30s(int degrees, int forceHigherOrLower) {
    const int stepSize = DEGREES_FOR_ONE_HOUR;
    int floor = (degrees / stepSize) * stepSize;
    const int ceiling = floor + stepSize;
    if (forceHigherOrLower == 1) {
        degrees = ceiling;
    } else if (forceHigherOrLower == -1) {
        if (degrees == floor) {
            floor -= stepSize;
        }
        degrees = floor;
    } else {
        if ((degrees - floor) < (ceiling - degrees)) {
            degrees = floor;
        } else {
            degrees = ceiling;
        }
    }
    return degrees;
}

DECLARE_WIDGET(RadialTimePickerView);

RadialTimePickerView::RadialTimePickerView(Context* context, const AttributeSet& attrs)
    : View(context, attrs) {
    staticInit();

    mHours12Texts.resize(12);
    mOuterHours24Texts.resize(12);
    mInnerHours24Texts.resize(12);
    mMinutesTexts.resize(12);

    applyAttributes(attrs);

    // TODO: theme.resolveAttribute(android.R.attr.disabledAlpha) is not wired in
    // cdroid; use the platform default disabled alpha until it is.
    mDisabledAlpha = 0.3f;

    mTypeface = Typeface::create("sans-serif", Typeface::NORMAL);

    mPaint[HOURS].setAntiAlias(true);
    mPaint[HOURS].setTextAlign(Paint::Align::CENTER);
    mPaint[MINUTES].setAntiAlias(true);
    mPaint[MINUTES].setTextAlign(Paint::Align::CENTER);

    Context* ctx = getContext();
    mSelectorRadius   = ctx->getDimensionPixelSize("timepicker_selector_radius", 20);
    mSelectorStroke   = ctx->getDimensionPixelSize("timepicker_selector_stroke", 2);
    mSelectorDotRadius= ctx->getDimensionPixelSize("timepicker_selector_dot_radius", 4);
    mCenterDotRadius  = ctx->getDimensionPixelSize("timepicker_center_dot_radius", 3);
    mTextSize[HOURS]       = ctx->getDimensionPixelSize("timepicker_text_size_normal", 16);
    mTextSize[MINUTES]     = mTextSize[HOURS];
    mTextSize[HOURS_INNER] = ctx->getDimensionPixelSize("timepicker_text_size_inner", 14);
    mTextInset[HOURS]      = ctx->getDimensionPixelSize("timepicker_text_inset_normal", 22);
    mTextInset[MINUTES]    = mTextInset[HOURS];
    mTextInset[HOURS_INNER]= ctx->getDimensionPixelSize("timepicker_text_inset_inner", 10);

    mShowHours = true;
    mHoursToMinutes = (float) HOURS;
    mIs24HourMode = false;
    mAmOrPm = AM;
    mHoursToMinutesAnimator = nullptr;
    mSelectorPath = new Path();

    // Set up accessibility components.
    mTouchHelper = new RadialPickerTouchHelper(this);
    setAccessibilityDelegate(mTouchHelper);
    if (getImportantForAccessibility() == IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
        setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);
    }

    initHoursAndMinutesText();
    initData();

    // Initial values from the current time.
    auto calendar = Calendar::getInstance();
    const int currentHour = calendar->get(Calendar::HOUR_OF_DAY);
    const int currentMinute = calendar->get(Calendar::MINUTE);
    setCurrentHourInternal(currentHour, false, false);
    setCurrentMinuteInternal(currentMinute, false);

    setHapticFeedbackEnabled(true);
}

RadialTimePickerView::~RadialTimePickerView() {
    if (mHoursToMinutesAnimator != nullptr) {
        mHoursToMinutesAnimator->cancel();
        delete mHoursToMinutesAnimator;
        mHoursToMinutesAnimator = nullptr;
    }
    delete mTouchHelper;
    delete mSelectorPath;
}

void RadialTimePickerView::applyAttributes(const AttributeSet& attrs) {
    RefPtr<ColorStateList> numbersTextColor = attrs.getColorStateList("numbersTextColor");
    RefPtr<ColorStateList> numbersInnerTextColor = attrs.getColorStateList("numbersInnerTextColor");
    mTextColor[HOURS] = numbersTextColor ? numbersTextColor : ColorStateList::valueOf(MISSING_COLOR);
    mTextColor[HOURS_INNER] = numbersInnerTextColor ? numbersInnerTextColor
                                                    : ColorStateList::valueOf(MISSING_COLOR);
    mTextColor[MINUTES] = mTextColor[HOURS];

    // Set up various colors derived from the selector "activated" state.
    RefPtr<ColorStateList> selectorColors = attrs.getColorStateList("numbersSelectorColor");
    int selectorActivatedColor;
    if (selectorColors) {
        const std::vector<int> stateSetEnabledActivated = StateSet::get(
                StateSet::VIEW_STATE_ENABLED | StateSet::VIEW_STATE_ACTIVATED);
        selectorActivatedColor = selectorColors->getColorForState(stateSetEnabledActivated, 0);
    } else {
        selectorActivatedColor = MISSING_COLOR;
    }
    mCenterColor = selectorActivatedColor;

    const std::vector<int> stateSetActivated = StateSet::get(
            StateSet::VIEW_STATE_ENABLED | StateSet::VIEW_STATE_ACTIVATED);
    mSelectorColor = selectorActivatedColor;
    mSelectorDotColor = mTextColor[HOURS]->getColorForState(stateSetActivated, 0);

    mBackgroundColor = attrs.getColor("numbersBackgroundColor", 0);
}

void RadialTimePickerView::initialize(int hour, int minute, bool is24HourMode) {
    if (mIs24HourMode != is24HourMode) {
        mIs24HourMode = is24HourMode;
        initData();
    }
    setCurrentHourInternal(hour, false, false);
    setCurrentMinuteInternal(minute, false);
}

void RadialTimePickerView::setCurrentItemShowing(int item, bool animate) {
    switch (item) {
    case HOURS:   showHours(animate);   break;
    case MINUTES: showMinutes(animate); break;
    default: LOGE("ClockView does not support showing item %d", item);
    }
}

int RadialTimePickerView::getCurrentItemShowing() const {
    return mShowHours ? HOURS : MINUTES;
}

void RadialTimePickerView::setOnValueSelectedListener(const OnValueSelectedListener& listener) {
    mListener = listener;
}

void RadialTimePickerView::setCurrentHour(int hour) {
    setCurrentHourInternal(hour, true, false);
}

void RadialTimePickerView::setCurrentHourInternal(int hour, bool callback, bool autoAdvance) {
    const int degrees = (hour % 12) * DEGREES_FOR_ONE_HOUR;
    mSelectionDegrees[HOURS] = degrees;

    // 0 is 12 AM (midnight) and 12 is 12 PM (noon).
    const int amOrPm = (hour == 0 || (hour % 24) < 12) ? AM : PM;
    const bool isOnInnerCircle = getInnerCircleForHour(hour);
    if (mAmOrPm != amOrPm || mIsOnInnerCircle != isOnInnerCircle) {
        mAmOrPm = amOrPm;
        mIsOnInnerCircle = isOnInnerCircle;
        initData();
        mTouchHelper->invalidateRoot();
    }

    invalidate();

    if (callback && mListener) {
        mListener(HOURS, hour, autoAdvance);
    }
}

int RadialTimePickerView::getCurrentHour() const {
    return getHourForDegrees(mSelectionDegrees[HOURS], mIsOnInnerCircle);
}

int RadialTimePickerView::getHourForDegrees(int degrees, bool innerCircle) const {
    int hour = (degrees / DEGREES_FOR_ONE_HOUR) % 12;
    if (mIs24HourMode) {
        // Convert the 12-hour value into 24-hour time based on where the
        // selector is positioned.
        if (!innerCircle && hour == 0) {
            hour = 12; // Outer circle is 1 through 12.
        } else if (innerCircle && hour != 0) {
            hour += 12; // Inner circle is 13 through 23 and 0.
        }
    } else if (mAmOrPm == PM) {
        hour += 12;
    }
    return hour;
}

int RadialTimePickerView::getDegreesForHour(int hour) {
    if (mIs24HourMode) {
        if (hour >= 12) hour -= 12;
    } else if (hour == 12) {
        hour = 0;
    }
    return hour * DEGREES_FOR_ONE_HOUR;
}

bool RadialTimePickerView::getInnerCircleForHour(int hour) {
    return mIs24HourMode && (hour == 0 || hour > 12);
}

void RadialTimePickerView::setCurrentMinute(int minute) {
    setCurrentMinuteInternal(minute, true);
}

void RadialTimePickerView::setCurrentMinuteInternal(int minute, bool callback) {
    mSelectionDegrees[MINUTES] = (minute % MINUTES_IN_CIRCLE) * DEGREES_FOR_ONE_MINUTE;
    invalidate();
    if (callback && mListener) {
        mListener(MINUTES, minute, false);
    }
}

int RadialTimePickerView::getCurrentMinute() const {
    return getMinuteForDegrees(mSelectionDegrees[MINUTES]);
}

int RadialTimePickerView::getMinuteForDegrees(int degrees) const {
    return degrees / DEGREES_FOR_ONE_MINUTE;
}

int RadialTimePickerView::getDegreesForMinute(int minute) const {
    return minute * DEGREES_FOR_ONE_MINUTE;
}

bool RadialTimePickerView::setAmOrPm(int amOrPm) {
    if (mAmOrPm == amOrPm || mIs24HourMode) {
        return false;
    }
    mAmOrPm = amOrPm;
    invalidate();
    mTouchHelper->invalidateRoot();
    return true;
}

int RadialTimePickerView::getAmOrPm() const {
    return mAmOrPm;
}

void RadialTimePickerView::showHours(bool animate) { showPicker(true, animate); }
void RadialTimePickerView::showMinutes(bool animate) { showPicker(false, animate); }

void RadialTimePickerView::initHoursAndMinutesText() {
    for (int i = 0; i < 12; i++) {
        mHours12Texts[i]      = fmtInt("%d",  HOURS_NUMBERS[i]);
        mInnerHours24Texts[i] = fmtInt("%02d", HOURS_NUMBERS_24[i]);
        mOuterHours24Texts[i] = fmtInt("%d",  HOURS_NUMBERS[i]);
        mMinutesTexts[i]      = fmtInt("%02d", MINUTES_NUMBERS[i]);
    }
}

void RadialTimePickerView::initData() {
    if (mIs24HourMode) {
        mOuterTextHours = mOuterHours24Texts;
        mInnerTextHours = mInnerHours24Texts;
    } else {
        mOuterTextHours = mHours12Texts;
        mInnerTextHours = mHours12Texts;
    }
    mMinutesText = mMinutesTexts;
}

void RadialTimePickerView::onLayout(bool changed, int left, int top, int right, int bottom) {
    if (!changed) return;

    mXCenter = getWidth() / 2;
    mYCenter = getHeight() / 2;
    mCircleRadius = std::min(mXCenter, mYCenter);

    mMinDistForInnerNumber = mCircleRadius - mTextInset[HOURS_INNER] - mSelectorRadius;
    mMaxDistForOuterNumber = mCircleRadius - mTextInset[HOURS] + mSelectorRadius;
    mHalfwayDist = mCircleRadius - (mTextInset[HOURS] + mTextInset[HOURS_INNER]) / 2;

    calculatePositionsHours();
    calculatePositionsMinutes();

    mTouchHelper->invalidateRoot();
}

void RadialTimePickerView::onDraw(Canvas& canvas) {
    const float alphaMod = mInputEnabled ? 1.f : mDisabledAlpha;

    drawCircleBackground(canvas);

    drawSelector(canvas, mSelectorPath);
    drawHours(canvas, *mSelectorPath, alphaMod);
    drawMinutes(canvas, *mSelectorPath, alphaMod);
    drawCenter(canvas, alphaMod);
}

void RadialTimePickerView::showPicker(bool hours, bool animate) {
    if (mShowHours == hours) return;
    mShowHours = hours;

    if (animate) {
        animatePicker(hours, ANIM_DURATION_NORMAL);
    } else {
        if (mHoursToMinutesAnimator != nullptr && mHoursToMinutesAnimator->isStarted()) {
            mHoursToMinutesAnimator->cancel();
        }
        mHoursToMinutes = hours ? 0.0f : 1.0f;
    }

    initData();
    invalidate();
    mTouchHelper->invalidateRoot();
}

void RadialTimePickerView::animatePicker(bool hoursToMinutes, long duration) {
    const float target = hoursToMinutes ? (float) HOURS : (float) MINUTES;
    if (mHoursToMinutes == target) {
        if (mHoursToMinutesAnimator != nullptr && mHoursToMinutesAnimator->isStarted()) {
            mHoursToMinutesAnimator->cancel();
        }
        return; // Already showing the correct picker.
    }

    if (mHoursToMinutesAnimator != nullptr) {
        mHoursToMinutesAnimator->cancel();
        delete mHoursToMinutesAnimator;
        mHoursToMinutesAnimator = nullptr;
    }
    mHoursToMinutesAnimator = ObjectAnimator::ofFloat(this, &HOURS_TO_MINUTES,
            { mHoursToMinutes, target });
    mHoursToMinutesAnimator->setAutoCancel(true);
    mHoursToMinutesAnimator->setDuration(duration);
    mHoursToMinutesAnimator->start();
}

void RadialTimePickerView::drawCircleBackground(Canvas& canvas) {
    canvas.set_color(mBackgroundColor);
    canvas.arc(mXCenter, mYCenter, mCircleRadius, 0, 2.0 * M_PI);
    canvas.fill();
}

void RadialTimePickerView::drawHours(Canvas& canvas, Path& selectorPath, float alphaMod) {
    const int hoursAlpha = (int) (255.f * (1.f - mHoursToMinutes) * alphaMod + 0.5f);
    if (hoursAlpha <= 0) return;

    // Exclude the selector region, then draw inner/outer hours with no
    // activated states. The even-odd rule punches the selector circle out of
    // the full bounds (equivalent to Region.Op.DIFFERENCE).
    canvas.save();
    canvas.set_fill_rule(Cairo::Context::FillRule::EVEN_ODD);
    canvas.rectangle(0, 0, getWidth(), getHeight());
    selectorPath.append_to_context(&canvas);
    canvas.clip();
    drawHoursClipped(canvas, mPaint[HOURS], hoursAlpha, false);
    canvas.restore();

    // Intersect the selector region, then draw hours with only activated states.
    canvas.save();
    selectorPath.append_to_context(&canvas);
    canvas.clip();
    drawHoursClipped(canvas, mPaint[HOURS], hoursAlpha, true);
    canvas.restore();
}

void RadialTimePickerView::drawHoursClipped(Canvas& canvas, Paint& paint, int hoursAlpha, bool showActivated) {
    // Draw outer hours.
    drawTextElements(canvas, mTextSize[HOURS], mTypeface, mTextColor[HOURS].get(), mOuterTextHours,
            mOuterTextX[HOURS], mOuterTextY[HOURS], paint, hoursAlpha,
            showActivated && !mIsOnInnerCircle, mSelectionDegrees[HOURS], showActivated);

    // Draw inner hours (13-00) for 24-hour time.
    if (mIs24HourMode && !mInnerTextHours.empty()) {
        drawTextElements(canvas, mTextSize[HOURS_INNER], mTypeface, mTextColor[HOURS_INNER].get(),
                mInnerTextHours, mInnerTextX, mInnerTextY, paint, hoursAlpha,
                showActivated && mIsOnInnerCircle, mSelectionDegrees[HOURS], showActivated);
    }
}

void RadialTimePickerView::drawMinutes(Canvas& canvas, Path& selectorPath, float alphaMod) {
    const int minutesAlpha = (int) (255.f * mHoursToMinutes * alphaMod + 0.5f);
    if (minutesAlpha <= 0) return;

    canvas.save();
    canvas.set_fill_rule(Cairo::Context::FillRule::EVEN_ODD);
    canvas.rectangle(0, 0, getWidth(), getHeight());
    selectorPath.append_to_context(&canvas);
    canvas.clip();
    drawMinutesClipped(canvas, mPaint[MINUTES], minutesAlpha, false);
    canvas.restore();

    canvas.save();
    selectorPath.append_to_context(&canvas);
    canvas.clip();
    drawMinutesClipped(canvas, mPaint[MINUTES], minutesAlpha, true);
    canvas.restore();
}

void RadialTimePickerView::drawMinutesClipped(Canvas& canvas, Paint& paint, int minutesAlpha, bool showActivated) {
    drawTextElements(canvas, mTextSize[MINUTES], mTypeface, mTextColor[MINUTES].get(), mMinutesText,
            mOuterTextX[MINUTES], mOuterTextY[MINUTES], paint, minutesAlpha,
            showActivated, mSelectionDegrees[MINUTES], showActivated);
}

void RadialTimePickerView::drawCenter(Canvas& canvas, float alphaMod) {
    const int a = (int) (255.f * alphaMod + 0.5f);
    const int c = (mCenterColor & 0x00FFFFFF) | ((a & 0xFF) << 24);
    canvas.set_color(c);
    canvas.arc(mXCenter, mYCenter, mCenterDotRadius, 0, 2.0 * M_PI);
    canvas.fill();
}

int RadialTimePickerView::getMultipliedAlpha(int argb, int alpha) const {
    return (int) (Color::alpha(argb) * (alpha / 255.0) + 0.5);
}

void RadialTimePickerView::drawSelector(Canvas& canvas, Path* selectorPath) {
    // Determine the current length, angle, and dot scaling factor.
    const int hoursIndex = mIsOnInnerCircle ? HOURS_INNER : HOURS;
    const int hoursInset = mTextInset[hoursIndex];
    const int hoursAngleDeg = mSelectionDegrees[hoursIndex % 2];
    const float hoursDotScale = (mSelectionDegrees[hoursIndex % 2] % 30 != 0) ? 1.f : 0.f;

    const int minutesIndex = MINUTES;
    const int minutesInset = mTextInset[minutesIndex];
    const int minutesAngleDeg = mSelectionDegrees[minutesIndex];
    const float minutesDotScale = (mSelectionDegrees[minutesIndex] % 30 != 0) ? 1.f : 0.f;

    // Calculate the current radius at which to place the selection circle.
    const int selRadius = mSelectorRadius;
    const float selLength = mCircleRadius
            - MathUtils::lerp((float) hoursInset, (float) minutesInset, mHoursToMinutes);
    const double selAngleRad =
            lerpDeg((float) hoursAngleDeg, (float) minutesAngleDeg, mHoursToMinutes) * M_PI / 180.0;
    const float selCenterX = mXCenter + selLength * (float) std::sin(selAngleRad);
    const float selCenterY = mYCenter - selLength * (float) std::cos(selAngleRad);

    // Draw the selection circle.
    canvas.set_color(mSelectorColor);
    canvas.arc(selCenterX, selCenterY, selRadius, 0, 2.0 * M_PI);
    canvas.fill();

    // If needed, set up the clip path for later.
    if (selectorPath != nullptr) {
        selectorPath->reset();
        selectorPath->arc(selCenterX, selCenterY, selRadius, 0, 2.0 * M_PI);
    }

    // Draw the dot if we're between two items.
    const float dotScale = MathUtils::lerp(hoursDotScale, minutesDotScale, mHoursToMinutes);
    if (dotScale > 0) {
        canvas.set_color(mSelectorDotColor);
        canvas.arc(selCenterX, selCenterY, mSelectorDotRadius * dotScale, 0, 2.0 * M_PI);
        canvas.fill();
    }

    // Shorten the line to go from the edge of the center dot to the edge of the
    // selection circle.
    const double sin = std::sin(selAngleRad);
    const double cos = std::cos(selAngleRad);
    const float lineLength = selLength - selRadius;
    const int centerX = mXCenter + (int) (mCenterDotRadius * sin);
    const int centerY = mYCenter - (int) (mCenterDotRadius * cos);
    const float linePointX = centerX + (int) (lineLength * sin);
    const float linePointY = centerY - (int) (lineLength * cos);

    // Draw the line.
    canvas.set_color(mSelectorColor);
    canvas.set_line_width(mSelectorStroke);
    canvas.move_to(mXCenter, mYCenter);
    canvas.line_to(linePointX, linePointY);
    canvas.stroke();
}

void RadialTimePickerView::calculatePositionsHours() {
    const float numbersRadius = mCircleRadius - mTextInset[HOURS];
    calculatePositions(mPaint[HOURS], numbersRadius, mXCenter, mYCenter,
            mTextSize[HOURS], mOuterTextX[HOURS], mOuterTextY[HOURS]);

    if (mIs24HourMode) {
        const int innerNumbersRadius = mCircleRadius - mTextInset[HOURS_INNER];
        calculatePositions(mPaint[HOURS], innerNumbersRadius, mXCenter, mYCenter,
                mTextSize[HOURS_INNER], mInnerTextX, mInnerTextY);
    }
}

void RadialTimePickerView::calculatePositionsMinutes() {
    const float numbersRadius = mCircleRadius - mTextInset[MINUTES];
    calculatePositions(mPaint[MINUTES], numbersRadius, mXCenter, mYCenter,
            mTextSize[MINUTES], mOuterTextX[MINUTES], mOuterTextY[MINUTES]);
}

void RadialTimePickerView::calculatePositions(Paint& paint, float radius, float xCenter, float yCenter,
        float textSize, float* x, float* y) {
    // Adjust yCenter to account for the text's baseline.
    paint.setTextSize(textSize);
    yCenter -= (paint.descent() + paint.ascent()) / 2.f;

    for (int i = 0; i < NUM_POSITIONS; i++) {
        x[i] = xCenter - radius * COS_30[i];
        y[i] = yCenter - radius * SIN_30[i];
    }
}

void RadialTimePickerView::drawTextElements(Canvas& canvas, float textSize, Typeface* typeface,
        ColorStateList* textColor, const std::vector<std::string>& texts, float* textX, float* textY,
        Paint& paint, int alpha, bool showActivated, int activatedDegrees, bool activatedOnly) {
    paint.setTextSize(textSize);
    paint.setTypeface(typeface);
    paint.setTextAlign(Paint::Align::CENTER);

    // The activated index can touch a range of elements.
    const float activatedIndex = activatedDegrees / (360.0f / NUM_POSITIONS);
    const int activatedFloor = (int) activatedIndex;
    const int activatedCeil = ((int) std::ceil(activatedIndex)) % NUM_POSITIONS;

    for (int i = 0; i < 12; i++) {
        const bool activated = (activatedFloor == i || activatedCeil == i);
        if (activatedOnly && !activated) continue;

        const int stateMask = StateSet::VIEW_STATE_ENABLED
                | (showActivated && activated ? StateSet::VIEW_STATE_ACTIVATED : 0);
        const int color = textColor->getColorForState(StateSet::get(stateMask), 0);

        // Paint::drawTextRun reads getColor() (which ignores the separate alpha
        // field), so fold the multiplied alpha into the color we set.
        const int finalAlpha = getMultipliedAlpha(color, alpha);
        paint.setColor((color & 0x00FFFFFF) | ((finalAlpha & 0xFF) << 24));

        const std::u16string u16 = TextUtils::utf8_utf16(texts[i]);
        paint.drawTextRun(canvas, (const char16_t*) u16.c_str(), 0, u16.length(),
                0, 0, textX[i], textY[i], false);
    }
}

int RadialTimePickerView::getDegreesFromXY(float x, float y, bool constrainOutside) {
    int innerBound;
    int outerBound;
    if (mIs24HourMode && mShowHours) {
        innerBound = mMinDistForInnerNumber;
        outerBound = mMaxDistForOuterNumber;
    } else {
        const int index = mShowHours ? HOURS : MINUTES;
        const int center = mCircleRadius - mTextInset[index];
        innerBound = center - mSelectorRadius;
        outerBound = center + mSelectorRadius;
    }

    const double dX = x - mXCenter;
    const double dY = y - mYCenter;
    const double distFromCenter = std::sqrt(dX * dX + dY * dY);
    if (distFromCenter < innerBound || (constrainOutside && distFromCenter > outerBound)) {
        return -1;
    }

    const int degrees = (int) (std::atan2(dY, dX) * 180.0 / M_PI + 90.0 + 0.5);
    if (degrees < 0) {
        return degrees + 360;
    }
    return degrees;
}

bool RadialTimePickerView::getInnerCircleFromXY(float x, float y) {
    if (mIs24HourMode && mShowHours) {
        const double dX = x - mXCenter;
        const double dY = y - mYCenter;
        const double distFromCenter = std::sqrt(dX * dX + dY * dY);
        return distFromCenter <= mHalfwayDist;
    }
    return false;
}

bool RadialTimePickerView::onTouchEvent(MotionEvent& event) {
    if (!mInputEnabled) {
        return true;
    }

    const int action = event.getActionMasked();
    if (action == MotionEvent::ACTION_MOVE
            || action == MotionEvent::ACTION_UP
            || action == MotionEvent::ACTION_DOWN) {
        bool forceSelection = false;
        bool autoAdvance = false;

        if (action == MotionEvent::ACTION_DOWN) {
            // New event stream, reset whether the value changed.
            mChangedDuringTouch = false;
        } else if (action == MotionEvent::ACTION_UP) {
            autoAdvance = true;
            // Down/up pair without a value change => single-tap selection.
            if (!mChangedDuringTouch) {
                forceSelection = true;
            }
        }

        mChangedDuringTouch |= handleTouchInput(event.getX(), event.getY(), forceSelection, autoAdvance);
    }

    return true;
}

bool RadialTimePickerView::handleTouchInput(float x, float y, bool forceSelection, bool autoAdvance) {
    const bool isOnInnerCircle = getInnerCircleFromXY(x, y);
    const int degrees = getDegreesFromXY(x, y, false);
    if (degrees == -1) {
        return false;
    }

    // Ensure we're showing the correct picker.
    animatePicker(mShowHours, ANIM_DURATION_TOUCH);

    int type;
    int newValue;
    bool valueChanged;

    if (mShowHours) {
        const int snapDegrees = snapOnly30s(degrees, 0) % 360;
        valueChanged = mIsOnInnerCircle != isOnInnerCircle
                || mSelectionDegrees[HOURS] != snapDegrees;
        mIsOnInnerCircle = isOnInnerCircle;
        mSelectionDegrees[HOURS] = snapDegrees;
        type = HOURS;
        newValue = getCurrentHour();
    } else {
        const int snapDegrees = snapPrefer30s(degrees) % 360;
        valueChanged = mSelectionDegrees[MINUTES] != snapDegrees;
        mSelectionDegrees[MINUTES] = snapDegrees;
        type = MINUTES;
        newValue = getCurrentMinute();
    }

    if (valueChanged || forceSelection || autoAdvance) {
        // Fire the listener even if we just need to auto-advance.
        if (mListener) {
            mListener(type, newValue, autoAdvance);
        }
        // Only provide feedback if the value actually changed.
        if (valueChanged || forceSelection) {
            performHapticFeedback(HapticFeedbackConstants::CLOCK_TICK);
            invalidate();
        }
        return true;
    }
    return false;
}

bool RadialTimePickerView::dispatchHoverEvent(MotionEvent& event) {
    // First right-of-refusal goes to the touch exploration helper.
    if (mTouchHelper->dispatchHoverEvent(event)) {
        return true;
    }
    return View::dispatchHoverEvent(event);
}

void RadialTimePickerView::setInputEnabled(bool inputEnabled) {
    mInputEnabled = inputEnabled;
    invalidate();
}

PointerIcon* RadialTimePickerView::onResolvePointerIcon(MotionEvent& event, int pointerIndex) {
    if (!isEnabled()) {
        return nullptr;
    }
    // Mouse-hover hand icon intentionally omitted (pointer-icon resolution for
    // in-circle hover not required); defer to the default behavior.
    return View::onResolvePointerIcon(event, pointerIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// RadialPickerTouchHelper
//
// Compiled for completeness; accessibility behavior is not verified (the host
// codebase ports accessibility best-effort). Method bodies mirror the Android
// original, adapted to the cdroid AccessibilityNodeInfo/Event API surface.

RadialTimePickerView::RadialPickerTouchHelper::RadialPickerTouchHelper(RadialTimePickerView* v)
    : ExploreByTouchHelper(v) {
}

void RadialTimePickerView::RadialPickerTouchHelper::onInitializeAccessibilityNodeInfo(
        View& host, AccessibilityNodeInfo& info) {
    ExploreByTouchHelper::onInitializeAccessibilityNodeInfo(host, info);
    info.addAction(AccessibilityNodeInfo::ACTION_SCROLL_FORWARD);
    info.addAction(AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD);
}

bool RadialTimePickerView::RadialPickerTouchHelper::performAccessibilityAction(
        View& host, int action, Bundle* arguments) {
    if (ExploreByTouchHelper::performAccessibilityAction(host, action, arguments)) {
        return true;
    }
    switch (action) {
    case AccessibilityNodeInfo::ACTION_SCROLL_FORWARD:  adjustPicker(1); return true;
    case AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD: adjustPicker(-1); return true;
    }
    return false;
}

void RadialTimePickerView::RadialPickerTouchHelper::adjustPicker(int step) {
    auto* pkv = (RadialTimePickerView*) mView;
    int stepSize, initialStep, minValue, maxValue;
    if (pkv->mShowHours) {
        stepSize = 1;
        const int currentHour24 = pkv->getCurrentHour();
        if (pkv->mIs24HourMode) {
            initialStep = currentHour24;
            minValue = 0;
            maxValue = 23;
        } else {
            initialStep = hour24To12(currentHour24);
            minValue = 1;
            maxValue = 12;
        }
    } else {
        stepSize = 5;
        initialStep = pkv->getCurrentMinute() / stepSize;
        minValue = 0;
        maxValue = 55;
    }

    const int nextValue = (initialStep + step) * stepSize;
    const int clampedValue = MathUtils::constrain(nextValue, minValue, maxValue);
    if (pkv->mShowHours) {
        pkv->setCurrentHour(clampedValue);
    } else {
        pkv->setCurrentMinute(clampedValue);
    }
}

int RadialTimePickerView::RadialPickerTouchHelper::getVirtualViewAt(float x, float y) {
    auto* pkv = (RadialTimePickerView*) mView;
    int id;
    const int degrees = pkv->getDegreesFromXY(x, y, true);
    if (degrees != -1) {
        const int snapDegrees = snapOnly30s(degrees, 0) % 360;
        if (pkv->mShowHours) {
            const bool isOnInnerCircle = pkv->getInnerCircleFromXY(x, y);
            const int hour24 = pkv->getHourForDegrees(snapDegrees, isOnInnerCircle);
            const int hour = pkv->mIs24HourMode ? hour24 : hour24To12(hour24);
            id = makeId(TYPE_HOUR, hour);
        } else {
            const int current = pkv->getCurrentMinute();
            const int touched = pkv->getMinuteForDegrees(degrees);
            const int snapped = pkv->getMinuteForDegrees(snapDegrees);
            // If the touched minute is closer to the current minute than to the
            // snapped minute, return current.
            const int currentOffset = getCircularDiff(current, touched, MINUTES_IN_CIRCLE);
            const int snappedOffset = getCircularDiff(snapped, touched, MINUTES_IN_CIRCLE);
            id = makeId(TYPE_MINUTE, (currentOffset < snappedOffset) ? current : snapped);
        }
    } else {
        id = INVALID_ID;
    }
    return id;
}

int RadialTimePickerView::RadialPickerTouchHelper::getCircularDiff(int first, int second, int max) {
    const int diff = std::abs(first - second);
    const int midpoint = max / 2;
    return (diff > midpoint) ? (max - diff) : diff;
}

void RadialTimePickerView::RadialPickerTouchHelper::getVisibleVirtualViews(std::vector<int>& virtualViewIds) {
    auto* pkv = (RadialTimePickerView*) mView;
    if (pkv->mShowHours) {
        const int min = pkv->mIs24HourMode ? 0 : 1;
        const int max = pkv->mIs24HourMode ? 23 : 12;
        for (int i = min; i <= max; i++) {
            virtualViewIds.push_back(makeId(TYPE_HOUR, i));
        }
    } else {
        const int current = pkv->getCurrentMinute();
        for (int i = 0; i < MINUTES_IN_CIRCLE; i += MINUTE_INCREMENT) {
            virtualViewIds.push_back(makeId(TYPE_MINUTE, i));
            // If the current minute falls between two increments, insert an extra node.
            if (current > i && current < i + MINUTE_INCREMENT) {
                virtualViewIds.push_back(makeId(TYPE_MINUTE, current));
            }
        }
    }
}

void RadialTimePickerView::RadialPickerTouchHelper::onPopulateEventForVirtualView(
        int virtualViewId, AccessibilityEvent& event) {
    // AccessibilityEvent has no setClassName/setContentDescription in this port;
    // intentionally left minimal (accessibility not required for this view).
    (void) virtualViewId;
    (void) event;
}

void RadialTimePickerView::RadialPickerTouchHelper::onPopulateNodeForVirtualView(
        int virtualViewId, AccessibilityNodeInfo& node) {
    node.addAction(AccessibilityNodeInfo::ACTION_CLICK);

    const int type = getTypeFromId(virtualViewId);
    const int value = getValueFromId(virtualViewId);
    const std::string description = getVirtualViewDescription(type, value);
    node.setClassName("RadialTimePickerView");
    node.setContentDescription(description);

    getBoundsForVirtualView(virtualViewId, mTempRect);
    node.setBoundsInParent(mTempRect);

    node.setSelected(isVirtualViewSelected(type, value));

    const int nextId = getVirtualViewIdAfter(type, value);
    if (nextId != INVALID_ID) {
        node.setTraversalBefore(mView, nextId);
    }
}

int RadialTimePickerView::RadialPickerTouchHelper::getVirtualViewIdAfter(int type, int value) {
    auto* pkv = (RadialTimePickerView*) mView;
    if (type == TYPE_HOUR) {
        const int nextValue = value + 1;
        const int max = pkv->mIs24HourMode ? 23 : 12;
        if (nextValue <= max) {
            return makeId(type, nextValue);
        }
    } else if (type == TYPE_MINUTE) {
        const int current = pkv->getCurrentMinute();
        const int snapValue = value - (value % MINUTE_INCREMENT);
        const int nextValue = snapValue + MINUTE_INCREMENT;
        if (value < current && nextValue > current) {
            return makeId(type, current); // current value is between two snap values
        } else if (nextValue < MINUTES_IN_CIRCLE) {
            return makeId(type, nextValue);
        }
    }
    return INVALID_ID;
}

bool RadialTimePickerView::RadialPickerTouchHelper::onPerformActionForVirtualView(
        int virtualViewId, int action, Bundle* arguments) {
    (void) arguments;
    if (action == AccessibilityNodeInfo::ACTION_CLICK) {
        auto* pkv = (RadialTimePickerView*) mView;
        const int type = getTypeFromId(virtualViewId);
        const int value = getValueFromId(virtualViewId);
        if (type == TYPE_HOUR) {
            const int hour = pkv->mIs24HourMode ? value : hour12To24(value, pkv->mAmOrPm);
            pkv->setCurrentHour(hour);
            return true;
        } else if (type == TYPE_MINUTE) {
            pkv->setCurrentMinute(value);
            return true;
        }
    }
    return false;
}

int RadialTimePickerView::RadialPickerTouchHelper::hour12To24(int hour12, int amOrPm) {
    int hour24 = hour12;
    if (hour12 == 12) {
        if (amOrPm == AM) hour24 = 0;
    } else if (amOrPm == PM) {
        hour24 += 12;
    }
    return hour24;
}

int RadialTimePickerView::RadialPickerTouchHelper::hour24To12(int hour24) {
    if (hour24 == 0) return 12;
    if (hour24 > 12) return hour24 - 12;
    return hour24;
}

void RadialTimePickerView::RadialPickerTouchHelper::getBoundsForVirtualView(int virtualViewId, Rect& bounds) {
    auto* pkv = (RadialTimePickerView*) mView;
    const int type = getTypeFromId(virtualViewId);
    const int value = getValueFromId(virtualViewId);
    float centerRadius;
    float degrees;
    float radius;
    if (type == TYPE_HOUR) {
        const bool innerCircle = pkv->getInnerCircleForHour(value);
        centerRadius = pkv->mCircleRadius
                - pkv->mTextInset[innerCircle ? HOURS_INNER : HOURS];
        radius = pkv->mSelectorRadius;
        degrees = pkv->getDegreesForHour(value);
    } else if (type == TYPE_MINUTE) {
        centerRadius = pkv->mCircleRadius - pkv->mTextInset[MINUTES];
        degrees = pkv->getDegreesForMinute(value);
        radius = pkv->mSelectorRadius;
    } else {
        // This should never happen.
        centerRadius = 0;
        degrees = 0;
        radius = 0;
    }

    const double radians = degrees * M_PI / 180.0;
    const float xCenter = pkv->mXCenter + centerRadius * (float) std::sin(radians);
    const float yCenter = pkv->mYCenter - centerRadius * (float) std::cos(radians);

    // Rect::set is (x, y, width, height) in this port.
    bounds.set((int) (xCenter - radius), (int) (yCenter - radius),
               (int) (2.f * radius), (int) (2.f * radius));
}

std::string RadialTimePickerView::RadialPickerTouchHelper::getVirtualViewDescription(int type, int value) {
    if (type == TYPE_HOUR || type == TYPE_MINUTE) {
        return std::to_string(value);
    }
    return {};
}

bool RadialTimePickerView::RadialPickerTouchHelper::isVirtualViewSelected(int type, int value) {
    auto* pkv = (RadialTimePickerView*) mView;
    if (type == TYPE_HOUR)   return pkv->getCurrentHour() == value;
    if (type == TYPE_MINUTE) return pkv->getCurrentMinute() == value;
    return false;
}

int RadialTimePickerView::RadialPickerTouchHelper::makeId(int type, int value) const {
    return (type << SHIFT_TYPE) | (value << SHIFT_VALUE);
}

int RadialTimePickerView::RadialPickerTouchHelper::getTypeFromId(int id) const {
    return (id >> SHIFT_TYPE) & MASK_TYPE;
}

int RadialTimePickerView::RadialPickerTouchHelper::getValueFromId(int id) const {
    return (id >> SHIFT_VALUE) & MASK_VALUE;
}

} /*endof namespace*/
