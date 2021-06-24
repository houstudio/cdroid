#include <widget/tablerow.h>
#include <widget/measurespec.h>
#include <cdlog.h>
namespace cdroid{

TableRow::LayoutParams::LayoutParams()
    :LinearLayoutParams(MATCH_PARENT, WRAP_CONTENT){
    column = -1;
    span = 1;
}

TableRow::LayoutParams::LayoutParams(int column):TableRowLayoutParams(){
    this->column=column;
}

TableRow::LayoutParams::LayoutParams(Context* c,const AttributeSet&attrs)
    :LinearLayoutParams(c,attrs){
    column=-1;
    span=1;
}

TableRow::LayoutParams::LayoutParams(int w, int h)
    :LinearLayoutParams(w,h){
    column=-1;
    span=1;
}

TableRow::LayoutParams::LayoutParams(int w, int h, float initWeight)
    :LinearLayoutParams(w,h,initWeight){
    column=-1;
    span=1;
}

TableRow::LayoutParams::LayoutParams(const LayoutParams& p)
    :LinearLayoutParams(p){
    column=-1;
    span=1;
}

TableRow::LayoutParams::LayoutParams(const MarginLayoutParams& source)
    :LinearLayoutParams(source){
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

TableRow::TableRow(int w,int h):LinearLayout(w,h){
    initTableRow();
}

TableRow::TableRow(Context* context,const AttributeSet& attrs)
	:LinearLayout(context, attrs){
    initTableRow();
}

void TableRow::initTableRow() {
    /*OnHierarchyChangeListener oldListener = mOnHierarchyChangeListener;
    mChildrenTracker = new ChildrenTracker();
    if (oldListener != null) {
        mChildrenTracker.setOnHierarchyChangeListener(oldListener);
    }
    super.setOnHierarchyChangeListener(mChildrenTracker);*/
}

void TableRow::setColumnCollapsed(int columnIndex, bool collapsed) {
    View* child = getVirtualChildAt(columnIndex);
    if (child != nullptr) {
        child->setVisibility(collapsed ? GONE : VISIBLE);
    }
}

void TableRow::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    // enforce horizontal layout
    measureHorizontal(widthMeasureSpec, heightMeasureSpec);
}

void TableRow::onLayout(bool changed, int l, int t, int w, int h) {
    // enforce horizontal layout
    layoutHorizontal(l, t, w, h);
}

View* TableRow::getVirtualChildAt(int i){
    if (mColumnToChildIndex.size()==0) {
        mapIndexAndColumns();
    }
    auto it=mColumnToChildIndex.find(i);
    int deflectedIndex = it==mColumnToChildIndex.end()?-1:it->second;
    if (deflectedIndex != -1) {
        return getChildAt(deflectedIndex);
    }
    return nullptr;
}

int TableRow::getVirtualChildCount() {
    if (mColumnToChildIndex.size()==0) {
        mapIndexAndColumns();
    }
    return mNumColumns;
}

void TableRow::mapIndexAndColumns() {
    if (mColumnToChildIndex.size() == 0) {
        int virtualCount = 0;
        int count = getChildCount();

        for (int i = 0; i < count; i++) {
            View* child = getChildAt(i);
            TableRowLayoutParams* layoutParams = (TableRowLayoutParams*) child->getLayoutParams();
            if (layoutParams->column >= virtualCount) {
                virtualCount = layoutParams->column;
            }

            for (int j = 0; j < layoutParams->span; j++) {
                mColumnToChildIndex[virtualCount++]= i;
            }
        }

        mNumColumns = virtualCount;
    }
}

int TableRow::measureNullChild(int childIndex) {
    return mConstrainedColumnWidths[childIndex];
}

void TableRow::measureChildBeforeLayout(View* child, int childIndex,int widthMeasureSpec,
        int totalWidth,int heightMeasureSpec, int totalHeight) {
    if (mConstrainedColumnWidths.size()) {
        TableRowLayoutParams* lp = (TableRowLayoutParams*) child->getLayoutParams();

        int measureMode = MeasureSpec::EXACTLY;
        int columnWidth = 0;

        int span = lp->span;
        for (int i = 0; i < span; i++) {
            columnWidth += mConstrainedColumnWidths[childIndex + i];
        }

        int gravity = lp->gravity;
        bool isHorizontalGravity = Gravity::isHorizontal(gravity);

        if (isHorizontalGravity) {
            measureMode = MeasureSpec::AT_MOST;
        }

        // no need to care about padding here,
        // ViewGroup.getChildMeasureSpec() would get rid of it anyway
        // because of the EXACTLY measure spec we use
        int childWidthMeasureSpec = MeasureSpec::makeMeasureSpec(
                    std::max(0, columnWidth - lp->leftMargin - lp->rightMargin), measureMode);
        int childHeightMeasureSpec = getChildMeasureSpec(heightMeasureSpec,
                    mPaddingTop + mPaddingBottom + lp->topMargin +
                    lp->bottomMargin + totalHeight, lp->height);

        child->measure(childWidthMeasureSpec, childHeightMeasureSpec);

        if (isHorizontalGravity) {
            int childWidth = child->getMeasuredWidth();
            lp->mOffset[TableRowLayoutParams::LOCATION_NEXT] = columnWidth - childWidth;

            int layoutDirection = getLayoutDirection();
            int absoluteGravity = Gravity::getAbsoluteGravity(gravity, layoutDirection);
            switch (absoluteGravity & Gravity::HORIZONTAL_GRAVITY_MASK) {
            case Gravity::LEFT:
                // don't offset on X axis
                break;
            case Gravity::RIGHT:
                lp->mOffset[TableRowLayoutParams::LOCATION] = lp->mOffset[TableRowLayoutParams::LOCATION_NEXT];
                break;
            case Gravity::CENTER_HORIZONTAL:
                lp->mOffset[TableRowLayoutParams::LOCATION] = lp->mOffset[TableRowLayoutParams::LOCATION_NEXT] / 2;
                break;
            }
        } else {
            lp->mOffset[TableRowLayoutParams::LOCATION] = lp->mOffset[TableRowLayoutParams::LOCATION_NEXT] = 0;
        }
    } else {
        // fail silently when column widths are not available
        LinearLayout::measureChildBeforeLayout(child, childIndex, widthMeasureSpec,
                    totalWidth, heightMeasureSpec, totalHeight);
    }
}

int TableRow::getChildrenSkipCount(View* child, int index) {
    TableRowLayoutParams* layoutParams = (TableRowLayoutParams*) child->getLayoutParams();
    // when the span is 1 (default), we need to skip 0 child
    return layoutParams->span - 1;
}

int TableRow::getLocationOffset(View* child) {
    return ((TableRowLayoutParams*) child->getLayoutParams())->mOffset[TableRowLayoutParams::LOCATION];
}

int TableRow::getNextLocationOffset(View* child) {
    return ((TableRowLayoutParams*) child->getLayoutParams())->mOffset[TableRowLayoutParams::LOCATION_NEXT];
}

std::vector<int> TableRow::getColumnsWidths(int widthMeasureSpec, int heightMeasureSpec) {
    int numColumns = getVirtualChildCount();
    if (numColumns != mColumnWidths.size()) {
        mColumnWidths.resize(numColumns);
    }

    for (int i = 0; i < numColumns; i++) {
        View* child = getVirtualChildAt(i);
        if (child != nullptr && child->getVisibility() != GONE) {
            TableRowLayoutParams* layoutParams = (TableRowLayoutParams*) child->getLayoutParams();
            if (layoutParams->span == 1) {
                int spec;
                switch (layoutParams->width) {
                case LayoutParams::WRAP_CONTENT:
                    spec = getChildMeasureSpec(widthMeasureSpec, 0, LayoutParams::WRAP_CONTENT);
                    break;
                case LayoutParams::MATCH_PARENT:
                    spec = MeasureSpec::makeSafeMeasureSpec(MeasureSpec::getSize(heightMeasureSpec),
                            MeasureSpec::UNSPECIFIED);
                    break;
                default:
                    spec = MeasureSpec::makeMeasureSpec(layoutParams->width, MeasureSpec::EXACTLY);
                }
                child->measure(spec, spec);

                int width = child->getMeasuredWidth() + layoutParams->leftMargin +layoutParams->rightMargin;
                mColumnWidths[i] = width;
            } else {
                mColumnWidths[i] = 0;
            }
        } else {
            mColumnWidths[i] = 0;
        }
    }

    return mColumnWidths;
}

void TableRow::setColumnsWidthConstraints(const std::vector<int>& columnWidths) {
    if (columnWidths.size() < getVirtualChildCount()) {
        LOGE("columnWidths should be >= getVirtualChildCount()");
    }
    mConstrainedColumnWidths = columnWidths;
}

ViewGroup::LayoutParams* TableRow::generateLayoutParams(const AttributeSet& attrs)const {
    return new TableRowLayoutParams(getContext(), attrs);
}

ViewGroup::LayoutParams* TableRow::generateDefaultLayoutParams()const {
    return new TableRowLayoutParams();
}

bool TableRow::checkLayoutParams(const ViewGroup::LayoutParams* p)const {
    return dynamic_cast<const TableRowLayoutParams*>(p);
}

ViewGroup::LayoutParams*TableRow::generateLayoutParams(const ViewGroup::LayoutParams* p)const {
    return new TableRowLayoutParams(*p);
}

}
