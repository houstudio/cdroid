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
#ifndef __TEXTVIEW_H__
#define __TEXTVIEW_H__

#include <view/view.h>
#include <core/typeface.h>
#include <widget/scroller.h>
#include <text/spanwatcher.h>
#include <text/textwatcher.h>
#include <text/textutils.h>
#include <text/spannablestring.h>
#include <text/boringlayout.h>
#include <text/dynamiclayout.h>
#include <text/editable.h>
#include <text/method/transformationmethod.h>
#include <text/method/keylistener.h>

namespace cdroid {
class Layout;
class CompletionInfo;
class PrecomputesText;
class Editor;
class MovementMethod;
class InputFilter;
class InputConnection;
class EditorInfo;
class InputMethodManager;
class TextView : public View{
    friend class Editor;   // Editor drives TextView's editing UX and reaches its internals
private:
    static constexpr int DEFAULT_TYPEFACE = -1;
    static constexpr int SANS = 1;
    static constexpr int SERIF= 2;
    static constexpr int MONOSPACE = 3;
    static constexpr int VERY_WIDE = 1024 * 1024;
    static constexpr int CHANGE_WATCHER_PRIORITY = 100;
    static constexpr int ANIMATED_SCROLL_GAP = 250;
    static constexpr int KEY_EVENT_NOT_HANDLED = 0;
    static constexpr int KEY_EVENT_HANDLED = -1;
    static constexpr int KEY_DOWN_HANDLED_BY_KEY_LISTENER = 1;
    static constexpr int KEY_DOWN_HANDLED_BY_MOVEMENT_METHOD = 2;
    class Marquee;
    class CharWrapper;
    class ChangeWatcher;
public:
    static constexpr int AUTO_SIZE_TEXT_TYPE_NONE = 0;
    static constexpr int AUTO_SIZE_TEXT_TYPE_UNIFORM = 1;
    class Drawables;
    enum BufferType {
        NORMAL, SPANNABLE, EDITABLE
    };
private:
    static constexpr int LINES = 1;
    static constexpr int EMS = LINES;
    static constexpr int PIXELS = 2;
    static constexpr int MARQUEE_FADE_NORMAL=0;
    static constexpr int MARQUEE_FADE_SWITCH_SHOW_ELLIPSIS =1;
    static constexpr int MARQUEE_FADE_SWITCH_SHOW_FADE =2;
    static constexpr int DEFAULT_AUTO_SIZE_MIN_TEXT_SIZE_IN_SP=12;
    static constexpr int DEFAULT_AUTO_SIZE_MAX_TEXT_SIZE_IN_SP = 112;
    static constexpr int DEFAULT_AUTO_SIZE_GRANULARITY_IN_PX = 1;
    static constexpr float UNSET_AUTO_SIZE_UNIFORM_CONFIGURATION_VALUE = -1.f;
    int mGravity;
    int mMaximum;
    int mMinimum;
    int mMaxMode;
    int mMinMode;
    int mMaxWidth;
    int mMinWidth;
    int mOldMaximum;
    int mOldMaxMode;
    int mMaxWidthMode;
    int mMinWidthMode;
    int mDesiredHeightAtMeasure;
    int mShadowColor;
    int mDeferScroll;
    int mAutoSizeTextType;
    float mShadowRadius, mShadowDx, mShadowDy;
    float mSpacingMult;
    float mSpacingAdd;
    int mBreakStrategy;
    int mHyphenationFrequency;
    int mJustificationMode;
    bool mHideHint;
    bool mSingleLine;
    bool mIncludePad;
    bool mHorizontallyScrolling;
    bool mNeedsAutoSizeText;
    bool mRestartMarquee;
    bool mUserSetTextScaleX;
    bool mHighlightPathBogus;
    bool mUseFallbackLineSpacing;
    bool mHasPresetAutoSizeValues;
    bool mPreDrawRegistered;
    bool mPreDrawListenerDetached;
    bool mTextSetFromXmlOrResourceId;
    bool mAllowTransformationLengthChange;
    bool mLinksClickable;
    bool mCursorVisible;
    bool mShowSoftInputOnFocus = true;
    bool mSelectAllOnFocus = false;   // Android: TextView field (was misplaced on Editor)
    bool mTextIsSelectable = false;   // Android: TextView field (was misplaced on Editor)
    // This is used to reflect the current user preference for changing font weight and making text
    // more bold.
    int mFontWeightAdjustment;
    int mAutoLinkMask;
    int mSelectionStart;
    int mSelectionEnd;
    int mLineBreakStyle;
    int mLineBreakWordStyle;
    TextPaint mTextPaint;
    ViewTreeObserver::OnPreDrawListener mOnPreDrawListener;

