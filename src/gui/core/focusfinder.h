#ifndef __FOCUS_FINDER_H__
#define __FOCUS_FINDER_H__
#include <widget/viewgroup.h>
namespace cdroid{

class FocusFinder{
private:
    FocusFinder();
    ViewGroup* getEffectiveRoot(ViewGroup* root, View* focused);
    View* findNextUserSpecifiedKeyboardNavigationCluster(View*root, View*currentCluster,int direction);
    View* findNextUserSpecifiedFocus(ViewGroup* root, View* focused, int direction);
    View* findNextFocus(ViewGroup* root, View* focused,RECT* focusedRECT,int direction, std::vector<View*>& focusables);
    View* findNextKeyboardNavigationCluster(View* root,View* currentCluster,std::vector<View*> clusters,int direction);
    void setFocusBottomRight(ViewGroup* root,RECT& focusedRECT)const;
    void setFocusTopLeft(ViewGroup* root, RECT& focusedRECT)const;
    bool isTouchCandidate(int x, int y,const RECT& destRect, int direction)const;
    static View* getNextFocusable(View* focused,std::vector<View*>& focusables, int count);
    static View* getPreviousFocusable(View* focused, std::vector<View*>& focusables, int count);
    static View* getNextKeyboardNavigationCluster(View* root,View* currentCluster,std::vector<View*>& clusters,int count);
    static View* getPreviousKeyboardNavigationCluster(View* root,View* currentCluster,std::vector<View*>& clusters,int count);
    static bool isValidId(int id) {return id != 0 && id !=-1/* View.NO_ID*/; }
protected:
    RECT mFocusedRect;
    RECT mOtherRect;
    RECT mBestCandidateRect;
    std::vector<View*>mTempList;
    static FocusFinder*mInst;
protected: 
    bool isBetterCandidate(int direction,const RECT& source,const RECT& rect1,const RECT& rect2);
    bool beamBeats(int direction,const RECT& source,const RECT&rect1,const RECT& rect2);
    int getWeightedDistanceFor(int majorAxisDistance, int minorAxisDistance) {
        return 13 * majorAxisDistance * majorAxisDistance
                + minorAxisDistance * minorAxisDistance;
    }
    bool isCandidate(const RECT& srcRect,const RECT& destRect, int direction);
    bool beamsOverlap(int direction,const RECT& rect1,const RECT& rect2);
    bool isToDirectionOf(int direction,const RECT& src,const RECT& dest);
    static int majorAxisDistance(int direction,const RECT& source,const RECT& dest) {
        return std::max(0, majorAxisDistanceRaw(direction, source, dest));
    }
    static int majorAxisDistanceRaw(int direction,const RECT& source,const RECT& dest);
    static int majorAxisDistanceToFarEdge(int direction,const RECT& source,const RECT& dest) {
        return std::max(1, majorAxisDistanceToFarEdgeRaw(direction, source, dest));
    }
    static int majorAxisDistanceToFarEdgeRaw(int direction,const RECT&source,const RECT& dest);
    static int minorAxisDistance(int direction,const RECT& source,const RECT& dest);
    View* findNextFocusInRelativeDirection(std::vector<View*>&focusables,ViewGroup*root,View*focused,const RECT*focusedRECT,int direction);
    View* findNextFocusInAbsoluteDirection(std::vector<View*>&focusables,ViewGroup*root,View*focused,const RECT* focusedRECT,int direction);
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
    View* findNextFocus(ViewGroup* root, View* focused, int direction);
    View* findNextFocus(ViewGroup* root, View* focused,RECT* focusedRect, int direction);
    View* findNextFocusFromRect(ViewGroup* root,const RECT* focusedRect, int direction);
    View* findNextKeyboardNavigationCluster(View* root/*notnull*/,View* currentCluster,int direction);
    View* findNearestTouchable(ViewGroup* root, int x, int y, int direction, std::vector<int> deltas);
};

}
#endif
