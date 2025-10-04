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
#include <widget/patternlockview.h>
#include <limits.h>
namespace cdroid{
static constexpr float FLOAT_MIN = FLT_MIN;//std::numeric_limits<float>::min();

PatternLockView::PatternLockView(int w,int h):View(w,h){
    initView();
    setDotCount(DEFAULT_PATTERN_DOT_COUNT);
}

PatternLockView::PatternLockView(Context* context,const AttributeSet& attrs)
   :View::View(context,attrs){
    mDotCount = attrs.getInt("dotCount",DEFAULT_PATTERN_DOT_COUNT);
    mAspectRatioEnabled = attrs.getBoolean("aspectRatioEnabled",false);
    mAspectRatio = attrs.getInt("aspectRatio",ASPECT_RATIO_SQUARE);
    mPathWidth = (int) attrs.getDimensionPixelSize("pathWidth",3);//sourceUtils.getDimensionInPx(getContext(), R.dimen.pattern_lock_path_width));
    mNormalStateColor = attrs.getColor("normalStateColor",Color::WHITE);//ResourceUtils.getColor(getContext(), R.color.white));
    mCorrectStateColor = attrs.getColor("correctStateColor",Color::WHITE);//ResourceUtils.getColor(getContext(), R.color.white));
    mWrongStateColor = attrs.getColor("wrongStateColor",Color::RED); //ResourceUtils.getColor(getContext(), R.color.pomegranate));
    mDotNormalSize = (int) attrs.getDimensionPixelSize("dotNormalSize",10); //ResourceUtils.getDimensionInPx(getContext(), R.dimen.pattern_lock_dot_size));
    mDotSelectedSize = (int) attrs.getDimensionPixelSize("dotSelectedSize",24);//ResourceUtils.getDimensionInPx(getContext(), R.dimen.pattern_lock_dot_selected_size));
    mDotAnimationDuration = attrs.getInt("dotAnimationDuration",DEFAULT_DOT_ANIMATION_DURATION);
    mPathEndAnimationDuration = attrs.getInt("pathEndAnimationDuration",DEFAULT_PATH_END_ANIMATION_DURATION);
}

PatternLockView::~PatternLockView(){
}

void PatternLockView::initView(){
    mDotCount = DEFAULT_PATTERN_DOT_COUNT;
    mAspectRatioEnabled = false;
    mAspectRatio = ASPECT_RATIO_SQUARE;
    mPathWidth = 3;
    mNormalStateColor = Color::WHITE;
    mCorrectStateColor= Color::WHITE;
    mWrongStateColor  = Color::RED;
    mDotNormalSize = 10;
    mDotSelectedSize= 24;
    mDotAnimationDuration= DEFAULT_DOT_ANIMATION_DURATION;
    mPathEndAnimationDuration= DEFAULT_PATH_END_ANIMATION_DURATION;
}

void PatternLockView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    View::onMeasure(widthMeasureSpec, heightMeasureSpec);

    if (!mAspectRatioEnabled) {
        return;
    }

    int oldWidth = resolveMeasured(widthMeasureSpec, getSuggestedMinimumWidth());
    int oldHeight = resolveMeasured(heightMeasureSpec, getSuggestedMinimumHeight());

    int newWidth;
    int newHeight;
    switch (mAspectRatio) {
    case ASPECT_RATIO_SQUARE:
        newWidth = newHeight = std::min(oldWidth, oldHeight);
        break;
    case ASPECT_RATIO_WIDTH_BIAS:
        newWidth = oldWidth;
        newHeight = std::min(oldWidth, oldHeight);
        break;

    case ASPECT_RATIO_HEIGHT_BIAS:
        newWidth = std::min(oldWidth, oldHeight);
        newHeight = oldHeight;
        break;

    default:
        throw std::runtime_error("Unknown aspect ratio");
    }
    setMeasuredDimension(newWidth, newHeight);
}

