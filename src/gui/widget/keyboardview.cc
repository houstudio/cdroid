#include <widget/keyboardview.h>
#include <utils/textutils.h>
#include <porting/cdlog.h>
#include <fstream>
#include <cstring>
#include <climits>
#include <float.h>
#if (__cplusplus >= 201703L)&&__has_include(<execution>)
#include <execution>
#endif

namespace cdroid{

DECLARE_WIDGET2(KeyboardView,"cdroid:attr/keyboardViewStyle")

KeyboardView::KeyboardView(int w,int h):View(w,h){
    init();
    mKeyBackground = new ColorDrawable(0xFF112211);
    resetMultiTap();
}

KeyboardView::KeyboardView(Context*ctx,const AttributeSet&atts)
  :View(ctx,atts){
    init();
    Drawable *dr = ctx->getDrawable("keyBackground");
    mKeyBackground = dr ? dr:new ColorDrawable(0xFF889988);
    mVerticalCorrection= atts.getDimensionPixelOffset("verticalCorrection",0);
    mPreviewOffset     = atts.getDimensionPixelOffset("keyPreviewOffset",0);
    mPreviewHeight     = atts.getDimensionPixelOffset("keyPreviewHeight",0);
    mKeyTextSize       = atts.getDimensionPixelOffset("keyTextSize",20);
    mKeyTextColor      = atts.getColor("keyTextColor",0xFF000000);
    mLabelTextSize     = atts.getDimensionPixelOffset("labelTextSize",20);
    resetMultiTap();
}

void KeyboardView::init(){
    mLabelTextSize= 20;
    mKeyTextSize  = 28;
    mKeyTextColor = 0xFFFFFFFF;
    mInMultiTap   = false;
    mShowPreview  = false;

    mKeyboard      = nullptr;
    mInvalidatedKey= nullptr;
    mKeyBackground = nullptr;
    //mSwipeTracker= nullptr;

    mVerticalCorrection  = 0;
    mMiniKeyboardOffsetX = 0;
    mMiniKeyboardOffsetY = 0;
    mProximityThreshold  = 0;
    mPadding.set(0,0,0,0);
    mMiniKeyboardOnScreen = false;
    mDistances.resize(MAX_NEARBY_KEYS);
    mKeyIndices.resize(MAX_NEARBY_KEYS);
    std::memset(&mKeyboardActionListener,0,sizeof(mKeyboardActionListener));
}

KeyboardView::~KeyboardView(){
    delete mKeyBackground;
    delete mKeyboard;
}

void KeyboardView::setOnKeyboardActionListener(const OnKeyboardActionListener& listener){
    mKeyboardActionListener=listener;
}

Keyboard* KeyboardView::getKeyboard() {
    return mKeyboard;
}

void KeyboardView::setKeyboard(Keyboard*keyboard){
    if ( mKeyboard ) {
        showPreview(NOT_A_KEY);
    }
    // Remove any pending messages
    //removeMessages();
    mKeyboard = keyboard;
    mKeys = mKeyboard->getKeys();
    requestLayout();
    // Hint to reallocate the buffer if the size changed
    mKeyboardChanged = true;
    invalidateAllKeys();
    computeProximityThreshold(keyboard);
    //mMiniKeyboardCache.clear(); // Not really necessary to do every time, but will free up views
    // Switching to a different keyboard should abort any pending keys so that the key up
    // doesn't get delivered to the old or new keyboard
    mAbortKey = true; // Until the next ACTION_DOWN
}

bool KeyboardView::setShifted(bool shifted) {
    if (mKeyboard ) {
        if (mKeyboard->setShifted(shifted)) {
            // The whole keyboard probably needs to be redrawn
            invalidateAllKeys();
            return true;
        }
    }
    return false;
}

bool KeyboardView::isShifted()const{
    return mKeyboard && mKeyboard->isShifted();
}

void KeyboardView::setPreviewEnabled(bool previewEnabled) {
    mShowPreview = previewEnabled;
}

/**Returns the enabled state of the key feedback popup.
 * @return whether or not the key feedback popup is enabled
 * @see #setPreviewEnabled(boolean)*/
bool KeyboardView::isPreviewEnabled()const{
    return mShowPreview;
}

void KeyboardView::setVerticalCorrection(int verticalOffset) {
    mVerticalCorrection = verticalOffset;
}

void KeyboardView::setPopupParent(View* v) {
    mPopupParent = v;
}

void KeyboardView::setPopupOffset(int x, int y) {
    mMiniKeyboardOffsetX = x;
    mMiniKeyboardOffsetY = y;
    /*if (mPreviewPopup.isShowing()) {
        mPreviewPopup.dismiss();
    }*/
}

/* When enabled, calls to {@link OnKeyboardActionListener#onKey} will include key
 * codes for adjacent keys.  When disabled, only the primary key code will be
 * reported.
 * @param enabled whether or not the proximity correction is enabled*/
void KeyboardView::setProximityCorrectionEnabled(bool enabled) {
    mProximityCorrectOn = enabled;
}

/* Returns true if proximity correction is enabled. */
bool KeyboardView::isProximityCorrectionEnabled()const{
    return mProximityCorrectOn;
}

void KeyboardView::onClick(View&v){
    dismissPopupKeyboard();
}

std::string KeyboardView::adjustCase(const std::string& label){
    if( mKeyboard->isShifted() && (label.size()<3) && islower(label[0]) ){
        std::string ups=label;
#if (__cplusplus >= 201703L)&&__has_include(<execution>)
        std::transform(std::execution::par, ups.begin(), ups.end(), ups.begin(), tolower);
#else
        std::transform(ups.begin(),ups.end(),ups.begin(),tolower);
#endif
        return ups;
    }
    return label;
}

void KeyboardView::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    if (mKeyboard == nullptr) {
        setMeasuredDimension(mPaddingLeft + mPaddingRight, mPaddingTop + mPaddingBottom);
    } else {
        int width = mKeyboard->getMinWidth() + mPaddingLeft + mPaddingRight;
        if (MeasureSpec::getSize(widthMeasureSpec) < width + 10) {
            width = MeasureSpec::getSize(widthMeasureSpec);
        }
        setMeasuredDimension(width, mKeyboard->getHeight() + mPaddingTop + mPaddingBottom);
    }
}

