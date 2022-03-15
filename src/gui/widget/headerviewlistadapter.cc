#include <widget/headerviewlistadapter.h>
#include <widget/listview.h>
#include <cdlog.h>

namespace cdroid{

//////////////////////////////////////////////////////////////////////
HeaderViewListAdapter::HeaderViewListAdapter(const std::vector<FixedViewInfo*>& headerViewInfos,
            const std::vector<FixedViewInfo*>& footerViewInfos,Adapter* adapter){
    mAdapter = adapter;
    mIsFilterable =false;// adapter instanceof Filterable;

    mHeaderViewInfos = headerViewInfos;

    mFooterViewInfos = footerViewInfos;

    mAreAllFixedViewsSelectable =areAllListInfosSelectable(mHeaderViewInfos)
                && areAllListInfosSelectable(mFooterViewInfos);
}

HeaderViewListAdapter::~HeaderViewListAdapter(){
    for_each(mHeaderViewInfos.begin(),mHeaderViewInfos.end(),[](const FixedViewInfo*f){delete f;});
    for_each(mFooterViewInfos.begin(),mFooterViewInfos.end(),[](const FixedViewInfo*f){delete f;});
    mHeaderViewInfos.clear();
    mFooterViewInfos.clear();
}
int HeaderViewListAdapter::getHeadersCount()const{
    return mHeaderViewInfos.size();
}

int HeaderViewListAdapter::getFootersCount()const {
    return mFooterViewInfos.size();
}

bool HeaderViewListAdapter::isEmpty()const{
    return mAdapter == nullptr || mAdapter->isEmpty();
}

bool HeaderViewListAdapter::areAllListInfosSelectable(const std::vector<FixedViewInfo*>& infos)const{
    for (auto info : infos) {
        if (!info->isSelectable) {
            return false;
        }
    }
    return true;
}

bool HeaderViewListAdapter::removeHeader(View* v) {
    for (int i = 0; i < mHeaderViewInfos.size(); i++) {
        FixedViewInfo* info = mHeaderViewInfos[i];
        if (info->view == v) {
            mHeaderViewInfos.erase(mHeaderViewInfos.begin()+i);
            mAreAllFixedViewsSelectable =areAllListInfosSelectable(mHeaderViewInfos)
                        && areAllListInfosSelectable(mFooterViewInfos);
            return true;
        }
    }

    return false;
}

bool HeaderViewListAdapter::removeFooter(View* v) {
    for (int i = 0; i < mFooterViewInfos.size(); i++) {
        FixedViewInfo* info = mFooterViewInfos[i];
        if (info->view == v) {
            mFooterViewInfos.erase(mFooterViewInfos.begin()+i);

            mAreAllFixedViewsSelectable =areAllListInfosSelectable(mHeaderViewInfos)
                        && areAllListInfosSelectable(mFooterViewInfos);
            return true;
        }
    }

    return false;
}

int HeaderViewListAdapter::getCount()const {
    return getFootersCount() + getHeadersCount() + (mAdapter?mAdapter->getCount():0);
}

bool HeaderViewListAdapter::areAllItemsEnabled()const{
    if (mAdapter != nullptr) {
        return mAreAllFixedViewsSelectable && mAdapter->areAllItemsEnabled();
    } else {
        return true;
    }
}

bool HeaderViewListAdapter::isEnabled(int position)const{
    int numHeaders = getHeadersCount();
    if (position < numHeaders) {
        return mHeaderViewInfos[position]->isSelectable;
    }

    // Adapter
    int adjPosition = position - numHeaders;
    int adapterCount = 0;
    if (mAdapter != nullptr) {
        adapterCount = mAdapter->getCount();
        if (adjPosition < adapterCount) {
            return mAdapter->isEnabled(adjPosition);
        }
    }

    // Footer (off-limits positions will throw an IndexOutOfBoundsException)
    if(adjPosition - adapterCount<0||adjPosition - adapterCount>=mFooterViewInfos.size())
        return false;
    return mFooterViewInfos[adjPosition - adapterCount]->isSelectable;
}

void* HeaderViewListAdapter::getItem(int position)const{
    // Header (negative positions will throw an IndexOutOfBoundsException)
    int numHeaders = getHeadersCount();
    if (position < numHeaders) {
        return mHeaderViewInfos[position]->data;
    }

    // Adapter
    int adjPosition = position - numHeaders;
    int adapterCount = 0;
    if (mAdapter != nullptr) {
        adapterCount = mAdapter->getCount();
        if (adjPosition < adapterCount) {
            return mAdapter->getItem(adjPosition);
        }
    }

    // Footer (off-limits positions will throw an IndexOutOfBoundsException)
    return mFooterViewInfos[adjPosition - adapterCount]->data;
}

long HeaderViewListAdapter::getItemId(int position)const{
    int numHeaders = getHeadersCount();
    if (mAdapter  && position >= numHeaders) {
        int adjPosition = position - numHeaders;
        int adapterCount = mAdapter->getCount();
        if (adjPosition < adapterCount) {
            return mAdapter->getItemId(adjPosition);
        }
    }
    return -1;
}

bool HeaderViewListAdapter::hasStableIds()const{
    return mAdapter && mAdapter->hasStableIds();
}

View* HeaderViewListAdapter::getView(int position, View* convertView, ViewGroup* parent) {
    // Header (negative positions will throw an IndexOutOfBoundsException)
    int numHeaders = getHeadersCount();
    if (position < numHeaders) {
        return mHeaderViewInfos[position]->view;
    }

    // Adapter
    int adjPosition = position - numHeaders;
    int adapterCount = 0;
    if (mAdapter != nullptr) {
        adapterCount = mAdapter->getCount();
        if (adjPosition < adapterCount) {
            return mAdapter->getView(adjPosition, convertView, parent);
        }
    }

    // Footer (off-limits positions will throw an IndexOutOfBoundsException)
    return mFooterViewInfos[adjPosition - adapterCount]->view;
}

int HeaderViewListAdapter::getItemViewType(int position)const {
    int numHeaders = getHeadersCount();
    if (mAdapter != nullptr && position >= numHeaders) {
        int adjPosition = position - numHeaders;
        int adapterCount = mAdapter->getCount();
        if (adjPosition < adapterCount) {
            return mAdapter->getItemViewType(adjPosition);
        }
    }

    return AdapterView::ITEM_VIEW_TYPE_HEADER_OR_FOOTER;
}

int HeaderViewListAdapter::getViewTypeCount()const{
    if (mAdapter != nullptr) {
        return mAdapter->getViewTypeCount();
    }
    return 1;
}

void HeaderViewListAdapter::registerDataSetObserver(DataSetObserver observer) {
    if (mAdapter != nullptr) {
        mAdapter->registerDataSetObserver(observer);
    }
}

void HeaderViewListAdapter::unregisterDataSetObserver(DataSetObserver observer) {
    if (mAdapter != nullptr) {
        mAdapter->unregisterDataSetObserver(observer);
    }
}

Adapter* HeaderViewListAdapter::getWrappedAdapter() {
    return mAdapter;
}

}//namespace
