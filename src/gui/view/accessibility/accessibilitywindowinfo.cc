#include <view/accessibility/accessibilitynodeinfo.h>
#include <view/accessibility/accessibilitywindowinfo.h>
#include <view/accessibility/accessibilityevent.h>

namespace cdroid{

int AccessibilityWindowInfo::sNumInstancesInUse = 0;
Pools::SimplePool<AccessibilityWindowInfo> AccessibilityWindowInfo::sPool(MAX_POOL_SIZE);

AccessibilityWindowInfo::AccessibilityWindowInfo() {
    /* do nothing - hide constructor */
}

/**
 * Gets the title of the window.
 *
 * @return The title of the window, or {@code null} if none is available.
 */
std::string AccessibilityWindowInfo::getTitle() const{
    return mTitle;
}

/**
 * Sets the title of the window.
 *
 * @param title The title.
 *
 * @hide
 */
void AccessibilityWindowInfo::setTitle(const std::string& title) {
    mTitle = title;
}

/**
 * Gets the type of the window.
 *
 * @return The type.
 *
 * @see #TYPE_APPLICATION
 * @see #TYPE_INPUT_METHOD
 * @see #TYPE_SYSTEM
 * @see #TYPE_ACCESSIBILITY_OVERLAY
 */
int AccessibilityWindowInfo::getType() const{
    return mType;
}

/**
 * Sets the type of the window.
 *
 * @param type The type
 *
 * @hide
 */
void AccessibilityWindowInfo::setType(int type) {
    mType = type;
}

/**
 * Gets the layer which determines the Z-order of the window. Windows
 * with greater layer appear on top of windows with lesser layer.
 *
 * @return The window layer.
 */
int AccessibilityWindowInfo::getLayer() const{
    return mLayer;
}

/**
 * Sets the layer which determines the Z-order of the window. Windows
 * with greater layer appear on top of windows with lesser layer.
 *
 * @param layer The window layer.
 *
 * @hide
 */
void AccessibilityWindowInfo::setLayer(int layer) {
    mLayer = layer;
}

/**
 * Gets the root node in the window's hierarchy.
 *
 * @return The root node.
 */
AccessibilityNodeInfo* AccessibilityWindowInfo::getRoot() {
    if (mConnectionId == UNDEFINED_WINDOW_ID) {
        return nullptr;
    }
    /*AccessibilityInteractionClient client = AccessibilityInteractionClient.getInstance();
    return client.findAccessibilityNodeInfoByAccessibilityId(mConnectionId,
            mId, AccessibilityNodeInfo::ROOT_NODE_ID,
            true, AccessibilityNodeInfo::FLAG_PREFETCH_DESCENDANTS, nullptr);*/
    return nullptr;
}

void AccessibilityWindowInfo::setAnchorId(long anchorId) {
    mAnchorId = anchorId;
}

/**
 * Gets the node that anchors this window to another.
 *
 * @return The anchor node, or {@code null} if none exists.
 */
AccessibilityNodeInfo* AccessibilityWindowInfo::getAnchor() {
    if ((mConnectionId == UNDEFINED_WINDOW_ID)
            || (mAnchorId == AccessibilityNodeInfo::UNDEFINED_NODE_ID)
            || (mParentId == UNDEFINED_WINDOW_ID)) {
        return nullptr;
    }
    /*AccessibilityInteractionClient client = AccessibilityInteractionClient.getInstance();
    return client.findAccessibilityNodeInfoByAccessibilityId(mConnectionId,
            mParentId, mAnchorId, true, 0, nullptr);*/
    return  nullptr;
}

void AccessibilityWindowInfo::setPictureInPicture(bool pictureInPicture) {
    setBooleanProperty(BOOLEAN_PROPERTY_PICTURE_IN_PICTURE, pictureInPicture);
}

bool AccessibilityWindowInfo::isInPictureInPictureMode() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_PICTURE_IN_PICTURE);
}