void KeyboardView::computeProximityThreshold(Keyboard* keyboard){
    if ((keyboard == nullptr) || (mKeys.size() == 0)) return;
    const int length = mKeys.size();
    int dimensionSum = 0;
    for (Keyboard::Key*key:mKeys){
        dimensionSum += std::min(key->width, key->height) + key->gap;
    }
    if (dimensionSum < 0 || length == 0) return;
    mProximityThreshold = (int) (dimensionSum * 1.4f / length);
    mProximityThreshold *= mProximityThreshold; // Square it
}

void KeyboardView::onSizeChanged(int w, int h, int oldw, int oldh) {
    View::onSizeChanged(w, h, oldw, oldh);
    if (mKeyboard) mKeyboard->resize(w, h);
    // Release the buffer, if any and it will be reallocated on the next draw
}

void KeyboardView::onDraw(Canvas& canvas) {
    canvas.save();
    //canvas.rectangle(mDirtyRect.left,mDirtyRect.top,mDirtyRect.width,mDirtyRect.height);
    mDirtyRect.setEmpty();
    //canvas.clip();

    Drawable* keyBackground = mKeyBackground;
    Rect clipRegion = mClipRegion;
    Rect padding = mPadding;
    int kbdPaddingLeft = mPaddingLeft;
    int kbdPaddingTop = mPaddingTop;
    Keyboard::Key* invalidKey = mInvalidatedKey;
    LOGD("%d keys size=%dx%d",mKeys.size(),getWidth(),getHeight());
    canvas.set_color(mKeyTextColor);
    bool drawSingleKey = false;
    double cx1,cy1,cx2,cy2;
    canvas.get_clip_extents(cx1,cy1,cx2,cy2);
      // Is clipRegion completely contained within the invalidated key?
    if ( invalidKey&& (invalidKey->x + kbdPaddingLeft - 1 <= cx1) && (invalidKey->y + kbdPaddingTop - 1 <= cy1 )&&
          (invalidKey->x + invalidKey->width + kbdPaddingLeft + 1 >= cx2) &&
          (invalidKey->y + invalidKey->height + kbdPaddingTop + 1 >= cy2) ) {
        drawSingleKey = true;
    }
    //canvas.drawColor(0x00000000, PorterDuff.Mode.CLEAR);
    for (Keyboard::Key*key:mKeys){
        if (drawSingleKey && invalidKey != key) {
            continue;
        }
        std::vector<int> drawableState = key->getCurrentDrawableState();
        keyBackground->setState(drawableState);

        // Switch the character to uppercase if shift is pressed
        std::string label = adjustCase(key->label);

        Rect bounds = keyBackground->getBounds();
        if (key->width != bounds.right() ||  key->height != bounds.bottom()) {
            keyBackground->setBounds(0, 0, key->width, key->height);
        }
        canvas.translate(key->x + kbdPaddingLeft, key->y + kbdPaddingTop);
        LOGV("(%d,%d,%d,%d)-%d",key->x,key->y,key->width,key->height,key->gap);
        keyBackground->draw(canvas);
        if (label.empty()==false) {
            // For characters, use large font. For labels like "Done", use small font.
            canvas.set_color(mKeyTextColor);
            if( (label.length() > 1 ) && (key->codes.size() < 2)) {
                canvas.set_font_size(mLabelTextSize);
            } else {
                canvas.set_font_size(mKeyTextSize);
                //paint.setTypeface(Typeface.DEFAULT);
            }
            // Draw a drop shadow for the text
            //paint.setShadowLayer(mShadowRadius, 0, 0, mShadowColor);
            // Draw the text
            Rect rctxt={padding.left,padding.top,key->width-padding.left - padding.width,key->height-padding.top - padding.height};
            canvas.draw_text(rctxt,label,Gravity::CENTER);
            // Turn off drop shadow
        } else if (key->icon) {
            const int drawableX = (key->width - padding.left - padding.width
                            - key->icon->getIntrinsicWidth()) / 2 + padding.left;
            const int drawableY = (key->height - padding.top - padding.height
                    - key->icon->getIntrinsicHeight()) / 2 + padding.top;
            key->icon->setBounds(drawableX, drawableY, key->icon->getIntrinsicWidth(), key->icon->getIntrinsicHeight());
            key->icon->draw(canvas);
            LOGV("iconsize=%dx%d %d,%d",key->icon->getIntrinsicWidth(), key->icon->getIntrinsicHeight(),drawableX,drawableY);
        }
        canvas.translate(-key->x - kbdPaddingLeft, -key->y - kbdPaddingTop);
    }
    mInvalidatedKey = nullptr;
    // Overlay a dark rectangle to dim the keyboard
    if (mMiniKeyboardOnScreen) {
        canvas.set_color((int) (mBackgroundDimAmount * 0xFF) << 24);
        canvas.rectangle(0, 0, getWidth(), getHeight());
        canvas.fill();
    }
    canvas.restore();
}