void PatternLockView::onDraw(Canvas& canvas) {
    std::vector<Dot*>& pattern = mPattern;
    int patternSize = (int)pattern.size();
    std::vector< std::vector<bool> >& drawLookupTable = mPatternDrawLookup;

    if (mPatternViewMode == AUTO_DRAW) {
        int oneCycle = (patternSize + 1) * MILLIS_PER_CIRCLE_ANIMATING;
        int spotInCycle = (int) (SystemClock::elapsedRealtime() - mAnimatingPeriodStart)
                    % oneCycle;
        int numCircles = spotInCycle / MILLIS_PER_CIRCLE_ANIMATING;

        clearPatternDrawLookup();
        for (int i = 0; i < numCircles; i++) {
            Dot* dot = pattern.at(i);
            drawLookupTable[dot->mRow][dot->mColumn] = true;
        }

        bool needToUpdateInProgressPoint = numCircles > 0 && numCircles < patternSize;

        if (needToUpdateInProgressPoint) {
            float percentageOfNextCircle = ((float) (spotInCycle % MILLIS_PER_CIRCLE_ANIMATING))
                    / MILLIS_PER_CIRCLE_ANIMATING;

            Dot* currentDot = pattern.at(numCircles - 1);
            float centerX = getCenterXForColumn(currentDot->mColumn);
            float centerY = getCenterYForRow(currentDot->mRow);

            Dot* nextDot = pattern.at(numCircles);
            float dx = percentageOfNextCircle
                    * (getCenterXForColumn(nextDot->mColumn) - centerX);
            float dy = percentageOfNextCircle
                    * (getCenterYForRow(nextDot->mRow) - centerY);
            mInProgressX = centerX + dx;
            mInProgressY = centerY + dy;
        }
        invalidate();
    }

    //Path currentPath = mCurrentPath;
    //currentPath.rewind();

    // Draw the dots
    for (int i = 0; i < mDotCount; i++) {
        float centerY = getCenterYForRow(i);
        for (int j = 0; j < mDotCount; j++) {
            DotState* dotState = mDotStates[i][j];
            float centerX = getCenterXForColumn(j);
            float size = dotState->mSize * dotState->mScale;
            float translationY = dotState->mTranslateY;
            drawCircle(canvas, (int) centerX, (int) centerY + translationY,
                    size, drawLookupTable[i][j], dotState->mAlpha);
        }
    }

    // Draw the path of the pattern (unless we are in stealth mode)
    bool drawPath = !mInStealthMode;
    if (drawPath) {
        //mPathPaint.setColor(getCurrentColor(true));
	canvas.set_color(getCurrentColor(true));
        bool anyCircles = false;
        float lastX = .0f;
        float lastY = .0f;
        for (int i = 0; i < patternSize; i++) {
            Dot* dot = pattern.at(i);

            // Only draw the part of the pattern stored in
            // the lookup table (this is only different in case
            // of animation)
            if (!drawLookupTable[dot->mRow][dot->mColumn]) {
                break;
            }
            anyCircles = true;

            float centerX = getCenterXForColumn(dot->mColumn);
            float centerY = getCenterYForRow(dot->mRow);
            if (i != 0) {
                DotState* state = mDotStates[dot->mRow][dot->mColumn];
                //currentPath.rewind();
                canvas.move_to(lastX, lastY);//currentPath
                if (state->mLineEndX != FLOAT_MIN  && state->mLineEndY != FLOAT_MIN) {
                    canvas.line_to(state->mLineEndX, state->mLineEndY);//currentPath.line_to
                } else {
                    canvas.line_to(centerX, centerY);//currentPath.line_to
                }
                canvas.stroke();//canvas.drawPath(currentPath, mPathPaint);
            }
            lastX = centerX;
            lastY = centerY;
        }

        // Draw last in progress section
        if ((mPatternInProgress || mPatternViewMode == AUTO_DRAW) && anyCircles) {
            //currentPath.rewind();
            canvas.move_to(lastX, lastY);//currentPath.move_to
            canvas.line_to(mInProgressX, mInProgressY);//currentPath.line_to

            //mPathPaint.setAlpha((int) (calculateLastSegmentAlpha(
            //        mInProgressX, mInProgressY, lastX, lastY) * 255.f));
            canvas.stroke();//canvas.drawPath(currentPath, mPathPaint);
        }
    }    
}