    cdroid::RefPtr<ColorStateList> mTextColor;
    cdroid::RefPtr<ColorStateList> mHintTextColor;
    cdroid::RefPtr<ColorStateList> mLinkTextColor;
    int mCurTextColor;
    int mCurHintTextColor;
    int mHighlightColor;
    int mAutoSizeMinTextSizeInPx;
    int mAutoSizeMaxTextSizeInPx;
    int mAutoSizeStepGranularityInPx;
    std::vector<int>mAutoSizeTextSizesInPx;
    std::vector<TextWatcher>mListeners;
    std::vector<InputFilter*> mFilters;

    Drawables*mDrawables;
    Marquee*mMarquee;
    CharWrapper* mCharWrapper;
    mutable Drawable* mCursorDrawable;
    Editor* mEditor = nullptr;
    MovementMethod* mMovement = nullptr;   // Android: mMovement — arrow/nav/scroll handling
    KeyListener* mKeyListener = nullptr;
    // Android: mPreventDefaultMovement — once the movement method consumes an
    // initial key down, swallow subsequent focus-traversal defaults until key up.
    bool mPreventDefaultMovement = false;
    TextUtils::TruncateAt mEllipsize;
    int  mMarqueeFadeMode;
    int  mMarqueeRepeatLimit;
    int  mLastLayoutDirection;
    int64_t mLastScroll;
    Cairo::RefPtr<Path> mHighlightPath;
    Paint mHighlightPaint;
    BoringLayout* mSavedLayout;
    BoringLayout* mSavedHintLayout;
    Layout* mSavedMarqueeModeLayout;
    const TextDirectionHeuristic* mTextDir;
    Scroller*mScroller;
    BoringLayout::Metrics* mBoring;
    BoringLayout::Metrics* mHintBoring;
private:
    // Android: doKeyDown — shared key-down logic for onKeyDown/onKeyMultiple.
    int doKeyDown(int keyCode, KeyEvent& event, KeyEvent* otherEvent);

    // Android TextView statics (TextView.java:7747/7867) — InputType classification.
    static bool isPasswordInputType(int inputType);
    static bool isVisiblePasswordInputType(int inputType);
    static bool isMultilineInputType(int type);

    void initView();
    InputMethodManager* getInputMethodManager();
    void setTextInternal(CharSequence* text);
    void setHintInternal(CharSequence* hint);
    bool setupAutoSizeUniformPresetSizesConfiguration();
    void validateAndSetAutoSizeTextTypeUniformConfiguration(float autoSizeMinTextSizeInPx,
            float autoSizeMaxTextSizeInPx, float autoSizeStepGranularityInPx);
    void clearAutoSizeConfiguration();
    std::vector<int> cleanupAutoSizePresetSizes(std::vector<int>&presetValues);
    bool setupAutoSizeText();
    void setTypefaceFromAttrs(Typeface* typeface,const std::string& familyName,
           int typefaceIndex,int style,int weight);
    void resolveStyleAndSetTypeface(Typeface* typeface,int style,int weight);
    void setRelativeDrawablesIfNeeded(Drawable* start, Drawable* end);///
    Layout::Alignment getLayoutAlignment()const;
    void applyCompoundDrawableTint();
    void registerForPreDraw();
    void unregisterForPreDraw();
    int  getVerticalOffset(bool forceNormal);
    int  getBottomVerticalOffset(bool forceNormal);
    void updateTextColors();
    int findLargestTextSizeWhichFits(const RectF& availableSpace);
    bool suggestedSizeFitsInSpace(int suggestedSizeInPx,const RectF& availableSpace);
    int  getDesiredHeight();
    int  getDesiredHeight(Layout* layout, bool cap);
    void getInterestingRect(Rect& r, int line);
    void convertFromViewportToContentCoordinates(Rect&);