void KeyboardView::invalidateAllKeys() {
    mDirtyRect.Union(0, 0, getWidth(), getHeight());
    invalidate();
}

void KeyboardView::invalidateKey(int keyIndex) {
    if (keyIndex < 0 || keyIndex >= mKeys.size()) {
        return;
    }
    Keyboard::Key* key = mKeys[keyIndex];
    mInvalidatedKey = key;
    mDirtyRect.Union(key->x + mPaddingLeft, key->y + mPaddingTop,key->width, key->height);
    invalidate(key->x + mPaddingLeft, key->y + mPaddingTop,key->width, key->height);
}

bool KeyboardView::openPopupIfRequired(MotionEvent& me){
    if ((mPopupLayout == 0)||(mCurrentKey < 0) || (mCurrentKey >= mKeys.size())) {
        return false;
    }
    Keyboard::Key* popupKey = mKeys[mCurrentKey];
    const bool result = onLongPress(popupKey);
    if (result) {
        mAbortKey = true;
        showPreview(NOT_A_KEY);
    }
    return result;
}
template<class T>
static void arrayCopy(std::vector<T>&src,int srcPos,std::vector<T>&dst,int destPos,int length){
    for(int i=0;i<length;i++){
        dst[destPos+i]=src[srcPos+i];
    }
}

