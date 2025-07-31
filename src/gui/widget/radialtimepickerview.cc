#include <widget/radialtimepickerview.h>
namespace cdroid{

static int HOURS_NUMBERS[] = {12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
static int HOURS_NUMBERS_24[] = {0, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
static int MINUTES_NUMBERS[] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55};
static int SNAP_PREFER_30S_MAP[361];
static float COS_30[NUM_POSITIONS];
static float SIN_30[NUM_POSITIONS];
static void preparePrefer30sMap() {
    int snappedOutputDegrees = 0;
    int count = 1;
    int expectedCount = 8;
    for (int degrees = 0; degrees < 361; degrees++) {
        // Save the input-output mapping.
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
static std::once_flag sInit;
std::call_once(sInit[](){
    // Prepare mapping to snap touchable degrees to selectable degrees.
    preparePrefer30sMap();    
    const double increment = 2.0 * M_PI / NUM_POSITIONS;
    double angle = M_PI / 2.0;
    for (int i = 0; i < NUM_POSITIONS; i++) {
        COS_30[i] = (float) std::cos(angle);
        SIN_30[i] = (float) std::sin(angle);
        angle += increment;
    }
});

private FloatProperty<RadialTimePickerView> HOURS_TO_MINUTES =
    new FloatProperty<RadialTimePickerView>("hoursToMinutes") {
        @Override
        public Float get(RadialTimePickerView radialTimePickerView) {
            return radialTimePickerView.mHoursToMinutes;
        }

        @Override
        public void setValue(RadialTimePickerView object, float value) {
            object.mHoursToMinutes = value;
            object.invalidate();
        }
    };

int RadialTimePickerView::snapPrefer30s(int degrees) {
    if (SNAP_PREFER_30S_MAP == null) {
        return -1;
    }
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


RadialTimePickerView::RadialTimePickerView(Context* context,const AttributeSet& attrs)
    :View(context, attrs){

    applyAttributes(attrs, defStyleAttr, defStyleRes);

    // Pull disabled alpha from theme.
    final TypedValue outValue = new TypedValue();
    context.getTheme().resolveAttribute(android.R.attr.disabledAlpha, outValue, true);
    mDisabledAlpha = outValue.getFloat();

    mTypeface = Typeface::create("sans-serif", Typeface::NORMAL);

    mPaint[HOURS] = new Paint();
    mPaint[HOURS].setAntiAlias(true);
    mPaint[HOURS].setTextAlign(Paint.Align.CENTER);

    mPaint[MINUTES] = new Paint();
    mPaint[MINUTES].setAntiAlias(true);
    mPaint[MINUTES].setTextAlign(Paint.Align.CENTER);

    mPaintCenter.setAntiAlias(true);

    mPaintSelector[SELECTOR_CIRCLE] = new Paint();
    mPaintSelector[SELECTOR_CIRCLE].setAntiAlias(true);

    mPaintSelector[SELECTOR_DOT] = new Paint();
    mPaintSelector[SELECTOR_DOT].setAntiAlias(true);

    mPaintSelector[SELECTOR_LINE] = new Paint();
    mPaintSelector[SELECTOR_LINE].setAntiAlias(true);
    mPaintSelector[SELECTOR_LINE].setStrokeWidth(2);

    mPaintBackground.setAntiAlias(true);

    final Resources res = getResources();
    mSelectorRadius = res.getDimensionPixelSize(R.dimen.timepicker_selector_radius);
    mSelectorStroke = res.getDimensionPixelSize(R.dimen.timepicker_selector_stroke);
    mSelectorDotRadius = res.getDimensionPixelSize(R.dimen.timepicker_selector_dot_radius);
    mCenterDotRadius = res.getDimensionPixelSize(R.dimen.timepicker_center_dot_radius);

    mTextSize[HOURS] = res.getDimensionPixelSize(R.dimen.timepicker_text_size_normal);
    mTextSize[MINUTES] = res.getDimensionPixelSize(R.dimen.timepicker_text_size_normal);
    mTextSize[HOURS_INNER] = res.getDimensionPixelSize(R.dimen.timepicker_text_size_inner);

    mTextInset[HOURS] = res.getDimensionPixelSize(R.dimen.timepicker_text_inset_normal);
    mTextInset[MINUTES] = res.getDimensionPixelSize(R.dimen.timepicker_text_inset_normal);
    mTextInset[HOURS_INNER] = res.getDimensionPixelSize(R.dimen.timepicker_text_inset_inner);

    mShowHours = true;
    mHoursToMinutes = HOURS;
    mIs24HourMode = false;
    mAmOrPm = AM;

    // Set up accessibility components.
    mTouchHelper = new RadialPickerTouchHelper();
    setAccessibilityDelegate(mTouchHelper);

    if (getImportantForAccessibility() == IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
        setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);
    }

    initHoursAndMinutesText();
    initData();

    // Initial values
    Calendar calendar = Calendar.getInstance(Locale.getDefault());
    const int currentHour = calendar.get(Calendar::HOUR_OF_DAY);
    const int currentMinute = calendar.get(Calendar::MINUTE);

    setCurrentHourInternal(currentHour, false, false);
    setCurrentMinuteInternal(currentMinute, false);

    setHapticFeedbackEnabled(true);
}

void RadialTimePickerView::applyAttributes(const AttributeSet& attrs) {
    Context* context = getContext();
    final TypedArray a = getContext().obtainStyledAttributes(attrs,
            R.styleable.TimePicker, defStyleAttr, defStyleRes);
    saveAttributeDataForStyleable(context, R.styleable.TimePicker,
            attrs, a, defStyleAttr, defStyleRes);

    ColorStateList* numbersTextColor = a.getColorStateList(
            R.styleable.TimePicker_numbersTextColor);
    ColorStateList* numbersInnerTextColor = a.getColorStateList(
            R.styleable.TimePicker_numbersInnerTextColor);
    mTextColor[HOURS] = numbersTextColor == null ?
            ColorStateList.valueOf(MISSING_COLOR) : numbersTextColor;
    mTextColor[HOURS_INNER] = numbersInnerTextColor == null ?
            ColorStateList.valueOf(MISSING_COLOR) : numbersInnerTextColor;
    mTextColor[MINUTES] = mTextColor[HOURS];

    // Set up various colors derived from the selector "activated" state.
    ColorStateList* selectorColors = a.getColorStateList(
            R.styleable.TimePicker_numbersSelectorColor);
    int selectorActivatedColor;
    if (selectorColors != null) {
        std::vector<int> stateSetEnabledActivated = StateSet.get(
                StateSet.VIEW_STATE_ENABLED | StateSet.VIEW_STATE_ACTIVATED);
        selectorActivatedColor = selectorColors.getColorForState(
                stateSetEnabledActivated, 0);
    }  else {
        selectorActivatedColor = MISSING_COLOR;
    }

    mPaintCenter.setColor(selectorActivatedColor);

    std::vector<int> stateSetActivated = StateSet::get(
            StateSet::VIEW_STATE_ENABLED | StateSet::VIEW_STATE_ACTIVATED);

    mSelectorColor = selectorActivatedColor;
    mSelectorDotColor = mTextColor[HOURS].getColorForState(stateSetActivated, 0);

    mPaintBackground.setColor(a.getColor(R.styleable.TimePicker_numbersBackgroundColor,
            context.getColor(R.color.timepicker_default_numbers_background_color_material)));
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
    switch (item){
    case HOURS:   showHours(animate);   break;
    case MINUTES: showMinutes(animate); break;
    default:LOGE("ClockView does not support showing item %d" ,item);
    }
}

int RadialTimePickerView::getCurrentItemShowing() {
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
        mTouchHelper.invalidateRoot();
    }

    invalidate();

    if (callback && mListener != null) {
        mListener.onValueSelected(HOURS, hour, autoAdvance);
    }
}

int RadialTimePickerView::getCurrentHour() {
    return getHourForDegrees(mSelectionDegrees[HOURS], mIsOnInnerCircle);
}

int RadialTimePickerView::getHourForDegrees(int degrees, bool innerCircle) {
    const int hour = (degrees / DEGREES_FOR_ONE_HOUR) % 12;
    if (mIs24HourMode) {
        // Convert the 12-hour value into 24-hour time based on where the
        // selector is positioned.
        if (!innerCircle && hour == 0) {
            // Outer circle is 1 through 12.
            hour = 12;
        } else if (innerCircle && hour != 0) {
            // Inner circle is 13 through 23 and 0.
            hour += 12;
        }
    } else if (mAmOrPm == PM) {
        hour += 12;
    }
    return hour;
}

int RadialTimePickerView::getDegreesForHour(int hour) {
    // Convert to be 0-11.
    if (mIs24HourMode) {
        if (hour >= 12) {
            hour -= 12;
        }
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

    if (callback && mListener != null) {
        mListener.onValueSelected(MINUTES, minute, false);
    }
}

// Returns minutes in 0-59 range
int RadialTimePickerView::getCurrentMinute() const{
    return getMinuteForDegrees(mSelectionDegrees[MINUTES]);
}

int RadialTimePickerView::getMinuteForDegrees(int degrees) const{
    return degrees / DEGREES_FOR_ONE_MINUTE;
}

int RadialTimePickerView::getDegreesForMinute(int minute) const{
    return minute * DEGREES_FOR_ONE_MINUTE;
}

bool RadialTimePickerView::setAmOrPm(int amOrPm) {
    if (mAmOrPm == amOrPm || mIs24HourMode) {
        return false;
    }

    mAmOrPm = amOrPm;
    invalidate();
    mTouchHelper.invalidateRoot();
    return true;
}

int RadialTimePickerView::getAmOrPm() const{
    return mAmOrPm;
}

void RadialTimePickerView::showHours(bool animate) {
    showPicker(true, animate);
}

void RadialTimePickerView::showMinutes(bool animate) {
    showPicker(false, animate);
}

void RadialTimePickerView::initHoursAndMinutesText() {
    // Initialize the hours and minutes numbers.
    for (int i = 0; i < 12; i++) {
        mHours12Texts[i] = String.format("%d", HOURS_NUMBERS[i]);
        mInnerHours24Texts[i] = String.format("%02d", HOURS_NUMBERS_24[i]);
        mOuterHours24Texts[i] = String.format("%d", HOURS_NUMBERS[i]);
        mMinutesTexts[i] = String.format("%02d", MINUTES_NUMBERS[i]);
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
    if (!changed) {
        return;
    }

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
    const float alphaMod = mInputEnabled ? 1 : mDisabledAlpha;

    drawCircleBackground(canvas);

    Path selectorPath = mSelectorPath;
    drawSelector(canvas, selectorPath);
    drawHours(canvas, selectorPath, alphaMod);
    drawMinutes(canvas, selectorPath, alphaMod);
    drawCenter(canvas, alphaMod);
}

void RadialTimePickerView::showPicker(bool hours, bool animate) {
    if (mShowHours == hours) {
        return;
    }

    mShowHours = hours;

    if (animate) {
        animatePicker(hours, ANIM_DURATION_NORMAL);
    } else {
        // If we have a pending or running animator, cancel it.
        if (mHoursToMinutesAnimator != null && mHoursToMinutesAnimator.isStarted()) {
            mHoursToMinutesAnimator.cancel();
            mHoursToMinutesAnimator = null;
        }
        mHoursToMinutes = hours ? 0.0f : 1.0f;
    }

    initData();
    invalidate();
    mTouchHelper.invalidateRoot();
}

void RadialTimePickerView::animatePicker(bool hoursToMinutes, long duration) {
    const float target = hoursToMinutes ? HOURS : MINUTES;
    if (mHoursToMinutes == target) {
        // If we have a pending or running animator, cancel it.
        if (mHoursToMinutesAnimator != null && mHoursToMinutesAnimator.isStarted()) {
            mHoursToMinutesAnimator.cancel();
            mHoursToMinutesAnimator = null;
        }

        // We're already showing the correct picker.
        return;
    }

    mHoursToMinutesAnimator = ObjectAnimator.ofFloat(this, HOURS_TO_MINUTES, target);
    mHoursToMinutesAnimator.setAutoCancel(true);
    mHoursToMinutesAnimator.setDuration(duration);
    mHoursToMinutesAnimator.start();
}

void RadialTimePickerView::drawCircleBackground(Canvas& canvas) {
    canvas.drawCircle(mXCenter, mYCenter, mCircleRadius, mPaintBackground);
}

void RadialTimePickerView::drawHours(Canvas& canvas, Path selectorPath, float alphaMod) {
    const int hoursAlpha = (int) (255f * (1f - mHoursToMinutes) * alphaMod + 0.5f);
    if (hoursAlpha > 0) {
        // Exclude the selector region, then draw inner/outer hours with no
        // activated states.
        canvas.save(Canvas.CLIP_SAVE_FLAG);
        canvas.clipPath(selectorPath, Region.Op.DIFFERENCE);
        drawHoursClipped(canvas, hoursAlpha, false);
        canvas.restore();

        // Intersect the selector region, then draw minutes with only
        // activated states.
        canvas.save(Canvas.CLIP_SAVE_FLAG);
        canvas.clipPath(selectorPath, Region.Op.INTERSECT);
        drawHoursClipped(canvas, hoursAlpha, true);
        canvas.restore();
    }
}

void RadialTimePickerView::drawHoursClipped(Canvas& canvas, int hoursAlpha, bool showActivated) {
    // Draw outer hours.
    drawTextElements(canvas, mTextSize[HOURS], mTypeface, mTextColor[HOURS], mOuterTextHours,
            mOuterTextX[HOURS], mOuterTextY[HOURS], mPaint[HOURS], hoursAlpha,
            showActivated && !mIsOnInnerCircle, mSelectionDegrees[HOURS], showActivated);

    // Draw inner hours (13-00) for 24-hour time.
    if (mIs24HourMode && mInnerTextHours != null) {
        drawTextElements(canvas, mTextSize[HOURS_INNER], mTypeface, mTextColor[HOURS_INNER],
                mInnerTextHours, mInnerTextX, mInnerTextY, mPaint[HOURS], hoursAlpha,
                showActivated && mIsOnInnerCircle, mSelectionDegrees[HOURS], showActivated);
    }
}

void RadialTimePickerView::drawMinutes(Canvas& canvas, Path selectorPath, float alphaMod) {
    const int minutesAlpha = (int) (255f * mHoursToMinutes * alphaMod + 0.5f);
    if (minutesAlpha > 0) {
        // Exclude the selector region, then draw minutes with no
        // activated states.
        canvas.save(Canvas.CLIP_SAVE_FLAG);
        canvas.clipPath(selectorPath, Region.Op.DIFFERENCE);
        drawMinutesClipped(canvas, minutesAlpha, false);
        canvas.restore();

        // Intersect the selector region, then draw minutes with only
        // activated states.
        canvas.save(Canvas.CLIP_SAVE_FLAG);
        canvas.clipPath(selectorPath, Region.Op.INTERSECT);
        drawMinutesClipped(canvas, minutesAlpha, true);
        canvas.restore();
    }
}

void RadialTimePickerView::drawMinutesClipped(Canvas& canvas, int minutesAlpha, bool showActivated) {
    drawTextElements(canvas, mTextSize[MINUTES], mTypeface, mTextColor[MINUTES], mMinutesText,
            mOuterTextX[MINUTES], mOuterTextY[MINUTES], mPaint[MINUTES], minutesAlpha,
            showActivated, mSelectionDegrees[MINUTES], showActivated);
}

void RadialTimePickerView::drawCenter(Canvas& canvas, float alphaMod) {
    mPaintCenter.setAlpha((int) (255 * alphaMod + 0.5f));
    canvas.drawCircle(mXCenter, mYCenter, mCenterDotRadius, mPaintCenter);
}

private int getMultipliedAlpha(int argb, int alpha) {
    return (int) (Color.alpha(argb) * (alpha / 255.0) + 0.5);
}

void RadialTimePickerView::drawSelector(Canvas& canvas, Path selectorPath) {
    // Determine the current length, angle, and dot scaling factor.
    const int hoursIndex = mIsOnInnerCircle ? HOURS_INNER : HOURS;
    const int hoursInset = mTextInset[hoursIndex];
    const int hoursAngleDeg = mSelectionDegrees[hoursIndex % 2];
    const float hoursDotScale = mSelectionDegrees[hoursIndex % 2] % 30 != 0 ? 1 : 0;

    const int minutesIndex = MINUTES;
    const int minutesInset = mTextInset[minutesIndex];
    const int minutesAngleDeg = mSelectionDegrees[minutesIndex];
    const float minutesDotScale = mSelectionDegrees[minutesIndex] % 30 != 0 ? 1 : 0;

    // Calculate the current radius at which to place the selection circle.
    const int selRadius = mSelectorRadius;
    const float selLength = mCircleRadius - MathUtils::lerp(hoursInset, minutesInset, mHoursToMinutes);
    const double selAngleRad = Math.toRadians(MathUtils::lerpDeg(hoursAngleDeg, minutesAngleDeg, mHoursToMinutes));
    const float selCenterX = mXCenter + selLength * (float) std::sin(selAngleRad);
    coinst float selCenterY = mYCenter - selLength * (float) std::cos(selAngleRad);

    // Draw the selection circle.
    final Paint paint = mPaintSelector[SELECTOR_CIRCLE];
    paint.setColor(mSelectorColor);
    canvas.drawCircle(selCenterX, selCenterY, selRadius, paint);

    // If needed, set up the clip path for later.
    if (selectorPath != null) {
        selectorPath.reset();
        selectorPath.addCircle(selCenterX, selCenterY, selRadius, Path.Direction.CCW);
    }

    // Draw the dot if we're between two items.
    const float dotScale = MathUtils::lerp(hoursDotScale, minutesDotScale, mHoursToMinutes);
    if (dotScale > 0) {
        final Paint dotPaint = mPaintSelector[SELECTOR_DOT];
        dotPaint.setColor(mSelectorDotColor);
        canvas.drawCircle(selCenterX, selCenterY, mSelectorDotRadius * dotScale, dotPaint);
    }

    // Shorten the line to only go from the edge of the center dot to the
    // edge of the selection circle.
    const double sin = std::sin(selAngleRad);
    const double cos = std::cos(selAngleRad);
    const float lineLength = selLength - selRadius;
    const int centerX = mXCenter + (int) (mCenterDotRadius * sin);
    const int centerY = mYCenter - (int) (mCenterDotRadius * cos);
    const float linePointX = centerX + (int) (lineLength * sin);
    const float linePointY = centerY - (int) (lineLength * cos);

    // Draw the line.
    final Paint linePaint = mPaintSelector[SELECTOR_LINE];
    linePaint.setColor(mSelectorColor);
    linePaint.setStrokeWidth(mSelectorStroke);
    canvas.drawLine(mXCenter, mYCenter, linePointX, linePointY, linePaint);
}

void RadialTimePickerView::calculatePositionsHours() {
    // Calculate the text positions
    const float numbersRadius = mCircleRadius - mTextInset[HOURS];

    // Calculate the positions for the 12 numbers in the main circle.
    calculatePositions(mPaint[HOURS], numbersRadius, mXCenter, mYCenter,
            mTextSize[HOURS], mOuterTextX[HOURS], mOuterTextY[HOURS]);

    // If we have an inner circle, calculate those positions too.
    if (mIs24HourMode) {
        const int innerNumbersRadius = mCircleRadius - mTextInset[HOURS_INNER];
        calculatePositions(mPaint[HOURS], innerNumbersRadius, mXCenter, mYCenter,
                mTextSize[HOURS_INNER], mInnerTextX, mInnerTextY);
    }
}

void RadialTimePickerView::calculatePositionsMinutes() {
    // Calculate the text positions
    const float numbersRadius = mCircleRadius - mTextInset[MINUTES];

    // Calculate the positions for the 12 numbers in the main circle.
    calculatePositions(mPaint[MINUTES], numbersRadius, mXCenter, mYCenter,
            mTextSize[MINUTES], mOuterTextX[MINUTES], mOuterTextY[MINUTES]);
}

void RadialTimePickerView::calculatePositions(Paint paint, float radius, float xCenter, float yCenter,
        float textSize, float[] x, float[] y) {
    // Adjust yCenter to account for the text's baseline.
    paint.setTextSize(textSize);
    yCenter -= (paint.descent() + paint.ascent()) / 2;

    for (int i = 0; i < NUM_POSITIONS; i++) {
        x[i] = xCenter - radius * COS_30[i];
        y[i] = yCenter - radius * SIN_30[i];
    }
}

void RadialTimePickerView::drawTextElements(Canvas& canvas, float textSize, Typeface typeface,
        ColorStateList textColor, String[] texts, float[] textX, float[] textY, Paint paint,
        int alpha, bool showActivated, int activatedDegrees, bool activatedOnly) {
    paint.setTextSize(textSize);
    paint.setTypeface(typeface);

    // The activated index can touch a range of elements.
    const float activatedIndex = activatedDegrees / (360.0f / NUM_POSITIONS);
    const int activatedFloor = (int) activatedIndex;
    const int activatedCeil = ((int) std::ceil(activatedIndex)) % NUM_POSITIONS;

    for (int i = 0; i < 12; i++) {
        const bool activated = (activatedFloor == i || activatedCeil == i);
        if (activatedOnly && !activated) {
            continue;
        }

        const int stateMask = StateSet::VIEW_STATE_ENABLED
                | (showActivated && activated ? StateSet::VIEW_STATE_ACTIVATED : 0);
        const int color = textColor.getColorForState(StateSet::get(stateMask), 0);
        paint.setColor(color);
        paint.setAlpha(getMultipliedAlpha(color, alpha));

        canvas.drawText(texts[i], textX[i], textY[i], paint);
    }
}

int RadialTimePickerView::getDegreesFromXY(float x, float y, bool constrainOutside) {
    // Ensure the point is inside the touchable area.
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
    const double distFromCenter = Math.sqrt(dX * dX + dY * dY);
    if (distFromCenter < innerBound || constrainOutside && distFromCenter > outerBound) {
        return -1;
    }

    // Convert to degrees.
    const int degrees = (int) (std::toDegrees(std::atan2(dY, dX) + M_PI / 2) + 0.5);
    if (degrees < 0) {
        return degrees + 360;
    } else {
        return degrees;
    }
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
            // This is a new event stream, reset whether the value changed.
            mChangedDuringTouch = false;
        } else if (action == MotionEvent::ACTION_UP) {
            autoAdvance = true;

            // If we saw a down/up pair without the value changing, assume
            // this is a single-tap selection and force a change.
            if (!mChangedDuringTouch) {
                forceSelection = true;
            }
        }

        mChangedDuringTouch |= handleTouchInput( event.getX(), event.getY(), forceSelection, autoAdvance);
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
        if (mListener != null) {
            mListener.onValueSelected(type, newValue, autoAdvance);
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
    // First right-of-refusal goes the touch exploration helper.
    if (mTouchHelper.dispatchHoverEvent(event)) {
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
    if (event.isFromSource(InputDevice.SOURCE_MOUSE)) {
        const int degrees = getDegreesFromXY(event.getX(), event.getY(), false);
        if (degrees != -1) {
            const int pointerIcon = enableArrowIconOnHoverWhenClickable()
                    ? PointerIcon::TYPE_ARROW : PointerIcon::TYPE_HAND;
            return PointerIcon::getSystemIcon(getContext(), pointerIcon);
        }
    }
    return View::onResolvePointerIcon(event, pointerIndex);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

RadialTimePickerView::RadialPickerTouchHelper::RadialPickerTouchHelper() {
    super(RadialTimePickerView.this);
}

void RadialTimePickerView::RadialPickerTouchHelper::onInitializeAccessibilityNodeInfo(View host, AccessibilityNodeInfo info) {
    super.onInitializeAccessibilityNodeInfo(host, info);

    info.addAction(AccessibilityNodeInfo.AccessibilityAction.ACTION_SCROLL_FORWARD);
    info.addAction(AccessibilityNodeInfo.AccessibilityAction.ACTION_SCROLL_BACKWARD);
}

bool RadialTimePickerView::RadialPickerTouchHelper::performAccessibilityAction(View host, int action, Bundle arguments) {
    if (super.performAccessibilityAction(host, action, arguments)) {
        return true;
    }

    switch (action) {
        case AccessibilityNodeInfo.ACTION_SCROLL_FORWARD:
            adjustPicker(1);
            return true;
        case AccessibilityNodeInfo.ACTION_SCROLL_BACKWARD:
            adjustPicker(-1);
            return true;
    }

    return false;
}

void RadialTimePickerView::RadialPickerTouchHelper::adjustPicker(int step) {
    int stepSize,initialStep;
    int maxValue,minValue;
    if (mShowHours) {
        stepSize = 1;

        const int currentHour24 = getCurrentHour();
        if (mIs24HourMode) {
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
        initialStep = getCurrentMinute() / stepSize;
        minValue = 0;
        maxValue = 55;
    }

    const int nextValue = (initialStep + step) * stepSize;
    const int clampedValue = MathUtils.constrain(nextValue, minValue, maxValue);
    if (mShowHours) {
        setCurrentHour(clampedValue);
    } else {
        setCurrentMinute(clampedValue);
    }
}

int RadialTimePickerView::RadialPickerTouchHelper::getVirtualViewAt(float x, float y) {
    int id;
    const int degrees = getDegreesFromXY(x, y, true);
    if (degrees != -1) {
        const int snapDegrees = snapOnly30s(degrees, 0) % 360;
        if (mShowHours) {
            const bool isOnInnerCircle = getInnerCircleFromXY(x, y);
            const int hour24 = getHourForDegrees(snapDegrees, isOnInnerCircle);
            const int hour = mIs24HourMode ? hour24 : hour24To12(hour24);
            id = makeId(TYPE_HOUR, hour);
        } else {
            const int current = getCurrentMinute();
            const int touched = getMinuteForDegrees(degrees);
            const int snapped = getMinuteForDegrees(snapDegrees);

            // If the touched minute is closer to the current minute
            // than it is to the snapped minute, return current.
            const int currentOffset = getCircularDiff(current, touched, MINUTES_IN_CIRCLE);
            const int snappedOffset = getCircularDiff(snapped, touched, MINUTES_IN_CIRCLE);
            int minute;
            if (currentOffset < snappedOffset) {
                minute = current;
            } else {
                minute = snapped;
            }
            id = makeId(TYPE_MINUTE, minute);
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

void RadialTimePickerView::RadialPickerTouchHelper::getVisibleVirtualViews(IntArray virtualViewIds) {
    if (mShowHours) {
        const int min = mIs24HourMode ? 0 : 1;
        const int max = mIs24HourMode ? 23 : 12;
        for (int i = min; i <= max ; i++) {
            virtualViewIds.add(makeId(TYPE_HOUR, i));
        }
    } else {
        const int current = getCurrentMinute();
        for (int i = 0; i < MINUTES_IN_CIRCLE; i += MINUTE_INCREMENT) {
            virtualViewIds.add(makeId(TYPE_MINUTE, i));

            // If the current minute falls between two increments,
            // insert an extra node for it.
            if (current > i && current < i + MINUTE_INCREMENT) {
                virtualViewIds.add(makeId(TYPE_MINUTE, current));
            }
        }
    }
}

void RadialTimePickerView::RadialPickerTouchHelper::onPopulateEventForVirtualView(int virtualViewId, AccessibilityEvent& event) {
    event.setClassName(getClass().getName());

    const int type = getTypeFromId(virtualViewId);
    const int value = getValueFromId(virtualViewId);
    CharSequence description = getVirtualViewDescription(type, value);
    event.setContentDescription(description);
}

void RadialTimePickerView::RadialPickerTouchHelper::onPopulateNodeForVirtualView(int virtualViewId, AccessibilityNodeInfo& node) {
    node.setClassName(getClass().getName());
    node.addAction(AccessibilityAction.ACTION_CLICK);

    const int type = getTypeFromId(virtualViewId);
    const int value = getValueFromId(virtualViewId);
    const CharSequence description = getVirtualViewDescription(type, value);
    node.setContentDescription(description);

    getBoundsForVirtualView(virtualViewId, mTempRect);
    node.setBoundsInParent(mTempRect);

    const bool selected = isVirtualViewSelected(type, value);
    node.setSelected(selected);

    const int nextId = getVirtualViewIdAfter(type, value);
    if (nextId != INVALID_ID) {
        node.setTraversalBefore(RadialTimePickerView.this, nextId);
    }
}

int RadialTimePickerView::RadialPickerTouchHelper::getVirtualViewIdAfter(int type, int value) {
    if (type == TYPE_HOUR) {
        const int nextValue = value + 1;
        const int max = mIs24HourMode ? 23 : 12;
        if (nextValue <= max) {
            return makeId(type, nextValue);
        }
    } else if (type == TYPE_MINUTE) {
        const int current = getCurrentMinute();
        const int snapValue = value - (value % MINUTE_INCREMENT);
        const int nextValue = snapValue + MINUTE_INCREMENT;
        if (value < current && nextValue > current) {
            // The current value is between two snap values.
            return makeId(type, current);
        } else if (nextValue < MINUTES_IN_CIRCLE) {
            return makeId(type, nextValue);
        }
    }
    return INVALID_ID;
}

bool RadialTimePickerView::RadialPickerTouchHelper::onPerformActionForVirtualView(int virtualViewId, int action,Bundle arguments) {
    if (action == AccessibilityNodeInfo.ACTION_CLICK) {
        const int type = getTypeFromId(virtualViewId);
        const int value = getValueFromId(virtualViewId);
        if (type == TYPE_HOUR) {
            const int hour = mIs24HourMode ? value : hour12To24(value, mAmOrPm);
            setCurrentHour(hour);
            return true;
        } else if (type == TYPE_MINUTE) {
            setCurrentMinute(value);
            return true;
        }
    }
    return false;
}

int RadialTimePickerView::RadialPickerTouchHelper::hour12To24(int hour12, int amOrPm) {
    int hour24 = hour12;
    if (hour12 == 12) {
        if (amOrPm == AM) {
            hour24 = 0;
        }
    } else if (amOrPm == PM) {
        hour24 += 12;
    }
    return hour24;
}

int RadialTimePickerView::RadialPickerTouchHelper::hour24To12(int hour24) {
    if (hour24 == 0) {
        return 12;
    } else if (hour24 > 12) {
        return hour24 - 12;
    } else {
        return hour24;
    }
}

void RadialTimePickerView::RadialPickerTouchHelper::getBoundsForVirtualView(int virtualViewId, Rect& bounds) {
    float radius;
    const int type = getTypeFromId(virtualViewId);
    const int value = getValueFromId(virtualViewId);
    float centerRadius;
    float degrees;
    if (type == TYPE_HOUR) {
        const bool innerCircle = getInnerCircleForHour(value);
        if (innerCircle) {
            centerRadius = mCircleRadius - mTextInset[HOURS_INNER];
            radius = mSelectorRadius;
        } else {
            centerRadius = mCircleRadius - mTextInset[HOURS];
            radius = mSelectorRadius;
        }

        degrees = getDegreesForHour(value);
    } else if (type == TYPE_MINUTE) {
        centerRadius = mCircleRadius - mTextInset[MINUTES];
        degrees = getDegreesForMinute(value);
        radius = mSelectorRadius;
    } else {
        // This should never happen.
        centerRadius = 0;
        degrees = 0;
        radius = 0;
    }

    const double radians = Math.toRadians(degrees);
    const float xCenter = mXCenter + centerRadius * (float) Math.sin(radians);
    const float yCenter = mYCenter - centerRadius * (float) Math.cos(radians);

    bounds.set((int) (xCenter - radius), (int) (yCenter - radius),
            (int) (xCenter + radius), (int) (yCenter + radius));
}

std::string RadialTimePickerView::RadialPickerTouchHelper::getVirtualViewDescription(int type, int value) {
    CharSequence description;
    if (type == TYPE_HOUR || type == TYPE_MINUTE) {
        description = Integer.toString(value);
    } else {
        description = null;
    }
    return description;
}

bool RadialTimePickerView::RadialPickerTouchHelper::isVirtualViewSelected(int type, int value) {
    bool selected;
    if (type == TYPE_HOUR) {
        selected = getCurrentHour() == value;
    } else if (type == TYPE_MINUTE) {
        selected = getCurrentMinute() == value;
    } else {
        selected = false;
    }
    return selected;
}

int RadialTimePickerView::RadialPickerTouchHelper::makeId(int type, int value) const{
    return type << SHIFT_TYPE | value << SHIFT_VALUE;
}

int RadialTimePickerView::RadialPickerTouchHelper::getTypeFromId(int id) const{
    return id >>> SHIFT_TYPE & MASK_TYPE;
}

int RadialTimePickerView::RadialPickerTouchHelper::getValueFromId(int id) const{
    return id >>> SHIFT_VALUE & MASK_VALUE;
}
}/*endof namespace*/

