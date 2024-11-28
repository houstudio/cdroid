#ifndef __ACCESSIBILITY_EVENT_H__
#define __ACCESSIBILITY_EVENT_H__
#include <view/accessibility/accessibilityrecord.h>
namespace cdroid{
class AccessibilityEvent:public AccessibilityRecord{
private:
    static constexpr bool Debug = false;
    static constexpr int MAX_POOL_SIZE = 10;
public:
    static constexpr bool DEBUG_ORIGIN = false;

    /**
     * Invalid selection/focus position.
     *
     * @see #getCurrentItemIndex()
     */
    static constexpr int INVALID_POSITION = -1;

    /**
     * Maximum length of the text fields.
     *
     * @see #getBeforeText()
     * @see #getText()
     * </br>
     * Note: This constant is no longer needed since there
     *       is no limit on the length of text that is contained
     *       in an accessibility event anymore.
     */
    static constexpr int MAX_TEXT_LENGTH = 500;

    static constexpr int TYPES_ALL_MASK = 0xFFFFFFFF;
    /**
     * Represents the event of clicking on a {@link android.view.View} like
     * {@link android.widget.Button}, {@link android.widget.CompoundButton}, etc.
     */
    static constexpr int TYPE_VIEW_CLICKED = 0x00000001;

    /**
     * Represents the event of long clicking on a {@link android.view.View} like
     * {@link android.widget.Button}, {@link android.widget.CompoundButton}, etc.
     */
    static constexpr int TYPE_VIEW_LONG_CLICKED = 0x00000002;

    /**
     * Represents the event of selecting an item usually in the context of an
     * {@link android.widget.AdapterView}.
     */
    static constexpr int TYPE_VIEW_SELECTED = 0x00000004;

    /**
     * Represents the event of setting input focus of a {@link android.view.View}.
     */
    static constexpr int TYPE_VIEW_FOCUSED = 0x00000008;

    /**
     * Represents the event of changing the text of an {@link android.widget.EditText}.
     */
    static constexpr int TYPE_VIEW_TEXT_CHANGED = 0x00000010;

    /**
     * Represents the event of a change to a visually distinct section of the user interface.
     * These events should only be dispatched from {@link android.view.View}s that have
     * accessibility pane titles, and replaces {@link #TYPE_WINDOW_CONTENT_CHANGED} for those
     * sources. Details about the change are available from {@link #getContentChangeTypes()}.
     */
    static constexpr int TYPE_WINDOW_STATE_CHANGED = 0x00000020;

    /**
     * Represents the event showing a {@link android.app.Notification}.
     */
    static constexpr int TYPE_NOTIFICATION_STATE_CHANGED = 0x00000040;

    /**
     * Represents the event of a hover enter over a {@link android.view.View}.
     */
    static constexpr int TYPE_VIEW_HOVER_ENTER = 0x00000080;

    /**
     * Represents the event of a hover exit over a {@link android.view.View}.
     */
    static constexpr int TYPE_VIEW_HOVER_EXIT = 0x00000100;

    /**
     * Represents the event of starting a touch exploration gesture.
     */
    static constexpr int TYPE_TOUCH_EXPLORATION_GESTURE_START = 0x00000200;

    /**
     * Represents the event of ending a touch exploration gesture.
     */
    static constexpr int TYPE_TOUCH_EXPLORATION_GESTURE_END = 0x00000400;

    /**
     * Represents the event of changing the content of a window and more
     * specifically the sub-tree rooted at the event's source.
     */
    static constexpr int TYPE_WINDOW_CONTENT_CHANGED = 0x00000800;

    /**
     * Represents the event of scrolling a view. This event type is generally not sent directly.
     * @see View#onScrollChanged(int, int, int, int)
     */
    static constexpr int TYPE_VIEW_SCROLLED = 0x00001000;

    /**
     * Represents the event of changing the selection in an {@link android.widget.EditText}.
     */
    static constexpr int TYPE_VIEW_TEXT_SELECTION_CHANGED = 0x00002000;

    /**
     * Represents the event of an application making an announcement.
     */
    static constexpr int TYPE_ANNOUNCEMENT = 0x00004000;

