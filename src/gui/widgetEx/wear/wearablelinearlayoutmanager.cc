#include <widgetEx/wear/wearablerecyclerview.h>
#include <widgetEx/wear/curvinglayoutcallback.h>
#include <widgetEx/wear/wearablelinearlayoutmanager.h>

namespace cdroid{

WearableLinearLayoutManager::WearableLinearLayoutManager(Context* context,const LayoutCallback& layoutCallback)
    :LinearLayoutManager(context, VERTICAL, false){
    mLayoutCallback = layoutCallback;
}

WearableLinearLayoutManager::WearableLinearLayoutManager(Context* context)
    :WearableLinearLayoutManager(context, nullptr){
}

void WearableLinearLayoutManager::setLayoutCallback(const LayoutCallback& layoutCallback) {
    mLayoutCallback = layoutCallback;
}

WearableLinearLayoutManager::LayoutCallback WearableLinearLayoutManager::getLayoutCallback()const {
    return mLayoutCallback;
}

int WearableLinearLayoutManager::scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    const int scrolled = LinearLayoutManager::scrollVerticallyBy(dy, recycler, state);

    updateLayout();
    return scrolled;
}

void WearableLinearLayoutManager::onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    LinearLayoutManager::onLayoutChildren(recycler, state);
    if (getChildCount() == 0) {
        return;
    }

    updateLayout();
}

void WearableLinearLayoutManager::updateLayout() {
    if (mLayoutCallback == nullptr) {
        return;
    }
    const int childCount = getChildCount();
    for (int count = 0; count < childCount; count++) {
        View* child = getChildAt(count);
        RecyclerView*rv = (RecyclerView*)child->getParent();
        mLayoutCallback/*->onLayoutFinished*/(*child, *rv);
    }
}
}/*endof namespace*/