    void checkForResize();
    void checkForRelayout();
    bool isShowingHint()const;
    bool bringTextIntoView();
    void autoSizeText();
    bool compressText(float width);
    static int desired(Layout*);
    int  getBoxHeight(Layout* l);
    void prepareDrawableForDisplay(Drawable*d);

    void createEditorIfNeeded();
    void assumeLayout();
    bool isAutoSizeEnabled()const;
    bool isMarqueeFadeEnabled()const;
    void startStopMarquee(bool start);
    float getHorizontalFadingEdgeStrength(float position1, float position2);
    void applySingleLine(bool singleLine, bool applyTransformation, bool changeMaxLines);
    void restartMarqueeIfNeeded();
    void invalidateCursor(int a, int b, int c);
    void setRawTextSize(float size, bool shouldRequestLayout);
    void setTextSizeInternal(int unit, float size, bool shouldRequestLayout);
    void applyTextAppearance(class TextAppearanceAttributes *atts);
    void setText(CharSequence* text, BufferType type, bool notifyBefore, int oldlen);
    void sendBeforeTextChanged(CharSequence& text, int start, int before, int after);
    void removeIntersectingNonAdjacentSpans(int,int,const SpanFilter&type);
    void removeAdjacentSuggestionSpans(int pos);
    void sendAfterTextChanged(Editable& text);
    void spanChange(Spanned& buf,const ParcelableSpan* what, int oldStart, int newStart, int oldEnd, int newEnd);
    void sendOnTextChanged(CharSequence& text, int start, int before, int after);
protected:
    int mMaxLength;
    bool mBlinkOn;
    Cairo::RefPtr<Cairo::FontFace>mTypeFace;
    Layout* mLayout;
    Layout* mHintLayout;
    TransformationMethod* mTransformation;
    CharSequence*mText;
    CharSequence*mHint;
    Spannable*mSpannable;
    PrecomputedText* mPrecomputed;
    CharSequence*mTransformed;
    ChangeWatcher* mChangeWatcher;
    BufferType mBufferType = BufferType::NORMAL;
    virtual bool getDefaultEditable()const;
    void invalidateCursorPath();
    void invalidateCursor();
    void invalidateRegion(int start, int end, bool invalidateCursor);
    Cairo::RefPtr<cdroid::Path>getUpdatedHighlightPath();
    void drawableStateChanged()override;
    bool verifyDrawable(Drawable* who)const override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    bool supportsAutoSizeText()const;
    bool isPaddingOffsetRequired()const override;
    bool setFrame(int l, int t, int w, int h)override;
    int getLeftPaddingOffset()override;
    int getTopPaddingOffset()override;
    int getBottomPaddingOffset()override;
    int getRightPaddingOffset()override;
    bool canMarquee()const;
    void startMarquee();
    void stopMarquee();
    virtual void onTextChanged(CharSequence& text, int start, int lengthBefore, int lengthAfter);
    virtual void onSelectionChanged(int selStart, int selEnd);
    void updateAfterEdit();
    // Attach this TextView's ChangeWatcher (plus the transformation/movement
    // spans) to an editable Spannable buffer — the span-setup block Android does
    // inline in TextView.setText. Factored out so setEditable() (which wraps a
    // non-editable buffer on the fly) attaches the same watchers; without it,
    // edits mutate the buffer but never notify this view, so no refresh.
    void addChangeWatcher(Spannable& sp);
    CharSequence* removeSuggestionSpans(CharSequence* text);
    void handleTextChanged(CharSequence& buffer, int start, int before, int after);
    void onAttachedToWindow();
    void onDetachedFromWindowInternal()override;
    bool onPreDraw();
    virtual void onDraw(Canvas& canvas) override;
    // Hook invoked by Editor::drawCursor to paint the caret. Override to customize
    // the caret appearance (the blink cadence and geometry are owned by Editor).
    virtual int getHorizontalOffsetForDrawables()const;
    void onLayout(bool changed, int left, int top, int w, int h)override;
    void onFocusChanged(bool focused, int direction, Rect* previouslyFocusedRect)override;
    void onWindowFocusChanged(bool hasWindowFocus)override;
    void onVisibilityChanged(View& changedView, int visibility)override;
    void viewClicked(class InputMethodManager*);
    void resetResolvedDrawables()override;
    int viewportToContentHorizontalOffset();
    int viewportToContentVerticalOffset();
    float getLeftFadingEdgeStrength()override;
    float getRightFadingEdgeStrength()override;
    Layout* makeSingleLayout(int wantWidth, BoringLayout::Metrics* boring, int ellipsisWidth,
        Layout::Alignment alignment, bool shouldEllipsize, TextUtils::TruncateAt effectiveEllipsize, bool useSaved);
public:
    TextView(Context*ctx,const AttributeSet&attrs);
    TextView(int width, int height);
    TextView(const std::string& text, int width, int height);
    ~TextView()override;
    void setAutoSizeTextTypeWithDefaults(int autoSizeTextType);
    void setAutoSizeTextTypeUniformWithConfiguration(int autoSizeMinTextSize,
        int autoSizeMaxTextSize, int autoSizeStepGranularity, int unit);
    void setAutoSizeTextTypeUniformWithPresetSizes(const std::vector<int>& presetSizes, int unit);
    int getAutoSizeTextType()const;
    int getAutoSizeStepGranularity()const;
    int getAutoSizeMinTextSize()const;
    int getAutoSizeMaxTextSize()const;
    std::vector<int> getAutoSizeTextAvailableSizes()const;
    void setEnabled(bool);
    void setTypeface(Typeface* tf);
    void setTypeface(Typeface* tf,int style);///
    Typeface* getTypeface()const;
    int getTypefaceStyle() const;
    virtual void setText(const std::string&txt);
    virtual void setText(CharSequence* txt);
    virtual void setText(CharSequence* text, BufferType type);
    void setText(const std::vector<char16_t>&text, int start, int len);
    void append(const CharSequence& text);
    void append(const CharSequence& text, int start, int end);
    virtual CharSequence& getText();
    int length()const;
    CharSequence* getTransformed()const;
    Editable* getEditableText()const;
    Editor* getEditor();
    // Android: movement method — arrow/nav/scroll handling delegated here.
    void setMovementMethod(MovementMethod* movement);
    MovementMethod* getMovementMethod() const { return mMovement; }
    void setTextCursorDrawable(Drawable*);
    Drawable* getTextCursorDrawable()const;
    void setTextAppearance(const std::string&);
    void setTextAppearance(Context*,const std::string&);
    virtual void setHint(const std::string&txt);
    virtual void setHint(CharSequence*);
    CharSequence* getHint()const;
    bool bringPointIntoView(int offset);
    bool moveCursorToVisibleOffset();
    void computeScroll()override;
    bool isSuggestionsEnabled()const;
    bool canSelectText()const;
    bool canSelectAllText()const;
    bool selectAllText();
    int getSelectionStart()const;
    int getSelectionEnd()const;
    bool hasSelection()const;
    std::string getSelectedText()const;
    // Android public API — touch→offset, cursor visibility, IME-on-focus,
    // select-all-on-focus, batch editing (all delegate to Editor when editable).
    int  getOffsetForPosition(float x, float y);
    void setCursorVisible(bool visible);
    bool isCursorVisible()const;
    void setShowSoftInputOnFocus(bool show);
    bool getShowSoftInputOnFocus()const;
    void setSelectAllOnFocus(bool selectAll);
    void setTextIsSelectable(bool selectable);
    bool isTextSelectable()const;
    bool isTextEditable()const;
    void beginBatchEdit();
    void endBatchEdit();
    virtual void setSingleLine(bool single);
    bool isSingleLine()const;
    bool hasPasswordTransformationMethod()const;
    void setBreakStrategy(int breakStrategy);
    int getBreakStrategy()const;
    void setHyphenationFrequency(int hyphenationFrequency);
    int getHyphenationFrequency()const;
    void setAutoLinkMask(int mask);
    int getAutoLinkMask()const;
    void setLinksClickable(bool whether);
    bool getLinksClickable()const;
    void setKeyListener(KeyListener* input);
    KeyListener* getKeyListener()const;
    void setInputType(int inputType);
    int  getInputType()const;
    bool isAnyPasswordInputType()const;   // Android TextView (TextView.java:7862)
    void setFilters(const std::vector<InputFilter*>& filters);
    std::vector<InputFilter*> getFilters();
    int getLineHeight()const;
    void setLineHeight(int height);
    void setLineBreakStyle(int lineBreakStyle);
    int getLineBreakStyle()const;
    void setLineBreakWordStyle(int lineBreakWordStyle);
    int getLineBreakWordStyle()const;
    void setTextSize(float size);
    void setTextSize(int unit, float size);
    float getTextSize()const;
    float getScaledTextSize() const;
    float getTextScaleX()const;
    void setTextScaleX(float);
    void setElegantTextHeight(bool elegant);
    bool isElegantTextHeight()const;
    void setFallbackLineSpacing(bool);
    bool isFallbackLineSpacing()const;
    float getLetterSpacing() const;
    void setLetterSpacing(float letterSpacing);
    void setJustificationMode(int justificationMode);
    int getJustificationMode() const;
    void setTextColor(int color);
    void setTextColor(const cdroid::RefPtr<ColorStateList>& colors);
    Layout* getLayout()const;
    Layout* getHintLayout()const;
    void setShadowLayer(float radius, float dx, float dy, int color);
    float getShadowRadius()const;
    float getShadowDx()const;
    float getShadowDy()const;
    int getShadowColor()const;
    const TextPaint& getPaint()const;
    int getPaintFlags() const;
    void setPaintFlags(int);
    void setLineSpacing(float add, float mult);
    float getLineSpacingMultiplier()const;
    float getLineSpacingExtra()const;

