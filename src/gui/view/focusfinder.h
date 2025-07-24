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
#ifndef __FOCUS_FINDER_H__
#define __FOCUS_FINDER_H__
#include <view/viewgroup.h>
namespace cdroid{

class FocusFinder{
private:
    FocusFinder();
    ViewGroup* getEffectiveRoot(ViewGroup* root, View* focused);
    View* findNextUserSpecifiedKeyboardNavigationCluster(View*root, View*currentCluster,int direction);
    View* findNextUserSpecifiedFocus(ViewGroup* root, View* focused, int direction);
    View* findNextFocus(ViewGroup* root, View* focused,Rect* focusedRect,int direction, std::vector<View*>& focusables);
    View* findNextKeyboardNavigationCluster(View* root,View* currentCluster,std::vector<View*> clusters,int direction);
    void setFocusBottomRight(ViewGroup* root,Rect& focusedRect)const;
    void setFocusTopLeft(ViewGroup* root, Rect& focusedRect)const;
    bool isTouchCandidate(int x, int y,const Rect& destRect, int direction)const;
    static View* getNextFocusable(View* focused,std::vector<View*>& focusables, int count,bool*outLooped);
    static View* getPreviousFocusable(View* focused, std::vector<View*>& focusables, int count,bool*outLooped);
    static View* getNextKeyboardNavigationCluster(View* root,View* currentCluster,std::vector<View*>& clusters,int count);
    static View* getPreviousKeyboardNavigationCluster(View* root,View* currentCluster,std::vector<View*>& clusters,int count);
    static bool isValidId(int id) {return id != 0 && id !=-1/* View.NO_ID*/; }
protected:
    Rect mFocusedRect;
    Rect mOtherRect;
    Rect mBestCandidateRect;
    std::vector<View*>mTempList;
    static FocusFinder*mInst;
protected: 
    bool isBetterCandidate(int direction,const Rect& source,const Rect& rect1,const Rect& rect2);
    bool beamBeats(int direction,const Rect& source,const Rect&rect1,const Rect& rect2);
    int getWeightedDistanceFor(int majorAxisDistance, int minorAxisDistance);
    bool isCandidate(const Rect& srcRect,const Rect& destRect, int direction);
    bool beamsOverlap(int direction,const Rect& rect1,const Rect& rect2);
    bool isToDirectionOf(int direction,const Rect& src,const Rect& dest);
    static int majorAxisDistance(int direction,const Rect& source,const Rect& dest);
    static int majorAxisDistanceRaw(int direction,const Rect& source,const Rect& dest);
    static int majorAxisDistanceToFarEdge(int direction,const Rect& source,const Rect& dest);
    static int majorAxisDistanceToFarEdgeRaw(int direction,const Rect&source,const Rect& dest);
    static int minorAxisDistance(int direction,const Rect& source,const Rect& dest);
    View* findNextFocusInRelativeDirection(std::vector<View*>&focusables,ViewGroup*root,View*focused,const Rect*focusedRect,int direction);
    View* findNextFocusInAbsoluteDirection(std::vector<View*>&focusables,ViewGroup*root,View*focused,const Rect* focusedRect,int direction);
public:
   /**
     * Find the next view to take focus in root's descendants, starting from the view
     * that currently is focused.
     * @param root Contains focused. Cannot be null.
     * @param focused Has focus now.
     * @param direction Direction to look.
     * @return The next focusable view, or null if none exists.
     */
    static FocusFinder&getInstance();
    static void sort(std::vector<View*>&views, int start, int end, ViewGroup* root, bool isRtl);
    View* findNextFocus(ViewGroup* root, View* focused, int direction);
    View* findNextFocus(ViewGroup* root, View* focused,Rect* focusedRect, int direction);
    View* findNextFocusFromRect(ViewGroup* root,const Rect* focusedRect, int direction);
    View* findNextKeyboardNavigationCluster(View* root/*notnull*/,View* currentCluster,int direction);
    View* findNearestTouchable(ViewGroup* root, int x, int y, int direction, std::vector<int> deltas);
};

}
#endif