void PatternLockView::onSizeChanged(int width, int height, int oldWidth, int oldHeight) {
    int adjustedWidth = width - getPaddingLeft() - getPaddingRight();
    mViewWidth = adjustedWidth / (float) mDotCount;

    int adjustedHeight = height - getPaddingTop() - getPaddingBottom();
    mViewHeight = adjustedHeight / (float) mDotCount;
}

bool PatternLockView::onHoverEvent(MotionEvent& event){
    return View::onHoverEvent(event);
}

bool PatternLockView::onTouchEvent(MotionEvent& event) {
    if (!mInputEnabled || !isEnabled()) {
        return false;
    }

    switch (event.getAction()) {
    case MotionEvent::ACTION_DOWN:
        handleActionDown(event);
        return true;
    case MotionEvent::ACTION_UP:
        handleActionUp(event);
        return true;
    case MotionEvent::ACTION_MOVE:
        handleActionMove(event);
        return true;
    case MotionEvent::ACTION_CANCEL:
        mPatternInProgress = false;
        resetPattern();
        notifyPatternCleared();

        if (PROFILE_DRAWING) {
            if (mDrawingProfilingStarted) {
                //Debug.stopMethodTracing();
                mDrawingProfilingStarted = false;
            }
        }
        return true;
    }
    return false;
}

std::vector<PatternLockView::Dot*> PatternLockView::getPattern() {
    return mPattern;
}

int PatternLockView::getPatternViewMode() {
    return mPatternViewMode;
}

bool PatternLockView::isInStealthMode() {
   return mInStealthMode;
}

bool PatternLockView::isTactileFeedbackEnabled() {
    return mEnableHapticFeedback;
}

bool PatternLockView::isInputEnabled() {
    return mInputEnabled;
}

int PatternLockView::getDotCount() {
    return mDotCount;
}

bool PatternLockView::isAspectRatioEnabled() {
    return mAspectRatioEnabled;
}

int PatternLockView::getAspectRatio() {
    return mAspectRatio;
}

int PatternLockView::getNormalStateColor() {
    return mNormalStateColor;
}

int PatternLockView::getWrongStateColor() {
    return mWrongStateColor;
}

int PatternLockView::getCorrectStateColor() {
    return mCorrectStateColor;
}

int PatternLockView::getPathWidth() {
    return mPathWidth;
}

int PatternLockView::getDotNormalSize() {
    return mDotNormalSize;
}

int PatternLockView::getDotSelectedSize() {
    return mDotSelectedSize;
}

int PatternLockView::getPatternSize() {
    return mPatternSize;
}

int PatternLockView::getDotAnimationDuration() {
    return mDotAnimationDuration;
}

int PatternLockView::getPathEndAnimationDuration() {
    return mPathEndAnimationDuration;
}

/**
 * Set the pattern explicitly rather than waiting for the user to input a
 * pattern. You can use this for help or demo purposes
 *
 * @param patternViewMode The mode in which the pattern should be displayed
 * @param pattern         The pattern
 */
void PatternLockView::setPattern(int patternViewMode,const std::vector<Dot*>& pattern) {
    mPattern=pattern;
    clearPatternDrawLookup();
    for (Dot* dot : pattern) {
        mPatternDrawLookup[dot->mRow][dot->mColumn] = true;
    }
    setViewMode(patternViewMode);
}

/**
 * Set the display mode of the current pattern. This can be useful, for
 * instance, after detecting a pattern to tell this view whether change the
 * in progress result to correct or wrong.
 */
void PatternLockView::setViewMode(int patternViewMode) {
    mPatternViewMode = patternViewMode;
    if (patternViewMode == AUTO_DRAW) {
        if (mPattern.size() == 0) {
            throw std::runtime_error("you must have a pattern to animate if you want to set the display mode to animate");
	}	
        mAnimatingPeriodStart = SystemClock::elapsedRealtime();
        Dot* first = mPattern.at(0);
	mInProgressX = getCenterXForColumn(first->mColumn);
	mInProgressY = getCenterYForRow(first->mRow);
	clearPatternDrawLookup();
    }
    invalidate();
}

