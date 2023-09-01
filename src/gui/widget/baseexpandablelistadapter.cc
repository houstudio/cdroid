#include <widget/baseexpandablelistadapter.h>
namespace cdroid{

void BaseExpandableListAdapter::registerDataSetObserver(DataSetObserver* observer) {
    //mDataSetObservable->registerObserver(observer);
    auto it = std::find(mDataObservers.begin(),mDataObservers.end(),observer);
    if(it != mDataObservers.end())mDataObservers.push_back(observer);
}

void BaseExpandableListAdapter::unregisterDataSetObserver(DataSetObserver* observer) {
    //mDataSetObservable->unregisterObserver(observer);
    auto it = std::find(mDataObservers.begin(),mDataObservers.end(),observer);
    if(it != mDataObservers.end())mDataObservers.erase(it);
}

void BaseExpandableListAdapter::notifyDataSetInvalidated() {
    //mDataSetObservable->notifyInvalidated();
    for(auto it=mDataObservers.begin();it!=mDataObservers.end();it++)
       (*it)->onInvalidated();
}

void BaseExpandableListAdapter::notifyDataSetChanged() {
    //mDataSetObservable.notifyChanged();
    for(auto it=mDataObservers.begin();it!=mDataObservers.end();it++)
       (*it)->onChanged();
}

}/*endof namespace*/

