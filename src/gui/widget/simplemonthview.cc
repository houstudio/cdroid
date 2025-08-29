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
#include <widget/simplemonthview.h>
#include <core/mathutils.h>
#include <porting/cdtypes.h>
#include <porting/cdlog.h>

namespace cdroid{

SimpleMonthView::SimpleMonthView(int w,int h):View(w,h){
    setFocusable(true);
    initMonthView();
    mPaddedWidth = w;
    mPaddedHeight = h;

    // We may have been laid out smaller than our preferred size. If so,
    // scale all dimensions to fit.
    //const int measuredPaddedHeight = h;// - paddingTop - paddingBottom;
    const float scaleH = 1.0f;//paddedHeight / (float) measuredPaddedHeight;
    int monthHeight = (int) (mDesiredMonthHeight * scaleH);
    int cellWidth = mPaddedWidth / DAYS_IN_WEEK;
    mMonthHeight = monthHeight;
    mMonth=0;
    mDayOfWeekHeight = (int) (mDesiredDayOfWeekHeight * scaleH);
    mDayHeight = (int) (mDesiredDayHeight * scaleH);
    mCellWidth = cellWidth;
    mWeekStart = Calendar::SUNDAY;

    // Compute the largest day selector radius that's still within the clip
    // bounds and desired selector radius.
    const int maxSelectorWidth = cellWidth / 2 + 0;//std::min(paddingLeft, paddingRight);
    const int maxSelectorHeight = mDayHeight / 2 + 0;//paddingBottom;
    mDaySelectorRadius = std::min(mDesiredDaySelectorRadius,std::min(maxSelectorWidth, maxSelectorHeight));
}

SimpleMonthView::SimpleMonthView(Context*ctx,const AttributeSet&atts)
   :View(ctx,atts){
    initMonthView();
    mDesiredMonthHeight = atts.getDimensionPixelSize("month_height");
    mDesiredDayOfWeekHeight =atts.getDimensionPixelSize("day_of_week_height");
    mDesiredDayHeight = atts.getDimensionPixelSize("day_height");
    mDesiredCellWidth  = atts.getDimensionPixelSize("day_width");
    mDesiredDaySelectorRadius=atts.getDimensionPixelSize("day_selector_radius");

    // Set up accessibility components.
    setAccessibilityDelegate(mTouchHelper);
    setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);

    std::string res = atts.getString("monthTextAppearance");
    if(!res.empty())setMonthTextAppearance(res);
    res = atts.getString("dayOfWeekTextAppearance");
    if(!res.empty())setDayOfWeekTextAppearance(res);
    res = atts.getString("dayTextAppearance");
    if(!res.empty())setDayTextAppearance(res);
}

SimpleMonthView::~SimpleMonthView(){
    delete mTouchHelper;
}

void SimpleMonthView::initMonthView(){
    mOnDayClickListener = nullptr;
    mDayTextColor = nullptr;
    mDesiredMonthHeight = 30;
    mDesiredDayHeight = 30;
    mDesiredDayOfWeekHeight = 30;
    mDesiredDaySelectorRadius= 15;
    mDayTypeface = nullptr;
    mMonthTypeface = nullptr;
    mDayOfWeekTypeface = nullptr;
    mMonth = 0;
    mPaddedWidth = 0;
    mPaddedHeight= 0;
    mDayTextSize = 18;
    mMonthTextSize=24;
    mDayOfWeekTextSize=18;
    mTouchHelper = new MonthViewTouchHelper(this);
}

void SimpleMonthView::updateMonthYearLabel(){
    mMonthYearLabel = std::to_string(mYear)+"/"+std::to_string(mMonth+(1-Calendar::JANUARY));//mCalendar.get(Calendar::YEAR));
}

void SimpleMonthView::updateDayOfWeekLabels(){
    const char*tinyWeekdayNames[]={"SUN","MON","TUE","WED","THU","FRI","SAT"};
    for (int i = 0; i < DAYS_IN_WEEK; i++) {
        mDayOfWeekLabels[i] = tinyWeekdayNames[(mWeekStart + i+Calendar::SUNDAY ) % DAYS_IN_WEEK ];
    }
}

