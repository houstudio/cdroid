#include <widget/simplemonthview.h>
#include <cdtypes.h>
#include <cdlog.h>

namespace cdroid{

SimpleMonthView::SimpleMonthView(int w,int h):View(w,h){
    mOnDayClickListener=nullptr;
    mDayTextColor=nullptr;
    mDesiredMonthHeight=30;
    mDesiredDayHeight=30;
    mDesiredDayOfWeekHeight=30;
    mDesiredDaySelectorRadius=15;
    setFocusable(true);

    mPaddedWidth = w;
    mPaddedHeight = h;

    // We may have been laid out smaller than our preferred size. If so,
    // scale all dimensions to fit.
    int measuredPaddedHeight = h;// - paddingTop - paddingBottom;
    float scaleH = 1.0f;//paddedHeight / (float) measuredPaddedHeight;
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
    int maxSelectorWidth = cellWidth / 2 + 0;//std::min(paddingLeft, paddingRight);
    int maxSelectorHeight = mDayHeight / 2 + 0;//paddingBottom;
    mDaySelectorRadius = std::min(mDesiredDaySelectorRadius,std::min(maxSelectorWidth, maxSelectorHeight));
}

SimpleMonthView::SimpleMonthView(Context*ctx,const AttributeSet&atts):View(ctx,atts){
    
}

void SimpleMonthView::updateMonthYearLabel(){
    mMonthYearLabel=std::to_string(mYear)+"/"+std::to_string(mMonth+(1-Calendar::JANUARY));//mCalendar.get(Calendar::YEAR));
}

void SimpleMonthView::updateDayOfWeekLabels(){
    const char*tinyWeekdayNames[]={"SUN","MON","TUE","WED","THU","FRI","SAT"};
    for (int i = 0; i < DAYS_IN_WEEK; i++) {
        mDayOfWeekLabels[i] = tinyWeekdayNames[(mWeekStart + i+Calendar::SUNDAY ) % DAYS_IN_WEEK ];
    }
}

ColorStateList* SimpleMonthView::applyTextAppearance(const std::string& resId){
    return nullptr;
}

void SimpleMonthView::setMonthTextAppearance(const std::string& resId) {
    applyTextAppearance(resId);

    invalidate();
}

void SimpleMonthView::setDayOfWeekTextAppearance(const std::string& resId) {
    applyTextAppearance(resId);
    invalidate();
}

void SimpleMonthView::setDayTextAppearance(const std::string& resId) {
    ColorStateList* textColor = applyTextAppearance(resId);
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

void SimpleMonthView::setMonthTextColor(ColorStateList* monthTextColor){
}

void SimpleMonthView::setDayOfWeekTextColor(ColorStateList* dayOfWeekTextColor){
}

void SimpleMonthView::setDayTextColor(ColorStateList* dayTextColor){
}

void SimpleMonthView::setDaySelectorColor(ColorStateList* dayBackgroundColor){
}

void SimpleMonthView::setDayHighlightColor(ColorStateList* dayHighlightColor){
}

void SimpleMonthView::setOnDayClickListener(OnDayClickListener listener){
    mOnDayClickListener = listener;
}

bool SimpleMonthView::onTouchEvent(MotionEvent& event){
    int x = (int) (event.getX() + 0.5f);
    int y = (int) (event.getY() + 0.5f);

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
    case KEY_DPAD_LEFT:
        if (event.hasNoModifiers()) {
            focusChanged = moveOneDay(isLayoutRtl());
        }
        break;
    case KEY_DPAD_RIGHT:
        if (event.hasNoModifiers()) {
            focusChanged = moveOneDay(!isLayoutRtl());
        }
        break;
    case KEY_DPAD_UP:
        if (event.hasNoModifiers()) {
            ensureFocusedDay();
            if (mHighlightedDay > 7) {
                mHighlightedDay -= 7;
                focusChanged = true;
            }
        }
        break;
    case KEY_DPAD_DOWN:
        if (event.hasNoModifiers()) {
            ensureFocusedDay();
            if (mHighlightedDay <= mDaysInMonth - 7) {
                mHighlightedDay += 7;
                focusChanged = true;
            }
        }
        break;
    case KEY_DPAD_CENTER:
    case KEY_ENTER:
        if (mHighlightedDay != -1) {
            onDayClicked(mHighlightedDay);
            return true;
        }
        break;
    case KEY_TAB: {
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
        int offset = findDayOffset();
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

static int constrain(int amount, int low, int high) {//get the
     return amount < low ? low : (amount > high ? high : amount);
}

int SimpleMonthView::findClosestRow(const RECT* previouslyFocusedRect){
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

        row = constrain(row, 0, maxRows);
        LOGD("(%d,%d %d,%d) row=%d",previouslyFocusedRect->x,previouslyFocusedRect->y,previouslyFocusedRect->width,previouslyFocusedRect->height,row);
        return row;
    }
}

int SimpleMonthView::findClosestColumn(const RECT*previouslyFocusedRect){
    if (previouslyFocusedRect == nullptr) {
        return DAYS_IN_WEEK / 2;
    } else if (mCellWidth == 0) {
        return 0; // There hasn't been a layout, so we can just choose the first column
    } else {
        int centerX = previouslyFocusedRect->centerX() - mPaddingLeft;
        int columnFromLeft =constrain(centerX / mCellWidth, 0, DAYS_IN_WEEK - 1);
        return isLayoutRtl() ? DAYS_IN_WEEK - columnFromLeft - 1: columnFromLeft;
    }
}

void SimpleMonthView::getFocusedRect(RECT& r){
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

bool SimpleMonthView::isFirstDayOfWeek(int day){
    int offset = findDayOffset();
    return (offset + day - 1) % DAYS_IN_WEEK == 0;
}

bool SimpleMonthView::isLastDayOfWeek(int day) {
    int offset = findDayOffset();
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
    float x = mPaddedWidth / 2.f;
    // Vertically centered within the month header height.
    canvas.set_color(0xFFFFFFFF);
    RECT rctxt={0,0,mPaddedWidth,mMonthHeight};
    canvas.draw_text(rctxt,mMonthYearLabel,DT_CENTER|DT_VCENTER);
}

const std::string SimpleMonthView::getMonthYearLabel(){
    return mMonthYearLabel;
}

void SimpleMonthView::drawDaysOfWeek(Canvas& canvas){
    int headerHeight = mMonthHeight;
    int rowHeight = mDayOfWeekHeight;
    int colWidth = mCellWidth;

    // Text is vertically centered within the day of week height.
    int rowCenter = headerHeight + rowHeight / 2;
    
    canvas.set_color(0xFFFFFFFF);
    canvas.move_to(0,headerHeight);
    canvas.line_to(mWidth,headerHeight);
    canvas.stroke();
    RECT rctxt={0,headerHeight,mCellWidth,rowHeight};
    for (int col = 0; col < DAYS_IN_WEEK; col++) {
        int colCenter = colWidth * col + colWidth / 2;
        int colCenterRtl;
        if (isLayoutRtl()) {
            colCenterRtl = mPaddedWidth - colCenter;
        } else {
            colCenterRtl = colCenter;
        }

        std::string label = mDayOfWeekLabels[col];
        canvas.draw_text(rctxt,label,DT_CENTER|DT_VCENTER);
        rctxt.offset(mCellWidth,0);
    }
    canvas.move_to(0,headerHeight+rowHeight);
    canvas.line_to(mWidth,headerHeight+rowHeight);
    canvas.stroke();
}

void SimpleMonthView::drawDays(Canvas& canvas){
    int headerHeight = mMonthHeight + mDayOfWeekHeight;
    int rowHeight = mDayHeight;
    int colWidth = mCellWidth;

    // Text is vertically centered within the row height.
    int rowCenter = headerHeight + rowHeight / 2;
    RECT rctxt={0,headerHeight,mCellWidth,mDayHeight};
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

        bool bDayEnabled = isDayEnabled(day);
        if (bDayEnabled) {
            stateMask |= StateSet::VIEW_STATE_ENABLED;
        }

        bool isDayActivated = mActivatedDay == day;
        bool isDayHighlighted = mHighlightedDay == day;
        canvas.set_color(0x8000FF00);
        if (isDayActivated) {
            stateMask |= StateSet::VIEW_STATE_ACTIVATED;

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
            dayTextColor =0xFFFFFFFF;// mDayTextColor->getColorForState(stateSet, 0);
        }
        canvas.set_color(dayTextColor);
        rctxt.x=colWidth * col;
        canvas.draw_text(rctxt,std::to_string(day),DT_CENTER|DT_VCENTER);
        col++;

        if (col == DAYS_IN_WEEK) {
            col = 0;
            rowCenter += rowHeight;
            rctxt.y += rowHeight;
        }
    }
    canvas.set_color(0xFF00FF00);
    for(int i=0;i<5;i++){
        canvas.move_to(0,headerHeight+i*rowHeight);
        canvas.line_to(mWidth,headerHeight+i*rowHeight);
        canvas.stroke();
    }
}

bool SimpleMonthView::isDayEnabled(int day)const{
    return day >= mEnabledDayStart && day <= mEnabledDayEnd;
}

bool SimpleMonthView::isValidDayOfMonth(int day)const{
    return day >= 1 && day <= mDaysInMonth;
}

bool SimpleMonthView::isValidDayOfWeek(int day) {
    return day >= Calendar::SUNDAY && day <= Calendar::SATURDAY;
}

bool SimpleMonthView::isValidMonth(int month) {
    return month >= Calendar::JANUARY && month <= Calendar::DECEMBER;
}

void SimpleMonthView::setSelectedDay(int dayOfMonth) {
    mActivatedDay = dayOfMonth;
    // Invalidate cached accessibility information.
    //mTouchHelper.invalidateRoot();
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
    //mTouchHelper.invalidateRoot();
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
    mEnabledDayStart= constrain(enabledDayStart, 1, mDaysInMonth);
    mEnabledDayEnd  = constrain(enabledDayEnd, mEnabledDayStart, mDaysInMonth);

    updateMonthYearLabel();
    updateDayOfWeekLabels();

    // Invalidate cached accessibility information.
    //mTouchHelper.invalidateRoot();
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
    int preferredHeight = mDesiredDayHeight * MAX_WEEKS_IN_MONTH
                + mDesiredDayOfWeekHeight + mDesiredMonthHeight
                + getPaddingTop() + getPaddingBottom();
    int preferredWidth = mDesiredCellWidth * DAYS_IN_WEEK
                + getPaddingStart() + getPaddingEnd();
    int resolvedWidth = resolveSize(preferredWidth, widthMeasureSpec);
    int resolvedHeight = resolveSize(preferredHeight, heightMeasureSpec);
    setMeasuredDimension(resolvedWidth, resolvedHeight);
}

void SimpleMonthView::onLayout(bool changed, int left, int top, int w, int h){
    if (!changed) return;

    // Let's initialize a completely reasonable number of variables.
    int paddingLeft = getPaddingLeft();
    int paddingTop = getPaddingTop();
    int paddingRight = getPaddingRight();
    int paddingBottom = getPaddingBottom();
    int paddedRight = w - paddingRight;
    int paddedBottom = h - paddingBottom;
    int paddedWidth = paddedRight - paddingLeft;
    int paddedHeight = paddedBottom - paddingTop;
    if (paddedWidth == mPaddedWidth || paddedHeight == mPaddedHeight) {
        return;
    }

    mPaddedWidth = paddedWidth;
    mPaddedHeight = paddedHeight;

    // We may have been laid out smaller than our preferred size. If so,
    // scale all dimensions to fit.
    int measuredPaddedHeight = getMeasuredHeight() - paddingTop - paddingBottom;
    float scaleH = paddedHeight / (float) measuredPaddedHeight;
    int monthHeight = (int) (mDesiredMonthHeight * scaleH);
    int cellWidth = mPaddedWidth / DAYS_IN_WEEK;
    mMonthHeight = monthHeight;
    mDayOfWeekHeight = (int) (mDesiredDayOfWeekHeight * scaleH);
    mDayHeight = (int) (mDesiredDayHeight * scaleH);
    mCellWidth = cellWidth;

    // Compute the largest day selector radius that's still within the clip
    // bounds and desired selector radius.
    int maxSelectorWidth = cellWidth / 2 + std::min(paddingLeft, paddingRight);
    int maxSelectorHeight = mDayHeight / 2 + paddingBottom;
    mDaySelectorRadius = std::min(mDesiredDaySelectorRadius,
                                  std::min(maxSelectorWidth, maxSelectorHeight));

    // Invalidate cached accessibility information.
    //mTouchHelper.invalidateRoot();
}

int SimpleMonthView::findDayOffset(){
    int offset = mDayOfWeekStart - mWeekStart;
    if (mDayOfWeekStart < mWeekStart) {
        return offset + DAYS_IN_WEEK;
    }
    return offset;
}

int SimpleMonthView::getDayAtLocation(int x, int y) {
    int paddedX = x - getPaddingLeft();
    if (paddedX < 0 || paddedX >= mPaddedWidth) {
        return -1;
    }

    int headerHeight = mMonthHeight + mDayOfWeekHeight;
    int paddedY = y - getPaddingTop();
    if (paddedY < headerHeight || paddedY >= mPaddedHeight) {
        return -1;
    }

    // Adjust for RTL after applying padding.
    int paddedXRtl;
    if (isLayoutRtl()) {
        paddedXRtl = mPaddedWidth - paddedX;
    } else {
        paddedXRtl = paddedX;
    }

    int row = (paddedY - headerHeight) / mDayHeight;
    int col = (paddedXRtl * DAYS_IN_WEEK) / mPaddedWidth;
    int index = col + row * DAYS_IN_WEEK;
    int day = index + 1 - findDayOffset();
    if (!isValidDayOfMonth(day)) {
        return -1;
    }

    return day;
}

bool SimpleMonthView::getBoundsForDay(int id,RECT&outBounds){
    if (!isValidDayOfMonth(id)) {
        return false;
    }

    int index = id - 1 + findDayOffset();

    // Compute left edge, taking into account RTL.
    int col = index % DAYS_IN_WEEK;
    int colWidth = mCellWidth;
    int left;
    if (isLayoutRtl()) {
        left = getWidth() - getPaddingRight() - (col + 1) * colWidth;
    } else {
        left = getPaddingLeft() + col * colWidth;
    }

    // Compute top edge.
    int row = index / DAYS_IN_WEEK;
    int rowHeight = mDayHeight;
    int headerHeight = mMonthHeight + mDayOfWeekHeight;
    int top = getPaddingTop() + headerHeight + row * rowHeight;

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
    //mTouchHelper.sendEventForVirtualView(day, AccessibilityEvent.TYPE_VIEW_CLICKED);
    return true;
}

}//namespace
