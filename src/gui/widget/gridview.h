#ifndef __GRID_VIEW_H__
#define __GRID_VIEW_H__
#include <widget/abslistview.h>
namespace cdroid{

class GridView:public AbsListView{
public:
    enum {
	AUTO_FIT        =-1,
	NO_STRETCH      = 0,
	STRETCH_SPACING = 1,
	STRETCH_COLUMN_WIDTH =2,
	STRETCH_SPACING_UNIFORM =3,
    };
private:
    int mNumColumns ;
    int mHorizontalSpacing = 0;
    int mRequestedHorizontalSpacing;
    int mVerticalSpacing = 0;
    int mStretchMode = STRETCH_COLUMN_WIDTH;
    int mColumnWidth;
    int mRequestedColumnWidth;
    int mRequestedNumColumns;
    View* mReferenceView = nullptr;
    View* mReferenceViewInSelectedRow = nullptr;
    int mGravity = Gravity::START;

    void initGridView();
    View* fillDown(int pos, int nextTop);
    View* makeRow(int startPos, int y, bool flow);
    View* fillUp(int pos, int nextBottom);
    View* fillFromTop(int nextTop);
    View* fillFromBottom(int lastPosition, int nextBottom);
    View* fillSelection(int childrenTop, int childrenBottom);
    View* fillSpecific(int position, int top);
    void pinToTop(int childrenTop);
    void pinToBottom(int childrenBottom);
    void correctTooHigh(int numColumns, int verticalSpacing, int childCount);
    void correctTooLow(int numColumns, int verticalSpacing, int childCount);
    View* fillFromSelection(int selectedTop, int childrenTop, int childrenBottom);
    int getBottomSelectionPixel(int childrenBottom, int fadingEdgeLength,
            int numColumns, int rowStart);
    int getTopSelectionPixel(int childrenTop, int fadingEdgeLength, int rowStart);
    void adjustForBottomFadingEdge(View* childInSelectedRow,int topSelectionPixel, int bottomSelectionPixel);
    void adjustForTopFadingEdge(View* childInSelectedRow,int topSelectionPixel, int bottomSelectionPixel);

    View* moveSelection(int delta, int childrenTop, int childrenBottom);
    bool determineColumns(int availableSpace);
    View* makeAndAddView(int position, int y, bool flow, int childrenLeft,bool selected, int where);
    void setupChild(View* child, int position, int y, bool flowDown, int childrenLeft,
            bool selected, bool isAttachedToWindow, int where);
    void adjustViewsUpOrDown();
    bool commonKey(int keyCode, int count, KeyEvent& event);
protected:
    int lookForSelectablePosition(int position, bool lookDown)override;
    void fillGap(bool down)override; 
    int findMotionRow(int y)override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void layoutChildren() override;
    void setSelectionInt(int position)override;
    bool pageScroll(int direction);
    bool fullScroll(int direction);
    bool arrowScroll(int direction);
    bool sequenceScroll(int direction);
    
    int computeVerticalScrollExtent()override;
    int computeVerticalScrollOffset()override;
    int computeVerticalScrollRange()override;
public:
    GridView(int w,int h);
    GridView(Context*ctx,const AttributeSet&atts,const std::string&defstyle=nullptr);
    Adapter*getAdapter()override;
    void setAdapter(Adapter* adapter)override;
    void setSelection(int position);
    bool dispatchKeyEvent(KeyEvent& event)override;
    bool onKeyDown(int keyCode, KeyEvent& event)override;
    bool onKeyUp(int keyCode, KeyEvent& event)override;
    bool onKeyMultiple(int keyCode, int repeatCount, KeyEvent& event);
    void setGravity(int gravity);
    int getGravity()const;
    void setHorizontalSpacing(int horizontalSpacing);
    int getHorizontalSpacing()const;
    int getRequestedHorizontalSpacing()const;
    void setVerticalSpacing(int verticalSpacing);
    int getVerticalSpacing()const;
    void setStretchMode( int stretchMode);
    int getStretchMode()const;
    void setColumnWidth(int columnWidth);
    int getColumnWidth()const;
    int getRequestedColumnWidth()const;
    void setNumColumns(int numColumns);
    int getNumColumns()const;

};

}//namespace
#endif //__GRID_VIEW_H__