int KeyboardView::getKeyIndices(int x, int y, std::vector<int>* allKeys){
    int primaryIndex = NOT_A_KEY;
    int closestKey = NOT_A_KEY;
    int closestKeyDist = mProximityThreshold + 1;
    std::vector<Keyboard::Key*>& keys = mKeys;

    std::vector<int> nearestKeyIndices = mKeyboard->getNearestKeys(x, y);
    const int keyCount = nearestKeyIndices.size();

    std::fill(mDistances.begin(),mDistances.end(),INT_MAX);//for(int i=0;i<mDistances.size();i++)mDistances[i]=INT_MAX;

    for (int i = 0; i < keyCount; i++) {
        Keyboard::Key* key = keys[nearestKeyIndices[i]];
        int dist = 0;
        bool isInside = key->isInside(x,y);
        if (isInside) {
            primaryIndex = nearestKeyIndices[i];
        }

        if (((mProximityCorrectOn && (dist = key->squaredDistanceFrom(x, y)) < mProximityThreshold)
                || isInside) && key->codes[0] > 32) {
            // Find insertion point
            const int nCodes = key->codes.size();
            if (dist < closestKeyDist) {
                closestKeyDist = dist;
                closestKey = nearestKeyIndices[i];
            }

            if (allKeys == nullptr) continue;

            for (int j = 0; j < mDistances.size(); j++) {
                if (mDistances[j] > dist) {
                    // Make space for nCodes codes.//arraycopy(Object src, int srcPos, Object dest, int destPos, int length)
                    arrayCopy(mDistances, j, mDistances, j + nCodes, mDistances.size() - j - nCodes);
                    arrayCopy(*allKeys, j, *allKeys, j + nCodes, allKeys->size() - j - nCodes);
                     
                    for (int c = 0; c < nCodes; c++) {
                        (*allKeys)[j + c] = key->codes[c];
                        mDistances[j + c] = dist;
                    }
                    break;
                }
            }
        }
    }
    if (primaryIndex == NOT_A_KEY) {
        primaryIndex = closestKey;
    }
    return primaryIndex;
}

void KeyboardView::detectAndSendKey(int index, int x, int y, long eventTime){
    if (index != NOT_A_KEY && index < mKeys.size()) {
        Keyboard::Key* key = mKeys[index];
        if (key->text.size()) {
            mKeyboardActionListener.onText(key->text);
            mKeyboardActionListener.onRelease(NOT_A_KEY);
        } else {
            int code = key->codes[0];
            std::vector<int>codes;
            codes.resize(MAX_NEARBY_KEYS);
            std::fill(codes.begin(),codes.end(),(int)NOT_A_KEY);
            getKeyIndices(x, y,&codes);
            // Multi-tap
            if (mInMultiTap) {
                if (mTapCount != -1) {
                    mKeyboardActionListener.onKey(Keyboard::KEYCODE_DELETE, {KeyEvent::KEYCODE_DEL});
                } else {
                    mTapCount = 0;
                }
                code = key->codes[mTapCount];
            }
            if(mKeyboardActionListener.onKey)
                mKeyboardActionListener.onKey(code, codes);
            if(mKeyboardActionListener.onRelease)
                mKeyboardActionListener.onRelease(code);
        }
        mLastSentIndex = index;
        mLastTapTime = eventTime;
    }
}

std::string KeyboardView::getPreviewText(Keyboard::Key* key){
    if (mInMultiTap) {
       std::wstring label; 
       label.append(1,key->codes[mTapCount < 0 ? 0 : mTapCount]);
       std::string u8label=TextUtils::unicode2utf8(label);
       return adjustCase(u8label);
    } else {
       return adjustCase(key->label);
    }
}

void KeyboardView::showPreview(int){
}

void KeyboardView::showKey(int keyIndex){
}