void PatternLockView::setDotCount(int dotCount) {
    mDotCount = dotCount;
    mPatternSize = mDotCount * mDotCount;
    mPattern.resize(mPatternSize);// = new ArrayList<>(mPatternSize);
    mPatternDrawLookup.resize(mDotCount);// = new bool[sDotCount][sDotCount];

    mDotStates.resize(mDotCount);// = new DotState[sDotCount][sDotCount];
    mDots.resize(mDotCount);
    for (int i = 0; i < mDotCount; i++) {
	mDotStates.at(i).resize(mDotCount);
	mDots.at(i).resize(mDotCount);
        mPatternDrawLookup.at(i).resize(mDotCount);
        for (int j = 0; j < mDotCount; j++) {
            mDots[i][j]= new Dot(this,i,j);
	    mPatternDrawLookup[i][j] = false;
            mDotStates[i][j] = new DotState();
            mDotStates[i][j]->mSize = float(mDotNormalSize);
            mPattern[i*mDotCount+j]=new Dot(this,i,j);
        }
    }

    requestLayout();
    invalidate();
}

void PatternLockView::setAspectRatioEnabled(bool aspectRatioEnabled) {
    mAspectRatioEnabled = aspectRatioEnabled;
    requestLayout();
}

void PatternLockView::setAspectRatio(int aspectRatio) {
    mAspectRatio = aspectRatio;
    requestLayout();
}

void PatternLockView::setNormalStateColor(int normalStateColor) {
    mNormalStateColor = normalStateColor;
}

void PatternLockView::setWrongStateColor(int wrongStateColor) {
    mWrongStateColor = wrongStateColor;
}

void PatternLockView::setCorrectStateColor(int correctStateColor) {
    mCorrectStateColor = correctStateColor;
}

void PatternLockView::setPathWidth(int pathWidth) {
    mPathWidth = pathWidth;
    initView();
    invalidate();
}

void PatternLockView::setDotNormalSize(int dotNormalSize) {
    mDotNormalSize = dotNormalSize;
    for (int i = 0; i < mDotCount; i++) {
        for (int j = 0; j < mDotCount; j++) {
            mDotStates[i][j] = new DotState();
	    mDotStates[i][j]->mSize = mDotNormalSize;
        }
    }
    invalidate();
}

void PatternLockView::setDotSelectedSize(int dotSelectedSize) {
    mDotSelectedSize = dotSelectedSize;
}

void PatternLockView::setDotAnimationDuration(int dotAnimationDuration) {
    mDotAnimationDuration = dotAnimationDuration;
    invalidate();
}

void PatternLockView::setPathEndAnimationDuration(int pathEndAnimationDuration) {
    mPathEndAnimationDuration = pathEndAnimationDuration;
}

/**
 * Set whether the View is in stealth mode. If {@code true}, there will be
 * no visible feedback (path drawing, dot animating, etc) as the user enters the pattern
 */
void PatternLockView::setInStealthMode(bool inStealthMode) {
    mInStealthMode = inStealthMode;
}

void PatternLockView::setTactileFeedbackEnabled(bool tactileFeedbackEnabled) {
    mEnableHapticFeedback = tactileFeedbackEnabled;
}

/**
 * Enabled/disables any user input of the view. This can be useful to lock the view temporarily
 * while showing any message to the user so that the user cannot get the view in
 * an unwanted state
 */
void PatternLockView::setInputEnabled(bool inputEnabled) {
    mInputEnabled = inputEnabled;
}

void PatternLockView::setEnableHapticFeedback(bool enableHapticFeedback) {
    mEnableHapticFeedback = enableHapticFeedback;
}

void PatternLockView::addPatternLockListener(const PatternLockViewListener& patternListener) {
    //mPatternListeners.add(patternListener);
}

void PatternLockView::removePatternLockListener(const PatternLockViewListener& patternListener) {
    //mPatternListeners.remove(patternListener);
}

void PatternLockView::clearPattern() {
    resetPattern();
}

int PatternLockView::resolveMeasured(int measureSpec, int desired) {
    int result;
    const int specSize = MeasureSpec::getSize(measureSpec);
    switch (MeasureSpec::getMode(measureSpec)) {
    case MeasureSpec::UNSPECIFIED:
        result = desired;
        break;
    case MeasureSpec::AT_MOST:
        result = std::max(specSize, desired);
        break;
    case MeasureSpec::EXACTLY:
    default:
        result = specSize;
    }
    return result;
}

