#ifndef __LISTFRAGMENT_H__
#define __LISTFRAGMENT_H__
/*********************************************************************************
 * Port of androidx.fragment.app.ListFragment. A Fragment that hosts a ListView;
 * subclasses override onListItemClick and call setListAdapter.
 *********************************************************************************/
#include <fragment/fragment.h>
#include <widget/listview.h>
namespace cdroid{
namespace fragment{

class ListFragment : public Fragment{
public:
    ListFragment();
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle* savedInstanceState) override;
    void onViewCreated(cdroid::View* view, cdroid::Bundle* savedInstanceState) override;

    void setListAdapter(cdroid::Adapter* adapter);
    cdroid::ListView* getListView() const { return mList; }
protected:
    // Override to handle item clicks.
    virtual void onListItemClick(cdroid::ListView* l, cdroid::View* v, int position, long id){}
private:
    cdroid::ListView* mList = nullptr;
    cdroid::Adapter* mAdapter = nullptr;
};

}//namespace fragment
}//namespace cdroid
#endif
