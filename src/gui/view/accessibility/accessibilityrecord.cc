#if 0
public class AccessibilityRecord {
    /** @hide */
    protected static final bool DEBUG_CONCISE_TOSTRING = false;
private:
    static constexpr int UNDEFINED = -1;

    static constexpr int PROPERTY_CHECKED = 0x00000001;
    static constexpr int PROPERTY_ENABLED = 0x00000002;
    static constexpr int PROPERTY_PASSWORD = 0x00000004;
    static constexpr int PROPERTY_FULL_SCREEN= 0x00000080;
    static constexpr int PROPERTY_SCROLLABLE = 0x00000100;
    static constexpr int PROPERTY_IMPORTANT_FOR_ACCESSIBILITY = 0x00000200;

    static constexpr int GET_SOURCE_PREFETCH_FLAGS =  AccessibilityNodeInfo::FLAG_PREFETCH_PREDECESSORS
        | AccessibilityNodeInfo.FLAG_PREFETCH_SIBLINGS   | AccessibilityNodeInfo.FLAG_PREFETCH_DESCENDANTS;

    // Housekeeping
    static constexpr int MAX_POOL_SIZE = 10;
    static constexpr Object sPoolLock = new Object();
    static AccessibilityRecord sPool;
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
    long mSourceNodeId = AccessibilityNodeInfo::UNDEFINED_NODE_ID;
    int mSourceWindowId = AccessibilityWindowInfo::UNDEFINED_WINDOW_ID;

    CharSequence mClassName;
    CharSequence mContentDescription;
    CharSequence mBeforeText;
    Parcelable mParcelableData;

    final List<CharSequence> mText = new ArrayList<CharSequence>();

    int mConnectionId = UNDEFINED;

    /*
     * Hide constructor.
     */
    AccessibilityRecord() {
    }
public:
    /**
     * Sets the event source.
     *
     * @param source The source.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setSource(View source) {
        setSource(source, AccessibilityNodeProvider::HOST_VIEW_ID);
    }

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
    public void setSource(@Nullable View* root, int virtualDescendantId) {
        enforceNotSealed();
        bool important = true;
        int rootViewId = AccessibilityNodeInfo::UNDEFINED_ITEM_ID;
        mSourceWindowId = AccessibilityWindowInfo::UNDEFINED_WINDOW_ID;
        if (root != nullptr) {
            important = root->isImportantForAccessibility();
            rootViewId = root->getAccessibilityViewId();
            mSourceWindowId = root->getAccessibilityWindowId();
        }
        setBooleanProperty(PROPERTY_IMPORTANT_FOR_ACCESSIBILITY, important);
        mSourceNodeId = AccessibilityNodeInfo::makeNodeId(rootViewId, virtualDescendantId);
    }

    /**
     * Set the source node ID directly
     *
     * @param sourceNodeId The source node Id
     * @hide
     */
    public void setSourceNodeId(long sourceNodeId) {
        mSourceNodeId = sourceNodeId;
    }

    /**
     * Gets the {@link AccessibilityNodeInfo} of the event source.
     * <p>
     *   <strong>Note:</strong> It is a client responsibility to recycle the received info
     *   by calling {@link AccessibilityNodeInfo#recycle() AccessibilityNodeInfo#recycle()}
     *   to avoid creating of multiple instances.
     * </p>
     * @return The info of the source.
     */
    public AccessibilityNodeInfo getSource() {
        enforceSealed();
        if ((mConnectionId == UNDEFINED)
                || (mSourceWindowId == AccessibilityWindowInfo::UNDEFINED_WINDOW_ID)
                || (AccessibilityNodeInfo::getAccessibilityViewId(mSourceNodeId)
                        == AccessibilityNodeInfo::UNDEFINED_ITEM_ID)) {
            return null;
        }
        AccessibilityInteractionClient client = AccessibilityInteractionClient::getInstance();
        return client.findAccessibilityNodeInfoByAccessibilityId(mConnectionId, mSourceWindowId,
                mSourceNodeId, false, GET_SOURCE_PREFETCH_FLAGS, null);
    }

    /**
     * Sets the window id.
     *
     * @param windowId The window id.
     *
     * @hide
     */
    public void setWindowId(int windowId) {
        mSourceWindowId = windowId;
    }

    /**
     * Gets the id of the window from which the event comes from.
     *
     * @return The window id.
     */
    public int getWindowId() {
        return mSourceWindowId;
    }