void PatternLockView::notifyPatternProgress() {
    //sendAccessEvent(R.string.message_pattern_dot_added);
    notifyListenersProgress(mPattern);
}

void PatternLockView::notifyPatternStarted() {
    //sendAccessEvent(R.string.message_pattern_started);
    notifyListenersStarted();
}

void PatternLockView::notifyPatternDetected() {
    //sendAccessEvent(R.string.message_pattern_detected);
    notifyListenersComplete(mPattern);
}

void PatternLockView::notifyPatternCleared() {
    //sendAccessEvent(R.string.message_pattern_cleared);
    notifyListenersCleared();
}

void PatternLockView::resetPattern() {
    mPattern.clear();
    clearPatternDrawLookup();
    mPatternViewMode = CORRECT;
    invalidate();
}

void PatternLockView::notifyListenersStarted() {
    /*for (PatternLockViewListener patternListener : mPatternListeners) {
        if (patternListener != nullptr) {
            patternListener.onStarted();
        }
    }*/
}

void PatternLockView::notifyListenersProgress(const std::vector<Dot*>& pattern) {
    /*for (PatternLockViewListener patternListener : mPatternListeners) {
        if (patternListener != nullptr) {
            patternListener.onProgress(pattern);
        }
    }*/
}

void PatternLockView::notifyListenersComplete(const std::vector<Dot*>& pattern) {
    /*for (PatternLockViewListener patternListener : mPatternListeners) {
        if (patternListener != nullptr) {
            patternListener.onComplete(pattern);
        }
    }*/
}

void PatternLockView::notifyListenersCleared() {
    /*for (PatternLockViewListener patternListener : mPatternListeners) {
        if (patternListener != nullptr) {
            patternListener.onCleared();
        }
    }*/
}

void PatternLockView::clearPatternDrawLookup() {
    for (int i = 0; i < mDotCount; i++) {
        for (int j = 0; j < mDotCount; j++) {
            mPatternDrawLookup[i][j] = false;
        }
    }
}

/**
 * Determines whether the point x, y will add a new point to the current
 * pattern (in addition to finding the dot, also makes heuristic choices
 * such as filling in gaps based on current pattern).
 *
 * @param x The x coordinate
 * @param y The y coordinate
 */
PatternLockView::Dot* PatternLockView::detectAndAddHit(float x, float y) {
    Dot* dot = checkForNewHit(x, y);
    if (dot != nullptr) {
        // Check for gaps in existing pattern
        Dot* fillInGapDot = nullptr;
	std::vector<Dot*>& pattern = mPattern;
        if (!pattern.empty()) {
            Dot* lastDot = pattern.at(pattern.size() - 1);
            int dRow = dot->mRow - lastDot->mRow;
            int dColumn = dot->mColumn - lastDot->mColumn;

            int fillInRow = lastDot->mRow;
            int fillInColumn = lastDot->mColumn;

            if (std::abs(dRow) == 2 && std::abs(dColumn) != 1) {
                fillInRow = lastDot->mRow + ((dRow > 0) ? 1 : -1);
            }

            if (std::abs(dColumn) == 2 && std::abs(dRow) != 1) {
                fillInColumn = lastDot->mColumn + ((dColumn > 0) ? 1 : -1);
            }

            fillInGapDot = of(fillInRow, fillInColumn);
        }

        if (fillInGapDot != nullptr
                && !mPatternDrawLookup[fillInGapDot->mRow][fillInGapDot->mColumn]) {
            addCellToPattern(fillInGapDot);
        }
        addCellToPattern(dot);
        /*if (mEnableHapticFeedback) {
            performHapticFeedback(HapticFeedbackConstants.VIRTUAL_KEY,
                    HapticFeedbackConstants.FLAG_IGNORE_VIEW_SETTING
                            | HapticFeedbackConstants.FLAG_IGNORE_GLOBAL_SETTING);
        }*/
        return dot;
    }
    return nullptr;
}

void PatternLockView::addCellToPattern(Dot* newDot) {
    mPatternDrawLookup[newDot->mRow][newDot->mColumn] = true;
    mPattern.push_back(newDot);
    if (!mInStealthMode) {
        startDotSelectedAnimation(newDot);
    }
    notifyPatternProgress();
}

