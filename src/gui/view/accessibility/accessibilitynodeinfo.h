#ifndef __ACCESSBILITY_NODEINFO_H__
#define __ACCESSBILITY_NODEINFO_H__
#include <string>
#include <vector>
#include <set>
#include <climits>
#include <core/rect.h>
#include <core/parcel.h>
#include <core/parcelable.h>
#include <view/accessibility/accessibilitynodeprovider.h>
#include <view/accessibility/accessibilitywindowinfo.h>
namespace cdroid{
using Bundle=void*;
class View;
class AccessibilityWindowInfo;
class AccessibilityNodeInfo{// implements Parcelable {
private:
    static constexpr bool Debug = false;
public:
    class RangeInfo;
    class CollectionInfo;
    class CollectionItemInfo;
    class AccessibilityAction;
public:
    static constexpr int VIRTUAL_DESCENDANT_ID_SHIFT = 32;
    static constexpr int UNDEFINED_CONNECTION_ID = -1;

    static constexpr int UNDEFINED_SELECTION_INDEX = -1;

    static constexpr int UNDEFINED_ITEM_ID = INT_MAX;

    static constexpr int ROOT_ITEM_ID = INT_MAX - 1;

    static constexpr int64_t UNDEFINED_NODE_ID = (int64_t(UNDEFINED_ITEM_ID)<<VIRTUAL_DESCENDANT_ID_SHIFT)|UNDEFINED_ITEM_ID;//makeNodeId(UNDEFINED_ITEM_ID, UNDEFINED_ITEM_ID);

    static constexpr int64_t ROOT_NODE_ID = (int64_t(ROOT_ITEM_ID)<<VIRTUAL_DESCENDANT_ID_SHIFT)|AccessibilityNodeProvider::HOST_VIEW_ID;//makeNodeId(ROOT_ITEM_ID,AccessibilityNodeProvider::HOST_VIEW_ID);

    static constexpr int FLAG_PREFETCH_PREDECESSORS = 0x00000001;

    static constexpr int FLAG_PREFETCH_SIBLINGS = 0x00000002;

    static constexpr int FLAG_PREFETCH_DESCENDANTS = 0x00000004;

    static constexpr int FLAG_INCLUDE_NOT_IMPORTANT_VIEWS = 0x00000008;

    static constexpr int FLAG_REPORT_VIEW_IDS = 0x00000010;

    static constexpr int ACTION_FOCUS =  0x00000001;

    static constexpr int ACTION_CLEAR_FOCUS = 0x00000002;

    static constexpr int ACTION_SELECT = 0x00000004;

    static constexpr int ACTION_CLEAR_SELECTION = 0x00000008;

    static constexpr int ACTION_CLICK = 0x00000010;

    static constexpr int ACTION_LONG_CLICK = 0x00000020;

    static constexpr int ACTION_ACCESSIBILITY_FOCUS = 0x00000040;

    static constexpr int ACTION_CLEAR_ACCESSIBILITY_FOCUS = 0x00000080;

    static constexpr int ACTION_NEXT_AT_MOVEMENT_GRANULARITY = 0x00000100;

    static constexpr int ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY = 0x00000200;

    static constexpr int ACTION_NEXT_HTML_ELEMENT = 0x00000400;

    static constexpr int ACTION_PREVIOUS_HTML_ELEMENT = 0x00000800;

    static constexpr int ACTION_SCROLL_FORWARD = 0x00001000;

    static constexpr int ACTION_SCROLL_BACKWARD = 0x00002000;

    static constexpr int ACTION_COPY = 0x00004000;

    static constexpr int ACTION_PASTE = 0x00008000;

    static constexpr int ACTION_CUT = 0x00010000;

    static constexpr int ACTION_SET_SELECTION = 0x00020000;

    static constexpr int ACTION_EXPAND = 0x00040000;

    static constexpr int ACTION_COLLAPSE = 0x00080000;

    static constexpr int ACTION_DISMISS = 0x00100000;

    static constexpr int ACTION_SET_TEXT = 0x00200000;

    static constexpr int LAST_LEGACY_STANDARD_ACTION = ACTION_SET_TEXT;

     static constexpr int ACTION_TYPE_MASK = 0xFF000000;

    static constexpr const char* ACTION_ARGUMENT_MOVEMENT_GRANULARITY_INT ="ACTION_ARGUMENT_MOVEMENT_GRANULARITY_INT";

