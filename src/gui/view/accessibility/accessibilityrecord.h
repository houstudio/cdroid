#ifndef __ACCESSIBILITY_RECORD_H__
#define __ACCESSIBILITY_RECORD_H__
#include <string>
#include <vector>
#include <view/accessibility/accessibilitynodeinfo.h>
#include <view/accessibility/accessibilitywindowinfo.h>
namespace cdroid{
class AccessibilityRecord {
protected:
    static constexpr bool DEBUG_CONCISE_TOSTRING = false;
private:
    static constexpr int UNDEFINED = -1;

    static constexpr int PROPERTY_CHECKED = 0x00000001;
    static constexpr int PROPERTY_ENABLED = 0x00000002;
    static constexpr int PROPERTY_PASSWORD = 0x00000004;
    static constexpr int PROPERTY_FULL_SCREEN = 0x00000080;
    static constexpr int PROPERTY_SCROLLABLE = 0x00000100;
    static constexpr int PROPERTY_IMPORTANT_FOR_ACCESSIBILITY = 0x00000200;

    static constexpr int GET_SOURCE_PREFETCH_FLAGS =AccessibilityNodeInfo::FLAG_PREFETCH_PREDECESSORS
            | AccessibilityNodeInfo::FLAG_PREFETCH_SIBLINGS | AccessibilityNodeInfo::FLAG_PREFETCH_DESCENDANTS;

    // Housekeeping
    static constexpr int MAX_POOL_SIZE = 10;
    //static final Object sPoolLock = new Object();
    static AccessibilityRecord* sPool;
    static int sPoolSize;
    AccessibilityRecord* mNext;
    bool mIsInPool;
protected:
    bool mSealed;
    int mBooleanProperties = 0;
    int mCurrentItemIndex = UNDEFINED;
    int mItemCount = UNDEFINED;
    int mFromIndex = UNDEFINED;
    int mToIndex = UNDEFINED;
    int mScrollX = UNDEFINED;
    int mScrollY = UNDEFINED;

    int mScrollDeltaX = UNDEFINED;
    int mScrollDeltaY = UNDEFINED;
    int mMaxScrollX = UNDEFINED;
    int mMaxScrollY = UNDEFINED;

    int mAddedCount= UNDEFINED;
    int mRemovedCount = UNDEFINED;
    int mSourceWindowId = AccessibilityWindowInfo::UNDEFINED_WINDOW_ID;
    int64_t mSourceNodeId = AccessibilityNodeInfo::UNDEFINED_NODE_ID;

    std::string mClassName;
    std::string mContentDescription;
    std::string mBeforeText;
    Parcelable* mParcelableData;

    std::vector<std::string> mText;

    int mConnectionId = UNDEFINED;
protected:
    virtual void clear(); 
public:
    /*
     * Hide constructor.
     */
    AccessibilityRecord();

    /**
     * Sets the event source.
     *
     * @param source The source.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setSource(View* source);

    /**
     * Sets the source to be a virtual descendant of the given <code>root</code>.
     * If <code>virtualDescendantId</code> equals to {@link View#NO_ID} the root
     * is set as the source.
     * <p>
     * A virtual descendant is an imaginary View that is reported as a part of the view
     * hierarchy for accessibility purposes. This enables custom views that draw complex
     * content to report them selves as a tree of virtual views, thus conveying their
     * logical structure.
     * </p>
     *
     * @param root The root of the virtual subtree.
     * @param virtualDescendantId The id of the virtual descendant.
     */
    void setSource(View* root, int virtualDescendantId);

    /**
     * Set the source node ID directly
     *
     * @param sourceNodeId The source node Id
     * @hide
     */
    void setSourceNodeId(long sourceNodeId);

    /**
     * Gets the {@link AccessibilityNodeInfo} of the event source.
     * <p>
     *   <strong>Note:</strong> It is a client responsibility to recycle the received info
     *   by calling {@link AccessibilityNodeInfo#recycle() AccessibilityNodeInfo#recycle()}
     *   to avoid creating of multiple instances.
     * </p>
     * @return The info of the source.
     */
    AccessibilityNodeInfo* getSource();

    /**
     * Sets the window id.
     *
     * @param windowId The window id.
     *
     * @hide
     */
    void setWindowId(int windowId);

    /**
     * Gets the id of the window from which the event comes from.
     *
     * @return The window id.
     */
    int getWindowId();

    /**
     * Gets if the source is checked.
     *
     * @return True if the view is checked, false otherwise.
     */
    bool isChecked();

    /**
     * Sets if the source is checked.
     *
     * @param isChecked True if the view is checked, false otherwise.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setChecked(bool isChecked);

    /**
     * Gets if the source is enabled.
     *
     * @return True if the view is enabled, false otherwise.
     */
    bool isEnabled();