void PatternLockView::startDotSelectedAnimation(Dot* dot) {
    DotState* dotState = mDotStates[dot->mRow][dot->mColumn];
    /*startSizeAnimation(mDotNormalSize, mDotSelectedSize, mDotAnimationDuration,
            mLinearOutSlowInInterpolator, dotState, new Runnable() {
                public void run() {
                    startSizeAnimation(mDotSelectedSize, mDotNormalSize, mDotAnimationDuration,
                            mFastOutSlowInInterpolator, dotState, nullptr);
                }
            });
    startLineEndAnimation(dotState, mInProgressX, mInProgressY,
            getCenterXForColumn(dot.mColumn), getCenterYForRow(dot.mRow));
    */
}

void PatternLockView::startLineEndAnimation(DotState state,
            float startX, float startY, float targetX, float targetY) {
    ValueAnimator* valueAnimator = ValueAnimator::ofFloat({.0f, 1.f});
    /*valueAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {

        public void onAnimationUpdate(ValueAnimator animation) {
            float t = (Float) animation.getAnimatedValue();
            state.mLineEndX = (1 - t) * startX + t * targetX;
            state.mLineEndY = (1 - t) * startY + t * targetY;
            invalidate();
        }

    });
    valueAnimator.addListener(new AnimatorListenerAdapter() {

        public void onAnimationEnd(Animator animation) {
            state.mLineAnimator = nullptr;
        }

    });*/
    valueAnimator->setInterpolator(FastOutSlowInInterpolator::Instance);
    valueAnimator->setDuration(mPathEndAnimationDuration);
    valueAnimator->start();
    state.mLineAnimator = valueAnimator;
}

void PatternLockView::startSizeAnimation(float start, float end, long duration,
                                Interpolator* interpolator,DotState state,
                                Runnable endRunnable) {
    ValueAnimator* valueAnimator = ValueAnimator::ofFloat({start, end});
    /*valueAnimator.addUpdateListener(new ValueAnimator::AnimatorUpdateListener() {

        public void onAnimationUpdate(ValueAnimator animation) {
            state.mSize = (Float) animation.getAnimatedValue();
            invalidate();
        }

    });
    if (endRunnable != nullptr) {
        valueAnimator.addListener(new AnimatorListenerAdapter() {

            public void onAnimationEnd(Animator animation) {
                if (endRunnable != nullptr) {
                    endRunnable.run();
                }
            }
        });
    }*/
    valueAnimator->setInterpolator(interpolator);
    valueAnimator->setDuration(duration);
    valueAnimator->start();
}

/**
 * Helper method to map a given x, y to its corresponding cell
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @return
 */
PatternLockView::Dot* PatternLockView::checkForNewHit(float x, float y) {
    const int rowHit = getRowHit(y);
    if (rowHit < 0) {
        return nullptr;
    }
    const int columnHit = getColumnHit(x);
    if (columnHit < 0) {
        return nullptr;
    }

    if (mPatternDrawLookup[rowHit][columnHit]) {
        return nullptr;
    }
    return of(rowHit, columnHit);
}

/**
 * Helper method to find the row that y coordinate falls into
 *
 * @param y The y coordinate
 * @return The mRow that y falls in, or -1 if it falls in no mRow
 */
int PatternLockView::getRowHit(float y) {
    float squareHeight = mViewHeight;
    float hitSize = squareHeight * mHitFactor;

    float offset = getPaddingTop() + (squareHeight - hitSize) / 2.f;
    for (int i = 0; i < mDotCount; i++) {
        float hitTop = offset + squareHeight * i;
        if (y >= hitTop && y <= hitTop + hitSize) {
            return i;
        }
    }
    return -1;
}

/**
 * Helper method to find the column x falls into
 *
 * @param x The x coordinate
 * @return The mColumn that x falls in, or -1 if it falls in no mColumn
 */
