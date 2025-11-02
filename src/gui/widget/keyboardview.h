#ifndef __KEYBOARD_VIEW_H__
#define __KEYBOARD_VIEW_H__

#include <widget/edittext.h>
#include <vector>
#include <core/keyboard.h>

namespace cdroid{

class KeyboardView:public View{
public:
    class  OnKeyboardActionListener {
    public:
        /**Called when the user presses a key. This is sent before the {@link #onKey} is called.
         * For keys that repeat, this is only called once.
         * @param primaryCode the unicode of the key being pressed. If the touch is not on a valid
         * key, the value will be zero.*/
        std::function<void(int primaryCode)>onPress;

        /**Called when the user releases a key. This is sent after the {@link #onKey} is called.
         * For keys that repeat, this is only called once.
         * @param primaryCode the code of the key that was released */
        std::function<void(int primaryCode)>onRelease;

        /**Send a key press to the listener.
         * @param primaryCode this is the key that was pressed
         * @param keyCodes the codes for all the possible alternative keys
         * with the primary code being the first. If the primary key code is
         * a single character such as an alphabet or number or symbol, the alternatives
         * will include other characters that may be on the same key or adjacent keys.
         * These codes are useful to correct for accidental presses of a key adjacent to
         * the intended key.*/
        std::function<void(int primaryCode,const std::vector<int>&keyCodes)>onKey;

        /* Sends a sequence of characters to the listener.
         * @param text the sequence of characters to be displayed. */
        std::function<void(std::string&text)>onText;

        /* Called when the user quickly moves the finger from right to left. */
        std::function<void()>swipeLeft;

        /* Called when the user quickly moves the finger from left to right. */
        std::function<void()>swipeRight;

        /* Called when the user quickly moves the finger from up to down. */
        std::function<void()>swipeDown;

        /* Called when the user quickly moves the finger from down to up. */
        std::function<void()>swipeUp;
    };
private:
    static constexpr int NOT_A_KEY = -1;
    static constexpr int MSG_SHOW_PREVIEW = 1;
    static constexpr int MSG_REMOVE_PREVIEW = 2;
    static constexpr int MSG_REPEAT = 3;
    static constexpr int MSG_LONGPRESS = 4;

    static constexpr int DELAY_BEFORE_PREVIEW= 0;
    static constexpr int DELAY_AFTER_PREVIEW = 70;
    static constexpr int DEBOUNCE_TIME   = 70;
    static constexpr int REPEAT_INTERVAL = 50;
    static constexpr int REPEAT_START_DELAY = 400;
    static constexpr int MAX_NEARBY_KEYS = 12;
    static constexpr int MULTITAP_INTERVAL =800;

    class SwipeTracker{
    protected:
        static constexpr int NUM_PAST =4 ;
        static constexpr int LONGEST_PAST_TIME = 200;
        float mPastX[NUM_PAST];
        float mPastY[NUM_PAST];
        int64_t mPastTime[NUM_PAST]; 
        float mXVelocity;
        float mYVelocity;
    public:
        void clear();
        void addMovement(MotionEvent&ev);
        void addPoint(float x,float y,int64_t time);
        void computeCurrentVelocity(int units);
        void computeCurrentVelocity(int units,float maxVelocity);
        float getXVelocity()const;
        float getYVelocity()const;
    };
 
    Keyboard* mKeyboard;
    int  mCurrentKeyIndex = NOT_A_KEY;
    int  mLabelTextSize;
    int  mKeyTextSize;
    int  mKeyTextColor;
    float mShadowRadius;
    int  mShadowColor;
    float mBackgroundDimAmount;

    TextView* mPreviewText;
    //PopupWindow mPreviewPopup;
    int  mPreviewTextSizeLarge;
    int  mPreviewOffset;
    int  mPreviewHeight;
    int  mCoordinates[2];
    View* mPopupParent;
    bool mMiniKeyboardOnScreen;
    int  mMiniKeyboardOffsetX;
    int  mMiniKeyboardOffsetY;
    std::vector<Keyboard::Key*>mKeys;
    OnKeyboardActionListener mKeyboardActionListener;
    int  mVerticalCorrection;
    int  mProximityThreshold;

    bool mPreviewCentered = false;
    bool mShowPreview = true;
    bool mShowTouchPoints = true;
    int  mPopupPreviewX;
    int  mPopupPreviewY;

    int  mLastX;
    int  mLastY;
    int  mStartX;
    int  mStartY;

    bool mProximityCorrectOn;

    Rect mPadding;

    int64_t mDownTime;
    int64_t mLastMoveTime;
    int  mLastKey;
    int  mLastCodeX;
    int  mLastCodeY;
    int  mCurrentKey = NOT_A_KEY;
    int  mDownKey = NOT_A_KEY;
    int64_t mLastKeyTime;
    int64_t mCurrentKeyTime;

    std::vector<int>  mKeyIndices;
    int  mPopupX;
    int  mPopupY;
    int  mRepeatKeyIndex = NOT_A_KEY;
    int  mPopupLayout;
    bool mAbortKey;
    Keyboard::Key* mInvalidatedKey;
    Rect mClipRegion;
    bool mPossiblePoly;
    //SwipeTracker *mSwipeTracker;
    int  mSwipeThreshold;
    bool mDisambiguateSwipe;

    // Variables for dealing with multiple pointers
    int  mOldPointerCount = 1;
    float mOldPointerX;
    float mOldPointerY;

    Drawable* mKeyBackground;
    std::vector<int> mDistances;//[MAX_NEARBY_KEYS];

    // For multi-tap
    int  mLastSentIndex;
    int  mTapCount;
    int64_t mLastTapTime;
    bool mInMultiTap;
    bool mKeyboardChanged;
    Rect mDirtyRect;
private:
    void init();
    std::string adjustCase(const std::string& label);
    void computeProximityThreshold(Keyboard* keyboard);
    std::string getPreviewText(Keyboard::Key* key);
    void showPreview(int keyIndex);
    void showKey(int keyIndex);
    int  getKeyIndices(int x, int y, std::vector<int>* allKeys);
    void detectAndSendKey(int index, int x, int y, long eventTime);
    bool openPopupIfRequired(MotionEvent& me);
    bool onModifiedTouchEvent(MotionEvent& me, bool possiblePoly);
    bool repeatKey();
    void resetMultiTap();
    void checkMultiTap(int64_t eventTime, int keyIndex);
    void dismissPopupKeyboard();
protected:
    bool onLongPress(Keyboard::Key* popupKey);
public:
    KeyboardView(int w,int h);
    KeyboardView(Context*context,const AttributeSet&atts);
    ~KeyboardView()override;
    void setOnKeyboardActionListener(const OnKeyboardActionListener& listener);
    Keyboard*getKeyboard();
    void setKeyboard(Keyboard*k);
    bool setShifted(bool shifted);
    bool isShifted()const;
    void setPreviewEnabled(bool previewEnabled);
    bool isPreviewEnabled()const;
    void setVerticalCorrection(int verticalOffset);
    void setPopupParent(View* v);
    void setPopupOffset(int x, int y);
    void setProximityCorrectionEnabled(bool enabled);
    bool isProximityCorrectionEnabled()const;
    void onClick(View&v);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onSizeChanged(int w, int h, int oldw, int oldh)override;
    void onDraw(Canvas& canvas)override;
    void invalidateAllKeys();
    void invalidateKey(int keyIndex);
    bool onHoverEvent(MotionEvent& event)override;
    bool onTouchEvent(MotionEvent& me)override;
    void closing();
    void onDetachedFromWindow()override;
};
}//namespace
#endif
