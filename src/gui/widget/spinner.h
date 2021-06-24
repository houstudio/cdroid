#ifndef __SPINNER_H__
#define __SPINNER_H__
#include <widget/absspinner.h>
#include <widget/listview.h>
namespace cdroid{
class SpinnerPopup {
protected:
    class Spinner*mSpinner;
    Adapter*mAdapter;
    ListView*mListView;
    int mMode;
    int mDropDownWidth;
public:
    SpinnerPopup(Spinner*sp,int mode);
    virtual ~SpinnerPopup();
    virtual void setAdapter(Adapter* adapter);
    /**Show the popup */
    virtual void show(int textDirection, int textAlignment);
    /* Dismiss the popup */
    virtual void dismiss();
    /* @return true if the popup is showing, false otherwise. */
    virtual bool isShowing();
    /**Set hint text to be displayed to the user. This should providea description
     * of the choice being made. @param hintText Hint text to set.*/
    virtual void setPromptText(const std::string& hintText);
    virtual const std::string getHintText();
    virtual int getVerticalOffset();
    virtual void setVerticalOffset(int px);
    virtual int getHorizontalOffset();
    virtual void setHorizontalOffset(int px);
    virtual void setBackgroundDrawable(Drawable* bg);
    virtual Drawable* getBackground();

    virtual void computeContentWidth();
    virtual void setContentWidth(int width);
    //virtual int measureContentWidth(Adapter*,Drawable*);
    virtual ListView*getListView()const;
};

class Spinner:public AbsSpinner{
public:
    enum{
        MODE_DIALOG   =0,
        MODE_DROPDOWN =1
    };
private:
    int  mGravity;
    Context*mPopupContext;
    bool mDisableChildrenWhenDisabled;
    void setUpChild(View* child, bool addChild);
    View* makeView(int position, bool addChild);
protected:
    int mDropDownWidth;
    SpinnerPopup *mPopup;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t, int w, int h);
    void layout(int delta, bool animate)override;
    void computeContentWidth();
public:
    Spinner(int w,int h,int mode=0);
    Spinner(Context*ctx,const AttributeSet&atts);
    Context* getPopupContext()const;
    int measureContentWidth(Adapter* adapter, Drawable* background);
    void setPopupBackgroundDrawable(Drawable* background);
    void setPopupBackgroundResource(const std::string& resId);
    Drawable* getPopupBackground();
    void setDropDownVerticalOffset(int pixels);
    int getDropDownVerticalOffset();
    void setDropDownHorizontalOffset(int pixels);
    int getDropDownHorizontalOffset();
    void setDropDownWidth(int pixels);
    int getDropDownWidth()const;
    View& setEnabled(bool enabled)override;
    void setGravity(int gravity);
    int getGravity()const;
    void setAdapter(Adapter*adapter)override;
    int getBaseline()override;
    void show(int textDirection, int textAlignment);
    bool onTouchEvent(MotionEvent& event)override;
};

}//namespace
#endif