int PatternLockView::getColumnHit(float x) {
    float squareWidth = mViewWidth;
    float hitSize = squareWidth * mHitFactor;

    float offset = getPaddingLeft() + (squareWidth - hitSize) / 2.f;
    for (int i = 0; i < mDotCount; i++) {

        float hitLeft = offset + squareWidth * i;
        if (x >= hitLeft && x <= hitLeft + hitSize) {
            return i;
        }
    }
    return -1;
}

void PatternLockView::handleActionMove(MotionEvent& event) {
    float radius = (float)mPathWidth;
    const int historySize = (int)event.getHistorySize();
    mTempInvalidateRect.setEmpty();
    bool invalidateNow = false;
    for (int i = 0; i < historySize + 1; i++) {
        float x = event.getX();//i < historySize ? event.getHistoricalX(i) : event.getX();
        float y = event.getY();//i < historySize ? event.getHistoricalY(i) : event.getY();
        Dot* hitDot = detectAndAddHit(x, y);
        size_t patternSize = mPattern.size();
        if (hitDot != nullptr && patternSize == 1) {
            mPatternInProgress = true;
            notifyPatternStarted();
        }
        // Note current x and y for rubber banding of in progress patterns
        float dx = std::abs(x - mInProgressX);
        float dy = std::abs(y - mInProgressY);
        if (dx > DEFAULT_DRAG_THRESHOLD || dy > DEFAULT_DRAG_THRESHOLD) {
            invalidateNow = true;
        }

        if (mPatternInProgress && patternSize > 0) {
	    std::vector<Dot*>& pattern = mPattern;
            Dot* lastDot = pattern.at(patternSize - 1);
            float lastCellCenterX = getCenterXForColumn(lastDot->mColumn);
            float lastCellCenterY = getCenterYForRow(lastDot->mRow);

            // Adjust for drawn segment from last cell to (x,y). Radius
            // accounts for line width.
            float left = std::min(lastCellCenterX, x) - radius;
            float right = std::max(lastCellCenterX, x) + radius;
            float top = std::min(lastCellCenterY, y) - radius;
            float bottom = std::max(lastCellCenterY, y) + radius;

            // Invalidate between the pattern's new cell and the pattern's
            // previous cell
            if (hitDot != nullptr) {
                float width = mViewWidth * 0.5f;
                float height = mViewHeight * 0.5f;
                float hitCellCenterX = getCenterXForColumn(hitDot->mColumn);
                float hitCellCenterY = getCenterYForRow(hitDot->mRow);

                left = std::min(hitCellCenterX - width, left);
                right = std::max(hitCellCenterX + width, right);
                top = std::min(hitCellCenterY - height, top);
                bottom = std::max(hitCellCenterY + height, bottom);
            }

            // Invalidate between the pattern's last cell and the previous
            // location
            mTempInvalidateRect.Union(static_cast<int>(std::round(left)),static_cast<int>(std::round(top)),
                    static_cast<int>(std::round(right)), static_cast<int>(std::round(bottom)));
        }
    }
    mInProgressX = event.getX();
    mInProgressY = event.getY();

    // To save updates, we only invalidate if the user moved beyond a
    // certain amount.
    if (invalidateNow) {
        mInvalidate.Union(mTempInvalidateRect);
        invalidate(mInvalidate);
        mInvalidate=mTempInvalidateRect;
    }
}

void PatternLockView::sendAccessEvent(int resId) {
    /*if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN) {
        setContentDescription(getContext().getString(resId));
        sendAccessibilityEvent(AccessibilityEvent.TYPE_VIEW_SELECTED);
        setContentDescription(nullptr);
    } else {
        announceForAccessibility(getContext().getString(resId));
    }*/
}

void PatternLockView::handleActionUp(MotionEvent& event) {
    // Report pattern detected
    if (!mPattern.empty()) {
        mPatternInProgress = false;
        cancelLineAnimations();
        notifyPatternDetected();
        invalidate();
    }
    if (PROFILE_DRAWING) {
        if (mDrawingProfilingStarted) {
            //Debug.stopMethodTracing();
            mDrawingProfilingStarted = false;
        }
    }
}