    /**
     * Gets if the source is checked.
     *
     * @return True if the view is checked, false otherwise.
     */
    public bool isChecked() {
        return getBooleanProperty(PROPERTY_CHECKED);
    }

    /**
     * Sets if the source is checked.
     *
     * @param isChecked True if the view is checked, false otherwise.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setChecked(bool isChecked) {
        enforceNotSealed();
        setBooleanProperty(PROPERTY_CHECKED, isChecked);
    }

    /**
     * Gets if the source is enabled.
     *
     * @return True if the view is enabled, false otherwise.
     */
    public bool isEnabled() {
        return getBooleanProperty(PROPERTY_ENABLED);
    }

    /**
     * Sets if the source is enabled.
     *
     * @param isEnabled True if the view is enabled, false otherwise.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setEnabled(bool isEnabled) {
        enforceNotSealed();
        setBooleanProperty(PROPERTY_ENABLED, isEnabled);
    }

    /**
     * Gets if the source is a password field.
     *
     * @return True if the view is a password field, false otherwise.
     */
    public bool isPassword() {
        return getBooleanProperty(PROPERTY_PASSWORD);
    }

    /**
     * Sets if the source is a password field.
     *
     * @param isPassword True if the view is a password field, false otherwise.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setPassword(bool isPassword) {
        enforceNotSealed();
        setBooleanProperty(PROPERTY_PASSWORD, isPassword);
    }

    /**
     * Gets if the source is taking the entire screen.
     *
     * @return True if the source is full screen, false otherwise.
     */
    public bool isFullScreen() {
        return getBooleanProperty(PROPERTY_FULL_SCREEN);
    }

    /**
     * Sets if the source is taking the entire screen.
     *
     * @param isFullScreen True if the source is full screen, false otherwise.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setFullScreen(bool isFullScreen) {
        enforceNotSealed();
        setBooleanProperty(PROPERTY_FULL_SCREEN, isFullScreen);
    }

    /**
     * Gets if the source is scrollable.
     *
     * @return True if the source is scrollable, false otherwise.
     */
    public bool isScrollable() {
        return getBooleanProperty(PROPERTY_SCROLLABLE);
    }

    /**
     * Sets if the source is scrollable.
     *
     * @param scrollable True if the source is scrollable, false otherwise.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setScrollable(bool scrollable) {
        enforceNotSealed();
        setBooleanProperty(PROPERTY_SCROLLABLE, scrollable);
    }

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
    public bool isImportantForAccessibility() {
        return getBooleanProperty(PROPERTY_IMPORTANT_FOR_ACCESSIBILITY);
    }

    /**
     * Sets if the source is important for accessibility.
     *
     * @param importantForAccessibility True if the source is important for accessibility,
     *                                  false otherwise.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     * @hide
     */
    public void setImportantForAccessibility(bool importantForAccessibility) {
        enforceNotSealed();
        setBooleanProperty(PROPERTY_IMPORTANT_FOR_ACCESSIBILITY, importantForAccessibility);
    }

    /**
     * Gets the number of items that can be visited.
     *
     * @return The number of items.
     */
    public int getItemCount() {
        return mItemCount;
    }

    /**
     * Sets the number of items that can be visited.
     *
     * @param itemCount The number of items.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setItemCount(int itemCount) {
        enforceNotSealed();
        mItemCount = itemCount;
    }

    /**
     * Gets the index of the source in the list of items the can be visited.
     *
     * @return The current item index.
     */
    public int getCurrentItemIndex() {
        return mCurrentItemIndex;
    }

    /**
     * Sets the index of the source in the list of items that can be visited.
     *
     * @param currentItemIndex The current item index.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setCurrentItemIndex(int currentItemIndex) {
        enforceNotSealed();
        mCurrentItemIndex = currentItemIndex;
    }

    /**
     * Gets the index of the first character of the changed sequence,
     * or the beginning of a text selection or the index of the first
     * visible item when scrolling.
     *
     * @return The index of the first character or selection
     *        start or the first visible item.
     */
    public int getFromIndex() {
        return mFromIndex;
    }

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
    public void setFromIndex(int fromIndex) {
        enforceNotSealed();
        mFromIndex = fromIndex;
    }

    /**
     * Gets the index of text selection end or the index of the last
     * visible item when scrolling.
     *
     * @return The index of selection end or last item index.
     */
    public int getToIndex() {
        return mToIndex;
    }

    /**
     * Sets the index of text selection end or the index of the last
     * visible item when scrolling.
     *
     * @param toIndex The index of selection end or last item index.
     */
    public void setToIndex(int toIndex) {
        enforceNotSealed();
        mToIndex = toIndex;
    }