ColorStateList* SimpleMonthView::applyTextAppearance(Typeface*&face,int& textSize,const std::string& resId){
    AttributeSet attrs = mContext->obtainStyledAttributes(resId);
    std::string fontFamily = attrs.getString("fontFamily");
    textSize = attrs.getDimensionPixelSize("textSize",textSize);
    ColorStateList* textColor = attrs.getColorStateList("textColor");
    if(!fontFamily.empty()) face = Typeface::create(fontFamily,0);
    return textColor;
}

void SimpleMonthView::setMonthTextAppearance(const std::string& resId) {
    mMonthTextColor = applyTextAppearance(mMonthTypeface,mMonthTextSize,resId);

    invalidate();
}

void SimpleMonthView::setDayOfWeekTextAppearance(const std::string& resId) {
    mDayOfWeekTextColor = applyTextAppearance(mDayOfWeekTypeface,mDayOfWeekTextSize,resId);
    invalidate();
}

void SimpleMonthView::setDayTextAppearance(const std::string& resId) {
    ColorStateList* textColor = applyTextAppearance(mDayTypeface,mDayTextSize,resId);
    if (textColor != nullptr) {
        mDayTextColor = textColor;
    }

    invalidate();
}

int SimpleMonthView::getMonthHeight()const{
    return mMonthHeight;
}

int SimpleMonthView::getCellWidth()const{
    return mCellWidth;
}

void SimpleMonthView::setMonthTextColor(const ColorStateList* monthTextColor){
    mMonthTextColor = monthTextColor;
    invalidate();
}

void SimpleMonthView::setDayOfWeekTextColor(const ColorStateList* dayOfWeekTextColor){
    //const int enabledColor = dayOfWeekTextColor->getColorForState(ENABLED_STATE_SET, 0);
    mDayOfWeekTextColor = dayOfWeekTextColor;
    invalidate();
}

void SimpleMonthView::setDayTextColor(const ColorStateList* dayTextColor){
    mDayTextColor = dayTextColor;
    invalidate();
}

void SimpleMonthView::setDaySelectorColor(const ColorStateList* dayBackgroundColor){
    mDaySelectorColor = dayBackgroundColor;
    invalidate();
}

void SimpleMonthView::setDayHighlightColor(const ColorStateList* dayHighlightColor){

}

void SimpleMonthView::setOnDayClickListener(const OnDayClickListener& listener){
    mOnDayClickListener = listener;
}

bool SimpleMonthView::onTouchEvent(MotionEvent& event){
    const int x = int(event.getX() + 0.5f);
    const int y = int(event.getY() + 0.5f);

    const int action = event.getAction();
    int touchedItem,clickedDay;
    switch (action) {
    case MotionEvent::ACTION_DOWN:
    case MotionEvent::ACTION_MOVE:
        touchedItem = getDayAtLocation(x, y);
        mIsTouchHighlighted = true;
        if (mHighlightedDay != touchedItem) {
            mHighlightedDay = touchedItem;
            mPreviouslyHighlightedDay = touchedItem;
            invalidate();
        }
        if (action == MotionEvent::ACTION_DOWN && touchedItem < 0) {
            // Touch something that's not an item, reject event.
            return false;
        }
        break;

    case MotionEvent::ACTION_UP:
        clickedDay = getDayAtLocation(x, y);
        onDayClicked(clickedDay);
        // Fall through.
    case MotionEvent::ACTION_CANCEL:
        // Reset touched day on stream end.
        mHighlightedDay = -1;
        mIsTouchHighlighted = false;
        invalidate();
        break;
    }
    return true;
}

