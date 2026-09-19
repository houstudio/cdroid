#include <view/viewstub.h>
#include <view/viewgroup.h>
#include <widget/framework_styleable.h>

namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET2(ViewStub, "android.view.ViewStub");

ViewStub::ViewStub(Context*ctx)
    :ViewStub(ctx,nullptr){}

ViewStub::ViewStub(Context* context,const AttributeSet* attrs):ViewStub(context,attrs,0){}

ViewStub::ViewStub(Context* context,const AttributeSet* pAttrs,int defStyleAttr):View(context,pAttrs, defStyleAttr){
    // AOSP ViewStub: ViewStub styleable (layout/inflatedId are references).
    auto a = context->obtainStyledAttributes(pAttrs, R::styleable::ViewStub, defStyleAttr);
    mInflatedId = a->getResourceId(R::styleable::ViewStub_inflatedId, View::NO_ID);
    mLayoutResource = a->getResourceId(R::styleable::ViewStub_layout, 0);
    mInflatedViewRef = nullptr;
    mInflateListener = nullptr;
    setVisibility(GONE);
    setWillNotDraw(true);
}

int ViewStub::getInflatedId()const{
    return mInflatedId;
}

int ViewStub::getLayoutResource()const{
    return mLayoutResource;
}

void ViewStub::setLayoutResource(int layoutResource){
    mLayoutResource = layoutResource;
}

void ViewStub::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    setMeasuredDimension(0, 0);
}

void ViewStub::draw(Canvas& canvas) {
}

void ViewStub::dispatchDraw(Canvas& canvas) {
}

void ViewStub::setVisibility(int visibility){
    if (mInflatedViewRef != nullptr) {
        View* view = mInflatedViewRef;
        if (view) {
            view->setVisibility(visibility);
        } else {
            throw "setVisibility called on un-referenced view";
        }
    } else {
        View::setVisibility(visibility);
        if (visibility == VISIBLE || visibility == INVISIBLE) {
            inflate();
        }
    }
}

View* ViewStub::inflateViewNoAdd(ViewGroup* parent) {
    View* view = LayoutInflater::from(mContext)->inflate(mLayoutResource, parent, false);

    if (mInflatedId != NO_ID) {
        view->setId(mInflatedId);
    }
    return view;
}

void ViewStub::replaceSelfWithView(View* view, ViewGroup* parent) {
    int index = parent->indexOfChild(this);
    parent->removeViewInLayout(this);

    ViewGroup::LayoutParams* layoutParams = getLayoutParams();
    if (layoutParams) {
        parent->addView(view, index, layoutParams);
    } else {
        parent->addView(view, index);
    }
}

/**
 * Inflates the layout resource identified by {@link #getLayoutResource()}
 * and replaces this StubbedView in its parent by the inflated layout resource.
 *
 * @return The inflated layout resource.
 */
View* ViewStub::inflate() {
    ViewGroup* parent = getParent();
    if (parent) {
        if (mLayoutResource != 0) {
            View* view = inflateViewNoAdd(parent);
            replaceSelfWithView(view, parent);

            mInflatedViewRef = view;//new WeakReference<>(view);
            if (view && mInflateListener) mInflateListener(*this,*view);
            return view;
        } else {
            throw "ViewStub must have a valid layoutResource";
        }
    } else {
        throw "ViewStub must have a non-null ViewGroup viewParent";
    }
}

void ViewStub::setOnInflateListener(const OnInflateListener& inflateListener){
    mInflateListener = inflateListener;
}
}//endof namespace