    static constexpr const char* ACTION_ARGUMENT_HTML_ELEMENT_STRING = "ACTION_ARGUMENT_HTML_ELEMENT_STRING";

    static constexpr const char* ACTION_ARGUMENT_EXTEND_SELECTION_BOOLEAN = "ACTION_ARGUMENT_EXTEND_SELECTION_BOOLEAN";

    static constexpr const char* ACTION_ARGUMENT_SELECTION_START_INT = "ACTION_ARGUMENT_SELECTION_START_INT";

    static constexpr const char* ACTION_ARGUMENT_SELECTION_END_INT = "ACTION_ARGUMENT_SELECTION_END_INT";

    static constexpr const char* ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE = "ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE";

    static constexpr const char* ACTION_ARGUMENT_ROW_INT = "android.view.accessibility.action.ARGUMENT_ROW_INT";

    static constexpr const char* ACTION_ARGUMENT_COLUMN_INT = "android.view.accessibility.action.ARGUMENT_COLUMN_INT";

    static constexpr const char* ACTION_ARGUMENT_PROGRESS_VALUE = "android.view.accessibility.action.ARGUMENT_PROGRESS_VALUE";

    static constexpr const char* ACTION_ARGUMENT_MOVE_WINDOW_X = "ACTION_ARGUMENT_MOVE_WINDOW_X";

    static constexpr const char* ACTION_ARGUMENT_MOVE_WINDOW_Y = "ACTION_ARGUMENT_MOVE_WINDOW_Y";

    static constexpr const char* ACTION_ARGUMENT_ACCESSIBLE_CLICKABLE_SPAN = "android.view.accessibility.action.ACTION_ARGUMENT_ACCESSIBLE_CLICKABLE_SPAN";

    static constexpr int FOCUS_INPUT = 1;

    static constexpr int FOCUS_ACCESSIBILITY = 2;

    // Movement granularities

    static constexpr int MOVEMENT_GRANULARITY_CHARACTER = 0x00000001;

    static constexpr int MOVEMENT_GRANULARITY_WORD = 0x00000002;

    static constexpr int MOVEMENT_GRANULARITY_LINE = 0x00000004;

    static constexpr int MOVEMENT_GRANULARITY_PARAGRAPH = 0x00000008;

    static constexpr int MOVEMENT_GRANULARITY_PAGE = 0x00000010;

    static constexpr const char* EXTRA_DATA_TEXT_CHARACTER_LOCATION_KEY = "android.view.accessibility.extra.DATA_TEXT_CHARACTER_LOCATION_KEY";

    static constexpr const char* EXTRA_DATA_TEXT_CHARACTER_LOCATION_ARG_START_INDEX = "android.view.accessibility.extra.DATA_TEXT_CHARACTER_LOCATION_ARG_START_INDEX";

    static constexpr const char* EXTRA_DATA_TEXT_CHARACTER_LOCATION_ARG_LENGTH = "android.view.accessibility.extra.DATA_TEXT_CHARACTER_LOCATION_ARG_LENGTH";

    static constexpr const char* EXTRA_DATA_REQUESTED_KEY = "android.view.accessibility.AccessibilityNodeInfo.extra_data_requested";
private:
    // Boolean attributes.
    static constexpr int BOOLEAN_PROPERTY_CHECKABLE = 0x00000001;

    static constexpr int BOOLEAN_PROPERTY_CHECKED = 0x00000002;

    static constexpr int BOOLEAN_PROPERTY_FOCUSABLE = 0x00000004;

    static constexpr int BOOLEAN_PROPERTY_FOCUSED = 0x00000008;

    static constexpr int BOOLEAN_PROPERTY_SELECTED = 0x00000010;

    static constexpr int BOOLEAN_PROPERTY_CLICKABLE = 0x00000020;

    static constexpr int BOOLEAN_PROPERTY_LONG_CLICKABLE = 0x00000040;

    static constexpr int BOOLEAN_PROPERTY_ENABLED = 0x00000080;

    static constexpr int BOOLEAN_PROPERTY_PASSWORD = 0x00000100;

    static constexpr int BOOLEAN_PROPERTY_SCROLLABLE = 0x00000200;

    static constexpr int BOOLEAN_PROPERTY_ACCESSIBILITY_FOCUSED = 0x00000400;

    static constexpr int BOOLEAN_PROPERTY_VISIBLE_TO_USER = 0x00000800;