    /**
     * Sets if the source is enabled.
     *
     * @param isEnabled True if the view is enabled, false otherwise.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setEnabled(bool isEnabled);

    /**
     * Gets if the source is a password field.
     *
     * @return True if the view is a password field, false otherwise.
     */
    bool isPassword();

    /**
     * Sets if the source is a password field.
     *
     * @param isPassword True if the view is a password field, false otherwise.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setPassword(bool isPassword);

    /**
     * Gets if the source is taking the entire screen.
     *
     * @return True if the source is full screen, false otherwise.
     */
    bool isFullScreen();

    /**
     * Sets if the source is taking the entire screen.
     *
     * @param isFullScreen True if the source is full screen, false otherwise.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setFullScreen(bool isFullScreen);

    /**
     * Gets if the source is scrollable.
     *
     * @return True if the source is scrollable, false otherwise.
     */
    bool isScrollable();

    /**
     * Sets if the source is scrollable.
     *
     * @param scrollable True if the source is scrollable, false otherwise.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setScrollable(bool scrollable);

    /**
     * Gets if the source is important for accessibility.
     *
     * <strong>Note:</strong> Used only internally to determine whether
     * to deliver the event to a given accessibility service since some
     * services may want to regard all views for accessibility while others
     * may want to regard only the important views for accessibility.
     *
     * @return True if the source is important for accessibility,
     *        false otherwise.
     *
     * @hide
     */
    bool isImportantForAccessibility();

    /**
     * Sets if the source is important for accessibility.
     *
     * @param importantForAccessibility True if the source is important for accessibility,
     *                                  false otherwise.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     * @hide
     */
    void setImportantForAccessibility(bool importantForAccessibility);

    /**
     * Gets the number of items that can be visited.
     *
     * @return The number of items.
     */
    int getItemCount();

    /**
     * Sets the number of items that can be visited.
     *
     * @param itemCount The number of items.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setItemCount(int itemCount);
    /**
     * Gets the index of the source in the list of items the can be visited.
     *
     * @return The current item index.
     */
    int getCurrentItemIndex()const;

    /**
     * Sets the index of the source in the list of items that can be visited.
     *
     * @param currentItemIndex The current item index.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setCurrentItemIndex(int currentItemIndex);

    /**
     * Gets the index of the first character of the changed sequence,
     * or the beginning of a text selection or the index of the first
     * visible item when scrolling.
     *
     * @return The index of the first character or selection
     *        start or the first visible item.
     */
    int getFromIndex()const;

    /**
     * Sets the index of the first character of the changed sequence
     * or the beginning of a text selection or the index of the first
     * visible item when scrolling.
     *
     * @param fromIndex The index of the first character or selection
     *        start or the first visible item.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setFromIndex(int fromIndex);

    /**
     * Gets the index of text selection end or the index of the last
     * visible item when scrolling.
     *
     * @return The index of selection end or last item index.
     */
    int getToIndex() const;

    /**
     * Sets the index of text selection end or the index of the last
     * visible item when scrolling.
     *
     * @param toIndex The index of selection end or last item index.
     */
    void setToIndex(int toIndex);

    /**
     * Gets the scroll offset of the source left edge in pixels.
     *
     * @return The scroll.
     */
    int getScrollX()const;

    /**
     * Sets the scroll offset of the source left edge in pixels.
     *
     * @param scrollX The scroll.
     */
    void setScrollX(int scrollX);

    /**
     * Gets the scroll offset of the source top edge in pixels.
     *
     * @return The scroll.
     */
    int getScrollY() const;

    /**
     * Sets the scroll offset of the source top edge in pixels.
     *
     * @param scrollY The scroll.
     */
    void setScrollY(int scrollY);

    /**
     * Gets the difference in pixels between the horizontal position before the scroll and the
     * current horizontal position
     *
     * @return the scroll delta x
     */
    int getScrollDeltaX() const;

    /**
     * Sets the difference in pixels between the horizontal position before the scroll and the
     * current horizontal position
     *
     * @param scrollDeltaX the scroll delta x
     */
    void setScrollDeltaX(int scrollDeltaX);

    /**
     * Gets the difference in pixels between the vertical position before the scroll and the
     * current vertical position
     *
     * @return the scroll delta y
     */
    int getScrollDeltaY()const;

    /**
     * Sets the difference in pixels between the vertical position before the scroll and the
     * current vertical position
     *
     * @param scrollDeltaY the scroll delta y
     */
    void setScrollDeltaY(int scrollDeltaY);

    /**
     * Gets the max scroll offset of the source left edge in pixels.
     *
     * @return The max scroll.
     */
    int getMaxScrollX()const;

    /**
     * Sets the max scroll offset of the source left edge in pixels.
     *
     * @param maxScrollX The max scroll.
     */
    void setMaxScrollX(int maxScrollX);

