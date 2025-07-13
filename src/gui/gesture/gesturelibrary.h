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
#ifndef __GESTURE_LIBRARY_H__
#define __GESTURE_LIBRARY_H__
#include <string>
#include <vector>
#include <gesture/learner.h>
#include <gesture/gesture.h>
#include <gesture/prediction.h>
#include <gesture/gesturestore.h>
namespace cdroid{
class GestureLibrary {
protected:
    GestureStore* mStore;
    GestureLibrary() {
        mStore = new GestureStore();
    }
public:
    virtual bool save()=0;
    virtual bool load()=0;
    virtual ~GestureLibrary(){
        delete mStore;
    }
    virtual bool isReadOnly()const {
        return false;
    }

    /** @hide */
    Learner& getLearner() {
        return mStore->getLearner();
    }

    void setOrientationStyle(int style) {
        mStore->setOrientationStyle(style);
    }

    int getOrientationStyle() {
        return mStore->getOrientationStyle();
    }

    void setSequenceType(int type) {
        mStore->setSequenceType(type);
    }

    int getSequenceType() {
        return mStore->getSequenceType();
    }

    std::vector<std::string> getGestureEntries() {
        return mStore->getGestureEntries();
    }

    std::vector<Prediction> recognize(const Gesture& gesture) {
        return mStore->recognize(gesture);
    }

    void addGesture(const std::string& entryName, Gesture* gesture) {
        mStore->addGesture(entryName, gesture);
    }

    void removeGesture(const std::string& entryName, Gesture* gesture) {
        mStore->removeGesture(entryName, gesture);
    }

    void removeEntry(const std::string& entryName) {
        mStore->removeEntry(entryName);
    }

    std::vector<Gesture*> getGestures(const std::string& entryName) {
        return mStore->getGestures(entryName);
    }
};
}/*endof namespace*/
#endif/*__GESTURE_LIBRARY_H__*/