AccessibilityWindowInfo* AccessibilityWindowInfo::getParent() {
    if (mConnectionId == UNDEFINED_WINDOW_ID || mParentId == UNDEFINED_WINDOW_ID) {
        return nullptr;
    }
    //AccessibilityInteractionClient client = AccessibilityInteractionClient.getInstance();
    return nullptr;//client.getWindow(mConnectionId, mParentId);
}

void AccessibilityWindowInfo::setParentId(int parentId) {
    mParentId = parentId;
}

int AccessibilityWindowInfo::getId() const{
    return mId;
}

void AccessibilityWindowInfo::setId(int id) {
    mId = id;
}

void AccessibilityWindowInfo::setConnectionId(int connectionId) {
    mConnectionId = connectionId;
}

void AccessibilityWindowInfo::getBoundsInScreen(Rect& outBounds) const{
    outBounds = mBoundsInScreen;
}

void AccessibilityWindowInfo::setBoundsInScreen(const Rect& bounds) {
    mBoundsInScreen = bounds;
}

bool AccessibilityWindowInfo::isActive() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_ACTIVE);
}

void AccessibilityWindowInfo::setActive(bool active) {
    setBooleanProperty(BOOLEAN_PROPERTY_ACTIVE, active);
}

bool AccessibilityWindowInfo::isFocused() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_FOCUSED);
}

void AccessibilityWindowInfo::setFocused(bool focused) {
    setBooleanProperty(BOOLEAN_PROPERTY_FOCUSED, focused);
}

bool AccessibilityWindowInfo::isAccessibilityFocused() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_ACCESSIBILITY_FOCUSED);
}

void AccessibilityWindowInfo::setAccessibilityFocused(bool focused) {
    setBooleanProperty(BOOLEAN_PROPERTY_ACCESSIBILITY_FOCUSED, focused);
}

int AccessibilityWindowInfo::getChildCount() const{
    return mChildIds.size();
}

AccessibilityWindowInfo* AccessibilityWindowInfo::getChild(int index) {
    if (mChildIds.empty()) {
        throw std::out_of_range("index is out of range");
    }
    if (mConnectionId == UNDEFINED_WINDOW_ID) {
        return nullptr;
    }
    const int childId = (int) mChildIds.at(index);
    //AccessibilityInteractionClient client = AccessibilityInteractionClient.getInstance();
    return nullptr;//client.getWindow(mConnectionId, childId);
}

void AccessibilityWindowInfo::addChild(int childId) {
    mChildIds.push_back(childId);
}

AccessibilityWindowInfo* AccessibilityWindowInfo::obtain() {
    AccessibilityWindowInfo* info = sPool.acquire();
    if (info == nullptr) {
        info = new AccessibilityWindowInfo();
    }
    sNumInstancesInUse ++;
    return info;
}

AccessibilityWindowInfo* AccessibilityWindowInfo::obtain(const AccessibilityWindowInfo& info) {
    AccessibilityWindowInfo* infoClone = obtain();

    infoClone->mType = info.mType;
    infoClone->mLayer = info.mLayer;
    infoClone->mBooleanProperties = info.mBooleanProperties;
    infoClone->mId = info.mId;
    infoClone->mParentId = info.mParentId;
    infoClone->mBoundsInScreen = info.mBoundsInScreen;
    infoClone->mTitle = info.mTitle;
    infoClone->mAnchorId = info.mAnchorId;

    if (info.mChildIds.size() > 0) {
        if (infoClone->mChildIds.empty()) {
            infoClone->mChildIds = info.mChildIds;
        } else {
            auto &t=info.mChildIds;
            infoClone->mChildIds.insert(infoClone->mChildIds.end(),t.begin(),t.end());
        }
    }

    infoClone->mConnectionId = info.mConnectionId;

    return infoClone;
}

void AccessibilityWindowInfo::setNumInstancesInUseCounter(int counter) {
    sNumInstancesInUse = counter;
}

void AccessibilityWindowInfo::recycle() {
    clear();
    sPool.release(this);
    sNumInstancesInUse--;
}

