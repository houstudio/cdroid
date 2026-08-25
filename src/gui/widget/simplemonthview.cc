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
#include <widget/internal_R.h>
#include <widget/simplemonthview.h>
#include <widget/framework_styleable.h>
#include <core/assets.h>
#include <content/dateformatsymbols.h>
#include <content/simpledateformat.h>
#include <cmath>
#include <text/paint.h>
#include <text/textutils.h>
#include <drawable/colorstatelist.h>
#include <drawable/stateset.h>
#include <core/typeface.h>
#include <utils/mathutils.h>
#include <porting/cdtypes.h>
#include <porting/cdlog.h>

namespace cdroid{
using namespace cdroid::internal;
DECLARE_WIDGET(SimpleMonthView);
SimpleMonthView::SimpleMonthView(Context*ctx)
    :SimpleMonthView(ctx,nullptr){}

SimpleMonthView::SimpleMonthView(Context*ctx,const AttributeSet* atts):SimpleMonthView(ctx,atts,0){}

SimpleMonthView::SimpleMonthView(Context*ctx,const AttributeSet* pAttrs,int defStyleAttr)
   :View(ctx,pAttrs, defStyleAttr){
    initMonthView();
    // Faithful to AOSP SimpleMonthView: the desired dimensions come from
    // R.dimen.date_picker_* resources, NOT from XML attributes (the month-item
    // layout declares none). Reading the missing attrs returned 0, leaving
    // mDayHeight==0 and dividing by zero in getDayAtLocation.
    // AOSP reads R.dimen.date_picker_* and the values always resolve. CDROID's
    // runtime table may not carry these framework dimens yet (getDimensionPixelSize
    // returns 0), which collapsed the whole month grid to zero height — keep the
    // material defaults from dimens_material.xml when resolution fails.
    auto dimenOr = [this](int resId, int fallbackPx) {
        const int v = mContext->getDimensionPixelSize(resId);
        return v > 0 ? v : fallbackPx;
    };
    const float density = mContext->getResources().getDisplayMetrics().density;
    mDesiredMonthHeight = dimenOr(R::dimen::date_picker_month_height, (int)(56 * density));
    mDesiredDayOfWeekHeight = dimenOr(R::dimen::date_picker_day_of_week_height, (int)(36 * density));
    mDesiredDayHeight = dimenOr(R::dimen::date_picker_day_height, (int)(40 * density));
    mDesiredCellWidth  = dimenOr(R::dimen::date_picker_day_width, (int)(44 * density));
    mDesiredDaySelectorRadius = dimenOr(R::dimen::date_picker_day_selector_radius, (int)(20 * density));

    // Set up accessibility components.
    setAccessibilityDelegate(mTouchHelper);
    setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);

    int res = pAttrs ? pAttrs->getAttributeResourceValue(std::string(), "monthTextAppearance", 0) : 0;
    if(res) setMonthTextAppearance(res);
    res = pAttrs ? pAttrs->getAttributeResourceValue(std::string(), "dayOfWeekTextAppearance", 0) : 0;
    if(res) setDayOfWeekTextAppearance(res);
    { auto ta2 = mContext->obtainStyledAttributes(pAttrs, R::styleable::SimpleMonthViewCdroid);
      const int dayTA = ta2->getResourceId(R::styleable::SimpleMonthViewCdroid_dayTextAppearance, 0);
      if(dayTA) setDayTextAppearance(dayTA);
    }
    updateMonthYearLabel();
    updateDayOfWeekLabels();
}

SimpleMonthView::~SimpleMonthView(){
    // mTouchHelper is owned by View (mAccessibilityDelegate, deleted in ~View);
    // deleting it here double-frees.
}

void SimpleMonthView::initMonthView(){
    mOnDayClickListener = nullptr;
    mDayTextColor = nullptr;
    mDesiredMonthHeight = 30;
    mDesiredDayHeight = 30;
    mDesiredDayOfWeekHeight = 30;
    mDesiredDaySelectorRadius= 15;
    mMonth = 0;
    mYear  = 1970;
    mDayHeight=20;
    mPaddedWidth = 0;
    mPaddedHeight= 0;
    mTouchHelper = new MonthViewTouchHelper(this);
    initPaints();
}

