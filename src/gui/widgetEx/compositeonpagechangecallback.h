/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
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