bool SimpleMonthView::onKeyDown(int keyCode, KeyEvent& event){
    // We need to handle focus change within the SimpleMonthView because we are simulating
    // multiple Views. The arrow keys will move between days until there is no space (no
    // day to the left, top, right, or bottom). Focus forward and back jumps out of the
    // SimpleMonthView, skipping over other SimpleMonthViews in the parent ViewPager
    // to the next focusable View in the hierarchy.
    bool focusChanged = false;
    switch (event.getKeyCode()) {
    case KeyEvent::KEYCODE_DPAD_LEFT:
        if (event.hasNoModifiers()) {
            focusChanged = moveOneDay(isLayoutRtl());
        }
        break;
    case KeyEvent::KEYCODE_DPAD_RIGHT:
        if (event.hasNoModifiers()) {
            focusChanged = moveOneDay(!isLayoutRtl());
        }
        break;
    case KeyEvent::KEYCODE_DPAD_UP:
        if (event.hasNoModifiers()) {
            ensureFocusedDay();
            if (mHighlightedDay > 7) {
                mHighlightedDay -= 7;
                focusChanged = true;
            }
        }
        break;
    case KeyEvent::KEYCODE_DPAD_DOWN:
        if (event.hasNoModifiers()) {
            ensureFocusedDay();
            if (mHighlightedDay <= mDaysInMonth - 7) {
                mHighlightedDay += 7;
                focusChanged = true;
            }
        }
        break;
    case KeyEvent::KEYCODE_DPAD_CENTER:
    case KeyEvent::KEYCODE_ENTER:
        if (mHighlightedDay != -1) {
            onDayClicked(mHighlightedDay);
            return true;
        }
        break;
    case KeyEvent::KEYCODE_TAB: {
        int focusChangeDirection = 0;
        if (event.hasNoModifiers()) {
            focusChangeDirection = View::FOCUS_FORWARD;
        } else if (event.hasModifiers(KeyEvent::META_SHIFT_ON)) {
            focusChangeDirection = View::FOCUS_BACKWARD;
        }
        if (focusChangeDirection != 0) {
            ViewGroup* parent = getParent();
            // move out of the ViewPager next/previous
            View* nextFocus = this;
            do {
                nextFocus = nextFocus->focusSearch(focusChangeDirection);
            } while (nextFocus != nullptr && nextFocus != this &&
                    nextFocus->getParent() == parent);
            if (nextFocus != nullptr) {
                nextFocus->requestFocus();
                return true;
            }
        }
        break;
        }
    }
    if (focusChanged) {
        invalidate();
        return true;
    } else {
        return View::onKeyDown(keyCode, event);
    }
}

bool SimpleMonthView::moveOneDay(bool positive){
    ensureFocusedDay();
    bool focusChanged = false;
    if (positive) {
        if (!isLastDayOfWeek(mHighlightedDay) && mHighlightedDay < mDaysInMonth) {
            mHighlightedDay++;
            focusChanged = true;
        }
    } else {
        if (!isFirstDayOfWeek(mHighlightedDay) && mHighlightedDay > 1) {
            mHighlightedDay--;
            focusChanged = true;
        }
    }
    return focusChanged;
}

void SimpleMonthView::onFocusChanged(bool gainFocus,int direction,Rect* previouslyFocusedRect){
    if (gainFocus) {
        // If we've gained focus through arrow keys, we should find the day closest
        // to the focus rect. If we've gained focus through forward/back, we should
        // focus on the selected day if there is one.
        const int offset = findDayOffset();
        switch(direction) {
        case View::FOCUS_RIGHT: {
            int row = findClosestRow(previouslyFocusedRect);
            mHighlightedDay = row == 0 ? 1 : (row * DAYS_IN_WEEK) - offset + 1;
            break;
        }
        case View::FOCUS_LEFT: {
            int row = findClosestRow(previouslyFocusedRect) + 1;
            mHighlightedDay = std::min(mDaysInMonth, (row * DAYS_IN_WEEK) - offset);
            break;
        }
        case View::FOCUS_DOWN: {
            int col = findClosestColumn(previouslyFocusedRect);
            int day = col - offset + 1;
            mHighlightedDay = day < 1 ? day + DAYS_IN_WEEK : day;
            break;
        }
        case View::FOCUS_UP: {
            int col = findClosestColumn(previouslyFocusedRect);
            int maxWeeks = (offset + mDaysInMonth) / DAYS_IN_WEEK;
            int day = col - offset + (DAYS_IN_WEEK * maxWeeks) + 1;
            mHighlightedDay = day > mDaysInMonth ? day - DAYS_IN_WEEK : day;
            break;
        }
        }
        ensureFocusedDay();
        invalidate();
    }
    View::onFocusChanged(gainFocus, direction, previouslyFocusedRect);
}