/**
 * Sets up the text and style properties for painting. Faithful port of Android
 * SimpleMonthView.initPaints; the typeface names/sizes come from R.string/
 * R.dimen in Android, but those string resources aren't present here, so the
 * "sans-serif" defaults and the dimens defaults (date_picker_*_text_size) are
 * baked in. setTextAppearance overrides them per-paint.
 */
void SimpleMonthView::initPaints(){
    mMonthPaint.setAntiAlias(true);
    mMonthPaint.setTextSize(14);
    mMonthPaint.setTypeface(Typeface::create("sans-serif", Typeface::NORMAL));
    mMonthPaint.setTextAlign(Paint::Align::CENTER);
    mMonthPaint.setStyle(Paint::Style::FILL);
    mMonthPaint.setColor(0xFFFFFFFF);

    mDayOfWeekPaint.setAntiAlias(true);
    mDayOfWeekPaint.setTextSize(12);
    mDayOfWeekPaint.setTypeface(Typeface::create("sans-serif", Typeface::NORMAL));
    mDayOfWeekPaint.setTextAlign(Paint::Align::CENTER);
    mDayOfWeekPaint.setStyle(Paint::Style::FILL);
    mDayOfWeekPaint.setColor(0xFFFFFFFF);

    mDaySelectorPaint.setAntiAlias(true);
    mDaySelectorPaint.setStyle(Paint::Style::FILL);

    mDayHighlightPaint.setAntiAlias(true);
    mDayHighlightPaint.setStyle(Paint::Style::FILL);
    mDayHighlightPaint.setColor(0xFF00FF00);

    mDayHighlightSelectorPaint.setAntiAlias(true);
    mDayHighlightSelectorPaint.setStyle(Paint::Style::FILL);
    mDayHighlightSelectorPaint.setColor(0x8000ff00);

    mDayPaint.setAntiAlias(true);
    mDayPaint.setTextSize(12);
    mDayPaint.setTypeface(Typeface::create("sans-serif", Typeface::NORMAL));
    mDayPaint.setTextAlign(Paint::Align::CENTER);
    mDayPaint.setStyle(Paint::Style::FILL);
}

void SimpleMonthView::updateMonthYearLabel(){
    // AOSP builds the label from the "MMMMy" skeleton via
    // DateFormat.getBestDateTimePattern; CDROID has no DTPG, so use the
    // equivalent standalone-month pattern (LLLL keeps the header form
    // correct for locales whose standalone months differ from format months).
    const Locale locale = Locale::getDefault();
    SimpleDateFormat formatter("LLLL yyyy", locale);
    mMonthYearLabel = formatter.format(mCalendar.getTimeInMillis());
}

void SimpleMonthView::setDayOfWeekNameLength(int length) {
    mDayOfWeekNameLength = length;
    updateDayOfWeekLabels();
    invalidate();
}

void SimpleMonthView::updateDayOfWeekLabels(){
    // AOSP SimpleMonthView.updateDayOfWeekLabels: tiny (single-character)
    // weekday names from DateFormatSymbols (ICU NARROW; the i18n engine
    // approximates narrow from the short name). The table layout matches
    // Calendar days, e.g. SUNDAY is index 1; the column for index i is the
    // weekday mWeekStart + i. The name length is a CDROID extension
    // (0 narrow = AOSP, 1 abbreviated, 2 wide).
    const Locale locale = Locale::getDefault();
    // The symbols object must outlive the references (getters return refs
    // into it; binding to the temporary directly dangles).
    const DateFormatSymbols dfs(locale);
    const std::vector<std::string>* names = &dfs.getTinyWeekdays();
    if (mDayOfWeekNameLength == 1) names = &dfs.getShortWeekdays();
    else if (mDayOfWeekNameLength == 2) names = &dfs.getWeekdays();
    for (int i = 0; i < DAYS_IN_WEEK; i++) {
        mDayOfWeekLabels[i] = (*names)[(mWeekStart + i - 1) % DAYS_IN_WEEK + 1];
    }
}

