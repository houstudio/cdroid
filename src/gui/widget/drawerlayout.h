#ifndef __DRAWER_LAYOUT_H__
#define __DRAWER_LAYOUT_H__
#include <widget/viewgroup.h>
namespace cdroid{

class DrawerLayout:public ViewGroup{
public:

    static constexpr int LOCK_MODE_UNLOCKED = 0;

    /**The drawer is locked closed. The user may not open it, though
     * the app may open it programmatically. */
    static constexpr int LOCK_MODE_LOCKED_CLOSED = 1;

    /**The drawer is locked open. The user may not close it, though the app
     * may close it programmatically.*/
    static constexpr int LOCK_MODE_LOCKED_OPEN = 2;

    /**The drawer's lock state is reset to default.*/
    static constexpr int LOCK_MODE_UNDEFINED = 3;
private:
    static constexpr int MIN_DRAWER_MARGIN = 64; // dp
    static constexpr int DRAWER_ELEVATION = 10; //dp

    static constexpr int DEFAULT_SCRIM_COLOR = 0x99000000;

    /**Length of time to delay before peeking the drawer. */
    static constexpr int PEEK_DELAY = 160; // ms

    /**Minimum velocity that will be detected as a fling */
    static constexpr int MIN_FLING_VELOCITY = 400; // dips per second

    /* Experimental feature.*/
    static constexpr bool ALLOW_EDGE_LOCK = false;
    static constexpr bool CHILDREN_DISALLOW_INTERCEPT = true;
    static constexpr float TOUCH_SLOP_SENSITIVITY = 1.f;
    static constexpr bool CAN_HIDE_DESCENDANTS=true;
    static constexpr bool SET_DRAWER_SHADOW_FROM_ELEVATION=false;
private:
    float mDrawerElevation;

    int mMinDrawerMargin;

    int mScrimColor = DEFAULT_SCRIM_COLOR;
    float mScrimOpacity;
    //Paint mScrimPaint = new Paint();

    /*ViewDragHelper mLeftDragger;
    ViewDragHelper mRightDragger;
    ViewDragCallback mLeftCallback;
    ViewDragCallback mRightCallback;*/
    int mDrawerState;
    bool mInLayout;
    bool mFirstLayout = true;

    int mLockModeLeft = LOCK_MODE_UNDEFINED;
    int mLockModeRight = LOCK_MODE_UNDEFINED;
    int mLockModeStart = LOCK_MODE_UNDEFINED;
    int mLockModeEnd = LOCK_MODE_UNDEFINED;

    bool mDisallowInterceptRequested;
    bool mChildrenCanceledTouch;

    //DrawerListener mListener;
    //List<DrawerListener> mListeners;

    float mInitialMotionX;
    float mInitialMotionY;

    Drawable* mStatusBarBackground;
    Drawable* mShadowLeftResolved;
    Drawable* mShadowRightResolved;

    std::string mTitleLeft;
    std::string mTitleRight;

    //Object mLastInsets;
    bool mDrawStatusBarBackground;

    /** Shadow drawables for different gravity */
    Drawable* mShadowStart = nullptr;
    Drawable* mShadowEnd = nullptr;
    Drawable* mShadowLeft = nullptr;
    Drawable* mShadowRight = nullptr;
};

}//endof namespace
#endif