    /**
     * Represents the event of gaining accessibility focus.
     */
    static constexpr int TYPE_VIEW_ACCESSIBILITY_FOCUSED = 0x00008000;

    /**
     * Represents the event of clearing accessibility focus.
     */
    static constexpr int TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED = 0x00010000;

    /**
     * Represents the event of traversing the text of a view at a given movement granularity.
     */
    static constexpr int TYPE_VIEW_TEXT_TRAVERSED_AT_MOVEMENT_GRANULARITY = 0x00020000;

    /**
     * Represents the event of beginning gesture detection.
     */
    static constexpr int TYPE_GESTURE_DETECTION_START = 0x00040000;

    /**
     * Represents the event of ending gesture detection.
     */
    static constexpr int TYPE_GESTURE_DETECTION_END = 0x00080000;

    /**
     * Represents the event of the user starting to touch the screen.
     */
    static constexpr int TYPE_TOUCH_INTERACTION_START = 0x00100000;

    /**
     * Represents the event of the user ending to touch the screen.
     */
    static constexpr int TYPE_TOUCH_INTERACTION_END = 0x00200000;

    /**
     * Represents the event change in the system windows shown on the screen. This event type should
     * only be dispatched by the system.
     */
    static constexpr int TYPE_WINDOWS_CHANGED = 0x00400000;

    /**
     * Represents the event of a context click on a {@link android.view.View}.
     */
    static constexpr int TYPE_VIEW_CONTEXT_CLICKED = 0x00800000;

    /**
     * Represents the event of the assistant currently reading the users screen context.
     */
    static constexpr int TYPE_ASSIST_READING_CONTEXT = 0x01000000;

    /**
     * Change type for {@link #TYPE_WINDOW_CONTENT_CHANGED} event:
     * The type of change is not defined.
     */
    static constexpr int CONTENT_CHANGE_TYPE_UNDEFINED = 0x00000000;

    /**
     * Change type for {@link #TYPE_WINDOW_CONTENT_CHANGED} event:
     * One or more content changes occurred in the the subtree rooted at the source node,
     * or the subtree's structure changed when a node was added or removed.
     */
    static constexpr int CONTENT_CHANGE_TYPE_SUBTREE = 0x00000001;

    /**
     * Change type for {@link #TYPE_WINDOW_CONTENT_CHANGED} event:
     * The node's text changed.
     */
    static constexpr int CONTENT_CHANGE_TYPE_TEXT = 0x00000002;

    /**
     * Change type for {@link #TYPE_WINDOW_CONTENT_CHANGED} event:
     * The node's content description changed.
     */
    static constexpr int CONTENT_CHANGE_TYPE_CONTENT_DESCRIPTION = 0x00000004;

    /**
     * Change type for {@link #TYPE_WINDOW_STATE_CHANGED} event:
     * The node's pane title changed.
     */
    static constexpr int CONTENT_CHANGE_TYPE_PANE_TITLE = 0x00000008;

    /**
     * Change type for {@link #TYPE_WINDOW_STATE_CHANGED} event:
     * The node has a pane title, and either just appeared or just was assigned a title when it
     * had none before.
     */
    static constexpr int CONTENT_CHANGE_TYPE_PANE_APPEARED = 0x00000010;

    /**
     * Change type for {@link #TYPE_WINDOW_STATE_CHANGED} event:
     * Can mean one of two slightly different things. The primary meaning is that the node has
     * a pane title, and was removed from the node hierarchy. It will also be sent if the pane
     * title is set to {@code null} after it contained a title.
     * No source will be returned if the node is no longer on the screen. To make the change more
     * clear for the user, the first entry in {@link #getText()} will return the value that would
     * have been returned by {@code getSource().getPaneTitle()}.
     */
    static constexpr int CONTENT_CHANGE_TYPE_PANE_DISAPPEARED = 0x00000020;

    /**
     * Change type for {@link #TYPE_WINDOWS_CHANGED} event:
     * The window was added.
     */
    static constexpr int WINDOWS_CHANGE_ADDED = 0x00000001;