    void setPadding(int left, int top, int right, int bottom)override;
    void setPaddingRelative(int start, int top, int end, int bottom)override;
    void setFirstBaselineToTopHeight(int firstBaselineToTopHeight);
    void setLastBaselineToBottomHeight(int lastBaselineToBottomHeight);
    int getFirstBaselineToTopHeight();
    int getLastBaselineToBottomHeight();

    void setIncludeFontPadding(bool includepad);
    bool getIncludeFontPadding()const;
    void setMarqueeRepeatLimit(int marqueeLimit);
    int  getMarqueeRepeatLimit() const;
    TextUtils::TruncateAt getEllipsize() const;
    virtual void setEllipsize(TextUtils::TruncateAt ellipsize);

    const cdroid::RefPtr<ColorStateList> getTextColors()const;
    int getCurrentTextColor()const;
    void setHighlightColor(int color);
    int getHighlightColor()const;
    void setHintTextColor(int color);
    void setHintTextColor(const cdroid::RefPtr<ColorStateList>& colors);
    const cdroid::RefPtr<ColorStateList> getHintTextColors()const;
    int getCurrentHintTextColor()const;
    void setLinkTextColor(int color);
    void setLinkTextColor(const cdroid::RefPtr<ColorStateList>& colors);
    const cdroid::RefPtr<ColorStateList> getLinkTextColors()const;
    void setMinWidth(int minPixels);
    int getMinWidth()const;
    void setMaxWidth(int maxPixels);
    int getMaxWidth()const;
    void setWidth(int pixels);