    /**
     * Gets the scroll offset of the source left edge in pixels.
     *
     * @return The scroll.
     */
    public int getScrollX() {
        return mScrollX;
    }

    /**
     * Sets the scroll offset of the source left edge in pixels.
     *
     * @param scrollX The scroll.
     */
    public void setScrollX(int scrollX) {
        enforceNotSealed();
        mScrollX = scrollX;
    }

    /**
     * Gets the scroll offset of the source top edge in pixels.
     *
     * @return The scroll.
     */
    public int getScrollY() {
        return mScrollY;
    }

    /**
     * Sets the scroll offset of the source top edge in pixels.
     *
     * @param scrollY The scroll.
     */
    public void setScrollY(int scrollY) {
        enforceNotSealed();
        mScrollY = scrollY;
    }

    /**
     * Gets the difference in pixels between the horizontal position before the scroll and the
     * current horizontal position
     *
     * @return the scroll delta x
     */
    public int getScrollDeltaX() {
        return mScrollDeltaX;
    }

    /**
     * Sets the difference in pixels between the horizontal position before the scroll and the
     * current horizontal position
     *
     * @param scrollDeltaX the scroll delta x
     */
    public void setScrollDeltaX(int scrollDeltaX) {
        enforceNotSealed();
        mScrollDeltaX = scrollDeltaX;
    }

    /**
     * Gets the difference in pixels between the vertical position before the scroll and the
     * current vertical position
     *
     * @return the scroll delta y
     */
    public int getScrollDeltaY() {
        return mScrollDeltaY;
    }

    /**
     * Sets the difference in pixels between the vertical position before the scroll and the
     * current vertical position
     *
     * @param scrollDeltaY the scroll delta y
     */
    public void setScrollDeltaY(int scrollDeltaY) {
        enforceNotSealed();
        mScrollDeltaY = scrollDeltaY;
    }

    /**
     * Gets the max scroll offset of the source left edge in pixels.
     *
     * @return The max scroll.
     */
    public int getMaxScrollX() {
        return mMaxScrollX;
    }

    /**
     * Sets the max scroll offset of the source left edge in pixels.
     *
     * @param maxScrollX The max scroll.
     */
    public void setMaxScrollX(int maxScrollX) {
        enforceNotSealed();
        mMaxScrollX = maxScrollX;
    }

    /**
     * Gets the max scroll offset of the source top edge in pixels.
     *
     * @return The max scroll.
     */
    public int getMaxScrollY() {
        return mMaxScrollY;
    }

    /**
     * Sets the max scroll offset of the source top edge in pixels.
     *
     * @param maxScrollY The max scroll.
     */
    public void setMaxScrollY(int maxScrollY) {
        enforceNotSealed();
        mMaxScrollY = maxScrollY;
    }

    /**
     * Gets the number of added characters.
     *
     * @return The number of added characters.
     */
    public int getAddedCount() {
        return mAddedCount;
    }

    /**
     * Sets the number of added characters.
     *
     * @param addedCount The number of added characters.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setAddedCount(int addedCount) {
        enforceNotSealed();
        mAddedCount = addedCount;
    }

    /**
     * Gets the number of removed characters.
     *
     * @return The number of removed characters.
     */
    public int getRemovedCount() {
        return mRemovedCount;
    }

    /**
     * Sets the number of removed characters.
     *
     * @param removedCount The number of removed characters.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setRemovedCount(int removedCount) {
        enforceNotSealed();
        mRemovedCount = removedCount;
    }

    /**
     * Gets the class name of the source.
     *
     * @return The class name.
     */
    public CharSequence getClassName() {
        return mClassName;
    }

    /**
     * Sets the class name of the source.
     *
     * @param className The lass name.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setClassName(CharSequence className) {
        enforceNotSealed();
        mClassName = className;
    }

    /**
     * Gets the text of the event. The index in the list represents the priority
     * of the text. Specifically, the lower the index the higher the priority.
     *
     * @return The text.
     */
    public List<CharSequence> getText() {
        return mText;
    }

    /**
     * Sets the text before a change.
     *
     * @return The text before the change.
     */
    public CharSequence getBeforeText() {
        return mBeforeText;
    }

    /**
     * Sets the text before a change.
     *
     * @param beforeText The text before the change.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setBeforeText(CharSequence beforeText) {
        enforceNotSealed();
        mBeforeText = (beforeText == null) ? null
                : beforeText.subSequence(0, beforeText.length());
    }

    /**
     * Gets the description of the source.
     *
     * @return The description.
     */
    public CharSequence getContentDescription() {
        return mContentDescription;
    }