int SimpleMonthView::findClosestRow(const Rect* previouslyFocusedRect){
    if (previouslyFocusedRect == nullptr) {
        return 3;
    } else if (mDayHeight == 0) {
        return 0; // There hasn't been a layout, so just choose the first row
    } else {
        int centerY = previouslyFocusedRect->centerY();

        int headerHeight = mMonthHeight + mDayOfWeekHeight;
        int rowHeight = mDayHeight;

        // Text is vertically centered within the row height.
        float halfLineHeight = 10;//(p.ascent() + p.descent()) / 2.f;
        int rowCenter = headerHeight + rowHeight / 2;

        centerY -= rowCenter - halfLineHeight;
        int row = std::round(centerY / (float) rowHeight);
        int maxDay = findDayOffset() + mDaysInMonth;
        int maxRows = (maxDay / DAYS_IN_WEEK) - ((maxDay % DAYS_IN_WEEK == 0) ? 1 : 0);

        row = MathUtils::constrain(row, 0, maxRows);
        LOGD("(%d,%d %d,%d) row=%d",previouslyFocusedRect->left,previouslyFocusedRect->top,previouslyFocusedRect->width,previouslyFocusedRect->height,row);
        return row;
    }
}

int SimpleMonthView::findClosestColumn(const Rect*previouslyFocusedRect){
    if (previouslyFocusedRect == nullptr) {
        return DAYS_IN_WEEK / 2;
    } else if (mCellWidth == 0) {
        return 0; // There hasn't been a layout, so we can just choose the first column
    } else {
        const int centerX = previouslyFocusedRect->centerX() - mPaddingLeft;
        const int columnFromLeft = MathUtils::constrain(centerX / mCellWidth, 0, DAYS_IN_WEEK - 1);
        return isLayoutRtl() ? DAYS_IN_WEEK - columnFromLeft - 1: columnFromLeft;
    }
}

void SimpleMonthView::getFocusedRect(Rect& r){
    if (mHighlightedDay > 0) {
        getBoundsForDay(mHighlightedDay, r);
    } else {
        View::getFocusedRect(r);
    }
}

void SimpleMonthView::onFocusLost(){
    if (!mIsTouchHighlighted) {
        // Unhighlight a day.
        mPreviouslyHighlightedDay = mHighlightedDay;
        mHighlightedDay = -1;
        invalidate();
    }
    View::onFocusLost();
}

void SimpleMonthView::ensureFocusedDay(){
    if (mHighlightedDay != -1) {
        return;
    }
    if (mPreviouslyHighlightedDay != -1) {
        mHighlightedDay = mPreviouslyHighlightedDay;
        return;
    }
    if (mActivatedDay != -1) {
        mHighlightedDay = mActivatedDay;
        return;
    }
    mHighlightedDay = 1;
}

bool SimpleMonthView::isFirstDayOfWeek(int day)const{
    int offset = findDayOffset();
    return (offset + day - 1) % DAYS_IN_WEEK == 0;
}

bool SimpleMonthView::isLastDayOfWeek(int day)const{
    const int offset = findDayOffset();
    return (offset + day) % DAYS_IN_WEEK == 0;
}

void SimpleMonthView::onDraw(Canvas& canvas){
    int paddingLeft = getPaddingLeft();
    int paddingTop = getPaddingTop();
    LOGV("mMonthHeight=%d,mDayOfWeekHeight=%d mDayHeight=%d mCellWidth=%d mDaySelectorRadius=%d findDayOffset=%d",
                  mMonthHeight,mDayOfWeekHeight,mDayHeight,mCellWidth,mDaySelectorRadius,findDayOffset());
    canvas.translate(paddingLeft, paddingTop);
    canvas.set_font_size(20);
    drawMonth(canvas);
    drawDaysOfWeek(canvas);
    drawDays(canvas);
    canvas.translate(-paddingLeft, -paddingTop);
}

void SimpleMonthView::drawMonth(Canvas& canvas){
    //float x = mPaddedWidth / 2.f;
    // Vertically centered within the month header height.
    canvas.set_color(0xFFFFFFFF);
    Rect rctxt={0,0,mPaddedWidth,mMonthHeight};
    canvas.draw_text(rctxt,mMonthYearLabel,Gravity::CENTER);
}

const std::string SimpleMonthView::getMonthYearLabel(){
    return mMonthYearLabel;
}