    /**
     * Gets the max scroll offset of the source top edge in pixels.
     *
     * @return The max scroll.
     */
    int getMaxScrollY()const;

    /**
     * Sets the max scroll offset of the source top edge in pixels.
     *
     * @param maxScrollY The max scroll.
     */
    void setMaxScrollY(int maxScrollY);

    /**
     * Gets the number of added characters.
     *
     * @return The number of added characters.
     */
    int getAddedCount() const;

    /**
     * Sets the number of added characters.
     *
     * @param addedCount The number of added characters.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setAddedCount(int addedCount);

    /**
     * Gets the number of removed characters.
     *
     * @return The number of removed characters.
     */
    int getRemovedCount()const;

    /**
     * Sets the number of removed characters.
     *
     * @param removedCount The number of removed characters.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setRemovedCount(int removedCount);

    /**
     * Gets the class name of the source.
     *
     * @return The class name.
     */
    std::string getClassName()const;

    /**
     * Sets the class name of the source.
     *
     * @param className The lass name.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setClassName(const std::string& className);

    /**
     * Gets the text of the event. The index in the list represents the priority
     * of the text. Specifically, the lower the index the higher the priority.
     *
     * @return The text.
     */
    std::vector<std::string> getText()const;

    /**
     * Sets the text before a change.
     *
     * @return The text before the change.
     */
    std::string getBeforeText()const;

    /**
     * Sets the text before a change.
     *
     * @param beforeText The text before the change.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setBeforeText(const std::string& beforeText);

    /**
     * Gets the description of the source.
     *
     * @return The description.
     */
    std::string getContentDescription()const;

    /**
     * Sets the description of the source.
     *
     * @param contentDescription The description.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setContentDescription(const std::string& contentDescription);

    /**
     * Gets the {@link Parcelable} data.
     *
     * @return The parcelable data.
     */
    Parcelable* getParcelableData()const;

    /**
     * Sets the {@link Parcelable} data of the event.
     *
     * @param parcelableData The parcelable data.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    void setParcelableData(Parcelable* parcelableData);

    /**
     * Gets the id of the source node.
     *
     * @return The id.
     *
     * @hide
     */
    long getSourceNodeId()const;

    /**
     * Sets the unique id of the IAccessibilityServiceConnection over which
     * this instance can send requests to the system.
     *
     * @param connectionId The connection id.
     *
     * @hide
     */
    void setConnectionId(int connectionId);

    /**
     * Sets if this instance is sealed.
     *
     * @param sealed Whether is sealed.
     *
     * @hide
     */
    void setSealed(bool sealed);

    /**
     * Gets if this instance is sealed.
     *
     * @return Whether is sealed.
     */
    bool isSealed()const;

    /**
     * Enforces that this instance is sealed.
     *
     * @throws IllegalStateException If this instance is not sealed.
     */
    void enforceSealed();

    /**
     * Enforces that this instance is not sealed.
     *
     * @throws IllegalStateException If this instance is sealed.
     */
    void enforceNotSealed();

    /**
     * Returns a cached instance if such is available or a new one is
     * instantiated. The instance is initialized with data from the
     * given record.
     *
     * @return An instance.
     */
    static AccessibilityRecord* obtain(const AccessibilityRecord& record);

    /**
     * Returns a cached instance if such is available or a new one is
     * instantiated.
     *
     * @return An instance.
     */
    static AccessibilityRecord* obtain();
    /**
     * Return an instance back to be reused.
     * <p>
     * <strong>Note:</strong> You must not touch the object after calling this function.
     *
     * @throws IllegalStateException If the record is already recycled.
     */
    virtual void recycle();
private:
    /**
     * Gets the value of a bool property.
     *
     * @param property The property.
     * @return The value.
     */
    bool getBooleanProperty(int property);

    /**
     * Sets a bool property.
     *
     * @param property The property.
     * @param value The value.
     */
    void setBooleanProperty(int property, bool value);


    /**
     * Initialize this record from another one.
     *
     * @param record The to initialize from.
     */
    void init(const AccessibilityRecord& record);

#if 0
    StringBuilder appendTo(StringBuilder builder);
    void appendUnless(bool defValue, int prop, StringBuilder builder);
    static String singleBooleanPropertyToString(int prop);

    void append(StringBuilder builder, String propName, int propValue);
        if (DEBUG_CONCISE_TOSTRING && propValue == UNDEFINED) return;
        appendPropName(builder, propName).append(propValue);
    }

    void append(StringBuilder builder, String propName, Object propValue);
        if (DEBUG_CONCISE_TOSTRING && propValue == null) return;
        appendPropName(builder, propName).append(propValue);
    }

    StringBuilder appendPropName(StringBuilder builder, String propName);
        return builder.append("; ").append(propName).append(": ");
    }
#endif
};
}/*endof namespace*/
#endif/*__ACCESSIBILITY_RECORD_H__*/

