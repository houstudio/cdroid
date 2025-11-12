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
#include <widget/gridlayout.h>
#include <widget/space.h>
#include <cdlog.h>
#include <sstream>

namespace cdroid{

DECLARE_WIDGET(GridLayout)

GridLayout::LayoutParams::LayoutParams(int width, int height, 
     int left, int top, int right, int bottom,const Spec& rowSpec,const Spec& columnSpec)
  :MarginLayoutParams(width,height){
    setMargins(left,top,right,bottom);
    this->rowSpec =rowSpec;
    this->columnSpec =columnSpec;
}

GridLayout::LayoutParams::LayoutParams(const Spec& rowSpec,const Spec& columnSpec):
    LayoutParams(DEFAULT_WIDTH,DEFAULT_HEIGHT,DEFAULT_MARGIN,DEFAULT_MARGIN,
            DEFAULT_MARGIN,DEFAULT_MARGIN,rowSpec,columnSpec){
}

GridLayout::LayoutParams::LayoutParams()
    :MarginLayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT){
    rowSpec=GridLayout::spec(GridLayout::UNDEFINED);
    columnSpec=GridLayout::spec(GridLayout::UNDEFINED);
}

GridLayout::LayoutParams::LayoutParams(const LayoutParams& source)
    :MarginLayoutParams(source){
    rowSpec   = source.rowSpec;
    columnSpec= source.columnSpec;
}

GridLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& params)
    :MarginLayoutParams(params){
    rowSpec=GridLayout::spec(GridLayout::UNDEFINED);
    columnSpec=GridLayout::spec(GridLayout::UNDEFINED);
}

GridLayout::LayoutParams::LayoutParams(const MarginLayoutParams& params)
    :MarginLayoutParams(params){
    rowSpec=GridLayout::spec(GridLayout::UNDEFINED);
    columnSpec=GridLayout::spec(GridLayout::UNDEFINED);
}

GridLayout::LayoutParams::LayoutParams(Context* context,const AttributeSet& attrs)
    :MarginLayoutParams(context,attrs){

    const int gravity = attrs.getGravity("layout_gravity", Gravity::NO_GRAVITY);
 
    const int column = attrs.getInt("layout_column", DEFAULT_COLUMN);
    const int colSpan = attrs.getInt("layout_columnSpan", DEFAULT_SPAN_SIZE);
    const float colWeight = attrs.getFloat("layout_columnWeight", Spec::DEFAULT_WEIGHT);
    this->columnSpec = spec(column, colSpan, getAlignment(gravity, true), colWeight);
 
    const int row = attrs.getInt("layout_row", DEFAULT_ROW);
    const int rowSpan = attrs.getInt("layout_rowSpan", DEFAULT_SPAN_SIZE);
    const float rowWeight = attrs.getFloat("layout_rowWeight", Spec::DEFAULT_WEIGHT);
    this->rowSpec = spec(row, rowSpan, getAlignment(gravity, false), rowWeight);
}

void GridLayout::LayoutParams::setGravity(int gravity){
    rowSpec = rowSpec.copyWriteAlignment(getAlignment(gravity, false));
    columnSpec = columnSpec.copyWriteAlignment(getAlignment(gravity, true));
}

void GridLayout::LayoutParams::setRowSpecSpan(const Interval&span){
    rowSpec = rowSpec.copyWriteSpan(span);
}

void GridLayout::LayoutParams::setColumnSpecSpan(const Interval&span){
    columnSpec = columnSpec.copyWriteSpan(span);
}