bool KeyboardView::onLongPress(Keyboard::Key* popupKey){
    return false;
}

bool KeyboardView::onHoverEvent(MotionEvent& event){
    return true;
}

bool KeyboardView::onTouchEvent(MotionEvent& me){
    const int pointerCount = me.getPointerCount();
    const int action = me.getAction();
    bool result = false;
    const int64_t now = me.getEventTime();

    if (pointerCount != mOldPointerCount) {
        if (pointerCount == 1) {
            // Send a down event for the latest pointer
            MotionEvent* down = MotionEvent::obtain(now, now, MotionEvent::ACTION_DOWN,
                    me.getX(), me.getY(), me.getMetaState());
            result = onModifiedTouchEvent(*down, false);
            down->recycle();
            // If it's an up action, then deliver the up as well.
            if (action == MotionEvent::ACTION_UP) {
                result = onModifiedTouchEvent(me, true);
            }
        } else {
            // Send an up event for the last pointer
            MotionEvent* up = MotionEvent::obtain(now, now, MotionEvent::ACTION_UP,
                    mOldPointerX, mOldPointerY, me.getMetaState());
            result = onModifiedTouchEvent(*up, true);
            up->recycle();
        }
    } else {
        if (pointerCount == 1) {
            result = onModifiedTouchEvent(me, false);
            mOldPointerX = me.getX();
            mOldPointerY = me.getY();
        } else {
            // Don't do anything when 2 pointers are down and moving.
            result = true;
        }
    }
    mOldPointerCount = pointerCount;

    return result;
}