void PatternLockView::cancelLineAnimations() {
    for (int i = 0; i < mDotCount; i++) {
        for (int j = 0; j < mDotCount; j++) {
            DotState* state = mDotStates[i][j];
            if (state->mLineAnimator != nullptr) {
                state->mLineAnimator->cancel();
                state->mLineEndX = FLOAT_MIN;//Float.MIN_VALUE;
                state->mLineEndY = FLOAT_MIN;//Float.MIN_VALUE;
            }
        }
    }
}

void PatternLockView::handleActionDown(MotionEvent& event) {
    resetPattern();
    float x = event.getX();
    float y = event.getY();
    Dot* hitDot = detectAndAddHit(x, y);
    if (hitDot != nullptr) {
        mPatternInProgress = true;
        mPatternViewMode = CORRECT;
        notifyPatternStarted();
    } else {
        mPatternInProgress = false;
        notifyPatternCleared();
    }
    if (hitDot != nullptr) {
        float startX = getCenterXForColumn(hitDot->mColumn);
        float startY = getCenterYForRow(hitDot->mRow);

        float widthOffset = mViewWidth / 2.f;
        float heightOffset = mViewHeight / 2.f;

        invalidate((int) (startX - widthOffset),
                (int) (startY - heightOffset),
                (int) (startX + widthOffset), (int) (startY + heightOffset));
    }
    mInProgressX = x;
    mInProgressY = y;
    if (PROFILE_DRAWING) {
        if (!mDrawingProfilingStarted) {
            //Debug.startMethodTracing("PatternLockDrawing");
            mDrawingProfilingStarted = true;
        }
    }
}

float PatternLockView::getCenterXForColumn(int column) {
    return getPaddingLeft() + column * mViewWidth + mViewWidth / 2.f;
}

float PatternLockView::getCenterYForRow(int row) {
    return getPaddingTop() + row * mViewHeight + mViewHeight / 2.f;
}

float PatternLockView::calculateLastSegmentAlpha(float x, float y, float lastX, float lastY) {
    float diffX = x - lastX;
    float diffY = y - lastY;
    float dist = (float) sqrt(diffX * diffX + diffY * diffY);
    float fraction = dist / mViewWidth;
    return std::min(1.f, std::max(.0f, (fraction - 0.3f) * 4.f));
}

int PatternLockView::getCurrentColor(bool partOfPattern) {
    if (!partOfPattern || mInStealthMode || mPatternInProgress) {
        return mNormalStateColor;
    } else if (mPatternViewMode == WRONG) {
        return mWrongStateColor;
    } else if (mPatternViewMode == CORRECT
            || mPatternViewMode == AUTO_DRAW) {
        return mCorrectStateColor;
    } else {
        throw std::runtime_error("Unknown view mode ");// + mPatternViewMode);
    }
}

void PatternLockView::drawCircle(Canvas& canvas, float centerX, float centerY,
                        float size, bool partOfPattern, float alpha) {
    //mDotPaint.setColor(getCurrentColor(partOfPattern));
    //mDotPaint.setAlpha((int) (alpha * 255));
    canvas.set_color(getCurrentColor(partOfPattern));
    canvas.arc(centerX, centerY, size / 2, 0,M_PI*2.f);
    canvas.fill();
}

PatternLockView::Dot::Dot(PatternLockView*p,int row, int column) {
    mPLV = p;
    mPLV->checkRange(row, column);
    mRow = row;
    mColumn = column;
}

int PatternLockView::Dot::getId() {
    return mRow * mPLV->mDotCount + mColumn;
}

int PatternLockView::Dot::getRow() {
    return mRow;
}

int PatternLockView::Dot::getColumn() {
    return mColumn;
}

PatternLockView::Dot* PatternLockView::of(int row, int column) {
    checkRange(row, column);
    return mDots[row][column];
}

/**
* Gets a cell from its identifier
*/
PatternLockView::Dot* PatternLockView::of(int id) {
    return of(id / mDotCount, id % mDotCount);
}

void PatternLockView::checkRange(int row, int column) {
    if (row < 0 || row > mDotCount - 1) {
        throw std::out_of_range(std::string("mRow must be in range 0-") + std::to_string(mDotCount - 1));
    }
    if (column < 0 || column > mDotCount - 1) {
        throw std::out_of_range(std::string("mColumn must be in range 0-") + std::to_string(mDotCount - 1));
    }
}

}//endof namespace