    static constexpr int BOOLEAN_PROPERTY_EDITABLE = 0x00001000;

    static constexpr int BOOLEAN_PROPERTY_OPENS_POPUP = 0x00002000;

    static constexpr int BOOLEAN_PROPERTY_DISMISSABLE = 0x00004000;

    static constexpr int BOOLEAN_PROPERTY_MULTI_LINE = 0x00008000;

    static constexpr int BOOLEAN_PROPERTY_CONTENT_INVALID = 0x00010000;

    static constexpr int BOOLEAN_PROPERTY_CONTEXT_CLICKABLE = 0x00020000;

    static constexpr int BOOLEAN_PROPERTY_IMPORTANCE = 0x0040000;

    static constexpr int BOOLEAN_PROPERTY_SCREEN_READER_FOCUSABLE = 0x0080000;

    static constexpr int BOOLEAN_PROPERTY_IS_SHOWING_HINT = 0x0100000;

    static constexpr int BOOLEAN_PROPERTY_IS_HEADING = 0x0200000;

    static constexpr int64_t VIRTUAL_DESCENDANT_ID_MASK = 0xffffffff00000000L;
    
    static int sNumInstancesInUse;
private:
    static constexpr int MAX_POOL_SIZE = 50;
    static Pools::SimplePool<AccessibilityNodeInfo> sPool;

    static const AccessibilityNodeInfo DEFAULT;// = new AccessibilityNodeInfo();

    bool mSealed;

    // Data.
    int mWindowId;// = AccessibilityWindowInfo::UNDEFINED_WINDOW_ID;
    int64_t mSourceNodeId = UNDEFINED_NODE_ID;
    int64_t mParentNodeId = UNDEFINED_NODE_ID;
    int64_t mLabelForId = UNDEFINED_NODE_ID;
    int64_t mLabeledById = UNDEFINED_NODE_ID;
    int64_t mTraversalBefore = UNDEFINED_NODE_ID;
    int64_t mTraversalAfter = UNDEFINED_NODE_ID;

    int mBooleanProperties;
    Rect mBoundsInParent;
    Rect mBoundsInScreen;
    int mDrawingOrderInParent;

    std::string mPackageName;
    std::string mClassName;
    // Hidden, unparceled value used to hold the original value passed to setText
    std::string mOriginalText;
    std::string mText;
    std::string mHintText;
    std::string mError;
    std::string mPaneTitle;
    std::string mContentDescription;
    std::string mTooltipText;
    std::string mViewIdResourceName;
    std::vector<std::string> mExtraDataKeys;

    std::vector<long> mChildNodeIds;
    std::vector<AccessibilityAction*> mActions;

    int mMaxTextLength = -1;
    int mMovementGranularities;

    int mTextSelectionStart = UNDEFINED_SELECTION_INDEX;
    int mTextSelectionEnd = UNDEFINED_SELECTION_INDEX;
    int mInputType;// = InputType::TYPE_NULL;
    int mLiveRegion;// = View::ACCESSIBILITY_LIVE_REGION_NONE;
    int mConnectionId = UNDEFINED_CONNECTION_ID;

    Bundle mExtras;

    RangeInfo *mRangeInfo;
    CollectionInfo* mCollectionInfo;
    CollectionItemInfo* mCollectionItemInfo;
private:
    AccessibilityNodeInfo();
    void addChildInternal(View* root, int virtualDescendantId, bool checked);
    void addActionUnchecked(AccessibilityAction* action);
    bool getBooleanProperty(int property) const;

    void setBooleanProperty(int property, bool value);
    void enforceValidFocusDirection(int direction);
    void enforceValidFocusType(int focusType);

    void init(const AccessibilityNodeInfo& other);

    void initFromParcel(Parcel parcel);

    void clear();

    static bool isDefaultStandardAction(const AccessibilityAction* action);

    static AccessibilityAction* getActionSingleton(int actionId);

    static AccessibilityAction* getActionSingletonBySerializationFlag(long flag);

    void addStandardActions(long serializationIdMask);

    static std::string getActionSymbolicName(int action);

    static std::string getMovementGranularitySymbolicName(int granularity);

