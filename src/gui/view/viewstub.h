#include <view/view.h>
namespace cdroid{

class ViewStub:public View{
public:
    DECLARE_UIEVENT(void,OnInflateListener,ViewStub&,View&inflated);
private:
    OnInflateListener mInflateListener;
    int mInflatedId;
    std::string mLayoutResource;
    View* mInflatedViewRef;
private:
    View*inflateViewNoAdd(ViewGroup* parent);
    void replaceSelfWithView(View* view, ViewGroup* parent);
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void dispatchDraw(Canvas& canvas)override;
public:
    ViewStub(Context* context,const AttributeSet& attrs);
    int getInflatedId()const;
    const std::string& getLayoutResource()const;
    void draw(Canvas&)override;
    void setVisibility(int visibility)override;
    View*inflate();
    void setOnInflateListener(const OnInflateListener& inflateListener);
};

}
