#ifndef __OBSERVABLE_H__
#define __OBSERVABLE_H__

namespace cdroid{

template<typename T>
class Observable{
protected:
    std::vector<T*> mObservers;
public:
    void registerObserver(T* observer) {
        LOGD_IF(observer==nullptr,"The observer is null.");
	auto it = std::find(mObservers.begin(),mObservers.end(),observer);
        LOGE_IF(it!=mObservers.end(),"Observer %p is already registered.",observer);
        mObservers.push_back(observer);
    }

    void unregisterObserver(T* observer) {
        LOGD_IF(observer == nullptr,"The observer is null.");
        auto it = std::find(mObservers.begin(),mObservers.end(),observer);
        LOGE_IF(it==mObservers.end(),"Observer %p was not registered.",observer);
        mObservers.erase(it);
    }
    void unregisterAll() {
        mObservers.clear();
    }
};

}/*endof namespace*/
#endif/*__OBSERVABLE_H__*/