    int getMinHeight()const;
    void setMinHeight(int minPixels);
    void setMaxLines(int maxLines);
    int getMaxLines()const;
    void setMinLines(int minLines);
    int getMinLines()const;
    int getMaxHeight()const;
    void setMaxHeight(int maxPixels);
    void setHeight(int pixels);
    void setLines(int lines);
    void setMinEms(int minEms);
    int getMinEms()const;
    void setMaxEms(int maxEms);
    int getMaxEms()const;
    void setEms(int ems);
    int getLineCount()const;
    int getLineBounds(int line, Rect&bounds);
    int getBaseline()override;
    int getBaselineOffset();
    int getGravity()const;
    void setGravity(int gravity);
    void setHorizontallyScrolling(bool whether);
    bool isHorizontallyScrollable()const;
    bool getHorizontallyScrolling()const;
    void setSelected(bool selected)override;
    TransformationMethod* getTransformationMethod()const;
    void setTransformationMethod(TransformationMethod*);
    // Android TextView offset-mapping helpers (TextView.java:10386/15827/15842).
    // True when the transformed text implements OffsetMapping (length-altering
    // TransformationMethod such as password). Used by movement/Editor.
    bool isOffsetMappingAvailable()const;
    int transformedToOriginal(int offset, int strategy)const;
    int originalToTransformed(int offset, int strategy)const;
    virtual int getCompoundPaddingLeft()const;
    virtual int getCompoundPaddingRight()const;
    virtual int getCompoundPaddingTop()const;
    virtual int getCompoundPaddingBottom()const;
    int getExtendedPaddingTop();
    int getExtendedPaddingBottom();
    virtual int getCompoundPaddingStart();
    virtual int getCompoundPaddingEnd();
    int getTotalPaddingLeft();
    int getTotalPaddingRight();
    int getTotalPaddingStart();
    int getTotalPaddingEnd();
    int getTotalPaddingTop();
    int getTotalPaddingBottom();

