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