    /**
     * Sets the description of the source.
     *
     * @param contentDescription The description.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setContentDescription(CharSequence contentDescription) {
        enforceNotSealed();
        mContentDescription = (contentDescription == null) ? null
                : contentDescription.subSequence(0, contentDescription.length());
    }

    /**
     * Gets the {@link Parcelable} data.
     *
     * @return The parcelable data.
     */
    public Parcelable getParcelableData() {
        return mParcelableData;
    }

    /**
     * Sets the {@link Parcelable} data of the event.
     *
     * @param parcelableData The parcelable data.
     *
     * @throws IllegalStateException If called from an AccessibilityService.
     */
    public void setParcelableData(Parcelable parcelableData) {
        enforceNotSealed();
        mParcelableData = parcelableData;
    }

    /**
     * Gets the id of the source node.
     *
     * @return The id.
     *
     * @hide
     */
    public long getSourceNodeId() {
        return mSourceNodeId;
    }

    /**
     * Sets the unique id of the IAccessibilityServiceConnection over which
     * this instance can send requests to the system.
     *
     * @param connectionId The connection id.
     *
     * @hide
     */
    public void setConnectionId(int connectionId) {
        enforceNotSealed();
        mConnectionId = connectionId;
    }

    /**
     * Sets if this instance is sealed.
     *
     * @param sealed Whether is sealed.
     *
     * @hide
     */
    public void setSealed(bool sealed) {
        mSealed = sealed;
    }

    /**
     * Gets if this instance is sealed.
     *
     * @return Whether is sealed.
     */
    bool isSealed() {
        return mSealed;
    }

    /**
     * Enforces that this instance is sealed.
     *
     * @throws IllegalStateException If this instance is not sealed.
     */
    void enforceSealed() {
        if (!isSealed()) {
            throw new IllegalStateException("Cannot perform this "
                    + "action on a not sealed instance.");
        }
    }

    /**
     * Enforces that this instance is not sealed.
     *
     * @throws IllegalStateException If this instance is sealed.
     */
    void enforceNotSealed() {
        if (isSealed()) {
            throw new IllegalStateException("Cannot perform this "
                    + "action on a sealed instance.");
        }
    }

    /**
     * Gets the value of a bool property.
     *
     * @param property The property.
     * @return The value.
     */
    private bool getBooleanProperty(int property) {
        return (mBooleanProperties & property) == property;
    }

    /**
     * Sets a bool property.
     *
     * @param property The property.
     * @param value The value.
     */
    private void setBooleanProperty(int property, bool value) {
        if (value) {
            mBooleanProperties |= property;
        } else {
            mBooleanProperties &= ~property;
        }
    }

    /**
     * Returns a cached instance if such is available or a new one is
     * instantiated. The instance is initialized with data from the
     * given record.
     *
     * @return An instance.
     */
    public static AccessibilityRecord obtain(AccessibilityRecord record) {
       AccessibilityRecord clone = AccessibilityRecord.obtain();
       clone.init(record);
       return clone;
    }

    /**
     * Returns a cached instance if such is available or a new one is
     * instantiated.
     *
     * @return An instance.
     */
    public static AccessibilityRecord obtain() {
        synchronized (sPoolLock) {
            if (sPool != null) {
                AccessibilityRecord record = sPool;
                sPool = sPool.mNext;
                sPoolSize--;
                record.mNext = null;
                record.mIsInPool = false;
                return record;
            }
            return new AccessibilityRecord();
        }
    }

    /**
     * Return an instance back to be reused.
     * <p>
     * <strong>Note:</strong> You must not touch the object after calling this function.
     *
     * @throws IllegalStateException If the record is already recycled.
     */
    public void recycle() {
        if (mIsInPool) {
            throw new IllegalStateException("Record already recycled!");
        }
        clear();
        synchronized (sPoolLock) {
            if (sPoolSize <= MAX_POOL_SIZE) {
                mNext = sPool;
                sPool = this;
                mIsInPool = true;
                sPoolSize++;
            }
        }
    }