void SimpleMonthView::drawDaysOfWeek(Canvas& canvas){
    const int headerHeight = mMonthHeight;
    const int rowHeight = mDayOfWeekHeight;
    const int colWidth = mCellWidth;

    // Text is vertically centered within the day of week height.
    //const int rowCenter = headerHeight + rowHeight / 2;
    const int color = mDayTextColor==nullptr?0xFFFFFFFF:
        mDayTextColor->getColorForState(StateSet::get(StateSet::VIEW_STATE_ENABLED),0);
    canvas.set_color(color);
    canvas.move_to(0,headerHeight);
    canvas.line_to(mRight-mLeft,headerHeight);
    canvas.stroke();
    canvas.set_font_size(mDayOfWeekTextSize);
    if(mDayOfWeekTypeface)
	canvas.set_font_face(mDayOfWeekTypeface->getFontFace()->get_font_face());
    Rect rctxt={0,headerHeight,mCellWidth,rowHeight};
    for (int col = 0; col < DAYS_IN_WEEK; col++) {
        int colCenter = colWidth * col + colWidth / 2;
        int colCenterRtl;
        if (isLayoutRtl()) {
            colCenterRtl = mPaddedWidth - colCenter;
        } else {
            colCenterRtl = colCenter;
        }
        int stateMask =0;
        std::string label = mDayOfWeekLabels[col];
        canvas.draw_text(rctxt,label,Gravity::CENTER);
        rctxt.offset(mCellWidth,0);
    }
    canvas.move_to(0,headerHeight+rowHeight);
    canvas.line_to(mRight-mLeft,headerHeight+rowHeight);
    canvas.stroke();
}

void SimpleMonthView::drawDays(Canvas& canvas){
    int headerHeight = mMonthHeight + mDayOfWeekHeight;
    int rowHeight = mDayHeight;
    int colWidth = mCellWidth;

    // Text is vertically centered within the row height.
    int rowCenter = headerHeight + rowHeight / 2;
    Rect rctxt={0,headerHeight,mCellWidth,mDayHeight};
    canvas.set_color(0xFFFFFFFF);
    for (int day = 1, col = findDayOffset(); day <= mDaysInMonth; day++) {
        int colCenter = colWidth * col + colWidth / 2;
        int colCenterRtl;
        if (isLayoutRtl()) {
            colCenterRtl = mPaddedWidth - colCenter;
        } else {
            colCenterRtl = colCenter;
        }

        int stateMask = 0;

        const bool bDayEnabled = isDayEnabled(day);
        if (bDayEnabled) {
            stateMask |= StateSet::VIEW_STATE_ENABLED;
        }

        const bool isDayActivated = (mActivatedDay == day);
        const bool isDayHighlighted = (mHighlightedDay == day);
        canvas.set_color(0x8000FF00);
        if (isDayActivated) {
            stateMask |= StateSet::VIEW_STATE_ACTIVATED;
            //ColorStateList* cls= isDayActivated?mDayHilight:mDaySelector;
            // Adjust the circle to be centered on the row.
            canvas.arc(colCenterRtl, rowCenter, mDaySelectorRadius, 0,2*M_PI);
            canvas.fill();
        } else if (isDayHighlighted) {
            stateMask |= StateSet::VIEW_STATE_PRESSED;

            if (bDayEnabled) {// Adjust the circle to be centered on the row.
                canvas.arc(colCenterRtl, rowCenter,mDaySelectorRadius, 0,2*M_PI);
                canvas.fill();
            }
        }

        bool isDayToday = mToday == day;
        int dayTextColor;
        if (isDayToday && !isDayActivated) {
            dayTextColor = 0xFF00FF00;//mDaySelectorPaint.getColor();
        } else {
            std::vector<int> stateSet = StateSet::get(stateMask);
            dayTextColor = 0xFFFFFFFF;// mDayTextColor->getColorForState(stateSet, 0);
        }
        canvas.set_color(dayTextColor);
        rctxt.left=colWidth * col;
        canvas.draw_text(rctxt,std::to_string(day),Gravity::CENTER);
        col++;

        if (col == DAYS_IN_WEEK) {
            col = 0;
            rowCenter += rowHeight;
            rctxt.top += rowHeight;
        }
    }
    canvas.set_color(0xFF00FF00);
    for(int i=0;i<5;i++){
        canvas.move_to(0,headerHeight+i*rowHeight);
        canvas.line_to(mRight-mLeft,headerHeight+i*rowHeight);
        canvas.stroke();
    }
}

bool SimpleMonthView::isDayEnabled(int day)const{
    return day >= mEnabledDayStart && day <= mEnabledDayEnd;
}

bool SimpleMonthView::isValidDayOfMonth(int day)const{
    return day >= 1 && day <= mDaysInMonth;
}