bool KeyboardView::onModifiedTouchEvent(MotionEvent& me, bool possiblePoly){
    int touchX = (int) me.getX() - mPaddingLeft;
    int touchY = (int) me.getY() - mPaddingTop;
    if (touchY >= -mVerticalCorrection)
        touchY += mVerticalCorrection;
    const int action = me.getAction();
    const int64_t eventTime = me.getEventTime();
    int keyIndex = getKeyIndices(touchX, touchY, nullptr);
    mPossiblePoly = possiblePoly;

    // Track the last few movements to look for spurious swipes.
    //if (action == MotionEvent::ACTION_DOWN) mSwipeTracker.clear();
    //mSwipeTracker.addMovement(me);

    // Ignore all motion events until a DOWN.
    if (mAbortKey && action != MotionEvent::ACTION_DOWN && action != MotionEvent::ACTION_CANCEL) {
        return true;
    }

    /*if (mGestureDetector.onTouchEvent(me)) {
        showPreview(NOT_A_KEY);
        mHandler.removeMessages(MSG_REPEAT);
        mHandler.removeMessages(MSG_LONGPRESS);
        return true;
    }*/

    // Needs to be called after the gesture detector gets a turn, as it may have
    // displayed the mini keyboard
    if (mMiniKeyboardOnScreen && action != MotionEvent::ACTION_CANCEL) {
        return true;
    }
    bool continueLongPress=false;
    switch (action) {
    case MotionEvent::ACTION_DOWN:
        mAbortKey = false;
        mStartX = touchX;
        mStartY = touchY;
        mLastCodeX = touchX;
        mLastCodeY = touchY;
        mLastKeyTime = 0;
        mCurrentKeyTime = 0;
        mLastKey = NOT_A_KEY;
        mCurrentKey = keyIndex;
        mDownKey = keyIndex;
        mDownTime = me.getEventTime();
        mLastMoveTime = mDownTime;
        checkMultiTap(eventTime, keyIndex);
        if(mKeyboardActionListener.onPress)
            mKeyboardActionListener.onPress(keyIndex != NOT_A_KEY ?  mKeys[keyIndex]->codes[0] : 0);
        if (mCurrentKey >= 0 && mKeys[mCurrentKey]->repeatable) {
            mRepeatKeyIndex = mCurrentKey;
            //Message msg = mHandler.obtainMessage(MSG_REPEAT);
            //mHandler.sendMessageDelayed(msg, REPEAT_START_DELAY);
            repeatKey();
            // Delivering the key could have caused an abort
            if (mAbortKey) {
                mRepeatKeyIndex = NOT_A_KEY;
                break;
            }
        }
        if (mCurrentKey != NOT_A_KEY) {
            //Message msg = mHandler.obtainMessage(MSG_LONGPRESS, me);
            //mHandler.sendMessageDelayed(msg, LONGPRESS_TIMEOUT);
        }
        showPreview(keyIndex);
        break;

    case MotionEvent::ACTION_MOVE:
        if (keyIndex != NOT_A_KEY) {
            if (mCurrentKey == NOT_A_KEY) {
                mCurrentKey = keyIndex;
                mCurrentKeyTime = eventTime - mDownTime;
            } else {
                if (keyIndex == mCurrentKey) {
                    mCurrentKeyTime += eventTime - mLastMoveTime;
                    continueLongPress = true;
                } else if (mRepeatKeyIndex == NOT_A_KEY) {
                    resetMultiTap();
                    mLastKey = mCurrentKey;
                    mLastCodeX = mLastX;
                    mLastCodeY = mLastY;
                    mLastKeyTime = mCurrentKeyTime + eventTime - mLastMoveTime;
                    mCurrentKey = keyIndex;
                    mCurrentKeyTime = 0;
                }
            }
        }
        /*if (!continueLongPress) {
            // Cancel old longpress
            mHandler.removeMessages(MSG_LONGPRESS);
            // Start new longpress if key has changed
            if (keyIndex != NOT_A_KEY) {
                Message msg = mHandler.obtainMessage(MSG_LONGPRESS, me);
                mHandler.sendMessageDelayed(msg, LONGPRESS_TIMEOUT);
            }
        }*/
        showPreview(mCurrentKey);
        mLastMoveTime = eventTime;
        break;

    case MotionEvent::ACTION_UP:
        //removeMessages();
        if (keyIndex == mCurrentKey) {
            mCurrentKeyTime += eventTime - mLastMoveTime;
        } else {
            resetMultiTap();
            mLastKey = mCurrentKey;
            mLastKeyTime = mCurrentKeyTime + eventTime - mLastMoveTime;
            mCurrentKey = keyIndex;
            mCurrentKeyTime = 0;
        }
        if (mCurrentKeyTime < mLastKeyTime && mCurrentKeyTime < DEBOUNCE_TIME
                && mLastKey != NOT_A_KEY) {
            mCurrentKey = mLastKey;
            touchX = mLastCodeX;
            touchY = mLastCodeY;
        }
        showPreview(NOT_A_KEY);
        for(int i=0;i<mKeyIndices.size();i++)
            mKeyIndices[i]=NOT_A_KEY;//Arrays.fill(mKeyIndices, NOT_A_KEY);
        // If we're not on a repeating key (which sends on a DOWN event)
        if (mRepeatKeyIndex == NOT_A_KEY && !mMiniKeyboardOnScreen && !mAbortKey) {
            detectAndSendKey(mCurrentKey, touchX, touchY, eventTime);
        }
        invalidateKey(keyIndex);
        mRepeatKeyIndex = NOT_A_KEY;
        break;
    case MotionEvent::ACTION_CANCEL:
        //removeMessages();
        dismissPopupKeyboard();
        mAbortKey = true;
        showPreview(NOT_A_KEY);
        invalidateKey(mCurrentKey);
        break;
    }
    mLastX = touchX;
    mLastY = touchY;
    return true;
}

bool KeyboardView::repeatKey() {
    Keyboard::Key* key = mKeys[mRepeatKeyIndex];
    detectAndSendKey(mCurrentKey, key->x, key->y, mLastTapTime);
    return true;
}

void KeyboardView::resetMultiTap() {
    mLastSentIndex = NOT_A_KEY;
    mTapCount = 0;
    mLastTapTime = -1;
    mInMultiTap = false;
}