    /**
     * Initialize this record from another one.
     *
     * @param record The to initialize from.
     */
    void init(AccessibilityRecord record) {
        mSealed = record.mSealed;
        mBooleanProperties = record.mBooleanProperties;
        mCurrentItemIndex = record.mCurrentItemIndex;
        mItemCount = record.mItemCount;
        mFromIndex = record.mFromIndex;
        mToIndex = record.mToIndex;
        mScrollX = record.mScrollX;
        mScrollY = record.mScrollY;
        mMaxScrollX = record.mMaxScrollX;
        mMaxScrollY = record.mMaxScrollY;
        mAddedCount = record.mAddedCount;
        mRemovedCount = record.mRemovedCount;
        mClassName = record.mClassName;
        mContentDescription = record.mContentDescription;
        mBeforeText = record.mBeforeText;
        mParcelableData = record.mParcelableData;
        mText.addAll(record.mText);
        mSourceWindowId = record.mSourceWindowId;
        mSourceNodeId = record.mSourceNodeId;
        mConnectionId = record.mConnectionId;
    }

    /**
     * Clears the state of this instance.
     */
    void clear() {
        mSealed = false;
        mBooleanProperties = 0;
        mCurrentItemIndex = UNDEFINED;
        mItemCount = UNDEFINED;
        mFromIndex = UNDEFINED;
        mToIndex = UNDEFINED;
        mScrollX = UNDEFINED;
        mScrollY = UNDEFINED;
        mMaxScrollX = UNDEFINED;
        mMaxScrollY = UNDEFINED;
        mAddedCount = UNDEFINED;
        mRemovedCount = UNDEFINED;
        mClassName = null;
        mContentDescription = null;
        mBeforeText = null;
        mParcelableData = null;
        mText.clear();
        mSourceNodeId = AccessibilityNodeInfo.UNDEFINED_ITEM_ID;
        mSourceWindowId = AccessibilityWindowInfo.UNDEFINED_WINDOW_ID;
        mConnectionId = UNDEFINED;
    }

    @Override
    public String toString() {
        return appendTo(new StringBuilder()).toString();
    }

    StringBuilder appendTo(StringBuilder builder) {
        builder.append(" [ ClassName: ").append(mClassName);
        if (!DEBUG_CONCISE_TOSTRING || !isEmpty(mText)) {
            appendPropName(builder, "Text").append(mText);
        }
        append(builder, "ContentDescription", mContentDescription);
        append(builder, "ItemCount", mItemCount);
        append(builder, "CurrentItemIndex", mCurrentItemIndex);

        appendUnless(true, PROPERTY_ENABLED, builder);
        appendUnless(false, PROPERTY_PASSWORD, builder);
        appendUnless(false, PROPERTY_CHECKED, builder);
        appendUnless(false, PROPERTY_FULL_SCREEN, builder);
        appendUnless(false, PROPERTY_SCROLLABLE, builder);

        append(builder, "BeforeText", mBeforeText);
        append(builder, "FromIndex", mFromIndex);
        append(builder, "ToIndex", mToIndex);
        append(builder, "ScrollX", mScrollX);
        append(builder, "ScrollY", mScrollY);
        append(builder, "MaxScrollX", mMaxScrollX);
        append(builder, "MaxScrollY", mMaxScrollY);
        append(builder, "AddedCount", mAddedCount);
        append(builder, "RemovedCount", mRemovedCount);
        append(builder, "ParcelableData", mParcelableData);
        builder.append(" ]");
        return builder;
    }

    private void appendUnless(bool defValue, int prop, StringBuilder builder) {
        bool value = getBooleanProperty(prop);
        if (DEBUG_CONCISE_TOSTRING && value == defValue) return;
        appendPropName(builder, singleBooleanPropertyToString(prop))
                .append(value);
    }

    private static String singleBooleanPropertyToString(int prop) {
        switch (prop) {
            case PROPERTY_CHECKED: return "Checked";
            case PROPERTY_ENABLED: return "Enabled";
            case PROPERTY_PASSWORD: return "Password";
            case PROPERTY_FULL_SCREEN: return "FullScreen";
            case PROPERTY_SCROLLABLE: return "Scrollable";
            case PROPERTY_IMPORTANT_FOR_ACCESSIBILITY:
                return "ImportantForAccessibility";
            default: return Integer.toHexString(prop);
        }
    }

    private void append(StringBuilder builder, String propName, int propValue) {
        if (DEBUG_CONCISE_TOSTRING && propValue == UNDEFINED) return;
        appendPropName(builder, propName).append(propValue);
    }

    private void append(StringBuilder builder, String propName, Object propValue) {
        if (DEBUG_CONCISE_TOSTRING && propValue == null) return;
        appendPropName(builder, propName).append(propValue);
    }

    private StringBuilder appendPropName(StringBuilder builder, String propName) {
        return builder.append("; ").append(propName).append(": ");
    }
}
#endif