bool SimpleMonthView::isValidDayOfWeek(int day){
    return day >= Calendar::SUNDAY && day <= Calendar::SATURDAY;
}

bool SimpleMonthView::isValidMonth(int month){
    return month >= Calendar::JANUARY && month <= Calendar::DECEMBER;
}

void SimpleMonthView::setSelectedDay(int dayOfMonth) {
    mActivatedDay = dayOfMonth;
    // Invalidate cached accessibility information.
    mTouchHelper->invalidateRoot();
    invalidate();
}

void SimpleMonthView::setFirstDayOfWeek(int weekStart) {
    if (isValidDayOfWeek(weekStart)) {
        mWeekStart = weekStart;
    } else {
        mWeekStart = mCalendar.getFirstDayOfWeek();
    }

    updateDayOfWeekLabels();

    // Invalidate cached accessibility information.
    mTouchHelper->invalidateRoot();
    invalidate();
}

void SimpleMonthView::setMonthParams(int selectedDay, int month, int year, int weekStart, int enabledDayStart,int enabledDayEnd){
    mActivatedDay = selectedDay;

    if (isValidMonth(month)) {
        mMonth = month;
    }
    mYear = year;

    mCalendar.set(Calendar::MONTH, mMonth);
    mCalendar.set(Calendar::YEAR, mYear);
    mCalendar.set(Calendar::DAY_OF_MONTH, 1);
    mDayOfWeekStart = mCalendar.get(Calendar::DAY_OF_WEEK);

    if (isValidDayOfWeek(weekStart)) {
        mWeekStart = weekStart;
    } else {
        mWeekStart =mCalendar.getFirstDayOfWeek();
    }
    // Figure out what day today is.
    Calendar today;//= Calendar::getInstance();
    mToday = -1;
    mDaysInMonth = getDaysInMonth(mMonth, mYear);
    for (int i = 0; i < mDaysInMonth; i++) {
        int day = i + 1;
        if (sameDay(day, today)) {
            mToday = day;
        }
    }
    mEnabledDayStart= MathUtils::constrain(enabledDayStart, 1, mDaysInMonth);
    mEnabledDayEnd  = MathUtils::constrain(enabledDayEnd, mEnabledDayStart, mDaysInMonth);

    updateMonthYearLabel();
    updateDayOfWeekLabels();

    // Invalidate cached accessibility information.
    mTouchHelper->invalidateRoot();
    invalidate();
}

int SimpleMonthView::getDaysInMonth(int month, int year){
    switch (month) {
    case Calendar::JANUARY:
    case Calendar::MARCH:
    case Calendar::MAY:
    case Calendar::JULY:
    case Calendar::AUGUST:
    case Calendar::OCTOBER:
    case Calendar::DECEMBER: return 31;
    case Calendar::APRIL:
    case Calendar::JUNE:
    case Calendar::SEPTEMBER:
    case Calendar::NOVEMBER: return 30;
    case Calendar::FEBRUARY: return (year % 4 == 0) ? 29 : 28;
    default: LOGE("Invalid Month");return 0;
    }
}

bool SimpleMonthView::sameDay(int day, Calendar& today){
    return mYear == today.get(Calendar::YEAR) && mMonth == today.get(Calendar::MONTH)
        && day == today.get(Calendar::DAY_OF_MONTH);
}

void SimpleMonthView::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    const int preferredHeight = mDesiredDayHeight * MAX_WEEKS_IN_MONTH
                + mDesiredDayOfWeekHeight + mDesiredMonthHeight
                + getPaddingTop() + getPaddingBottom();
    const int preferredWidth = mDesiredCellWidth * DAYS_IN_WEEK
                + getPaddingStart() + getPaddingEnd();
    const int resolvedWidth = resolveSize(preferredWidth, widthMeasureSpec);
    const int resolvedHeight = resolveSize(preferredHeight, heightMeasureSpec);
    setMeasuredDimension(resolvedWidth, resolvedHeight);
}