void KeyboardView::checkMultiTap(int64_t eventTime, int keyIndex){
    if (keyIndex == NOT_A_KEY) return;
    Keyboard::Key* key = mKeys[keyIndex];
    if (key->codes.size() > 1) {
        mInMultiTap = true;
        if (eventTime < mLastTapTime + MULTITAP_INTERVAL
                && keyIndex == mLastSentIndex) {
            mTapCount = (mTapCount + 1) % key->codes.size();
            return;
        } else {
            mTapCount = -1;
            return;
        }
    }
    if (eventTime > mLastTapTime + MULTITAP_INTERVAL || keyIndex != mLastSentIndex) {
        resetMultiTap();
    }
}

void KeyboardView::closing(){
    
    dismissPopupKeyboard();
}

void KeyboardView::onDetachedFromWindow(){
    View::onDetachedFromWindow();
    closing();
}

void KeyboardView::dismissPopupKeyboard(){
    /*if (mPopupKeyboard.isShowing()) {
         mPopupKeyboard.dismiss();
         mMiniKeyboardOnScreen = false;
         invalidateAllKeys();
    }*/
}


///////////////////////////////////////////////////////////////////////////////////////////
void  KeyboardView::SwipeTracker::clear(){
    mPastTime[0] = 0;
}

void  KeyboardView::SwipeTracker::addMovement(MotionEvent&ev){
    const int64_t time = ev.getEventTime();
    /*int N = ev.getHistorySize();
    for (int i=0; i<N; i++) {
        addPoint(ev.getHistoricalX(i), ev.getHistoricalY(i),
                ev.getHistoricalEventTime(i));
    }*/
    addPoint(ev.getX(), ev.getY(), time);
}

void  KeyboardView::SwipeTracker::addPoint(float x,float y,int64_t time){
    int drop = -1;
    int i;
    //final long[] pastTime = mPastTime;
    for (i=0; i<NUM_PAST; i++) {
        if (mPastTime[i] == 0) {
            break;
        } else if (mPastTime[i] < time-LONGEST_PAST_TIME) {
            drop = i;
        }
    }
    if (i == NUM_PAST && drop < 0) {
        drop = 0;
    }
    if (drop == i) drop--;
    //final float[] pastX = mPastX;
    //final float[] pastY = mPastY;
    if (drop >= 0) {
        int start = drop+1;
        int count = NUM_PAST-drop-1;
        /*arrayCopy(mPastX, start, mPastX, 0, count);
        arrayCopy(mPastY, start, mPastY, 0, count);
        arrayCopy(mPastTime, start, mPastTime, 0, count);*/
        i -= (drop+1);
    }
    mPastX[i] = x;
    mPastY[i] = y;
    mPastTime[i] = time;
    i++;
    if (i < NUM_PAST) {
        mPastTime[i] = 0;
    }    
}

void  KeyboardView::SwipeTracker::computeCurrentVelocity(int units){
    computeCurrentVelocity(units,FLT_MAX);
}

void  KeyboardView::SwipeTracker::computeCurrentVelocity(int units,float maxVelocity){
    float oldestX = mPastX[0];
    float oldestY = mPastY[0];
    int64_t oldestTime = mPastTime[0];
    float accumX = 0;
    float accumY = 0;
    int N=0;
    while (N < NUM_PAST) {
        if (mPastTime[N] == 0) {
            break;
        }
        N++;
    }

    for (int i=1; i < N; i++) {
        int dur = (int)(mPastTime[i] - oldestTime);
        if (dur == 0) continue;
        float dist = mPastX[i] - oldestX;
        float vel = (dist/dur) * units;   // pixels/frame.
        if (accumX == 0) accumX = vel;
        else accumX = (accumX + vel) * .5f;

        dist = mPastY[i] - oldestY;
        vel = (dist/dur) * units;   // pixels/frame.
        if (accumY == 0) accumY = vel;
        else accumY = (accumY + vel) * .5f;
    }
    mXVelocity = accumX < 0.0f ? std::max(accumX, -maxVelocity)
            : std::min(accumX, maxVelocity);
    mYVelocity = accumY < 0.0f ? std::max(accumY, -maxVelocity)
            : std::min(accumY, maxVelocity);
}

float KeyboardView::SwipeTracker::getXVelocity()const{
    return mXVelocity;
}

float KeyboardView::SwipeTracker::getYVelocity()const{
    return mYVelocity;
}

}//namespace