/*int AccessibilityWindowInfo::describeContents() {
    return 0;
}*/

void AccessibilityWindowInfo::writeToParcel(Parcel parcel, int flags) {
#if 0
    parcel.writeInt(mType);
    parcel.writeInt(mLayer);
    parcel.writeInt(mBooleanProperties);
    parcel.writeInt(mId);
    parcel.writeInt(mParentId);
    mBoundsInScreen.writeToParcel(parcel, flags);
    parcel.writeString(mTitle);
    parcel.writeLong(mAnchorId);

    auto childIds = mChildIds;
    if (childIds.empty()) {
        parcel.writeInt(0);
    } else {
        const int childCount = childIds.size();
        parcel.writeInt(childCount);
        for (int i = 0; i < childCount; i++) {
            parcel.writeInt((int) childIds.at(i));
        }
    }

    parcel.writeInt(mConnectionId);
#endif
}

void AccessibilityWindowInfo::initFromParcel(Parcel parcel) {
#if 0
    mType = parcel.readInt();
    mLayer = parcel.readInt();
    mBooleanProperties = parcel.readInt();
    mId = parcel.readInt();
    mParentId = parcel.readInt();
    mBoundsInScreen.readFromParcel(parcel);
    mTitle = parcel.readString();
    mAnchorId = parcel.readLong();

    const int childCount = parcel.readInt();
    if (childCount > 0) {
        for (int i = 0; i < childCount; i++) {
            const int childId = parcel.readInt();
            mChildIds.push_back(childId);
        }
    }
    mConnectionId = parcel.readInt();
#endif
}

#if 0
int AccessibilityWindowInfo::hashCode() {
    return mId;
}

bool AccessibilityWindowInfo::equals(Object obj) {
    if (this == obj) {
        return true;
    }
    if (obj == null) {
        return false;
    }
    if (getClass() != obj.getClass()) {
        return false;
    }
    AccessibilityWindowInfo other = (AccessibilityWindowInfo) obj;
    return (mId == other.mId);
}

std::string AccessibilityWindowInfo::toString() {
    StringBuilder builder = new StringBuilder();
    builder.append("AccessibilityWindowInfo[");
    builder.append("title=").append(mTitle);
    builder.append(", id=").append(mId);
    builder.append(", type=").append(typeToString(mType));
    builder.append(", layer=").append(mLayer);
    builder.append(", bounds=").append(mBoundsInScreen);
    builder.append(", focused=").append(isFocused());
    builder.append(", active=").append(isActive());
    builder.append(", pictureInPicture=").append(isInPictureInPictureMode());
    if (DEBUG) {
        builder.append(", parent=").append(mParentId);
        builder.append(", children=[");
        if (mChildIds != null) {
            final int childCount = mChildIds.size();
            for (int i = 0; i < childCount; i++) {
                builder.append(mChildIds.get(i));
                if (i < childCount - 1) {
                    builder.append(',');
                }
            }
        } else {
            builder.append("null");
        }
        builder.append(']');
    } else {
        builder.append(", hasParent=").append(mParentId != UNDEFINED_WINDOW_ID);
        builder.append(", isAnchored=")
                .append(mAnchorId != AccessibilityNodeInfo::UNDEFINED_NODE_ID);
        builder.append(", hasChildren=").append(mChildIds != null
                && mChildIds.size() > 0);
    }
    builder.append(']');
    return builder.toString();
}
#endif
/**
 * Clears the internal state.
 */
void AccessibilityWindowInfo::clear() {
    mType = UNDEFINED_WINDOW_ID;
    mLayer = UNDEFINED_WINDOW_ID;
    mBooleanProperties = 0;
    mId = UNDEFINED_WINDOW_ID;
    mParentId = UNDEFINED_WINDOW_ID;
    mBoundsInScreen.setEmpty();
    mChildIds.clear();
    mConnectionId = UNDEFINED_WINDOW_ID;
    mAnchorId = AccessibilityNodeInfo::UNDEFINED_NODE_ID;
    mTitle.clear();
}