void SimpleMonthView::onLayout(bool changed, int left, int top, int w, int h){
    if (!changed) return;

    // Let's initialize a completely reasonable number of variables.
    const int paddingLeft = getPaddingLeft();
    const int paddingTop = getPaddingTop();
    const int paddingRight = getPaddingRight();
    const int paddingBottom = getPaddingBottom();
    const int paddedRight = w - paddingRight;
    const int paddedBottom = h - paddingBottom;
    const int paddedWidth = paddedRight - paddingLeft;
    const int paddedHeight = paddedBottom - paddingTop;
    if (paddedWidth == mPaddedWidth || paddedHeight == mPaddedHeight) {
        return;
    }

    mPaddedWidth = paddedWidth;
    mPaddedHeight = paddedHeight;

    // We may have been laid out smaller than our preferred size. If so,
    // scale all dimensions to fit.
    const int measuredPaddedHeight = getMeasuredHeight() - paddingTop - paddingBottom;
    const float scaleH = paddedHeight / (float) measuredPaddedHeight;
    const int monthHeight = (int) (mDesiredMonthHeight * scaleH);
    const int cellWidth = mPaddedWidth / DAYS_IN_WEEK;
    mMonthHeight = monthHeight;
    mDayOfWeekHeight = (int) (mDesiredDayOfWeekHeight * scaleH);
    mDayHeight = (int) (mDesiredDayHeight * scaleH);
    mCellWidth = cellWidth;

    // Compute the largest day selector radius that's still within the clip
    // bounds and desired selector radius.
    const int maxSelectorWidth = cellWidth / 2 + std::min(paddingLeft, paddingRight);
    const int maxSelectorHeight = mDayHeight / 2 + paddingBottom;
    mDaySelectorRadius = std::min(mDesiredDaySelectorRadius,
                                  std::min(maxSelectorWidth, maxSelectorHeight));

    // Invalidate cached accessibility information.
    mTouchHelper->invalidateRoot();
}

int SimpleMonthView::findDayOffset()const{
    const int offset = mDayOfWeekStart - mWeekStart;
    if (mDayOfWeekStart < mWeekStart) {
        return offset + DAYS_IN_WEEK;
    }
    return offset;
}

int SimpleMonthView::getDayAtLocation(int x, int y) {
    const int paddedX = x - getPaddingLeft();
    if (paddedX < 0 || paddedX >= mPaddedWidth) {
        return -1;
    }

    const int headerHeight = mMonthHeight + mDayOfWeekHeight;
    const int paddedY = y - getPaddingTop();
    if (paddedY < headerHeight || paddedY >= mPaddedHeight) {
        return -1;
    }

    // Adjust for RTL after applying padding.
    const int paddedXRtl = isLayoutRtl()?(mPaddedWidth - paddedX):paddedX;

    const int row = (paddedY - headerHeight) / mDayHeight;
    const int col = (paddedXRtl * DAYS_IN_WEEK) / mPaddedWidth;
    const int index = col + row * DAYS_IN_WEEK;
    const int day = index + 1 - findDayOffset();
    if (!isValidDayOfMonth(day)) {
        return -1;
    }

    return day;
}

bool SimpleMonthView::getBoundsForDay(int id,Rect&outBounds){
    if (!isValidDayOfMonth(id)) {
        return false;
    }

    const int index = id - 1 + findDayOffset();

    // Compute left edge, taking into account RTL.
    const int col = index % DAYS_IN_WEEK;
    const int colWidth = mCellWidth;
    int left;
    if (isLayoutRtl()) {
        left = getWidth() - getPaddingRight() - (col + 1) * colWidth;
    } else {
        left = getPaddingLeft() + col * colWidth;
    }

    // Compute top edge.
    const int row = index / DAYS_IN_WEEK;
    const int rowHeight = mDayHeight;
    const int headerHeight = mMonthHeight + mDayOfWeekHeight;
    const int top = getPaddingTop() + headerHeight + row * rowHeight;

    outBounds.set(left, top,colWidth,rowHeight);

    return true;
}

bool SimpleMonthView::onDayClicked(int day){
    if (!isValidDayOfMonth(day) || !isDayEnabled(day)) {
        return false;
    }

    if (mOnDayClickListener != nullptr) {
        Calendar date ;
        date.setDate(mYear, mMonth, day);
        mOnDayClickListener(*this, date);
    }
    // This is a no-op if accessibility is turned off.
    mTouchHelper->sendEventForVirtualView(day, AccessibilityEvent::TYPE_VIEW_CLICKED);
    return true;
}