    bool canPerformRequestOverConnection(long accessibilityNodeId);
    AccessibilityNodeInfo* getNodeForAccessibilityId(long accessibilityId);
    static std::string idItemToString(int item);

protected:
    void enforceSealed();
    void enforceNotSealed();
public:
    static int getAccessibilityViewId(long accessibilityNodeId);
    static int getVirtualDescendantId(long accessibilityNodeId);
    static int64_t makeNodeId(int accessibilityViewId, int virtualDescendantId);

    void setSource(View* source);

    void setSource(View* root, int virtualDescendantId);

    AccessibilityNodeInfo* findFocus(int focus);

    AccessibilityNodeInfo* focusSearch(int direction);

    int getWindowId() const;

    bool refresh(Bundle arguments, bool bypassCache);

    bool refresh();

    bool refreshWithExtraData(const std::string& extraDataKey, Bundle args);

    std::vector<long> getChildNodeIds() const;

    long getChildId(int index)const;

    int getChildCount() const;

    AccessibilityNodeInfo* getChild(int index);

    void addChild(View* child);

    void addChildUnchecked(View* child);

    bool removeChild(View* child);

    void addChild(View* root, int virtualDescendantId);

    bool removeChild(View* root, int virtualDescendantId);

    std::vector<AccessibilityAction*> getActionList();

    int getActions();

    void addAction(AccessibilityAction* action);

    void addAction(int action);

    void removeAction(int action);

    bool removeAction(AccessibilityAction* action);

    void removeAllActions();

    AccessibilityNodeInfo* getTraversalBefore();

    void setTraversalBefore(View* view);

    void setTraversalBefore(View* root, int virtualDescendantId);

    AccessibilityNodeInfo* getTraversalAfter();

    void setTraversalAfter(View* view);

    void setTraversalAfter(View* root, int virtualDescendantId);

    std::vector<std::string> getAvailableExtraData();

    void setAvailableExtraData(const std::vector<std::string>& extraDataKeys);

    void setMaxTextLength(int max);

    int getMaxTextLength() const;

    void setMovementGranularities(int granularities);

    int getMovementGranularities()const;

    bool performAction(int action);

    bool performAction(int action, Bundle arguments);

    std::vector<AccessibilityNodeInfo*> findAccessibilityNodeInfosByText(const std::string& text);

    std::vector<AccessibilityNodeInfo*> findAccessibilityNodeInfosByViewId(const std::string& viewId);

    AccessibilityWindowInfo* getWindow();

    AccessibilityNodeInfo* getParent();

    long getParentNodeId() const;

    void setParent(View* parent);

    void setParent(View* root, int virtualDescendantId);

    void getBoundsInParent(Rect& outBounds)const;

    void setBoundsInParent(const Rect& bounds);

    void getBoundsInScreen(Rect& outBounds)const;

    Rect getBoundsInScreen() const;

    void setBoundsInScreen(const Rect& bounds);

    bool isCheckable() const;

    void setCheckable(bool checkable);

    bool isChecked() const;

    void setChecked(bool checked);

    bool isFocusable() const;

    void setFocusable(bool focusable);

    bool isFocused() const;

    void setFocused(bool focused);

    bool isVisibleToUser() const;

    void setVisibleToUser(bool visibleToUser);

    bool isAccessibilityFocused() const;

    void setAccessibilityFocused(bool focused);

    bool isSelected() const;

    void setSelected(bool selected);

    bool isClickable() const;

    void setClickable(bool clickable);

    bool isLongClickable() const;

    void setLongClickable(bool longClickable);

    bool isEnabled() const;

    void setEnabled(bool enabled);

    bool isPassword() const;

    void setPassword(bool password);

    bool isScrollable() const;

    void setScrollable(bool scrollable);

    bool isEditable() const;

    void setEditable(bool editable);

    void setPaneTitle(const std::string& paneTitle);

    std::string getPaneTitle() const;

    int getDrawingOrder() const;

    void setDrawingOrder(int drawingOrderInParent);

    CollectionInfo* getCollectionInfo() const;

    void setCollectionInfo(CollectionInfo* collectionInfo);

    CollectionItemInfo* getCollectionItemInfo() const;

    void setCollectionItemInfo(CollectionItemInfo* collectionItemInfo);

    RangeInfo* getRangeInfo() const;

    void setRangeInfo(RangeInfo* rangeInfo);

    bool isContentInvalid()const;

    void setContentInvalid(bool contentInvalid);

    bool isContextClickable()const;

    void setContextClickable(bool contextClickable);

    int getLiveRegion()const;

    void setLiveRegion(int mode);