bool AccessibilityWindowInfo::getBooleanProperty(int property) const{
    return (mBooleanProperties & property) != 0;
}

void AccessibilityWindowInfo::setBooleanProperty(int property, bool value) {
    if (value) {
        mBooleanProperties |= property;
    } else {
        mBooleanProperties &= ~property;
    }
}

std::string AccessibilityWindowInfo::typeToString(int type) {
    switch (type) {
    case TYPE_APPLICATION: return "TYPE_APPLICATION";
    case TYPE_INPUT_METHOD:return "TYPE_INPUT_METHOD";
    case TYPE_SYSTEM:      return "TYPE_SYSTEM";
    case TYPE_ACCESSIBILITY_OVERLAY:return "TYPE_ACCESSIBILITY_OVERLAY";
    case TYPE_SPLIT_SCREEN_DIVIDER: return "TYPE_SPLIT_SCREEN_DIVIDER";
    default:  return "<UNKNOWN>";
    }
}

bool AccessibilityWindowInfo::changed(const AccessibilityWindowInfo& other){
    if (other.mId != mId) {
        throw std::logic_error("Not same window.");
    }
    if (other.mType != mType) {
        throw std::logic_error("Not same type.");
    }
    if (mBoundsInScreen!=other.mBoundsInScreen) {
        return true;
    }
    if (mLayer != other.mLayer) {
        return true;
    }
    if (mBooleanProperties != other.mBooleanProperties) {
        return true;
    }
    if (mParentId != other.mParentId) {
        return true;
    }
    if (mChildIds.empty()) {
        if (other.mChildIds.size()) {
            return true;
        }
    } else if (mChildIds!=other.mChildIds) {
        return true;
    }
    return false;
}

int AccessibilityWindowInfo::differenceFrom(const AccessibilityWindowInfo& other) {
    if (other.mId != mId) {
        throw std::logic_error("Not same window.");
    }
    if (other.mType != mType) {
        throw std::logic_error("Not same type.");
    }
    int changes = 0;
    if (mTitle!=other.mTitle) {
        changes |= AccessibilityEvent::WINDOWS_CHANGE_TITLE;
    }

    if (mBoundsInScreen!=other.mBoundsInScreen) {
        changes |= AccessibilityEvent::WINDOWS_CHANGE_BOUNDS;
    }
    if (mLayer != other.mLayer) {
        changes |= AccessibilityEvent::WINDOWS_CHANGE_LAYER;
    }
    if (getBooleanProperty(BOOLEAN_PROPERTY_ACTIVE)
            != other.getBooleanProperty(BOOLEAN_PROPERTY_ACTIVE)) {
        changes |= AccessibilityEvent::WINDOWS_CHANGE_ACTIVE;
    }
    if (getBooleanProperty(BOOLEAN_PROPERTY_FOCUSED)
            != other.getBooleanProperty(BOOLEAN_PROPERTY_FOCUSED)) {
        changes |= AccessibilityEvent::WINDOWS_CHANGE_FOCUSED;
    }
    if (getBooleanProperty(BOOLEAN_PROPERTY_ACCESSIBILITY_FOCUSED)
            != other.getBooleanProperty(BOOLEAN_PROPERTY_ACCESSIBILITY_FOCUSED)) {
        changes |= AccessibilityEvent::WINDOWS_CHANGE_ACCESSIBILITY_FOCUSED;
    }
    if (getBooleanProperty(BOOLEAN_PROPERTY_PICTURE_IN_PICTURE)
            != other.getBooleanProperty(BOOLEAN_PROPERTY_PICTURE_IN_PICTURE)) {
        changes |= AccessibilityEvent::WINDOWS_CHANGE_PIP;
    }
    if (mParentId != other.mParentId) {
        changes |= AccessibilityEvent::WINDOWS_CHANGE_PARENT;
    }
    if (mChildIds!=other.mChildIds) {
        changes |= AccessibilityEvent::WINDOWS_CHANGE_CHILDREN;
    }
    return changes;
}
}/*endof namespace*/