PointerIcon* SimpleMonthView::onResolvePointerIcon(MotionEvent& event, int pointerIndex) {
    if (!isEnabled()) {
        return nullptr;
    }
    // Add 0.5f to event coordinates to match the logic in onTouchEvent.
    const int x = (int) (event.getX() + 0.5f);
    const int y = (int) (event.getY() + 0.5f);
    const int dayUnderPointer = getDayAtLocation(x, y);
    if (dayUnderPointer >= 0) {
        return PointerIcon::getSystemIcon(getContext(), PointerIcon::TYPE_HAND);
    }
    return View::onResolvePointerIcon(event, pointerIndex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Provides a virtual view hierarchy for interfacing with an accessibility
 * service.
 */
const std::string SimpleMonthView::MonthViewTouchHelper::DATE_FORMAT="dd MMMM yyyy";
SimpleMonthView::MonthViewTouchHelper::MonthViewTouchHelper(View* host)
    :ExploreByTouchHelper(host){
    mSMV=(SimpleMonthView*)host;
}

int SimpleMonthView::MonthViewTouchHelper::getVirtualViewAt(float x, float y) {
    const int day = mSMV->getDayAtLocation((int) (x + 0.5f), (int) (y + 0.5f));
    if (day != -1) {
        return day;
    }
    return ExploreByTouchHelper::INVALID_ID;
}

void SimpleMonthView::MonthViewTouchHelper::getVisibleVirtualViews(std::vector<int>& virtualViewIds) {
    for (int day = 1; day <= mSMV->mDaysInMonth; day++) {
        virtualViewIds.push_back(day);
    }
}

void SimpleMonthView::MonthViewTouchHelper::onPopulateEventForVirtualView(int virtualViewId, AccessibilityEvent& event) {
    event.setContentDescription(getDayDescription(virtualViewId));
}

void SimpleMonthView::MonthViewTouchHelper::onPopulateNodeForVirtualView(int virtualViewId, AccessibilityNodeInfo& node) {
    const bool hasBounds = mSMV->getBoundsForDay(virtualViewId, mTempRect);

    if (!hasBounds) {
        // The day is invalid, kill the node.
        mTempRect.setEmpty();
        node.setContentDescription("");
        node.setBoundsInParent(mTempRect);
        node.setVisibleToUser(false);
        return;
    }

    node.setText(getDayText(virtualViewId));
    node.setContentDescription(getDayDescription(virtualViewId));
    if (virtualViewId == mSMV->mToday) {
        /*RelativeDateTimeFormatter fmt = RelativeDateTimeFormatter.getInstance();
        node.setStateDescription(fmt.format(RelativeDateTimeFormatter.Direction.THIS,
                RelativeDateTimeFormatter.AbsoluteUnit.DAY));*/
    }
    if (virtualViewId == mSMV->mActivatedDay) {
        node.setSelected(true);
    }
    node.setBoundsInParent(mTempRect);

    const bool isDayEnabled = mSMV->isDayEnabled(virtualViewId);
    if (isDayEnabled) {
        node.addAction(AccessibilityNodeInfo::AccessibilityAction::ACTION_CLICK.getId());
    }

    node.setEnabled(isDayEnabled);
    node.setClickable(true);

    if (virtualViewId == mSMV->mActivatedDay) {
        // TODO: This should use activated once that's supported.
        node.setChecked(true);
    }

}

bool SimpleMonthView::MonthViewTouchHelper::onPerformActionForVirtualView(int virtualViewId, int action,Bundle* arguments) {
    switch (action) {
        case AccessibilityNodeInfo::ACTION_CLICK:
            return mSMV->onDayClicked(virtualViewId);
    }

    return false;
}

/**
 * Generates a description for a given virtual view.
 *
 * @param id the day to generate a description for
 * @return a description of the virtual view
 */
std::string SimpleMonthView::MonthViewTouchHelper::getDayDescription(int id) {
    if (mSMV->isValidDayOfMonth(id)) {
        //mTempCalendar.set(mSMV->mYear, mSMV->mMonth, id);
        return "";//DateFormat.format(DATE_FORMAT, mTempCalendar.getTimeInMillis());
    }

    return "";
}

/**
 * Generates displayed text for a given virtual view.
 *
 * @param id the day to generate text for
 * @return the visible text of the virtual view
 */
std::string SimpleMonthView::MonthViewTouchHelper::getDayText(int id) {
    if (mSMV->isValidDayOfMonth(id)) {
        return "";//mDayFormatter.format(id);
    }

    return "";
}

}//namespace