const cdroid::RefPtr<ColorStateList> SimpleMonthView::applyTextAppearance(Paint& p, int resId){
    auto ta = mContext->obtainStyledAttributes(resId, R::styleable::TextAppearance);
    if (!ta) return nullptr;
    const std::string fontFamily = ta->getString(R::styleable::TextAppearance_fontFamily);
    if (!fontFamily.empty()) {
        p.setTypeface(Typeface::create(fontFamily, 0));
    }
    p.setTextSize(ta->getDimensionPixelSize(R::styleable::TextAppearance_textSize, (int) p.getTextSize()));
    const auto textColor = ta->getColorStateList(R::styleable::TextAppearance_textColor);
    if (textColor != nullptr) {
        const int enabledColor = textColor->getColorForState(
                StateSet::get(StateSet::VIEW_STATE_ENABLED), 0);
        p.setColor(enabledColor);
    }
    return textColor;
}

void SimpleMonthView::setMonthTextAppearance(int resId) {
    applyTextAppearance(mMonthPaint, resId);
    invalidate();
}

void SimpleMonthView::setDayOfWeekTextAppearance(int resId) {
    applyTextAppearance(mDayOfWeekPaint, resId);
    invalidate();
}

void SimpleMonthView::setDayTextAppearance(int resId) {
    const auto textColor = applyTextAppearance(mDayPaint, resId);
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

void SimpleMonthView::setMonthTextColor(const cdroid::RefPtr<ColorStateList>& monthTextColor){
    const int enabledColor = monthTextColor->getColorForState(
            StateSet::get(StateSet::VIEW_STATE_ENABLED), 0);
    mMonthPaint.setColor(enabledColor);
    invalidate();
}

void SimpleMonthView::setDayOfWeekTextColor(const cdroid::RefPtr<ColorStateList>& dayOfWeekTextColor){
    const int enabledColor = dayOfWeekTextColor->getColorForState(
            StateSet::get(StateSet::VIEW_STATE_ENABLED), 0);
    mDayOfWeekPaint.setColor(enabledColor);
    invalidate();
}

void SimpleMonthView::setDayTextColor(const cdroid::RefPtr<ColorStateList>& dayTextColor){
    mDayTextColor = dayTextColor;
    invalidate();
}

void SimpleMonthView::setDaySelectorColor(const cdroid::RefPtr<ColorStateList>& dayBackgroundColor){
    const int activatedColor = dayBackgroundColor->getColorForState(
            StateSet::get(StateSet::VIEW_STATE_ENABLED | StateSet::VIEW_STATE_ACTIVATED), 0);
    mDaySelectorPaint.setColor(activatedColor);
    mDayHighlightSelectorPaint.setColor(activatedColor);
    mDayHighlightSelectorPaint.setAlpha(SELECTED_HIGHLIGHT_ALPHA);
    invalidate();
}

void SimpleMonthView::setDayHighlightColor(const cdroid::RefPtr<ColorStateList>& dayHighlightColor){
    const int pressedColor = dayHighlightColor->getColorForState(
            StateSet::get(StateSet::VIEW_STATE_ENABLED | StateSet::VIEW_STATE_PRESSED), 0);
    mDayHighlightPaint.setColor(pressedColor);
    invalidate();
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
        const TextPaint& p = mDayPaint;
        int centerY = previouslyFocusedRect->centerY();

        const int headerHeight = mMonthHeight + mDayOfWeekHeight;
        const int rowHeight = mDayHeight;

        // Text is vertically centered within the row height.
        const float halfLineHeight = (p.ascent() + p.descent()) / 2.f;
        const int rowCenter = headerHeight + rowHeight / 2;

        centerY -= rowCenter - halfLineHeight;
        int row = std::round(centerY / (float) rowHeight);
        const int maxDay = findDayOffset() + mDaysInMonth;
        const int maxRows = (maxDay / DAYS_IN_WEEK) - ((maxDay % DAYS_IN_WEEK == 0) ? 1 : 0);

        row = MathUtils::constrain(row, 0, maxRows);
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
    const int paddingLeft = getPaddingLeft();
    const int paddingTop = getPaddingTop();
    canvas.translate(paddingLeft, paddingTop);

    drawMonth(canvas);
    drawDaysOfWeek(canvas);
    drawDays(canvas);

    canvas.translate(-paddingLeft, -paddingTop);
}

void SimpleMonthView::drawMonth(Canvas& canvas){
    const float x = mPaddedWidth / 2.f;

    // Vertically centered within the month header height.
    const float lineHeight = mMonthPaint.ascent() + mMonthPaint.descent();
    const float y = (mMonthHeight - lineHeight) / 2.f;

    const std::u16string u16 = TextUtils::utf8_utf16(mMonthYearLabel);
    mMonthPaint.drawTextRun(canvas, (const char16_t*) u16.c_str(),
            0, u16.length(), 0, 0, x, y, false);
}

const std::string SimpleMonthView::getMonthYearLabel(){
    return mMonthYearLabel;
}

void SimpleMonthView::drawDaysOfWeek(Canvas& canvas){
    const Paint& p = mDayOfWeekPaint;
    const int headerHeight = mMonthHeight;
    const int rowHeight = mDayOfWeekHeight;
    const int colWidth = mCellWidth;

    // Text is vertically centered within the day of week height.
    const float halfLineHeight = (p.ascent() + p.descent()) / 2.f;
    const int rowCenter = headerHeight + rowHeight / 2;

    for (int col = 0; col < DAYS_IN_WEEK; col++) {
        const int colCenter = colWidth * col + colWidth / 2;
        const int colCenterRtl = isLayoutRtl() ? (mPaddedWidth - colCenter) : colCenter;

        const std::u16string u16 = TextUtils::utf8_utf16(mDayOfWeekLabels[col]);
        mDayOfWeekPaint.drawTextRun(canvas, (const char16_t*) u16.c_str(),
                0, u16.length(), 0, 0, colCenterRtl, rowCenter - halfLineHeight, false);
    }
}

/**
 * Draws the month days.
 */
void SimpleMonthView::drawDays(Canvas& canvas){
    const Paint& p = mDayPaint;
    const int headerHeight = mMonthHeight + mDayOfWeekHeight;
    const int rowHeight = mDayHeight;
    const int colWidth = mCellWidth;

    // Text is vertically centered within the row height.
    const float halfLineHeight = (p.ascent() + p.descent()) / 2.f;
    int rowCenter = headerHeight + rowHeight / 2;

    for (int day = 1, col = findDayOffset(); day <= mDaysInMonth; day++) {
        const int colCenter = colWidth * col + colWidth / 2;
        const int colCenterRtl = isLayoutRtl() ? (mPaddedWidth - colCenter) : colCenter;

        int stateMask = 0;

        const bool dayEnabled = isDayEnabled(day);
        if (dayEnabled) {
            stateMask |= StateSet::VIEW_STATE_ENABLED;
        }

        const bool isDayActivated = (mActivatedDay == day);
        const bool isDayHighlighted = (mHighlightedDay == day);
        if (isDayActivated) {
            stateMask |= StateSet::VIEW_STATE_ACTIVATED;

            // Adjust the circle to be centered on the row.
            const Paint& paint = isDayHighlighted ? mDayHighlightSelectorPaint : mDaySelectorPaint;
            canvas.set_color(paint.getColor());
            canvas.arc(colCenterRtl, rowCenter, mDaySelectorRadius, 0, 2.0 * M_PI);
            canvas.fill();
        } else if (isDayHighlighted) {
            stateMask |= StateSet::VIEW_STATE_PRESSED;

            if (dayEnabled) {
                // Adjust the circle to be centered on the row.
                canvas.set_color(mDayHighlightPaint.getColor());
                canvas.arc(colCenterRtl, rowCenter, mDaySelectorRadius, 0, 2.0 * M_PI);
                canvas.fill();
            }
        }

        const bool isDayToday = (mToday == day);
        int dayTextColor;
        if (isDayToday && !isDayActivated) {
            dayTextColor = mDaySelectorPaint.getColor();
        } else if (mDayTextColor != nullptr) {
            dayTextColor = mDayTextColor->getColorForState(StateSet::get(stateMask), 0);
        } else {
            dayTextColor = 0xFFFFFFFF;
        }
        mDayPaint.setColor(dayTextColor);

        const std::u16string u16 = TextUtils::utf8_utf16(std::to_string(day));
        mDayPaint.drawTextRun(canvas, (const char16_t*) u16.c_str(),
                0, u16.length(), 0, 0, colCenterRtl, rowCenter - halfLineHeight, false);

        col++;

        if (col == DAYS_IN_WEEK) {
            col = 0;
            rowCenter += rowHeight;
        }
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
    case Calendar::FEBRUARY: return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
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
