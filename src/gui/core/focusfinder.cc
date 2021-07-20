#include <focusfinder.h>
#include <viewconfiguration.h>
namespace cdroid {

FocusFinder*FocusFinder::mInst=nullptr;

FocusFinder&FocusFinder::getInstance(){
    if(mInst==nullptr)
       mInst=new FocusFinder();
    return *mInst;
}

FocusFinder::FocusFinder(){
}

View* FocusFinder::findNextFocus(ViewGroup* root, View* focused, int direction) {
    return findNextFocus(root, focused, nullptr, direction);
}

View* FocusFinder::findNextFocusFromRect(ViewGroup* root,const RECT* focusedRect,int direction) {
    mFocusedRect=*focusedRect;
    return findNextFocus(root, nullptr,&mFocusedRect, direction);
}

View* FocusFinder::findNextFocus(ViewGroup* root, View* focused,RECT* focusedRect, int direction) {
    View* next = nullptr;
    ViewGroup* effectiveRoot = getEffectiveRoot(root, focused);
    if (focused != nullptr) {
        next = findNextUserSpecifiedFocus(effectiveRoot, focused, direction);
    }
    if (next != nullptr) {
        return next;
    }
    std::vector<View*>&focusables = mTempList;
    focusables.clear();
    effectiveRoot->addFocusables(mTempList, direction,(effectiveRoot->isInTouchMode()?View::FOCUSABLES_TOUCH_MODE:View::FOCUSABLES_ALL));
    if (!focusables.empty()) {
        next = findNextFocus(effectiveRoot, focused, focusedRect, direction, focusables);
    }
    focusables.clear();
    return next;
}

ViewGroup* FocusFinder::getEffectiveRoot(ViewGroup* root, View* focused) {
    if (focused == nullptr || focused == root) {
        return root;
    }
    ViewGroup* effective = nullptr;
    ViewGroup* nextParent = (ViewGroup*)focused->getParent();
    do {
        if (nextParent == root) {
            return effective != nullptr ? effective : root;
        }
        ViewGroup* vg = (ViewGroup*) nextParent;
        if (vg&&vg->getTouchscreenBlocksFocus()
                //&& focused.getContext().getPackageManager().hasSystemFeature(PackageManager.FEATURE_TOUCHSCREEN)
                && vg->isKeyboardNavigationCluster()) {
            // Don't stop and return here because the cluster could be nested and we only
            // care about the top-most one.
            effective = vg;
        }
	if(nextParent==nullptr)break;
        nextParent = (ViewGroup*)nextParent->getParent();
    } while (nextParent);// instanceof ViewGroup);
    return root;
}

View* FocusFinder::findNextKeyboardNavigationCluster(View* root,
        View* currentCluster,int direction) {
    View* next = nullptr;
    if (currentCluster != nullptr) {
        next = findNextUserSpecifiedKeyboardNavigationCluster(root, currentCluster, direction);
        if (next != nullptr) {
            return next;
        }
    }

    std::vector<View*>& clusters = mTempList;
    clusters.clear();
    root->addKeyboardNavigationClusters(clusters, direction);
    if (!clusters.empty()) {
        next = findNextKeyboardNavigationCluster(
                       root, currentCluster, clusters, direction);
    }
    clusters.clear();
    return next;
}

View* FocusFinder::findNextUserSpecifiedKeyboardNavigationCluster(View* root,View* currentCluster, int direction) {
    View* userSetNextCluster =
        currentCluster->findUserSetNextKeyboardNavigationCluster(root, direction);
    if (userSetNextCluster != nullptr && userSetNextCluster->hasFocusable()) {
        return userSetNextCluster;
    }
    return nullptr;
}

View* FocusFinder::findNextUserSpecifiedFocus(ViewGroup* root, View* focused, int direction) {
    // check for user specified next focus
    View* userSetNextFocus = focused->findUserSetNextFocus(root, direction);
    while (userSetNextFocus != nullptr) {
        if (userSetNextFocus->isFocusable()
                && userSetNextFocus->getVisibility() == View::VISIBLE
                && (!userSetNextFocus->isInTouchMode()
                    || userSetNextFocus->isFocusableInTouchMode())) {
            return userSetNextFocus;
        }
        userSetNextFocus = userSetNextFocus->findUserSetNextFocus(root, direction);
    }
    return nullptr;
}

View* FocusFinder::findNextFocus(ViewGroup* root, View* focused,RECT* focusedRect,
                                 int direction,std::vector<View*>& focusables) {
    if (focused != nullptr) {
        if (focusedRect == nullptr) {
            focusedRect = &mFocusedRect;
        }
        // fill in interesting rect from focused
        focused->getFocusedRect(*focusedRect);
        root->offsetDescendantRectToMyCoords(focused, *focusedRect);
    } else {
        if (focusedRect == nullptr) {
            focusedRect = &mFocusedRect;
            // make up a rect at top left or bottom right of root
            switch (direction) {
            case View::FOCUS_RIGHT:
            case View::FOCUS_DOWN:
                setFocusTopLeft(root,*focusedRect);
                break;
            case View::FOCUS_FORWARD:
                if (root->isLayoutRtl()) {
                    setFocusBottomRight(root,*focusedRect);
                } else {
                    setFocusTopLeft(root, *focusedRect);
                }
                break;

            case View::FOCUS_LEFT:
            case View::FOCUS_UP:
                setFocusBottomRight(root,*focusedRect);
                break;
            case View::FOCUS_BACKWARD:
                if (root->isLayoutRtl()) {
                    setFocusTopLeft(root, *focusedRect);
                } else {
                    setFocusBottomRight(root, *focusedRect);
                    break;
                }
            }
        }
    }

    switch (direction) {
    case View::FOCUS_FORWARD:
    case View::FOCUS_BACKWARD:
        return findNextFocusInRelativeDirection(focusables,root,focused,focusedRect,direction);
    case View::FOCUS_UP:
    case View::FOCUS_DOWN:
    case View::FOCUS_LEFT:
    case View::FOCUS_RIGHT:
        return findNextFocusInAbsoluteDirection(focusables,root,focused,focusedRect,direction);
    default:break;
    }
    return nullptr;
}

View* FocusFinder::findNextKeyboardNavigationCluster(View* root,View* currentCluster,
        std::vector<View*> clusters,int direction) {
        // Note: This sort is stable.
    /*mUserSpecifiedClusterComparator.setFocusables(clusters, root);
    Collections.sort(clusters, mUserSpecifiedClusterComparator);
    mUserSpecifiedClusterComparator.recycle();*/
    const int count = clusters.size();

    switch (direction) {
    case View::FOCUS_FORWARD:
    case View::FOCUS_DOWN:
    case View::FOCUS_RIGHT: return getNextKeyboardNavigationCluster(root, currentCluster, clusters, count);
    case View::FOCUS_BACKWARD:
    case View::FOCUS_UP:
    case View::FOCUS_LEFT:  return getPreviousKeyboardNavigationCluster(root, currentCluster, clusters, count);
    default: return nullptr;
    }
}

View* FocusFinder::findNextFocusInRelativeDirection(std::vector<View*>& focusables, ViewGroup* root,
        View* focused,const RECT* focusedRect, int direction) {
        // Note: This sort is stable.
    /*mUserSpecifiedFocusComparator.setFocusables(focusables, root);
    Collections.sort(focusables, mUserSpecifiedFocusComparator);
    mUserSpecifiedFocusComparator.recycle();*/

    const int count = focusables.size();
    switch (direction) {
    case View::FOCUS_FORWARD : return getNextFocusable(focused, focusables, count);
    case View::FOCUS_BACKWARD: return getPreviousFocusable(focused, focusables, count);
    }
    return focusables.at(count - 1);
}

void FocusFinder::setFocusBottomRight(ViewGroup* root, RECT& focusedRect)const{
    const int rootBottom = root->getScrollY() + root->getHeight();
    const int rootRight = root->getScrollX() + root->getWidth();
    focusedRect.set(rootRight, rootBottom, rootRight, rootBottom);
}

void FocusFinder::setFocusTopLeft(ViewGroup* root, RECT&focusedRect)const{
    const int rootTop = root->getScrollY();
    const int rootLeft = root->getScrollX();
    focusedRect.set(rootLeft, rootTop, rootLeft, rootTop);
}

View* FocusFinder::findNextFocusInAbsoluteDirection(std::vector<View*>&focusables, ViewGroup* root, View* focused,
        const RECT*focusedRect, int direction) {
    // initialize the best candidate to something impossible
    // (so the first plausible view will become the best choice)
    mBestCandidateRect=*focusedRect;
    switch(direction) {
    case View::FOCUS_LEFT:   mBestCandidateRect.offset(focusedRect->width + 1, 0);        break;
    case View::FOCUS_RIGHT:  mBestCandidateRect.offset(-(focusedRect->width + 1), 0);     break;
    case View::FOCUS_UP:     mBestCandidateRect.offset(0, focusedRect->height + 1);       break;
    case View::FOCUS_DOWN:   mBestCandidateRect.offset(0, -(focusedRect->height + 1));    break;
    }

    View* closest = nullptr;

    int numFocusables = focusables.size();
    for (int i = 0; i < numFocusables; i++) {
        View* focusable = focusables.at(i);

        // only interested in other non-root views
        if (focusable == focused || focusable == root) continue;

        // get focus bounds of other view in same coordinate system
        focusable->getFocusedRect(mOtherRect);
        root->offsetDescendantRectToMyCoords(focusable, mOtherRect);

        if (isBetterCandidate(direction,*focusedRect, mOtherRect, mBestCandidateRect)) {
            mBestCandidateRect=mOtherRect;
            closest = focusable;
        }
    }
    return closest;
}

View* FocusFinder::getNextFocusable(View* focused,std::vector<View*>& focusables, int count) {
    if (focused != nullptr) {
        std::vector<View*>ss={focused};
        auto itr = std::find_end(focusables.begin(),focusables.end(),ss.begin(),ss.end());
        if ( (itr!=focusables.end()) && ((itr-focusables.begin() + 1) < count)) {
            return *(itr+1);
        }
    }
    if (!focusables.empty()) {
        return focusables.at(0);
    }
    return nullptr;
}

View* FocusFinder::getPreviousFocusable(View* focused,std::vector<View*>& focusables, int count) {
    if (focused != nullptr) {
        auto position = std::find(focusables.begin(),focusables.end(),focused);
        if (position!=focusables.end()) {
            return *position;
        }
    }
    if (!focusables.empty()) {
        return focusables.at(count - 1);
    }
    return nullptr;
}

View* FocusFinder::getNextKeyboardNavigationCluster(View* root,View* currentCluster,std::vector<View*>&clusters,int count) {
    if (currentCluster == nullptr) {
        // The current cluster is the default one.
        // The next cluster after the default one is the first one.
        // Note that the caller guarantees that 'clusters' is not empty.
        return clusters.at(0);
    }
    std::vector<View*>ss={currentCluster};
    auto itr = std::find_end(clusters.begin(),clusters.end(),ss.begin(),ss.end());//lastIndexOf(currentCluster);
    if ((itr!=clusters.end()) && ((itr-clusters.begin()+1) < count)) {
        // Return the next non-default cluster if we can find it.
        return *(itr+1);
    }

    // The current cluster is the last one. The next one is the default one, i.e. the
    // root.
    return root;
}

View* FocusFinder::getPreviousKeyboardNavigationCluster(View*root,View*currentCluster,std::vector<View*>&clusters,int count) {
    if (currentCluster == nullptr) {
        // The current cluster is the default one.
        // The previous cluster before the default one is the last one.
        // Note that the caller guarantees that 'clusters' is not empty.
        return clusters.at(count - 1);
    }

    auto position =std::find(clusters.begin(),clusters.end(),currentCluster);
    if (position !=clusters.end()) {
        // Return the previous non-default cluster if we can find it.
        return *position;
    }

    // The current cluster is the first one. The previous one is the default one, i.e.
    // the root.
    return root;
}

bool FocusFinder::isBetterCandidate(int direction,const RECT& source,const RECT& rect1,const RECT& rect2) {

    // to be a better candidate, need to at least be a candidate in the first
    // place :)
    if (!isCandidate(source, rect1, direction))  return false;

    // we know that rect1 is a candidate.. if rect2 is not a candidate,
    // rect1 is better
    if (!isCandidate(source, rect2, direction))  return true;

    // if rect1 is better by beam, it wins
    if (beamBeats(direction, source, rect1, rect2))return true;

    // if rect2 is better, then rect1 cant' be :)
    if (beamBeats(direction, source, rect2, rect1))return false;

    // otherwise, do fudge-tastic comparison of the major and minor axis
    return getWeightedDistanceFor(majorAxisDistance(direction, source, rect1), minorAxisDistance(direction, source, rect1))
            < getWeightedDistanceFor(majorAxisDistance(direction, source, rect2), minorAxisDistance(direction, source, rect2));
}

bool FocusFinder::beamBeats(int direction,const RECT& source,const RECT& rect1,const RECT& rect2) {
    const bool rect1InSrcBeam = beamsOverlap(direction, source, rect1);
    const bool rect2InSrcBeam = beamsOverlap(direction, source, rect2);

    // if rect1 isn't exclusively in the src beam, it doesn't win
    if (rect2InSrcBeam || !rect1InSrcBeam) {
        return false;
    }

    // we know rect1 is in the beam, and rect2 is not

    // if rect1 is to the direction of, and rect2 is not, rect1 wins.
    // for example, for direction left, if rect1 is to the left of the source
    // and rect2 is below, then we always prefer the in beam rect1, since rect2
    // could be reached by going down.
    if (!isToDirectionOf(direction, source, rect2)) return true;

    // for horizontal directions, being exclusively in beam always wins
    if ((direction == View::FOCUS_LEFT || direction == View::FOCUS_RIGHT)) return true;

    // for vertical directions, beams only beat up to a point:
    // now, as long as rect2 isn't completely closer, rect1 wins
    // e.g for direction down, completely closer means for rect2's top
    // edge to be closer to the source's top edge than rect1's bottom edge.
    return (majorAxisDistance(direction, source, rect1)
            < majorAxisDistanceToFarEdge(direction, source, rect2));
}

bool FocusFinder::isCandidate(const RECT& srcRect,const RECT& destRect, int direction) {
    switch (direction) {
    case View::FOCUS_LEFT:  return (srcRect.right() > destRect.right() || srcRect.x >= destRect.right()) && srcRect.x > destRect.x;
    case View::FOCUS_RIGHT: return (srcRect.x < destRect.x || srcRect.right() <= destRect.x) && srcRect.right() < destRect.right();
    case View::FOCUS_UP:    return (srcRect.bottom() > destRect.bottom() || srcRect.y >= destRect.bottom()) && srcRect.y > destRect.y;
    case View::FOCUS_DOWN:  return (srcRect.y < destRect.y || srcRect.bottom() <= destRect.y) && srcRect.bottom() < destRect.bottom();
    default:  return false;
    }
}

bool FocusFinder::beamsOverlap(int direction,const RECT& rect1,const RECT&rect2) {
    switch (direction) {
    case View::FOCUS_LEFT:
    case View::FOCUS_RIGHT: return (rect2.bottom() >= rect1.y) && (rect2.y <= rect1.bottom());
    case View::FOCUS_UP:
    case View::FOCUS_DOWN:  return (rect2.right() >= rect1.x) && (rect2.x <= rect1.right());
    default:  return false;
    }
}

bool FocusFinder::isToDirectionOf(int direction,const RECT& src,const RECT&dest) {
    switch (direction) {
    case View::FOCUS_LEFT:  return src.x >= dest.right();
    case View::FOCUS_RIGHT: return src.right() <= dest.x;
    case View::FOCUS_UP:    return src.y >= dest.bottom();
    case View::FOCUS_DOWN:  return src.bottom() <= dest.y;
    default:return false;
    }
}

int FocusFinder::majorAxisDistanceRaw(int direction,const RECT&source,const RECT&dest) {
    switch (direction) {
    case View::FOCUS_LEFT:  return source.x - dest.right();
    case View::FOCUS_RIGHT: return dest.x - source.right();
    case View::FOCUS_UP:    return source.y - dest.bottom();
    case View::FOCUS_DOWN:  return dest.y - source.bottom();
    default:return false;
    }
}

int FocusFinder::majorAxisDistanceToFarEdgeRaw(int direction,const RECT& source,const RECT& dest) {
    switch (direction) {
    case View::FOCUS_LEFT:  return source.x - dest.x;
    case View::FOCUS_RIGHT: return dest.right() - source.right();
    case View::FOCUS_UP:    return source.y - dest.y;
    case View::FOCUS_DOWN:  return dest.bottom() - source.bottom();
    default:return false;
    }
}

int FocusFinder::minorAxisDistance(int direction,const RECT& source,const RECT&dest) {
    switch (direction) {
    case View::FOCUS_LEFT:
    case View::FOCUS_RIGHT: // the distance between the center verticals
        return std::abs(((source.y + source.height / 2)- ((dest.y + dest.height / 2))));
    case View::FOCUS_UP:
    case View::FOCUS_DOWN:  // the distance between the center horizontals
        return std::abs(((source.x + source.width / 2) - ((dest.x + dest.width / 2))));
    default:return false;
    }
}

View* FocusFinder::findNearestTouchable(ViewGroup* root, int x, int y, int direction, std::vector<int> deltas) {
    std::vector<View*> touchables;
    int minDistance =INT_MAX;// Integer.MAX_VALUE;
    int numTouchables=0;
    View* closest = nullptr;
    root->addTouchables(touchables);
    numTouchables = touchables.size();

    int edgeSlop = 10;//ViewConfiguration.get(root.mContext).getScaledEdgeSlop();

    RECT closestBounds;
    RECT touchableBounds = mOtherRect;

    for (int i = 0; i < numTouchables; i++) {
        View* touchable = touchables.at(i);

        // get visible bounds of other view in same coordinate system
        touchableBounds=touchable->getDrawingRect();

        root->offsetRectBetweenParentAndChild(touchable, touchableBounds, true, true);

        if (!isTouchCandidate(x, y, touchableBounds, direction)) {
            continue;
        }

        int distance =INT_MAX;// Integer.MAX_VALUE;

        switch (direction) {
        case View::FOCUS_LEFT: distance = x - touchableBounds.right() + 1;  break;
        case View::FOCUS_RIGHT:distance = touchableBounds.x;                break;
        case View::FOCUS_UP:   distance = y - touchableBounds.bottom() + 1; break;
        case View::FOCUS_DOWN: distance = touchableBounds.y;                break;
        }

        if (distance < edgeSlop) {
            // Give preference to innermost views
            if (closest == nullptr ||
                    closestBounds.contains(touchableBounds) ||
                    (!touchableBounds.contains(closestBounds) && distance < minDistance)) {
                minDistance = distance;
                closest = touchable;
                closestBounds=touchableBounds;
                switch (direction) {
                case View::FOCUS_LEFT:  deltas[0] = -distance;    break;
                case View::FOCUS_RIGHT: deltas[0] = distance;     break;
                case View::FOCUS_UP:    deltas[1] = -distance;    break;
                case View::FOCUS_DOWN:  deltas[1] = distance;     break;
                }
            }
        }
    }
    return closest;
}

bool FocusFinder::isTouchCandidate(int x, int y,const RECT&destRect, int direction)const{
    switch (direction) {
    case View::FOCUS_LEFT:  return destRect.x <= x && destRect.y <= y && y <= destRect.bottom();
    case View::FOCUS_RIGHT: return destRect.x >= x && destRect.y <= y && y <= destRect.bottom();
    case View::FOCUS_UP:    return destRect.y <= y && destRect.x <= x && x <= destRect.right();
    case View::FOCUS_DOWN:  return destRect.y >= y && destRect.x <= x && x <= destRect.right();
    default:  return false;
    }
}

}//namespace