    bool isMultiLine()const;

    void setMultiLine(bool multiLine);

    bool canOpenPopup()const;

    void setCanOpenPopup(bool opensPopup);

    bool isDismissable()const;

    void setDismissable(bool dismissable);

    bool isImportantForAccessibility()const;

    void setImportantForAccessibility(bool important);

    bool isScreenReaderFocusable()const;

    void setScreenReaderFocusable(bool screenReaderFocusable);

    bool isShowingHintText()const;

    void setShowingHintText(bool showingHintText);

    bool isHeading()const;

    void setHeading(bool isHeading);

    std::string getPackageName()const;

    void setPackageName(const std::string& packageName);

    std::string getClassName()const;

    void setClassName(const std::string& className);

    std::string getText()const;

    std::string getOriginalText() const;

    void setText(const std::string& text);

    std::string getHintText()const;

    void setHintText(const std::string& hintText);

    void setError(const std::string& error);

    std::string getError()const;

    std::string getContentDescription()const;

    void setContentDescription(const std::string& contentDescription);

    std::string getTooltipText() const;

    void setTooltipText(const std::string& tooltipText);

    void setLabelFor(View* labeled);

    void setLabelFor(View* root, int virtualDescendantId);

    AccessibilityNodeInfo* getLabelFor();

    void setLabeledBy(View* label);

    void setLabeledBy(View* root, int virtualDescendantId);

    AccessibilityNodeInfo* getLabeledBy();

    void setViewIdResourceName(const std::string& viewIdResName);

    std::string getViewIdResourceName()const;

    int getTextSelectionStart() const;

    int getTextSelectionEnd() const;

    void setTextSelection(int start, int end);

    int getInputType() const;

    void setInputType(int inputType);

    Bundle getExtras();

    bool hasExtras() const;

    void setConnectionId(int connectionId);

    int getConnectionId()const;

    int describeContents();

    void setSourceNodeId(long sourceId, int windowId);

    long getSourceNodeId()const;

    void setSealed(bool sealed);

    bool isSealed() const;

    static AccessibilityNodeInfo* obtain(View* source);

    static AccessibilityNodeInfo* obtain(View* root, int virtualDescendantId);

    static AccessibilityNodeInfo* obtain();

    static AccessibilityNodeInfo* obtain(const AccessibilityNodeInfo& info);

    void recycle();

    static void setNumInstancesInUseCounter(int counter);

    void writeToParcel(Parcel parcel, int flags);

    void writeToParcelNoRecycle(Parcel parcel, int flags);
    
    bool operator==(const AccessibilityNodeInfo*other) const;

    int hashCode();

    std::string toString();

    static std::string idToString(long accessibilityId);

};/*endof AccessibilityNodeInfo*/
  
class AccessibilityNodeInfo::AccessibilityAction {
public:
    static std::set<AccessibilityAction*> sStandardActions;

    static const AccessibilityAction ACTION_FOCUS;

    static const AccessibilityAction ACTION_CLEAR_FOCUS;

    static const AccessibilityAction ACTION_SELECT;

    static const AccessibilityAction ACTION_CLEAR_SELECTION;

    static const AccessibilityAction ACTION_CLICK;

    static const AccessibilityAction ACTION_LONG_CLICK;

    static const AccessibilityAction ACTION_ACCESSIBILITY_FOCUS;

    static const AccessibilityAction ACTION_CLEAR_ACCESSIBILITY_FOCUS;

    static const AccessibilityAction ACTION_NEXT_AT_MOVEMENT_GRANULARITY;

    static const AccessibilityAction ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY;

    static const AccessibilityAction ACTION_NEXT_HTML_ELEMENT;

    static const AccessibilityAction ACTION_PREVIOUS_HTML_ELEMENT;

    static const AccessibilityAction ACTION_SCROLL_FORWARD;

    static const AccessibilityAction ACTION_SCROLL_BACKWARD;

    static const AccessibilityAction ACTION_COPY;

    static const AccessibilityAction ACTION_PASTE;

    static const AccessibilityAction ACTION_CUT;

    static const AccessibilityAction ACTION_SET_SELECTION;

    static const AccessibilityAction ACTION_EXPAND;

    static const AccessibilityAction ACTION_COLLAPSE;

    static const AccessibilityAction ACTION_DISMISS;

    static const AccessibilityAction ACTION_SET_TEXT;

