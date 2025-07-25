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
#include <widget/tablelayout.h>
#include <porting/cdlog.h>

namespace cdroid{

DECLARE_WIDGET(TableLayout)

TableLayout::LayoutParams::LayoutParams()
    :LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT){
}

TableLayout::LayoutParams::LayoutParams(Context* c, const AttributeSet&attrs)
    :LinearLayout::LayoutParams(c,attrs){
}
TableLayout::LayoutParams::LayoutParams(int w, int h)
    :LinearLayout::LayoutParams(w,h){
}

TableLayout::LayoutParams::LayoutParams(int w, int h, float initWeight)
    :LinearLayout::LayoutParams(w,h,initWeight){
}

TableLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& p)
    :LinearLayout::LayoutParams(p){
}
TableLayout::LayoutParams::LayoutParams(const ViewGroup::MarginLayoutParams& source)
    :LinearLayout::LayoutParams(source){
    width = MATCH_PARENT;
    try{
        const LayoutParams&tl=dynamic_cast<const LayoutParams&>(source);
        weight = tl.weight;
    }catch(std::bad_cast exp){
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TableLayout::TableLayout(int w,int h):LinearLayout(w,h){
    initTableLayout();
}

TableLayout::TableLayout(Context*ctx,const AttributeSet&atts)
  :LinearLayout(ctx,atts){
    initTableLayout();
}

static std::vector<std::string> split(const std::string& s,const std::string& delim){
    std::vector<std::string> elems;
    size_t pos = 0;
    size_t len = s.length();
    size_t delim_len = delim.length();
    if (delim_len == 0) return elems;
    while (pos < len){
        int find_pos = s.find(delim, pos);
        if (find_pos < 0){
            elems.push_back(s.substr(pos, len - pos));
            break;
        }
        elems.push_back(s.substr(pos, find_pos - pos));
        pos = find_pos + delim_len;
    }
    return elems;
}

SparseBooleanArray TableLayout::parseColumns(const std::string& sequence){
    SparseBooleanArray columns;
    std::vector<std::string>columnDefs=split(sequence,",");

    for (std::string columnIdentifier : columnDefs) {
        int columnIndex = std::atoi(columnIdentifier.c_str());
        // only valid, i.e. positive, columns indexes are handled
        if (columnIndex >= 0) {
            // putting true in this sparse array indicates that the
            // column index was defined in the XML file
            columns.put(columnIndex,true);
        }
    }
    return columns;
}

void TableLayout::initTableLayout(){
    // TableLayouts are always in vertical orientation; keep this tracked
    // for shared LinearLayout code.
    mShrinkAllColumns=false;
    mStretchAllColumns=false;
    mInitialized = true;
    setOrientation(VERTICAL);

    //mPassThroughListener = new PassThroughHierarchyChangeListener();
    // make sure to call the parent class method to avoid potential
    // infinite loops
    //super.setOnHierarchyChangeListener(mPassThroughListener);
}

void TableLayout::requestRowsLayout(){
    if (mInitialized) {
        int count = getChildCount();
        for (int i = 0; i < count; i++) {
            getChildAt(i)->requestLayout();
        }
    }
}

void TableLayout::requestLayout(){
    if (mInitialized) {
        int count = getChildCount();
        for (int i = 0; i < count; i++) {
            getChildAt(i)->forceLayout();
        }
    }
    LinearLayout::requestLayout();
}

bool TableLayout::isShrinkAllColumns()const{
    return mShrinkAllColumns;
}

void TableLayout::setShrinkAllColumns(bool shrinkAllColumns){
    mShrinkAllColumns = shrinkAllColumns;
}

bool TableLayout::isStretchAllColumns()const {
    return mStretchAllColumns;
}

void TableLayout::setStretchAllColumns(bool stretchAllColumns) {
    mStretchAllColumns = stretchAllColumns;
}

bool TableLayout::isColumnCollapsed(int columnIndex)const{
    return mCollapsedColumns.get(columnIndex);
}

void TableLayout::setColumnCollapsed(int columnIndex, bool isCollapsed){
    mCollapsedColumns.put(columnIndex,isCollapsed);

    int count = getChildCount();
    for (int i = 0; i < count; i++) {
        View* view = getChildAt(i);
        if (dynamic_cast<TableRow*>(view)) {
            ((TableRow*) view)->setColumnCollapsed(columnIndex, isCollapsed);
        }
    }
    requestRowsLayout();
}

bool TableLayout::isColumnStretchable(int columnIndex)const{
    return mStretchAllColumns || mStretchableColumns.get(columnIndex);
}

void TableLayout::setColumnStretchable(int columnIndex, bool isStretchable) {
    mStretchableColumns.put(columnIndex,isStretchable);
    requestRowsLayout();
}

bool TableLayout::isColumnShrinkable(int columnIndex)const{
    return mShrinkableColumns.get(columnIndex);
}

void TableLayout::setColumnShrinkable(int columnIndex, bool isShrinkable){
    mShrinkableColumns.put(columnIndex,isShrinkable);
    requestRowsLayout();
}

void TableLayout::trackCollapsedColumns(View* child){
    if (dynamic_cast<TableRow*>(child)) {
        TableRow* row = (TableRow*) child;
	int count=mCollapsedColumns.size();
        for (int i=0;i<count;i++){
            int columnIndex = mCollapsedColumns.keyAt(i);;
            bool isCollapsed= mCollapsedColumns.valueAt(i);
            // the collapse status is set only when the column should be
            // collapsed; otherwise, this might affect the default
            // visibility of the row's children
            if (isCollapsed) {
                row->setColumnCollapsed(columnIndex, isCollapsed);
            }
        }
    }
}

void TableLayout::addView(View* child){
    LinearLayout::addView(child);
    requestRowsLayout();
}

void TableLayout::addView(View*child, int index){
    LinearLayout::addView(child,index);
    requestRowsLayout();
}

void TableLayout::addView(View* child, ViewGroup::LayoutParams* params){
    LinearLayout::addView(child,params);
    requestRowsLayout();
}

void TableLayout::addView(View* child, int index, ViewGroup::LayoutParams* params){
    LinearLayout::addView(child,index,params);
    requestRowsLayout();
}

void TableLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    measureVertical(widthMeasureSpec, heightMeasureSpec);
}

void TableLayout::onLayout(bool changed, int l, int t, int w, int h){
    layoutVertical(l, t, w, h);
}

void TableLayout::measureChildBeforeLayout(View* child, int childIndex,int widthMeasureSpec,
            int totalWidth,int heightMeasureSpec, int totalHeight){
    if (dynamic_cast<TableRow*>(child)) {
        ((TableRow*) child)->setColumnsWidthConstraints(mMaxWidths);
    }

    LinearLayout::measureChildBeforeLayout(child, childIndex,
        widthMeasureSpec, totalWidth, heightMeasureSpec, totalHeight);
}

void TableLayout::measureVertical(int widthMeasureSpec, int heightMeasureSpec){
    findLargestCells(widthMeasureSpec, heightMeasureSpec);
    shrinkAndStretchColumns(widthMeasureSpec);
    LinearLayout::measureVertical(widthMeasureSpec, heightMeasureSpec);
}

void TableLayout::findLargestCells(int widthMeasureSpec, int heightMeasureSpec) {
    bool firstRow = true;

    // find the maximum width for each column
    // the total number of columns is dynamically changed if we find
    // wider rows as we go through the children
    // the array is reused for each layout operation; the array can grow
    // but never shrinks. Unused extra cells in the array are just ignored
    // this behavior avoids to unnecessary grow the array after the first/ layout operation
    const int count = getChildCount();
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() == GONE) continue;

        if (dynamic_cast<TableRow*>(child)==nullptr)continue;

        TableRow* row = (TableRow*) child;
        // forces the row's height
        LayoutParams* layoutParams = (LayoutParams*)row->getLayoutParams();
        layoutParams->height = LayoutParams::WRAP_CONTENT;

        std::vector<int>widths = row->getColumnsWidths(widthMeasureSpec, heightMeasureSpec);
        int newLength = (int)widths.size();
        // this is the first row, we just need to copy the values
        if (firstRow) {
            mMaxWidths=widths;//System.arraycopy(widths, 0, mMaxWidths, 0, newLength);
            firstRow = false;
        } else {
            int length = (int)mMaxWidths.size();
            // the current row is wider than the previous rows, so
            // we just grow the array and copy the values
            mMaxWidths.resize(newLength);
            for(i=length;i<newLength;i++)mMaxWidths[i] = widths[i];

            // the row is narrower or of the same width as the previous
            // rows, so we find the maximum width for each column
            // if the row is narrower than the previous ones,
            // difference will be negative
            length = std::min(length, newLength);
            for (int j = 0; j < length; j++) {
                mMaxWidths[j] = std::max(mMaxWidths[j], widths[j]);
            }
        }
    }
}

