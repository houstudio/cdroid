#ifndef __HEADERVIEW_LISTADAPTER_H__
#define __HEADERVIEW_LISTADAPTER_H__
#include <widget/adapter.h>
namespace cdroid{
class FixedViewInfo;

class HeaderViewListAdapter:public Adapter{
private:
    bool areAllListInfosSelectable(const std::vector<FixedViewInfo*>& infos)const;
protected:
    Adapter*mAdapter;
    std::vector<FixedViewInfo*>mHeaderViewInfos;
    std::vector<FixedViewInfo*>mFooterViewInfos;
    bool mAreAllFixedViewsSelectable;
    bool mIsFilterable;
public:
    HeaderViewListAdapter(const std::vector<FixedViewInfo*>& headerViewInfos,
                const std::vector<FixedViewInfo*>& footerViewInfos,Adapter* adapter);
    ~HeaderViewListAdapter();
    int getHeadersCount()const;
    int getFootersCount()const;
    bool isEmpty()const override;
    bool removeHeader(View* v);
    bool removeFooter(View* v);
    int getCount()const override;
    bool areAllItemsEnabled()const override;
    bool isEnabled(int position)const override;
    void* getItem(int position)const override;
    long getItemId(int position)const override;
    bool hasStableIds()const override;
    View* getView(int position, View* convertView, ViewGroup* parent)override;
    int getItemViewType(int position)const;
    int getViewTypeCount()const override;
    void registerDataSetObserver(DataSetObserver observer)override;
    void unregisterDataSetObserver(DataSetObserver observer)override;
    Adapter* getWrappedAdapter();
};

}//namespace
#endif
