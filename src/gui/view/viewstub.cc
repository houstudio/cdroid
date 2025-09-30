#include <view/viewstub.h>
#include <view/viewgroup.h>

namespace cdroid{

DECLARE_WIDGET(ViewStub)

ViewStub::ViewStub(Context* context,const AttributeSet& attrs):View(context,attrs){
    mInflatedId = attrs.getResourceId("inflatedId",View::NO_ID);
    mLayoutResource  = attrs.getString("layout");
    mInflatedViewRef = nullptr;
    mInflateListener = nullptr;
    setVisibility(GONE);
    setWillNotDraw(true);
}

int ViewStub::getInflatedId()const{
    return mInflatedId;
}

const std::string& ViewStub::getLayoutResource()const{
    return mLayoutResource;
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
        if (!mLayoutResource.empty()) {
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