    void setCompoundDrawablePadding(int pad);
    int getCompoundDrawablePadding()const;
    std::vector<Drawable*>getCompoundDrawables()const;
    std::vector<Drawable*> getCompoundDrawablesRelative() const;
    void setCompoundDrawableTintList(const cdroid::RefPtr<ColorStateList>& tint);
    const cdroid::RefPtr<ColorStateList> getCompoundDrawableTintList()const;
    void drawableHotspotChanged(float x,float y)override;
    void setCompoundDrawableTintMode(int tintMode);
    int getCompoundDrawableTintMode()const;
    void jumpDrawablesToCurrentState()override;
    void invalidateDrawable(Drawable& drawable)override;
    void setCompoundDrawables(Drawable* left,Drawable* top,Drawable* right,Drawable*bottom);
    void setCompoundDrawablesWithIntrinsicBounds(Drawable* left,Drawable* top,Drawable* right,Drawable*bottom);
    void setCompoundDrawablesWithIntrinsicBounds(const std::string& left, const std::string& top,
                const std::string& right,const std::string& bottom);
    int computeHorizontalScrollRange()override;
    int computeVerticalScrollRange()override;

    void addTextChangedListener(const TextWatcher& watcher);
    void removeTextChangedListener(const TextWatcher& watcher);
    const TextDirectionHeuristic*getTextDirectionHeuristic()const;
    void onResolveDrawables(int layoutDirection)override;
    bool onTouchEvent(MotionEvent& event)override;
    bool onKeyDown(int keyCode, KeyEvent& event)override;
    bool onKeyUp(int keyCode, KeyEvent& event)override;
    virtual void onCommitCompletion(CompletionInfo* completion);
    bool useDynamicLayout() const;
    void nullLayouts();
    void makeNewLayout(int wantWidth, int hintWidth, BoringLayout::Metrics* boring,
                    BoringLayout::Metrics* hintBoring,int ellipsisWidth, bool bringIntoView);

    std::string getAccessibilityClassName()const override;
    void onInitializeAccessibilityEventInternal(AccessibilityEvent& event)override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
    bool performAccessibilityActionInternal(int action, Bundle* arguments)override;
    void sendAccessibilityEventInternal(int eventType)override;
    void sendAccessibilityEventUnchecked(AccessibilityEvent& event)override;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TextView::Drawables {
public:
    enum{
        LEFT  = 0,
        TOP   = 1,
        RIGHT = 2,
        BOTTOM= 3
    };
    enum{
        DRAWABLE_NONE = -1,
        DRAWABLE_RIGHT= 0,
        DRAWABLE_LEFT = 1
    };
private:
    friend class TextView;
    Drawable* mShowing[4];
    Drawable* mDrawableStart, *mDrawableEnd, *mDrawableError, *mDrawableTemp;
    Drawable* mDrawableLeftInitial, *mDrawableRightInitial;
    bool mIsRtlCompatibilityMode;
    bool mOverride;
    bool mHasTint;
    cdroid::RefPtr<ColorStateList> mTintList;
    int mTintMode;
    int mDrawableSizeTop, mDrawableSizeBottom, mDrawableSizeLeft, mDrawableSizeRight;
    int mDrawableSizeStart, mDrawableSizeEnd, mDrawableSizeError, mDrawableSizeTemp;

    int mDrawableWidthTop, mDrawableWidthBottom, mDrawableHeightLeft, mDrawableHeightRight;
    int mDrawableHeightStart, mDrawableHeightEnd, mDrawableHeightError, mDrawableHeightTemp;
    int mDrawablePadding;
    int mDrawableSaved = DRAWABLE_NONE;
    Rect mCompoundRect;
private:
    void applyErrorDrawableIfNeeded(int layoutDirection);
public:
    Drawables(Context*ctx);
    ~Drawables();
    bool hasMetadata()const;
    bool resolveWithLayoutDirection(int layoutDirection);
    void setErrorDrawable(Drawable* dr, TextView* tv);
};
class TextView::Marquee {
private:
    static constexpr  float MARQUEE_DELTA_MAX = 0.07f;
    static constexpr  int MARQUEE_DELAY = 1200;
    static constexpr  int MARQUEE_DP_PER_SECOND = 30;

