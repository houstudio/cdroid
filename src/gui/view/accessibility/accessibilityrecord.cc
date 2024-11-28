#include <view/view.h>
#include <porting/cdlog.h>
#include <view/accessibility/accessibilityrecord.h>
namespace cdroid{

AccessibilityRecord*AccessibilityRecord::sPool = nullptr;
int AccessibilityRecord::sPoolSize=0;

AccessibilityRecord::AccessibilityRecord() {
    mSealed = false;
    mIsInPool=false;
}

void AccessibilityRecord::setSource(View* source) {
    setSource(source, AccessibilityNodeProvider::HOST_VIEW_ID);
}

void AccessibilityRecord::setSource(View* root, int virtualDescendantId) {
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

void AccessibilityRecord::setSourceNodeId(long sourceNodeId) {
    mSourceNodeId = sourceNodeId;
}

AccessibilityNodeInfo* AccessibilityRecord::getSource() {
    enforceSealed();
    if ((mConnectionId == UNDEFINED)
        || (mSourceWindowId == AccessibilityWindowInfo::UNDEFINED_WINDOW_ID)
        || (AccessibilityNodeInfo::getAccessibilityViewId(mSourceNodeId)
                == AccessibilityNodeInfo::UNDEFINED_ITEM_ID)) {
        return nullptr;
    }
    /*AccessibilityInteractionClient client = AccessibilityInteractionClient::getInstance();
    return client.findAccessibilityNodeInfoByAccessibilityId(mConnectionId, mSourceWindowId,
            mSourceNodeId, false, GET_SOURCE_PREFETCH_FLAGS, nullptr);*/
    return nullptr;
}

void AccessibilityRecord::setWindowId(int windowId) {
    mSourceWindowId = windowId;
}

int AccessibilityRecord::getWindowId() const{
    return mSourceWindowId;
}

bool AccessibilityRecord::isChecked() const{
    return getBooleanProperty(PROPERTY_CHECKED);
}

void AccessibilityRecord::setChecked(bool isChecked) {
    enforceNotSealed();
    setBooleanProperty(PROPERTY_CHECKED, isChecked);
}

bool AccessibilityRecord::isEnabled() const{
    return getBooleanProperty(PROPERTY_ENABLED);
}

void AccessibilityRecord::setEnabled(bool isEnabled) {
    enforceNotSealed();
    setBooleanProperty(PROPERTY_ENABLED, isEnabled);
}

bool AccessibilityRecord::isPassword() const{
    return getBooleanProperty(PROPERTY_PASSWORD);
}

void AccessibilityRecord::setPassword(bool isPassword) {
    enforceNotSealed();
    setBooleanProperty(PROPERTY_PASSWORD, isPassword);
}

bool AccessibilityRecord::isFullScreen() const{
    return getBooleanProperty(PROPERTY_FULL_SCREEN);
}

void AccessibilityRecord::setFullScreen(bool isFullScreen) {
    enforceNotSealed();
    setBooleanProperty(PROPERTY_FULL_SCREEN, isFullScreen);
}

bool AccessibilityRecord::isScrollable() const{
    return getBooleanProperty(PROPERTY_SCROLLABLE);
}

void AccessibilityRecord::setScrollable(bool scrollable) {
    enforceNotSealed();
    setBooleanProperty(PROPERTY_SCROLLABLE, scrollable);
}

bool AccessibilityRecord::isImportantForAccessibility() const{
    return getBooleanProperty(PROPERTY_IMPORTANT_FOR_ACCESSIBILITY);
}

void AccessibilityRecord::setImportantForAccessibility(bool importantForAccessibility) {
    enforceNotSealed();
    setBooleanProperty(PROPERTY_IMPORTANT_FOR_ACCESSIBILITY, importantForAccessibility);
}

int AccessibilityRecord::getItemCount() const{
    return mItemCount;
}

void AccessibilityRecord::setItemCount(int itemCount) {
    enforceNotSealed();
    mItemCount = itemCount;
}

int AccessibilityRecord::getCurrentItemIndex() const{
    return mCurrentItemIndex;
}

void AccessibilityRecord::setCurrentItemIndex(int currentItemIndex) {
    enforceNotSealed();
    mCurrentItemIndex = currentItemIndex;
}

int AccessibilityRecord::getFromIndex() const{
    return mFromIndex;
}

void AccessibilityRecord::setFromIndex(int fromIndex) {
    enforceNotSealed();
    mFromIndex = fromIndex;
}

int AccessibilityRecord::getToIndex() const{
    return mToIndex;
}

void AccessibilityRecord::setToIndex(int toIndex) {
    enforceNotSealed();
    mToIndex = toIndex;
}

int AccessibilityRecord::getScrollX() const{
    return mScrollX;
}

void AccessibilityRecord::setScrollX(int scrollX) {
    enforceNotSealed();
    mScrollX = scrollX;
}

int AccessibilityRecord::getScrollY() const{
    return mScrollY;
}

void AccessibilityRecord::setScrollY(int scrollY) {
    enforceNotSealed();
    mScrollY = scrollY;
}

int AccessibilityRecord::getScrollDeltaX() const{
    return mScrollDeltaX;
}

void AccessibilityRecord::setScrollDeltaX(int scrollDeltaX) {
    enforceNotSealed();
    mScrollDeltaX = scrollDeltaX;
}

int AccessibilityRecord::getScrollDeltaY() const{
    return mScrollDeltaY;
}

void AccessibilityRecord::setScrollDeltaY(int scrollDeltaY) {
    enforceNotSealed();
    mScrollDeltaY = scrollDeltaY;
}

int AccessibilityRecord::getMaxScrollX() const{
    return mMaxScrollX;
}

void AccessibilityRecord::setMaxScrollX(int maxScrollX) {
    enforceNotSealed();
    mMaxScrollX = maxScrollX;
}

int AccessibilityRecord::getMaxScrollY() const{
    return mMaxScrollY;
}

void AccessibilityRecord::setMaxScrollY(int maxScrollY) {
    enforceNotSealed();
    mMaxScrollY = maxScrollY;
}

int AccessibilityRecord::getAddedCount() const{
    return mAddedCount;
}

void AccessibilityRecord::setAddedCount(int addedCount) {
    enforceNotSealed();
    mAddedCount = addedCount;
}

int AccessibilityRecord::getRemovedCount() const{
    return mRemovedCount;
}

void AccessibilityRecord::setRemovedCount(int removedCount) {
    enforceNotSealed();
    mRemovedCount = removedCount;
}

std::string AccessibilityRecord::getClassName() const{
    return mClassName;
}

void AccessibilityRecord::setClassName(const std::string& className) {
    enforceNotSealed();
    mClassName = className;
}

std::vector<std::string>& AccessibilityRecord::getText(){
    return mText;
}

std::string AccessibilityRecord::getBeforeText() const{
    return mBeforeText;
}

void AccessibilityRecord::setBeforeText(const std::string& beforeText) {
    enforceNotSealed();
    mBeforeText =beforeText;
}

std::string AccessibilityRecord::getContentDescription() const{
    return mContentDescription;
}

void AccessibilityRecord::setContentDescription(const std::string& contentDescription) {
    enforceNotSealed();
    mContentDescription =contentDescription;
}

Parcelable* AccessibilityRecord::getParcelableData() const{
    return mParcelableData;
}

void AccessibilityRecord::setParcelableData(Parcelable* parcelableData) {
    enforceNotSealed();
    mParcelableData = parcelableData;
}

long AccessibilityRecord::getSourceNodeId() const{
    return mSourceNodeId;
}

void AccessibilityRecord::setConnectionId(int connectionId) {
    enforceNotSealed();
    mConnectionId = connectionId;
}

void AccessibilityRecord::setSealed(bool sealed) {
    mSealed = sealed;
}

bool AccessibilityRecord::isSealed() const{
    return mSealed;
}

void AccessibilityRecord::enforceSealed() {
    if (!isSealed()) {
        throw std::logic_error("Cannot perform this action on a not sealed instance.");
    }
}

void AccessibilityRecord::enforceNotSealed() {
    if (isSealed()) {
        throw std::logic_error("Cannot perform this action on a sealed instance.");
    }
}

bool AccessibilityRecord::getBooleanProperty(int property) const{
    return (mBooleanProperties & property) == property;
}

void AccessibilityRecord::setBooleanProperty(int property, bool value) {
    if (value) {
        mBooleanProperties |= property;
    } else {
        mBooleanProperties &= ~property;
    }
}

AccessibilityRecord* AccessibilityRecord::obtain(const AccessibilityRecord& record) {
   AccessibilityRecord* clone = AccessibilityRecord::obtain();
   clone->init(record);
   return clone;
}

AccessibilityRecord* AccessibilityRecord::obtain() {
    //synchronized (sPoolLock) 
    {
        if (sPool != nullptr) {
            AccessibilityRecord* record = sPool;
            sPool = sPool->mNext;
            sPoolSize--;
            record->mNext = nullptr;
            record->mIsInPool = false;
            return record;
        }
        return new AccessibilityRecord();
    }
}

void AccessibilityRecord::recycle() {
    if (mIsInPool) {
        throw std::logic_error("Record already recycled!");
    }
    clear();
    //synchronized (sPoolLock) 
    {
        LOGD("sPoolSize=%d",sPoolSize);
        if (sPoolSize <= MAX_POOL_SIZE) {
            mNext = sPool;
            sPool = this;
            mIsInPool = true;
            sPoolSize++;
        }
    }
}

void AccessibilityRecord::init(const AccessibilityRecord& record) {
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
    mText=record.mText;//.addAll(record.mText);
    mSourceWindowId = record.mSourceWindowId;
    mSourceNodeId = record.mSourceNodeId;
    mConnectionId = record.mConnectionId;
}

void AccessibilityRecord::clear() {
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
    mClassName.clear();// = null;
    mContentDescription.clear();// = null;
    mBeforeText.clear();// = null;
    mParcelableData = nullptr;
    mText.clear();
    mSourceNodeId = AccessibilityNodeInfo::UNDEFINED_ITEM_ID;
    mSourceWindowId = AccessibilityWindowInfo::UNDEFINED_WINDOW_ID;
    mConnectionId = UNDEFINED;
}

std::string AccessibilityRecord::toString() {
    std::ostringstream oss;
    appendTo(oss);
    return oss.str();
}

std::ostringstream& AccessibilityRecord::appendTo(std::ostringstream&builder) {
    builder<<" [ ClassName: "<<mClassName;
    if (!DEBUG_CONCISE_TOSTRING || !mText.empty()) {
        appendPropName(builder, "Text");//<<append(mText);
        for(auto t:mText)builder<<t<<",";
        builder<<std::endl;
    }
    appendPropName(builder, "ContentDescription")<<mContentDescription;
    append(builder, "ItemCount", mItemCount);
    append(builder, "CurrentItemIndex", mCurrentItemIndex);

    appendUnless(true, PROPERTY_ENABLED, builder);
    appendUnless(false, PROPERTY_PASSWORD, builder);
    appendUnless(false, PROPERTY_CHECKED, builder);
    appendUnless(false, PROPERTY_FULL_SCREEN, builder);
    appendUnless(false, PROPERTY_SCROLLABLE, builder);

    appendPropName(builder, "BeforeText")<<mBeforeText;
    append(builder, "FromIndex", mFromIndex);
    append(builder, "ToIndex", mToIndex);
    append(builder, "ScrollX", mScrollX);
    append(builder, "ScrollY", mScrollY);
    append(builder, "MaxScrollX", mMaxScrollX);
    append(builder, "MaxScrollY", mMaxScrollY);
    append(builder, "AddedCount", mAddedCount);
    append(builder, "RemovedCount", mRemovedCount);
    //append(builder, "ParcelableData", mParcelableData);
    builder<<" ]";
    return builder;
}

void AccessibilityRecord::appendUnless(bool defValue, int prop,std::ostringstream& builder) {
    bool value = getBooleanProperty(prop);
    if (DEBUG_CONCISE_TOSTRING && value == defValue) return;
    appendPropName(builder, singleBooleanPropertyToString(prop))<<value;
}

std::string AccessibilityRecord::singleBooleanPropertyToString(int prop) {
    switch (prop) {
    case PROPERTY_CHECKED: return "Checked";
    case PROPERTY_ENABLED: return "Enabled";
    case PROPERTY_PASSWORD: return "Password";
    case PROPERTY_FULL_SCREEN: return "FullScreen";
    case PROPERTY_SCROLLABLE: return "Scrollable";
    case PROPERTY_IMPORTANT_FOR_ACCESSIBILITY:
        return "ImportantForAccessibility";
    default: return std::to_string(prop);
    }
}

void AccessibilityRecord::append(std::ostringstream& builder,const std::string& propName, int propValue) {
    if (DEBUG_CONCISE_TOSTRING && propValue == UNDEFINED) return;
    appendPropName(builder, propName)<<propValue;
}

std::ostringstream& AccessibilityRecord::appendPropName(std::ostringstream& builder,const std::string& propName) {
    builder<<"; "<<propName<<": ";
    return builder;
}

}
