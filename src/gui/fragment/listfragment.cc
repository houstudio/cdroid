#include <fragment/listfragment.h>
#include <widget/adapter.h>
namespace cdroid{
namespace fragment{

ListFragment::ListFragment(){}

cdroid::View* ListFragment::onCreateView(cdroid::LayoutInflater* /*inflater*/,
                                         cdroid::ViewGroup* /*container*/,
                                         cdroid::Bundle* /*savedInstanceState*/){
    mList = new cdroid::ListView(0, 0);
    return mList;
}

void ListFragment::onViewCreated(cdroid::View* view, cdroid::Bundle* savedInstanceState){
    Fragment::onViewCreated(view, savedInstanceState);
    if(mAdapter && mList) mList->setAdapter(mAdapter);
}

void ListFragment::setListAdapter(cdroid::Adapter* adapter){
    mAdapter = adapter;
    if(mList) mList->setAdapter(adapter);
}

}//namespace fragment
}//namespace cdroid
