#include <view/view.h>
namespace cdroid{

class ViewStub:public View{
public:
    DECLARE_UIEVENT(void,OnInflateListener,ViewStub&,View&inflated);
private:
    OnInflateListener mInflateListener;
    int mInflatedId;
    int mLayoutResource = 0;   // AOSP: @LayoutRes int (0 == unset)
    View* mInflatedViewRef;
private:
    View*inflateViewNoAdd(ViewGroup* parent);
    void replaceSelfWithView(View* view, ViewGroup* parent);
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void dispatchDraw(Canvas& canvas)override;
public:
    ViewStub(Context*ctx);   // AOSP ViewStub(Context)
    ViewStub(Context* context,const AttributeSet* attrs);
    ViewStub(Context* context,const AttributeSet* attrs,int defStyleAttr);
    int getInflatedId()const;
    int getLayoutResource()const;
    void setLayoutResource(int layoutResource);
    void draw(Canvas&)override;
    void setVisibility(int visibility)override;
    View*inflate();
    void setOnInflateListener(const OnInflateListener& inflateListener);
};

}