void TableLayout::shrinkAndStretchColumns(int widthMeasureSpec) {
    // when we have no row, mMaxWidths is not initialized and the loop
    // below could cause a NPE
    if (mMaxWidths.size()==0)  return;

    // should we honor AT_MOST, EXACTLY and UNSPECIFIED?
    int totalWidth = 0;
    for (int width : mMaxWidths) {
        totalWidth += width;
    }

    int size = MeasureSpec::getSize(widthMeasureSpec) - mPaddingLeft - mPaddingRight;

    if ((totalWidth > size) && (mShrinkAllColumns || mShrinkableColumns.size() > 0)) {
        // oops, the largest columns are wider than the row itself
        // fairly redistribute the row's width among the columns
        mutateColumnsWidth(mShrinkableColumns, mShrinkAllColumns, size, totalWidth);
    } else if ((totalWidth < size) && (mStretchAllColumns || mStretchableColumns.size() > 0)) {
        // if we have some space left, we distribute it among the
        // expandable columns
        mutateColumnsWidth(mStretchableColumns, mStretchAllColumns, size, totalWidth);
    }
}
 
void TableLayout::mutateColumnsWidth(SparseBooleanArray& columns,bool allColumns, int size, int totalWidth) {
    int skipped = 0;
    size_t length = mMaxWidths.size();
    const size_t count = allColumns ? length : columns.size();
    int totalExtraSpace = size - totalWidth;
    int extraSpace = int(totalExtraSpace / count);

    // Column's widths are changed: force child table rows to re-measure.
    // (done by super.measureVertical after shrinkAndStretchColumns.)
    const int nbChildren = (int)getChildCount();
    for (int i = 0; i < nbChildren; i++) {
        View* child = getChildAt(i);
        if (dynamic_cast<TableRow*>(child)) {
            child->forceLayout();
        }
    }

    if (!allColumns) {
        for (size_t i=0;i<count;i++){
            int column = columns.keyAt(i);
            if (columns.valueAt(i)) {
                if (column < length) {
                    mMaxWidths[column] += extraSpace;
                } else {
                    skipped++;
                }
            }
        }
    } else {
        for (size_t i = 0; i < count; i++) mMaxWidths[i] += extraSpace;

        // we don't skip any column so we can return right away
        return;
    }

    if (skipped > 0 && skipped < count) {
        // reclaim any extra space we left to columns that don't exist
        extraSpace = skipped * extraSpace / int(count - skipped);
        for (size_t i=0;i<count;i++){
            int column = columns.keyAt(i);
            if ( columns.valueAt((int)i) && column < length) {
                if (extraSpace > mMaxWidths[column]) {
                    mMaxWidths[column] = 0;
                } else {
                    mMaxWidths[column] += extraSpace;
                }
            }
        }
    }
}

std::string TableLayout::getAccessibilityClassName()const{
    return "TableLayout";
}

TableLayout::LayoutParams* TableLayout::generateLayoutParams(const AttributeSet& attrs)const {
    return new LayoutParams(getContext(), attrs);
}

TableLayout::LayoutParams* TableLayout::generateDefaultLayoutParams()const {
    return new LayoutParams();
}

bool TableLayout::checkLayoutParams(const ViewGroup::LayoutParams* p)const{
    return dynamic_cast<const LayoutParams*>(p);
}

TableLayout::LayoutParams* TableLayout::generateLayoutParams(const ViewGroup::LayoutParams* p)const {
    return new LayoutParams(*p);
}

}//namespace