int GridLayout::LayoutParams::hashCode()const{
    int result = rowSpec.hashCode();
    result = 31 * result + columnSpec.hashCode();
    return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GridLayout::GridLayout(int w,int h)
    :ViewGroup(w,h){
    initGridLayout();
}

GridLayout::GridLayout(Context*ctx,const AttributeSet&attrs)
    :ViewGroup(ctx,attrs){
    initGridLayout();
    setOrientation(attrs.getInt("orientation",std::unordered_map<std::string,int>{
        {"horizontal",(int)HORIZONTAL},
        {"vertical",(int)VERTICAL}//
    },DEFAULT_ORIENTATION));
    setRowCount(attrs.getInt("rowCount", DEFAULT_COUNT));
    setColumnCount(attrs.getInt("columnCount", DEFAULT_COUNT));
    setUseDefaultMargins(attrs.getBoolean("useDefaultMargin", DEFAULT_USE_DEFAULT_MARGINS));
    setAlignmentMode(attrs.getInt("alignmentMode", DEFAULT_ALIGNMENT_MODE));
    setRowOrderPreserved(attrs.getBoolean("rowOrderPreserved", DEFAULT_ORDER_PRESERVED));
    setColumnOrderPreserved(attrs.getBoolean("columnOrderPreserved", DEFAULT_ORDER_PRESERVED));
}

GridLayout::~GridLayout(){
    delete mHorizontalAxis;
    delete mVerticalAxis;
}

void GridLayout::initGridLayout(){
    mDefaultGap = 0;
    mOrientation    = DEFAULT_ORIENTATION;//HORIZONTAL
    mAlignmentMode  = DEFAULT_ALIGNMENT_MODE;
    mHorizontalAxis = new Axis(this,true);
    mVerticalAxis   = new Axis(this,false);
    mUseDefaultMargins = DEFAULT_USE_DEFAULT_MARGINS;
    mLastLayoutParamsHashCode = UNINITIALIZED_HASH;
}

int GridLayout::getOrientation()const{
    return mOrientation;
}

void GridLayout::setOrientation(int orientation){
   if(mOrientation!=orientation){
       mOrientation = orientation;
       invalidateStructure();
       requestLayout();
   }
}

int GridLayout::getRowCount()const{
    return mVerticalAxis->getCount();
}

void GridLayout::setRowCount(int count){
    mVerticalAxis->setCount(count);
    invalidateStructure();
    requestLayout();
}

int GridLayout::getColumnCount()const{
    return mHorizontalAxis->getCount();
}

void GridLayout::setColumnCount(int columnCount){
    mHorizontalAxis->setCount(columnCount);
    invalidateStructure();
    requestLayout();
}

bool GridLayout::getUseDefaultMargins()const{
    return mUseDefaultMargins;
}

void GridLayout::setUseDefaultMargins(bool useDefaultMargins){
    mUseDefaultMargins = useDefaultMargins;
    requestLayout();
}

int GridLayout::getAlignmentMode()const{
    return  mAlignmentMode;
}

std::string GridLayout::getAccessibilityClassName()const{
    return "GridLayout";
}

void GridLayout::setAlignmentMode(int alignmentMode){
    mAlignmentMode= alignmentMode;
    requestLayout();
}

bool GridLayout::isRowOrderPreserved()const{
    return mVerticalAxis->isOrderPreserved();
}

void GridLayout::setRowOrderPreserved(bool rowOrderPreserved){
    mVerticalAxis->setOrderPreserved(rowOrderPreserved);
    invalidateStructure();
    requestLayout();
}

bool GridLayout::isColumnOrderPreserved()const{
    return mHorizontalAxis->isOrderPreserved();
}

void GridLayout::setColumnOrderPreserved(bool columnOrderPreserved){
    mHorizontalAxis->setOrderPreserved(columnOrderPreserved);
    invalidateStructure();
    requestLayout();
}

int GridLayout::max2(const std::vector<int>& array, int valueIfEmpty) {
    int result = valueIfEmpty;
    for (auto i:array) {
        result = std::max(result, i);
    }
    return result;
}

const GridLayout::Alignment* GridLayout::getAlignment(int gravity, bool horizontal){
    const int mask = horizontal ? Gravity::HORIZONTAL_GRAVITY_MASK : Gravity::VERTICAL_GRAVITY_MASK;
    const int shift = horizontal ? Gravity::AXIS_X_SHIFT : Gravity::AXIS_Y_SHIFT;
    const int flags = (gravity & mask) >> shift;
    switch (flags) {
    case (Gravity::AXIS_SPECIFIED | Gravity::AXIS_PULL_BEFORE):
        return horizontal ? LEFT : TOP;
    case (Gravity::AXIS_SPECIFIED | Gravity::AXIS_PULL_AFTER):
        return horizontal ? RIGHT : BOTTOM;
    case (Gravity::AXIS_SPECIFIED | Gravity::AXIS_PULL_BEFORE | Gravity::AXIS_PULL_AFTER):
        return FILL;
    case Gravity::AXIS_SPECIFIED:
        return CENTER;
    case (Gravity::AXIS_SPECIFIED | Gravity::AXIS_PULL_BEFORE | Gravity::RELATIVE_LAYOUT_DIRECTION):
        return START;
    case (Gravity::AXIS_SPECIFIED | Gravity::AXIS_PULL_AFTER | Gravity::RELATIVE_LAYOUT_DIRECTION):
        return END;
    default:return UNDEFINED_ALIGNMENT;
    }
}

int GridLayout::getDefaultMargin(View* c)const{
    if(!mUseDefaultMargins){
        return 0;
    }
    if (dynamic_cast<Space*>(c)) {
        return 0;
    }
    return mDefaultGap / 2;
}

int GridLayout::getMargin1(View* view, bool horizontal, bool leading){
    LayoutParams* lp = getLayoutParams(view);
    const int margin = horizontal ?
          (leading ? lp->leftMargin : lp->rightMargin) :
          (leading ? lp->topMargin : lp->bottomMargin);
    return margin == UNDEFINED ? getDefaultMargin(view) : margin;
}

int GridLayout::getMargin(View* view, bool horizontal, bool leading){
    if (mAlignmentMode == ALIGN_MARGINS) {
        return getMargin1(view, horizontal, leading);
    } else {
        Axis* axis = horizontal ? mHorizontalAxis : mVerticalAxis;
        std::vector<int>& margins=leading?axis->getLeadingMargins()
            :axis->getTrailingMargins();
        LayoutParams* lp = getLayoutParams(view);
        Spec spec = horizontal ? lp->columnSpec : lp->rowSpec;
        int index = leading ? spec.span.min : spec.span.max;
        return margins[index];
   }
}

int GridLayout::getTotalMargin(View* child, bool horizontal){
    return getMargin(child, horizontal, true) + getMargin(child, horizontal, false);
}

bool GridLayout::fits(std::vector<int>&a, int value, int start, int end){
    if (end > a.size()) {
       return false;
    }
    for (int i = start; i < end; i++) {
       if (a[i] > value) {
          return false;
       }
    }
    return true;
}

void GridLayout::procrusteanFill(std::vector<int>& a, int start, int end, int value){
    const int ms = std::min((size_t)start,a.size());
    const int me = std::min((size_t)end,a.size());
    for(int i = ms;i < me;i++)a[i]=value;
}

void GridLayout::setCellGroup(LayoutParams* lp, int row, int rowSpan, int col, int colSpan){
    lp->setRowSpecSpan(Interval(row,row+rowSpan));
    lp->setColumnSpecSpan(Interval(col,col+colSpan));
}

int GridLayout::clip(Interval minorRange, bool minorWasDefined, int count){
    const int size = minorRange.size();
    if (count == 0) {
        return size;
    }
    int min = minorWasDefined ? std::min(minorRange.min, count) : 0;
    return std::min(size, count - min);
}

void GridLayout::validateLayoutParams() {
    const bool horizontal = (mOrientation == HORIZONTAL);
    Axis* axis = horizontal ? mHorizontalAxis : mVerticalAxis;
    const int count = (axis->definedCount != UNDEFINED) ? axis->definedCount : 0;

    int major = 0;
    int minor = 0;
    std::vector<int>maxSizes;
    maxSizes.resize(count);

    for (int i = 0, N = getChildCount(); i < N; i++) {
        LayoutParams* lp = (LayoutParams*) getChildAt(i)->getLayoutParams();

        Spec& majorSpec = horizontal ? lp->rowSpec : lp->columnSpec;
        Interval& majorRange = majorSpec.span;
        bool majorWasDefined = majorSpec.startDefined;
        int majorSpan = majorRange.size();
        if (majorWasDefined) {
            major = majorRange.min;
        }

        Spec& minorSpec = horizontal ? lp->columnSpec : lp->rowSpec;
        Interval& minorRange = minorSpec.span;
        const bool minorWasDefined = minorSpec.startDefined;
        const int minorSpan = clip(minorRange, minorWasDefined, count);
        if (minorWasDefined) {
            minor = minorRange.min;
        }

        if (count != 0) {
            // Find suitable row/col values when at least one is undefined.
            if (!majorWasDefined || !minorWasDefined) {
                while (!fits(maxSizes, major, minor, minor + minorSpan)) {
                    if (minorWasDefined) {
                        major++;
                    } else {
                        if (minor + minorSpan <= count) {
                            minor++;
                        } else {
                            minor = 0;
                            major++;
                        }
                    }
                }
            }
            procrusteanFill(maxSizes, minor, minor + minorSpan, major + majorSpan);
        }

        if (horizontal) {
            setCellGroup(lp, major, majorSpan, minor, minorSpan);
        } else {
            setCellGroup(lp, minor, minorSpan, major, majorSpan);
        }
        minor = minor + minorSpan;
    }
}

void GridLayout::invalidateStructure(){
    mLastLayoutParamsHashCode = UNINITIALIZED_HASH;
    if(mHorizontalAxis!=nullptr) mHorizontalAxis->invalidateStructure();
    if(mVerticalAxis != nullptr) mVerticalAxis->invalidateStructure();
    invalidateValues();
}

void GridLayout::invalidateValues(){
    // Need null check because requestLayout() is called in View's initializer,
    // before we are set up.
    if (mHorizontalAxis != nullptr && mVerticalAxis != nullptr) {
        mHorizontalAxis->invalidateValues();
        mVerticalAxis->invalidateValues();
    }
}

void GridLayout::onSetLayoutParams(View* child,const ViewGroup::LayoutParams* layoutParams){
    ViewGroup::onSetLayoutParams(child,layoutParams);
    if (!checkLayoutParams(layoutParams)) {
        handleInvalidParams("supplied LayoutParams are of the wrong type");
    }
    invalidateStructure();
}

GridLayout::LayoutParams* GridLayout::getLayoutParams(View* c) {
    return (LayoutParams*) c->getLayoutParams();
}

void GridLayout::handleInvalidParams(const std::string& msg){
    throw std::runtime_error(msg.c_str());
}

void GridLayout::checkLayoutParams(const LayoutParams* lp, bool horizontal)const{
    std::string groupName = horizontal ? "column" : "row";
    Spec spec = horizontal ? lp->columnSpec : lp->rowSpec;
    Interval span = spec.span;
    if (span.min != UNDEFINED && span.min < 0) {
        handleInvalidParams(groupName + " indices must be positive");
    }
    Axis* axis = horizontal ? mHorizontalAxis : mVerticalAxis;
    int count = axis->definedCount;
    if (count != UNDEFINED) {
        if (span.max > count) {
            handleInvalidParams(groupName +
                    " indices (start + span) mustn't exceed the " + groupName + " count");
        }
        if (span.size() > count) {
            handleInvalidParams(groupName + " span mustn't exceed the " + groupName + " count");
        }
    }
}

bool GridLayout::checkLayoutParams(const ViewGroup::LayoutParams* p)const{
    if (dynamic_cast<const LayoutParams*>(p)==nullptr){
        return false;
    }
    LayoutParams* lp = (LayoutParams*) p;
    checkLayoutParams(lp, true);
    checkLayoutParams(lp, false);
    return true;
}

GridLayout::LayoutParams* GridLayout::generateDefaultLayoutParams()const{
    return new LayoutParams();
}

GridLayout::LayoutParams* GridLayout::generateLayoutParams(const AttributeSet&atts)const{
    return new LayoutParams(getContext(),atts);
}

GridLayout::LayoutParams* GridLayout::generateLayoutParams(const ViewGroup::LayoutParams* lp)const{
    if (sPreserveMarginParamsInLayoutParamConversion){
        if (dynamic_cast<const LayoutParams*>(lp)) {
            return new LayoutParams((const LayoutParams&)*lp);
        } else if (dynamic_cast<const MarginLayoutParams*>(lp)) {
           return new LayoutParams((const MarginLayoutParams&)*lp);
        }
    }
    return new LayoutParams(*lp);
}

void GridLayout::drawLine(Canvas& canvas, int x1, int y1, int x2, int y2){
    if(isLayoutRtl()){
       const int width = getWidth();
       canvas.move_to(width-x1,y1);
       canvas.line_to(width-x2,y2);
    }else{
       canvas.move_to(x1,y1);
       canvas.line_to(x2,y2);
    }
    canvas.stroke();
}

void GridLayout::onDebugDrawMargins(Canvas& canvas){
    LayoutParams* lp = new LayoutParams();
    for (int i = 0; i < getChildCount(); i++) {
        View* c = getChildAt(i);
        lp->setMargins(getMargin1(c, true, true),getMargin1(c, false, true),
             getMargin1(c, true, false),  getMargin1(c, false, false));
        lp->onDebugDraw(*c, canvas);
    }
}

void GridLayout::onDebugDraw(Canvas& canvas){
    Insets insets = getOpticalInsets();

    int top    = getPaddingTop()    + insets.top;
    int left   = getPaddingLeft()   + insets.left;
    int right  = getWidth()  - getPaddingRight()  - insets.right;
    int bottom = getHeight() - getPaddingBottom() - insets.bottom;

    std::vector<int>& xs = mHorizontalAxis->locations;
    for (int i = 0, length = xs.size(); i < length; i++) {
        int x = left + xs[i];
        drawLine(canvas, x, top, x, bottom);
    }

    std::vector<int>& ys = mVerticalAxis->locations;
    for (int i = 0, length = ys.size(); i < length; i++) {
        int y = top + ys[i];
        drawLine(canvas, left, y, right, y);
    }

    ViewGroup::onDebugDraw(canvas);
}

void GridLayout::onViewAdded(View*v){
    ViewGroup::onViewAdded(v);
    invalidateStructure();
}

void GridLayout::onViewRemoved(View*v){
    ViewGroup::onViewRemoved(v);
    invalidateStructure();
}

void GridLayout::onChildVisibilityChanged(View* child, int oldVisibility, int newVisibility) {
    ViewGroup::onChildVisibilityChanged(child, oldVisibility, newVisibility);
    if (oldVisibility == GONE || newVisibility == GONE) {
        invalidateStructure();
    }
}

int GridLayout::computeLayoutParamsHashCode() {
    int result = 1;
    for (int i = 0, N = getChildCount(); i < N; i++) {
        View* c = getChildAt(i);
        if (c->getVisibility() == View::GONE) continue;
        LayoutParams* lp = (LayoutParams*) c->getLayoutParams();
        result = 31 * result + lp->hashCode();
    }
    return result;
}

void GridLayout::consistencyCheck() {
    if (mLastLayoutParamsHashCode == UNINITIALIZED_HASH) {
        validateLayoutParams();
        mLastLayoutParamsHashCode = computeLayoutParamsHashCode();
    } else if (mLastLayoutParamsHashCode != computeLayoutParamsHashCode()) {
        LOGD("The fields of some layout parameters were modified in between "
             "layout operations. Check GridLayout.LayoutParams#rowSpec.");
        invalidateStructure();
        consistencyCheck();
    }
}

void GridLayout::measureChildWithMargins2(View* child, int parentWidthSpec, int parentHeightSpec,
        int childWidth, int childHeight){
    const int childWidthSpec = getChildMeasureSpec(parentWidthSpec,
             getTotalMargin(child, true), childWidth);
    const int childHeightSpec = getChildMeasureSpec(parentHeightSpec,
             getTotalMargin(child, false), childHeight);
    child->measure(childWidthSpec, childHeightSpec);
}

void GridLayout::measureChildrenWithMargins(int widthSpec, int heightSpec, bool firstPass){
    for (int i = 0, N = getChildCount(); i < N; i++) {
        View* c = getChildAt(i);
        if (c->getVisibility() == View::GONE) continue;
        LayoutParams* lp = getLayoutParams(c);
        if (firstPass) {
            measureChildWithMargins2(c, widthSpec, heightSpec, lp->width, lp->height);
        } else {
            bool horizontal = (mOrientation == HORIZONTAL);
            Spec& spec = horizontal ? lp->columnSpec : lp->rowSpec;
            if (spec.getAbsoluteAlignment(horizontal) == FILL) {
                Interval& span = spec.span;
                Axis* axis = horizontal ? mHorizontalAxis : mVerticalAxis;
                std::vector<int> locations = axis->getLocations();
                int cellSize = locations[span.max] - locations[span.min];
                int viewSize = cellSize - getTotalMargin(c, horizontal);
                if (horizontal) {
                    measureChildWithMargins2(c, widthSpec, heightSpec, viewSize, lp->height);
                } else {
                    measureChildWithMargins2(c, widthSpec, heightSpec, lp->width, viewSize);
                }
            }
        }
    }
}

int GridLayout::adjust(int measureSpec, int delta) {
    return MeasureSpec::makeMeasureSpec( MeasureSpec::getSize(measureSpec + delta),  MeasureSpec::getMode(measureSpec));
}

bool GridLayout::canStretch(int flexibility) {
    return (flexibility & CAN_STRETCH) != 0;
}

void GridLayout::onMeasure(int widthSpec, int heightSpec){
    consistencyCheck();

    /** If we have been called by {@link View#measure(int, int)}, one of width or height
     *  is  likely to have changed. We must invalidate if so. */
    invalidateValues();

    const int hPadding = getPaddingLeft() + getPaddingRight();
    const int vPadding = getPaddingTop()  + getPaddingBottom();

    const int widthSpecSansPadding =  adjust( widthSpec, -hPadding);
    const int heightSpecSansPadding = adjust(heightSpec, -vPadding);

    measureChildrenWithMargins(widthSpecSansPadding, heightSpecSansPadding, true);

    int widthSansPadding;
    int heightSansPadding;

    // Use the orientation property to decide which axis should be laid out first.
    if (mOrientation == HORIZONTAL) {
        widthSansPadding = mHorizontalAxis->getMeasure(widthSpecSansPadding);
        measureChildrenWithMargins(widthSpecSansPadding, heightSpecSansPadding, false);
        heightSansPadding = mVerticalAxis->getMeasure(heightSpecSansPadding);
    } else {
        heightSansPadding = mVerticalAxis->getMeasure(heightSpecSansPadding);
        measureChildrenWithMargins(widthSpecSansPadding, heightSpecSansPadding, false);
        widthSansPadding = mHorizontalAxis->getMeasure(widthSpecSansPadding);
    }

    const int measuredWidth  = std::max(widthSansPadding  + hPadding, getSuggestedMinimumWidth());
    const int measuredHeight = std::max(heightSansPadding + vPadding, getSuggestedMinimumHeight());

    setMeasuredDimension(
            resolveSizeAndState(measuredWidth,   widthSpec, 0),
            resolveSizeAndState(measuredHeight, heightSpec, 0));
}

int GridLayout::getMeasurement(View* c, bool horizontal) {
    return horizontal ? c->getMeasuredWidth() : c->getMeasuredHeight();
}

int GridLayout::getMeasurementIncludingMargin(View* c, bool horizontal){
    if (c->getVisibility() == View::GONE) {
        return 0;
    }
    return getMeasurement(c, horizontal) + getTotalMargin(c, horizontal);
}

void GridLayout::requestLayout() {
    ViewGroup::requestLayout();
    invalidateValues();
}

void GridLayout::onLayout(bool changed, int left, int top, int w, int h){
    consistencyCheck();

    int targetWidth = w;//right - left;
    int targetHeight= h;//bottom - top;

    int paddingLeft = getPaddingLeft();
    int paddingTop = getPaddingTop();
    int paddingRight = getPaddingRight();
    int paddingBottom = getPaddingBottom();

    mHorizontalAxis->layout(targetWidth - paddingLeft - paddingRight);
    mVerticalAxis->layout(targetHeight - paddingTop - paddingBottom);

    std::vector<int> hLocations = mHorizontalAxis->getLocations();
    std::vector<int> vLocations = mVerticalAxis->getLocations();

    for (int i = 0, N = getChildCount(); i < N; i++) {
        View* c = getChildAt(i);
        if (c->getVisibility() == View::GONE) continue;
        LayoutParams* lp = getLayoutParams(c);
        Spec& columnSpec = lp->columnSpec;
        Spec& rowSpec = lp->rowSpec;

        Interval& colSpan = columnSpec.span;
        Interval& rowSpan = rowSpec.span;

        const int x1 = hLocations[colSpan.min];
        const int y1 = vLocations[rowSpan.min];

        const int x2 = hLocations[colSpan.max];
        const int y2 = vLocations[rowSpan.max];

        const int cellWidth = x2 - x1;
        const int cellHeight = y2 - y1;

        const int pWidth = getMeasurement(c, true);
        const int pHeight = getMeasurement(c, false);

        const Alignment* hAlign = columnSpec.getAbsoluteAlignment(true);
        const Alignment* vAlign = rowSpec.getAbsoluteAlignment(false);

        Bounds& boundsX = mHorizontalAxis->getGroupBounds().getValue(i);
        Bounds& boundsY = mVerticalAxis->getGroupBounds().getValue(i);

        // Gravity offsets: the location of the alignment group relative to its cell group.
        const int gravityOffsetX = hAlign->getGravityOffset(c, cellWidth - boundsX.size(true));
        const int gravityOffsetY = vAlign->getGravityOffset(c, cellHeight - boundsY.size(true));

        const int leftMargin = getMargin(c, true, true);
        const int topMargin = getMargin(c, false, true);
        const int rightMargin = getMargin(c, true, false);
        const int bottomMargin = getMargin(c, false, false);

        const int sumMarginsX = leftMargin + rightMargin;
        const int sumMarginsY = topMargin + bottomMargin;

        // Alignment offsets: the location of the view relative to its alignment group.
        const int alignmentOffsetX = boundsX.getOffset(this, c, hAlign, pWidth + sumMarginsX, true);
        const int alignmentOffsetY = boundsY.getOffset(this, c, vAlign, pHeight + sumMarginsY, false);

        const int width = hAlign->getSizeInCell(c, pWidth, cellWidth - sumMarginsX);
        const int height = vAlign->getSizeInCell(c, pHeight, cellHeight - sumMarginsY);

        const int dx = x1 + gravityOffsetX + alignmentOffsetX;

        const int cx = !isLayoutRtl() ? paddingLeft + leftMargin + dx :
                targetWidth - width - paddingRight - rightMargin - dx;
        const int cy = paddingTop + y1 + gravityOffsetY + alignmentOffsetY + topMargin;

        if (width != c->getMeasuredWidth() || height != c->getMeasuredHeight()) {
            c->measure(MeasureSpec::makeMeasureSpec(width, MeasureSpec::EXACTLY), 
                       MeasureSpec::makeMeasureSpec(height, MeasureSpec::EXACTLY));
        }
        LOGV("child %p:%d pos=%d,%d",c,c->getId(),x1,y1);
        c->layout(cx, cy, width, height);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////

GridLayout::Interval::Interval(){
    this->min = this->max = 0;
}

GridLayout::Interval::Interval(int min,int max){
    this->min = min;
    this->max = max;
}

int GridLayout::Interval::size()const{
   return max-min;
}

GridLayout::Interval GridLayout::Interval::inverse(){
    return Interval(max,min);
}

int GridLayout::Interval::hashCode()const{
    return 31*min +max;
}

bool GridLayout::Interval::operator<(const Interval &other) const{
    const int h1=hashCode();
    const int h2=other.hashCode();
    return h1<h2;
}
//--------------------------------------------------------------------------
GridLayout::Arc::Arc(){
    valid =true;
}
GridLayout::Arc::Arc(const GridLayout::Interval& span,const GridLayout::MutableInt& value):Arc(){
    this->span = span;
    this->value= value;
}

//--------------------------------------------------------------------------

GridLayout::Bounds *GridLayout::Alignment::getBounds()const{
    return new Bounds();
}

int GridLayout::Alignment::hashCode()const{
    return (int)(long int)this;
}

//--------------------------------------------------------------------------

GridLayout::Bounds::Bounds(){
   reset();
}

void GridLayout::Bounds::reset(){
    before = after = INT_MIN;
    flexibility= CAN_STRETCH;
}

void GridLayout::Bounds::include(int before,int after){
    this->before = std::max(this->before, before);
    this->after = std::max(this->after, after);
}

int GridLayout::Bounds::size(bool min){
    if (!min) {
        if (canStretch(flexibility)) {
            return MAX_SIZE;
        }
    }
    return before + after;
}

int GridLayout::Bounds::getOffset(GridLayout*gl,View*c,const GridLayout::Alignment*a,int size,bool horizontal){
    return before - a->getAlignmentValue(c, size, gl->getLayoutMode());
}

void GridLayout::Bounds::include(GridLayout* gl, View* c,Spec* spec, Axis* axis, int size) {
    this->flexibility &= spec->getFlexibility();
    bool horizontal = axis->horizontal;
    const Alignment* alignment = spec->getAbsoluteAlignment(axis->horizontal);
    // todo test this works correctly when the returned value is UNDEFINED
    int before = alignment->getAlignmentValue(c, size, gl->getLayoutMode());
    include(before, size - before);
}

//--------------------------------------------------------------------------
const GridLayout::Spec  GridLayout::Spec::UNDEFINED=GridLayout::spec(GridLayout::UNDEFINED);

GridLayout::Spec::Spec(){
    alignment =GridLayout::UNDEFINED_ALIGNMENT;
}

GridLayout::Spec::Spec(bool startDefined,const Interval& span,const Alignment* alignment, float weight){
    this->startDefined = startDefined;
    this->span = span;
    this->alignment = alignment;
    this->weight = weight;
}

GridLayout::Spec::Spec(bool startDefined, int start, int size,const Alignment* alignment, float weight){
    this->startDefined = startDefined;
    this->span = Interval(start,start+size);
    this->alignment = alignment;
    this->weight = weight;
}

int GridLayout::Spec::hashCode()const{
    int result = span.hashCode();
    result = 31 * result + alignment->hashCode();
    return result;
}

bool GridLayout::Spec::operator<(const Spec &l1) const{
    return hashCode()<l1.hashCode();
}

const GridLayout::Alignment* GridLayout::Spec::getAbsoluteAlignment(bool horizontal){
    if (alignment != GridLayout::UNDEFINED_ALIGNMENT) {
        return alignment;
    }
    if (weight == 0.f) {
        return horizontal ? GridLayout::START : GridLayout::BASELINE;
    }
    return GridLayout::FILL;
}

GridLayout::Spec GridLayout::Spec::copyWriteSpan(Interval span){
    return Spec(startDefined, span, alignment, weight);
}

GridLayout::Spec GridLayout::Spec::copyWriteAlignment(const Alignment* alignment){
    return Spec(startDefined, span, alignment, weight);
}

int GridLayout::Spec::getFlexibility() {
    return (alignment == GridLayout::UNDEFINED_ALIGNMENT && weight == 0) ? GridLayout::INFLEXIBLE : GridLayout::CAN_STRETCH;
}

GridLayout::Spec GridLayout::spec(int start, int size,const Alignment* alignment, float weight){
    return Spec(start != UNDEFINED, start, size, alignment, weight);
}

GridLayout::Spec GridLayout::spec(int start,const Alignment* alignment, float weight){
    return spec(start, 1, alignment, weight);
}

GridLayout::Spec GridLayout::spec(int start, int size,float weight){
    return spec(start, size, UNDEFINED_ALIGNMENT, weight);
}

GridLayout::Spec GridLayout::spec(int start, float weight){
    return spec(start, 1, weight);
}

GridLayout::Spec GridLayout::spec(int start, int size,const Alignment* alignment){
    return spec(start, size, alignment, Spec::DEFAULT_WEIGHT);
}

GridLayout::Spec GridLayout::spec(int start,const Alignment* alignment){
    return spec(start, 1, alignment);
}

GridLayout::Spec GridLayout::spec(int start, int size){
    return spec(start, size, UNDEFINED_ALIGNMENT);
}

GridLayout::Spec GridLayout::spec(int start){
    return spec(start, 1);
}

//--------------------------------------------------------------------------

class UndefinedAlignment:public GridLayout::Alignment{
public:
    int getGravityOffset(View* view, int cellDelta) const override{
        return GridLayout::UNDEFINED;
    }
    int getAlignmentValue(View* view, int viewSize, int mode) const override{
        return GridLayout::UNDEFINED;
    }
};

class LeadingAlignment:public GridLayout::Alignment{
public:
    int getGravityOffset(View* view, int cellDelta) const override{
        return 0;
    }
    int getAlignmentValue(View* view, int viewSize, int mode) const override{
        return 0;
    }
};

class TrailingAlignment:public GridLayout::Alignment{
public:
    int getGravityOffset(View* view, int cellDelta) const override{
        return cellDelta;
    }
    int getAlignmentValue(View* view, int viewSize, int mode) const override{
        return viewSize;
    }
};

class CenterAlignment:public GridLayout::Alignment{
public:
    int getGravityOffset(View* view, int cellDelta) const override{
        return cellDelta>>1;
    }
    int getAlignmentValue(View* view, int viewSize, int mode) const override{
        return viewSize>>1;
    }
};

class SwitchAlignment:public GridLayout::Alignment{
private:
    GridLayout::Alignment*ltr,*rtl;
public:
    SwitchAlignment(GridLayout::Alignment*l2r,GridLayout::Alignment*r2l){
        ltr = l2r;
        rtl = r2l;
    }
    int getGravityOffset(View* view, int cellDelta) const override{
        return (!view->isLayoutRtl() ? ltr : rtl)->getGravityOffset(view, cellDelta);
    }

    int getAlignmentValue(View* view, int viewSize, int mode) const override{
        return (!view->isLayoutRtl() ? ltr : rtl)->getAlignmentValue(view, viewSize, mode);
    }
};

class BaselineAlignment:public GridLayout::Alignment{
protected:
    class BaseBounds:public GridLayout::Bounds{
    private:
        int mSize;
        void reset()override {
            Bounds::reset();
            mSize = INT_MIN;
        }
    protected:
   	    void include(int before, int after)override {
            Bounds::include(before, after);
            mSize = std::max(mSize, before + after);
        }

        int size(bool min)override{
            return std::max(Bounds::size(min), mSize);
        }

        int getOffset(GridLayout* gl, View* c,const Alignment* a, int size, bool hrz)override{
            return std::max(0, Bounds::getOffset(gl, c, a, size, hrz));
        }
    };
public:
    int getGravityOffset(View* view, int cellDelta)const override{ 
        return 0; // baseline gravity is top
    }
    int getAlignmentValue(View* view, int viewSize, int mode)const override{
        if (view->getVisibility() == View::GONE) {
            return 0;
        }
        const int baseline = view->getBaseline();
        return baseline == -1 ? GridLayout::UNDEFINED : baseline;
    }
    GridLayout::Bounds*getBounds()const{
        return new BaseBounds();
    }
};

class FillAlignment:public GridLayout::Alignment{
public:
    int getGravityOffset(View* view, int cellDelta) const override{
        return 0;
    }
    int getAlignmentValue(View* view, int viewSize, int mode) const override{
        return GridLayout::UNDEFINED;
    }
    int getSizeInCell(View* view, int viewSize, int cellSize) {
        return cellSize;
    }
};

namespace {
    BaselineAlignment __BASELINE__;
    LeadingAlignment __LEADING__;
    TrailingAlignment __TRAILING__;
    SwitchAlignment __LEFT__(&__LEADING__,&__TRAILING__);//GridLayout::START,GridLayout::END);
    SwitchAlignment __RIGHT__(&__TRAILING__,&__LEADING__);//GridLayout::END,GridLayout::START);
    CenterAlignment __CENTER__;
    FillAlignment __FILL__;
    UndefinedAlignment __UNDEFINED_ALIGNMENT__;
}

const GridLayout::Alignment*GridLayout::BASELINE = &__BASELINE__;//new BaselineAlignment();
const GridLayout::Alignment*GridLayout::LEADING = &__LEADING__;
const GridLayout::Alignment*GridLayout::TRAILING = &__TRAILING__;//new TrailingAlignment();
const GridLayout::Alignment*GridLayout::TOP   = &__LEADING__;
const GridLayout::Alignment*GridLayout::BOTTOM= &__TRAILING__;
const GridLayout::Alignment*GridLayout::START = &__LEADING__;
const GridLayout::Alignment*GridLayout::END   = &__TRAILING__;
const GridLayout::Alignment*GridLayout::LEFT  = &__LEFT__;//new SwitchAlignment(GridLayout::START,GridLayout::END);
const GridLayout::Alignment*GridLayout::RIGHT = &__RIGHT__;//new SwitchAlignment(GridLayout::END,GridLayout::START);
const GridLayout::Alignment*GridLayout::CENTER= &__CENTER__;//new CenterAlignment();
const GridLayout::Alignment*GridLayout::FILL  = &__FILL__;//new FillAlignment();
const GridLayout::Alignment*GridLayout::UNDEFINED_ALIGNMENT=&__UNDEFINED_ALIGNMENT__;

//--------------------------------------------------------------------------

GridLayout::Axis::Axis(GridLayout*g,bool horizontal){
    grd=g;
    maxIndex  = UNDEFINED;
    this->horizontal = horizontal;
    parentMin = 0;
    parentMax =-MAX_SIZE;
    definedCount = UNDEFINED;
    arcsValid = false;
    hasWeightsValid = false;
    locationsValid  = false;
    groupBoundsValid= false;
    forwardLinksValid = false;
    backwardLinksValid= false;
    leadingMarginsValid = false;
    trailingMarginsValid= false;
    orderPreserved = DEFAULT_ORDER_PRESERVED;
}

int GridLayout::Axis::calculateMaxIndex()const{
    int result = -1;
    for (int i = 0, N = grd->getChildCount(); i < N; i++) {
        View* c = grd->getChildAt(i);
        LayoutParams* params = grd->getLayoutParams(c);
        Spec& spec = horizontal ? params->columnSpec : params->rowSpec;
        Interval& span = spec.span;
        result = std::max(result, span.min);
        result = std::max(result, span.max);
        result = std::max(result, span.size());
    }
    return result == -1 ? UNDEFINED : result;
}

int GridLayout::Axis::getMaxIndex() {
    if(maxIndex==UNDEFINED)
       maxIndex =std::max(0,calculateMaxIndex());
    return maxIndex;
}

int GridLayout::Axis::getCount() {
    return std::max(definedCount,getMaxIndex());
}

void GridLayout::Axis::setCount(int count){
    if (count != UNDEFINED && count < getMaxIndex()) {
        handleInvalidParams((horizontal ? std::string("column") : std::string("row")) +
            "Count must be greater than or equal to the maximum of all grid indices " 
            "(and spans) defined in the LayoutParams of each child");
    }
    this->definedCount = count;
}

bool GridLayout::Axis::isOrderPreserved()const{
    return orderPreserved;
}

void GridLayout::Axis::setOrderPreserved(bool orderPreserved){
    this->orderPreserved = orderPreserved;
    invalidateStructure();
}

GridLayout::PackedMap<GridLayout::Spec,GridLayout::Bounds>GridLayout::Axis::createGroupBounds(){
    Assoc<Spec, Bounds> assoc;// = Assoc.of(Spec.class, Bounds.class);
    for (int i = 0, N = grd->getChildCount(); i < N; i++) {
        View* c = grd->getChildAt(i);
        // we must include views that are GONE here, see introductory javadoc
        LayoutParams* lp = grd->getLayoutParams(c);
        Spec& spec = horizontal ? lp->columnSpec : lp->rowSpec;
        Bounds bounds =*(Bounds*)spec.getAbsoluteAlignment(horizontal)->getBounds();
        assoc.put(spec, bounds);
    }
    return assoc.pack();
}

void GridLayout::Axis::computeGroupBounds(){
    std::vector<Bounds>& values = groupBounds.values;
    for (int i = 0; i < values.size(); i++) {
        values[i].reset();
    }
    for (int i = 0, N = grd->getChildCount(); i < N; i++) {
        View* c = grd->getChildAt(i);
        // we must include views that are GONE here, see introductory javadoc
        LayoutParams* lp = grd->getLayoutParams(c);
        Spec& spec = horizontal ? lp->columnSpec : lp->rowSpec;
        const int size = grd->getMeasurementIncludingMargin(c, horizontal) +
                ((spec.weight == 0) ? 0 : getDeltas()[i]);
        groupBounds.getValue(i).include(grd, c, &spec, this, size);
    }
}

int  GridLayout::Axis::size(const std::vector<int>&locations){
    return locations[getCount()];
}

void GridLayout::Axis::setParentConstraints(int min, int max) {
    parentMin = min;
    parentMax = -max;
    locationsValid = false;
}

int GridLayout::Axis::getMeasure(int min,int max){
    setParentConstraints(min,max);
    parentMax = max;
    std::vector<int> ls=getLocations();
    return size(ls);
}

int GridLayout::Axis::getMeasure(int measureSpec){
    const int mode = MeasureSpec::getMode(measureSpec);
    const int size = MeasureSpec::getSize(measureSpec);
    switch (mode) {
    case MeasureSpec::UNSPECIFIED:return getMeasure(0, MAX_SIZE);
    case MeasureSpec::EXACTLY:    return getMeasure(size, size);
    case MeasureSpec::AT_MOST:    return getMeasure(0, size);
    default:return 0;
    }
}

GridLayout::PackedMap<GridLayout::Spec,GridLayout::Bounds>&GridLayout::Axis::getGroupBounds(){
    if (groupBounds.size()==0) {
        groupBounds = createGroupBounds();
    }
    if (!groupBoundsValid) {
        computeGroupBounds();
        groupBoundsValid = true;
    }
    return groupBounds;
}

void GridLayout::Axis::layout(int size){
    setParentConstraints(size,size);
    getLocations();
}

void GridLayout::Axis::invalidateStructure(){
    maxIndex = UNDEFINED;

    groupBounds.clear();// = nullptr;
    forwardLinks.clear();// = nullptr;
    backwardLinks.clear();// = nullptr;

    leadingMargins.clear();// = nullptr;
    trailingMargins.clear();// = nullptr;
    arcs.clear();// = nullptr;

    locations.clear();// = nullptr;

    deltas.clear();// = nullptr;
    hasWeightsValid = false;

    invalidateValues();
}
void GridLayout::Axis::invalidateValues(){
    groupBoundsValid = false;
    forwardLinksValid = false;
    backwardLinksValid = false;

    leadingMarginsValid = false;
    trailingMarginsValid = false;
    arcsValid = false;
    locationsValid = false;
}

void GridLayout::Axis::computeMargins(bool leading){
    std::vector<int>& margins = leading ? leadingMargins : trailingMargins;
    for (int i = 0, N = grd->getChildCount(); i < N; i++) {
        View* c = grd->getChildAt(i);
        if (c->getVisibility() == View::GONE) continue;
        LayoutParams* lp = grd->getLayoutParams(c);
        Spec spec = horizontal ? lp->columnSpec : lp->rowSpec;
        Interval span = spec.span;
        int index = leading ? span.min : span.max;
        margins[index] = std::max(margins[index], grd->getMargin1(c, horizontal, leading));
    }
}

std::vector<int>& GridLayout::Axis::getLeadingMargins() {
    if (leadingMargins.size()==0) {
        leadingMargins.resize(getCount() + 1);
    }
    if (!leadingMarginsValid) {
        computeMargins(true);
        leadingMarginsValid = true;
    }
    return leadingMargins;
}

std::vector<int>& GridLayout::Axis::getTrailingMargins() {
    if (trailingMargins.size()==0) {
        trailingMargins.resize(getCount() + 1);
    }
    if (!trailingMarginsValid) {
        computeMargins(false);
        trailingMarginsValid = true;
    }
    return trailingMargins;
}

static std::string arcsToString(bool horizontal,std::vector<GridLayout::Arc>& arcs) {
    std::string var = horizontal ? "x" : "y";
    std::ostringstream result;
    bool first = true;
    for (GridLayout::Arc arc : arcs) {
        if (first) {
            first = false;
        } else {
            result <<",";//= result.append(", ");
        }
        int src = arc.span.min;
        int dst = arc.span.max;
        int value =arc.value.value;
        if(src<dst)
            result<<var<<dst<<"-"<<var<<src<<">="<<value;
        else
            result<<var<<src<<"-"<<var<<dst<<"<="<<-value;
    }
    return result.str();
}

void GridLayout::Axis::logError(const std::string& axisName, std::vector<Arc>&arcs, std::vector<bool>& culprits0){
    std::vector<Arc> culprits; 
    std::vector<Arc> removed;
    for (int c = 0; c < arcs.size(); c++) {
        Arc& arc = arcs[c];
        if (culprits0[c]) {
            culprits.push_back(arc);
        }
        if (!arc.valid) {
            removed.push_back(arc);
        }
    }
    LOG(DEBUG)<<axisName + " constraints: "<<arcsToString(horizontal,culprits)<<
             " are inconsistent; permanently removing: " << arcsToString(horizontal,removed);
}

bool GridLayout::Axis::relax(std::vector<int>&locations, GridLayout::Arc& entry){
    if (!entry.valid) {
        return false;
    }
    Interval& span = entry.span;
    const int u = span.min;
    const int v = span.max;
    const int candidate = locations[u] + entry.value.value;
    if (candidate > locations[v]) {
        locations[v] = candidate;
        return true;
    }
    return false;
}

void GridLayout::Axis::init(std::vector<int>& locations) {
    std::fill(locations.begin(),locations.end(), 0);
}

bool GridLayout::Axis::solve(std::vector<int>&a){
    return solve(getArcs(),a);
}

bool GridLayout::Axis::solve(std::vector<Arc>&arcs,std::vector<int>& locations,bool modifyOnError){
    std::string axisName = horizontal ? "horizontal" : "vertical";
    int N = getCount() + 1; // The number of vertices is the number of columns/rows + 1.
    std::shared_ptr<std::vector<bool>> originalCulprits = nullptr;

    for (int p = 0; p < arcs.size(); p++) {
        init(locations);

        // We take one extra pass over traditional Bellman-Ford (and omit their final step)
        for (int i = 0; i < N; i++) {
            bool changed = false;
            for (int j = 0, length = arcs.size(); j < length; j++) {
                changed |= relax(locations, arcs[j]);
            }
            if (!changed) {
                if (originalCulprits != nullptr) {
                    logError(axisName, arcs, *originalCulprits);
                }
                return true;
            }
        }

        if (!modifyOnError){
            return false; 
        }// cannot solve with these constraints

        std::shared_ptr<std::vector<bool>> culprits = std::make_shared<std::vector<bool>>(arcs.size(),false);
        for (int i = 0; i < N; i++) {
            for (int j = 0, length = arcs.size(); j < length; j++) {
                culprits->at(j) =culprits->at(j)| relax(locations, arcs[j]);
            }
        }

        if (p == 0) {
            originalCulprits = culprits;
        }

        for (int i = 0; i < arcs.size(); i++) {
            if (culprits->at(i)) {
                Arc& arc = arcs[i];
                // Only remove max values, min values alone cannot be inconsistent
                if (arc.span.min < arc.span.max) {
                    continue;
                }
                arc.valid = false;
                break;
            }
        }
    }
    return true;
}

void GridLayout::Axis::computeArcs() {
    // getting the links validates the values that are shared by the arc list
    getForwardLinks();
    getBackwardLinks();
}

// Group arcs by their first vertex, returning an array of arrays.
// This is linear in the number of arcs.
std::vector<std::vector<GridLayout::Arc>> GridLayout::Axis::groupArcsByFirstVertex(std::vector<GridLayout::Arc>& arcs) {
    const int N = getCount() + 1; // the number of vertices
    std::vector<std::vector<Arc>> result;
    std::vector<int>sizes;
	result.resize(N);
	sizes.resize(N);
    for (Arc arc : arcs) {
        sizes[arc.span.min]++;
    }
    for (int i = 0; i < sizes.size(); i++) {
        result[i].resize(sizes[i]);// = new Arc[sizes[i]];
    }
    // reuse the sizes array to hold the current last elements as we insert each arc
    for(int i=0;i<sizes.size();i++)sizes[i]=0;//Arrays.fill(sizes, 0);
    for (Arc arc : arcs) {
        int i = arc.span.min;
        result[i][sizes[i]++] = arc;
    }
    return result;
}

#define NEW      0
#define PENDING  1
#define COMPLETE 2
static void walk(std::vector<int>&visited,std::vector<std::vector<GridLayout::Arc>>&arcsByVertex,
    std::vector<GridLayout::Arc>&result,int&cursor,int loc){
    switch (visited[loc]) {
    case NEW:
         visited[loc] = PENDING;
        for (GridLayout::Arc arc : arcsByVertex[loc]) {
             walk(visited,arcsByVertex,result,cursor,arc.span.max);
             result[cursor--] = arc;
         }
         visited[loc] = COMPLETE;
         break;
    case PENDING:
         // le singe est dans l'arbre
         //assert(false);
         break;
    case COMPLETE:
         break;
    }
}

std::vector<GridLayout::Arc> GridLayout::Axis::topologicalSort(std::vector<GridLayout::Arc>& arcs){
    std::vector<Arc> result ;
    int cursor = arcs.size() - 1;
    result.resize(arcs.size());
    std::vector<std::vector<Arc>>arcsByVertex = groupArcsByFirstVertex(arcs);
    std::vector<int> visited;
    visited.resize(getCount()+1);
    for (int loc = 0, N = arcsByVertex.size(); loc < N; loc++) {
        walk(visited,arcsByVertex,result,cursor,loc);
    }
    return result;//topologicalSort(arcs.toArray(new Arc[arcs.size()]));
}

void GridLayout::Axis::addComponentSizes(std::vector<GridLayout::Arc>& result, 
  GridLayout::PackedMap<GridLayout::Interval,GridLayout::MutableInt*>& links) {
    for (int i = 0; i < links.keys.size(); i++) {
        Interval& key = links.keys[i];
        include(result, key, *links.values[i], false);
    }
}

std::vector<GridLayout::Arc>GridLayout::Axis::createArcs(){
    std::vector<Arc> mins;
    std::vector<Arc> maxs;

    // Add the minimum values from the components.
    addComponentSizes(mins, getForwardLinks());
    // Add the maximum values from the components.
    addComponentSizes(maxs, getBackwardLinks());

    // Add ordering constraints to prevent row/col sizes from going negative
    if (orderPreserved) {
        // Add a constraint for every row/col
        for (int i = 0; i < getCount(); i++) {
            include(mins,Interval(i, i + 1),MutableInt(0),true);
        }
    }

    // Add the container constraints. Use the version of include that allows
    // duplicate entries in case a child spans the entire grid.
    const int N = getCount();
    include(mins, Interval(0, N), parentMin, false);
    include(maxs, Interval(N, 0), parentMax, false);

    // Sort
    std::vector<Arc> sMins = topologicalSort(mins);
    std::vector<Arc> sMaxs = topologicalSort(maxs);
    sMins.insert(sMins.end(),sMaxs.begin(),sMaxs.end());
    return sMins;//append(sMins, sMaxs);
}

std::vector<GridLayout::Arc>& GridLayout::Axis::getArcs() {
    if (arcs.size()==0) {
       arcs = createArcs();
    }
    if (!arcsValid) {
       computeArcs();
       arcsValid = true;
    }
    return arcs;
}

bool GridLayout::Axis::computeHasWeights(){
   for (int i = 0, N = grd->getChildCount(); i < N; i++) {
       View* child = grd->getChildAt(i);
       if (child->getVisibility() == View::GONE) {
           continue;
       }
       LayoutParams* lp = grd->getLayoutParams(child);
       Spec& spec = horizontal ? lp->columnSpec : lp->rowSpec;
       if (spec.weight != 0) {
           return true;
       }
   }
   return false;
}
bool GridLayout::Axis::hasWeights(){
    if (!hasWeightsValid) {
        mHasWeights = computeHasWeights();
        hasWeightsValid = true;
    }
    return mHasWeights;
}

std::vector<int>& GridLayout::Axis::getDeltas(){
    deltas.resize(grd->getChildCount());
    return deltas;
}

GridLayout::PackedMap<GridLayout::Interval,GridLayout::MutableInt*>GridLayout::Axis::createLinks(bool min){
    Assoc<Interval, MutableInt*> result;
    std::vector<Spec>&keys = getGroupBounds().keys;
    for (int i = 0, N = keys.size(); i < N; i++) {
        Interval span = min ? keys[i].span : keys[i].span.inverse();
        result.put(span, new MutableInt());
    }
    return result.pack();
}

void GridLayout::Axis::computeLinks(GridLayout::PackedMap<GridLayout::Interval,GridLayout::MutableInt*>&links,bool min){
    std::vector<MutableInt*>&spans = links.values;
    for (int i = 0; i < spans.size(); i++) {
        spans[i]->reset();//INT_MIN;//spans[i].reset();
    }
    // Use getter to trigger a re-evaluation
    std::vector<Bounds>&bounds = getGroupBounds().values;
    for (int i = 0; i < bounds.size(); i++) {
        int size = bounds[i].size(min);
        MutableInt* valueHolder = links.getValue(i);
        // this effectively takes the max() of the minima and the min() of the maxima
        valueHolder->value = std::max(valueHolder->value, min ? size : -size);
    }
}

GridLayout::PackedMap<GridLayout::Interval,GridLayout::MutableInt*>& GridLayout::Axis::getForwardLinks(){
    if (forwardLinks.size()==0) {
       forwardLinks = createLinks(true);
    }
    if (!forwardLinksValid) {
       computeLinks(forwardLinks, true);
       forwardLinksValid = true;
    }
    return forwardLinks;
}

GridLayout::PackedMap<GridLayout::Interval,GridLayout::MutableInt*>& GridLayout::Axis::getBackwardLinks(){
    if (backwardLinks.size()==0) {
        backwardLinks = createLinks(false);
    }
    if (!backwardLinksValid) {
        computeLinks(backwardLinks, false);
        backwardLinksValid = true;
    }
    return backwardLinks;
}

void GridLayout::Axis::include(std::vector<GridLayout::Arc>& arcs, 
        GridLayout::Interval key,const MutableInt& size,bool ignoreIfAlreadyPresent){
    /*Remove self referential links.
      These appear:
        . as parental constraints when GridLayout has no children
        . when components have been marked as GONE*/
    if (key.size() == 0)return;
    // this bit below should really be computed outside here -
    // its just to stop default (row/col > 0) constraints obliterating valid entries
    if (ignoreIfAlreadyPresent) {
        for (Arc arc : arcs) {
            Interval& span = arc.span;
            if (span.min==key.min&&span.max==key.max){//span.equals(key)) {
                return;
            }
        }
    }
    arcs.push_back(Arc(key, size));
}

void GridLayout::Axis::solveAndDistributeSpace(std::vector<int>&a){
    for (int i=0;i<deltas.size();i++)deltas[i]=0;
    solve(a);
    int deltaMax = parentMin.value * grd->getChildCount() + 1; //exclusive
    if (deltaMax < 2) {
        return; //don't have any delta to distribute
    }
    int deltaMin = 0; //inclusive

    const float totalWeight = calculateTotalWeight();

    int validDelta = -1; //delta for which a solution exists
    bool validSolution = true;
    // do a binary search to find the max delta that won't conflict with constraints
    while(deltaMin < deltaMax) {
        // cast to long to prevent overflow.
        int delta = (int) (((long) deltaMin + deltaMax) / 2);
        invalidateValues();
        shareOutDelta(delta, totalWeight);
        validSolution = solve(getArcs(), a, false);
        if (validSolution) {
            validDelta = delta;
            deltaMin = delta + 1;
        } else {
            deltaMax = delta;
        }
    }
    if (validDelta > 0 && !validSolution) {
        // last solution was not successful but we have a successful one. Use it.
        invalidateValues();
        shareOutDelta(validDelta, totalWeight);
        solve(a);
    }
}

float GridLayout::Axis::calculateTotalWeight() {
    float totalWeight = .0f;
    for (int i = 0, N = grd->getChildCount(); i < N; i++) {
        View* c = grd->getChildAt(i);
        if (c->getVisibility() == View::GONE) {
            continue;
        }
        const LayoutParams* lp = grd->getLayoutParams(c);
        const Spec& spec = horizontal ? lp->columnSpec : lp->rowSpec;
        totalWeight += spec.weight;
    }
    return totalWeight;
}

void GridLayout::Axis::shareOutDelta(int totalDelta, float totalWeight){
    for (int i=0;i<deltas.size();i++)deltas[i]=0;
    for (int i = 0, N = grd->getChildCount(); i < N; i++) {
        View* c = grd->getChildAt(i);
        if (c->getVisibility() == View::GONE) {
            continue;
        }
        LayoutParams* lp = grd->getLayoutParams(c);
        Spec& spec = horizontal ? lp->columnSpec : lp->rowSpec;
        float weight = spec.weight;
        if (weight != 0) {
            int delta = std::round((weight * totalDelta / totalWeight));
            deltas[i] = delta;
            // the two adjustments below are to counter the above rounding and avoid
            // off-by-ones at the end
            totalDelta -= delta;
            totalWeight -= weight;
        }
    }
}

void GridLayout::Axis::computeLocations(std::vector<int>&a){
    if (!hasWeights()) {
        solve(a);
    } else {
        solveAndDistributeSpace(a);
    }
    if (!orderPreserved) {
        // Solve returns the smallest solution to the constraint system for which all
        // values are positive. One value is therefore zero - though if the row/col
        // order is not preserved this may not be the first vertex. For consistency,
        // translate all the values so that they measure the distance from a[0]; the
        // leading edge of the parent. After this transformation some values may be negative.
        int a0 = a[0];
        for (int i = 0, N = a.size(); i < N; i++) {
            a[i] = a[i] - a0;
        }
    }
}

std::vector<int>GridLayout::Axis::getLocations(){
    if(locations.size()==0){
        locations.resize(getCount()+1);
        for(int i=0;i<locations.size();i++)locations[i]=0;
    }
    if(!locationsValid){
        computeLocations(locations);
        locationsValid =true;
    }
    return locations;
}

}