    static const AccessibilityAction ACTION_SHOW_ON_SCREEN;

    static const AccessibilityAction ACTION_SCROLL_TO_POSITION;

    static const AccessibilityAction ACTION_SCROLL_UP;

    static const AccessibilityAction ACTION_SCROLL_LEFT;

    static const AccessibilityAction ACTION_SCROLL_DOWN;

    static const AccessibilityAction ACTION_SCROLL_RIGHT;

    static const AccessibilityAction ACTION_CONTEXT_CLICK;

    static const AccessibilityAction ACTION_SET_PROGRESS;

    static const AccessibilityAction ACTION_MOVE_WINDOW;

    static const AccessibilityAction ACTION_SHOW_TOOLTIP;

    static const AccessibilityAction ACTION_HIDE_TOOLTIP;

private:
    int mActionId;
    std::string mLabel;
    AccessibilityAction(int standardActionId);
public:
    long mSerializationFlag = -1L;

    AccessibilityAction(int actionId,const std::string& label);
    int getId() const;

    std::string getLabel() const;

    int hashCode() const;
    bool operator==(const AccessibilityAction*other)const;
    std::string toString()const;
};

class AccessibilityNodeInfo::RangeInfo {
private:
    static constexpr int MAX_POOL_SIZE = 10;
public:
    static constexpr int RANGE_TYPE_INT = 0;
    static constexpr int RANGE_TYPE_FLOAT = 1;
    static constexpr int RANGE_TYPE_PERCENT = 2;

private:
    static Pools::SimplePool<RangeInfo> sPool;

    int mType;
    float mMin;
    float mMax;
    float mCurrent;
private:
    RangeInfo(int type, float min, float max, float current);
    void clear();
public:
    static RangeInfo* obtain(const RangeInfo& other);
    static RangeInfo* obtain(int type, float min, float max, float current);
    int getType() const;

    float getMin() const;

    float getMax() const;

    float getCurrent()const;

    void recycle();
    bool operator==(const RangeInfo*)const;
};

class AccessibilityNodeInfo::CollectionInfo {
private
    :static constexpr int MAX_POOL_SIZE = 20;
public:
    /** Selection mode where items are not selectable. */
    static constexpr int SELECTION_MODE_NONE = 0;

    /** Selection mode where a single item may be selected. */
    static constexpr int SELECTION_MODE_SINGLE = 1;

    /** Selection mode where multiple items may be selected. */
    static constexpr int SELECTION_MODE_MULTIPLE = 2;

private:
    static Pools::SimplePool<CollectionInfo> sPool;

    int mRowCount;
    int mColumnCount;
    bool mHierarchical;
    int mSelectionMode;
private:
    CollectionInfo(int rowCount, int columnCount, bool hierarchical,int selectionMode);
    void clear();
public:
    static CollectionInfo* obtain(const CollectionInfo& other);

    static CollectionInfo* obtain(int rowCount, int columnCount,bool hierarchical);

    static CollectionInfo* obtain(int rowCount, int columnCount,bool hierarchical, int selectionMode);

    int getRowCount() const;

    int getColumnCount()const;

    bool isHierarchical()const;

    int getSelectionMode()const;

    void recycle();
};

class AccessibilityNodeInfo::CollectionItemInfo {
private:
    friend AccessibilityNodeInfo;
    static constexpr int MAX_POOL_SIZE = 20;
    static Pools::SimplePool<CollectionItemInfo> sPool;
    bool mHeading;
    int mColumnIndex;
    int mRowIndex;
    int mColumnSpan;
    int mRowSpan;
    bool mSelected;
private:
    CollectionItemInfo(int rowIndex, int rowSpan, int columnIndex, int columnSpan,
            bool heading, bool selected);
    void clear();
public:
    static CollectionItemInfo* obtain(const CollectionItemInfo& other);
    static CollectionItemInfo* obtain(int rowIndex, int rowSpan,
            int columnIndex, int columnSpan, bool heading);

    static CollectionItemInfo* obtain(int rowIndex, int rowSpan,
            int columnIndex, int columnSpan, bool heading, bool selected);
    int getColumnIndex() const;

    int getRowIndex()const;

    int getColumnSpan()const;

    int getRowSpan() const;

    bool isHeading()const;

    bool isSelected()const;

    void recycle();
};
}/*endof namespace*/
#endif/*__ACCESSBILITY_NODEINFO_H__*/
