#ifndef __COMPOSE_ONPAGE_CHANGE_CALLBACK_H__
#define __COMPOSE_ONPAGE_CHANGE_CALLBACK_H__
#include <widgetEx/viewpager2.h>
namespace cdroid{
class CompositeOnPageChangeCallback:public ViewPager2::OnPageChangeCallback {
private:
    std::vector<OnPageChangeCallback> mCallbacks;
public:
    CompositeOnPageChangeCallback(int initialCapacity) {
    }

    void addOnPageChangeCallback(OnPageChangeCallback callback) {
        mCallbacks.push_back(callback);
    }

    void removeOnPageChangeCallback(OnPageChangeCallback callback) {
        //mCallbacks.remove(callback);
    }

    void onPageScrolled(int position, float positionOffset,int positionOffsetPixels) {
        for (OnPageChangeCallback callback : mCallbacks) {
            callback.onPageScrolled(position, positionOffset, positionOffsetPixels);
        }
    }

    void onPageSelected(int position) {
         for (OnPageChangeCallback callback : mCallbacks) {
             callback.onPageSelected(position);
         }
    }

    void onPageScrollStateChanged(int state) {
         for (OnPageChangeCallback callback : mCallbacks) {
             callback.onPageScrollStateChanged(state);
         }
    }
};
}/*endof namespace*/
#endif /*_COMPOSE_ONPAGE_CHANGE_CALLBACK_H__*/