    static constexpr  int MARQUEE_STOPPED = 0x0;
    static constexpr  int MARQUEE_STARTING= 0x1;
    static constexpr  int MARQUEE_RUNNING = 0x2;
    friend TextView;
    TextView* mView;
    Choreographer*mChoreographer;
    int mStatus ;//= MARQUEE_STOPPED;
    float mPixelsPerMs;
    float mMaxScroll;
    float mMaxFadeScroll;
    float mGhostStart;
    float mGhostOffset;
    float mFadeStop;
    int mRepeatLimit;

    float mScroll;
    int64_t mLastAnimationMs;
    Choreographer::FrameCallback mTickCallback;
    Choreographer::FrameCallback mStartCallback;
    Choreographer::FrameCallback mRestartCallback;

private:
    void resetScroll();
public:
    Marquee(TextView* v);
    ~Marquee();
    void tick();
    void stop();
    void start(int repeatLimit);
    float getGhostOffset()const { return mGhostOffset; }
    float getScroll()const { return mScroll; }
    float getMaxFadeScroll()const { return mMaxFadeScroll; }
    bool shouldDrawLeftFade()const { return mScroll <= mFadeStop; }
    bool shouldDrawGhost()const { return (mStatus == MARQUEE_RUNNING) && (mScroll > mGhostStart); }
    bool isRunning()const{ return mStatus == MARQUEE_RUNNING; }
    bool isStopped()const{ return mStatus == MARQUEE_STOPPED; }
};

class TextView::CharWrapper:public CharSequence{//, GetChars, GraphicsOperations {
private:
    std::vector<char16_t> mChars;
    int mStart, mLength;
    friend TextView;
public:
    CharWrapper(const std::vector<char16_t>&chars, int start, int len);
    ~CharWrapper()override;
    void set(const std::vector<char16_t>& chars, int start, int len);
    size_t length() const override{
        return mLength;
    }
    int charAt(int off) const override;
    std::string toString() const override;
    std::u16string toU16String() const override;
    CharSequence* subSequence(int start, int end) const override;
    void getChars(int start, int end, char16_t* buf, int off) const override;
    void drawText(Canvas& c, int start, int end, float x, float y, Paint& p);
    void drawTextRun(Canvas& c, int start, int end,
            int contextStart, int contextEnd, float x, float y, bool isRtl, Paint& p);
    float measureText(int start, int end, Paint& p);
    int getTextWidths(int start, int end, float* widths, Paint& p);
    float getTextRunAdvances(int start, int end, int contextStart, int contextEnd,
            bool isRtl, float* advances, int advancesIndex,Paint& p);
    int getTextRunCursor(int contextStart, int contextEnd, bool isRtl,
            int offset, int cursorOpt, Paint& p);
};

class TextView::ChangeWatcher:virtual public TextWatcher,virtual public SpanWatcher {
private:
    CharSequence* mBeforeText;
    TextView*mTV;
public:
    ChangeWatcher(TextView*tv);
    // TextWatcher callbacks are wired by assigning the inherited std::function
    // members (beforeTextChanged/onTextChanged/afterTextChanged) in the ctor —
    // see textview.cc. (Defining same-named methods here would hide them.)
    void onSpanChanged(Spannable& buf,const ParcelableSpan* what, int s, int e, int st, int en)override;
    void onSpanAdded(Spannable& buf, const ParcelableSpan* what, int s, int e)override;
    void onSpanRemoved(Spannable& buf, const ParcelableSpan* what, int s, int e)override;
};

}  // namespace cdroid

#endif  //__TEXTVIEW_H__