    /**
     * Change type for {@link #TYPE_WINDOWS_CHANGED} event:
     * A window was removed.
     */
    static constexpr int WINDOWS_CHANGE_REMOVED = 0x00000002;

    /**
     * Change type for {@link #TYPE_WINDOWS_CHANGED} event:
     * The window's title changed.
     */
    static constexpr int WINDOWS_CHANGE_TITLE = 0x00000004;

    /**
     * Change type for {@link #TYPE_WINDOWS_CHANGED} event:
     * The window's bounds changed.
     */
    static constexpr int WINDOWS_CHANGE_BOUNDS = 0x00000008;

    /**
     * Change type for {@link #TYPE_WINDOWS_CHANGED} event:
     * The window's layer changed.
     */
    static constexpr int WINDOWS_CHANGE_LAYER = 0x00000010;

    /**
     * Change type for {@link #TYPE_WINDOWS_CHANGED} event:
     * The window's {@link AccessibilityWindowInfo#isActive()} changed.
     */
    static constexpr int WINDOWS_CHANGE_ACTIVE = 0x00000020;

    /**
     * Change type for {@link #TYPE_WINDOWS_CHANGED} event:
     * The window's {@link AccessibilityWindowInfo#isFocused()} changed.
     */
    static constexpr int WINDOWS_CHANGE_FOCUSED = 0x00000040;

    /**
     * Change type for {@link #TYPE_WINDOWS_CHANGED} event:
     * The window's {@link AccessibilityWindowInfo#isAccessibilityFocused()} changed.
     */
    static constexpr int WINDOWS_CHANGE_ACCESSIBILITY_FOCUSED = 0x00000080;

    /**
     * Change type for {@link #TYPE_WINDOWS_CHANGED} event:
     * The window's parent changed.
     */
    static constexpr int WINDOWS_CHANGE_PARENT = 0x00000100;

    /**
     * Change type for {@link #TYPE_WINDOWS_CHANGED} event:
     * The window's children changed.
     */
    static constexpr int WINDOWS_CHANGE_CHILDREN = 0x00000200;

    /**
     * Change type for {@link #TYPE_WINDOWS_CHANGED} event:
     * The window either entered or exited picture-in-picture mode.
     */
    static constexpr int WINDOWS_CHANGE_PIP = 0x00000400;
private:
    std::string mPackageName;
    int64_t mEventTime;
    std::vector<AccessibilityRecord*> mRecords;
    static Pools::SimplePool<AccessibilityEvent>sPool;
protected:
    int mEventType;
    int mMovementGranularity;
    int mAction;
    int mContentChangeTypes;
    int mWindowChangeTypes;
private:
    AccessibilityEvent();
    static std::string contentChangeTypesToString(int types);
    static std::string singleContentChangeTypeToString(int type);
    static std::string windowChangeTypesToString(int types);
    static std::string singleWindowChangeTypeToString(int type);
    static std::string singleEventTypeToString(int eventType);
protected:
    void init(const AccessibilityEvent&);
    void clear()override;
public:
    void setSealed(bool);
    size_t getRecordCount()const;
    void appendRecord(AccessibilityRecord*);
    AccessibilityRecord* getRecord(int);
    int getEventType()const;
    int getContentChangeTypes()const;
    void setContentChangeTypes(int changeTypes);
    int getWindowChanges()const;
    void setWindowChanges(int changes);
    void setEventType(int eventType);
    int64_t getEventTime()const;
    void setEventTime(int64_t eventTime);
    std::string getPackageName()const;
    void setPackageName(const std::string&);
    void setMovementGranularity(int granularity);
    int getMovementGranularity()const;
    void setAction(int action);
    int getAction()const;
    static AccessibilityEvent* obtainWindowsChangedEvent(int windowId, int windowChangeTypes);
    static AccessibilityEvent* obtain(int eventType);
    static AccessibilityEvent* obtain(const AccessibilityEvent& event);
    static AccessibilityEvent* obtain();
    void recycle()override;
    std::string toString();
    static std::string eventTypeToString(int eventType);
};
}
#endif
