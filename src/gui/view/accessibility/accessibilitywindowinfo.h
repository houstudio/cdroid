#ifndef __ACCESSIBILITY_WINDOW_INFO_H__
#define __ACCESSIBILITY_WINDOW_INFO_H__
#include <string>
#include <vector>
#include <core/pools.h>
#include <view/accessibility/accessibilitynodeinfo.h>
namespace cdroid{
class AccessibilityWindowInfo{// implements Parcelable {
private:
    static constexpr bool Debug = false;
    static constexpr int BOOLEAN_PROPERTY_ACTIVE = 1 << 0;
    static constexpr int BOOLEAN_PROPERTY_FOCUSED = 1 << 1;
    static constexpr int BOOLEAN_PROPERTY_ACCESSIBILITY_FOCUSED = 1 << 2;
    static constexpr int BOOLEAN_PROPERTY_PICTURE_IN_PICTURE = 1 << 3;
public:
    static constexpr int TYPE_APPLICATION = 1;

    static constexpr int TYPE_INPUT_METHOD = 2;

    static constexpr int TYPE_SYSTEM = 3;

    static constexpr int TYPE_ACCESSIBILITY_OVERLAY = 4;

    static constexpr int TYPE_SPLIT_SCREEN_DIVIDER = 5;

    /* Special values for window IDs */
    static constexpr int ACTIVE_WINDOW_ID = INT_MAX;
    static constexpr int UNDEFINED_WINDOW_ID = -1;
    static constexpr int ANY_WINDOW_ID = -2;
    static constexpr int PICTURE_IN_PICTURE_ACTION_REPLACER_WINDOW_ID = -3;
private:

    // Housekeeping.
    static constexpr int MAX_POOL_SIZE = 10;
    static Pools::SimplePool<AccessibilityWindowInfo> sPool;
    static int sNumInstancesInUse;

    // Data.
    int mType = UNDEFINED_WINDOW_ID;
    int mLayer = UNDEFINED_WINDOW_ID;
    int mBooleanProperties;
    int mId = UNDEFINED_WINDOW_ID;
    int mParentId = UNDEFINED_WINDOW_ID;
    int mConnectionId = UNDEFINED_WINDOW_ID;
    int64_t mAnchorId;// = AccessibilityNodeInfo::UNDEFINED_NODE_ID;
    Rect mBoundsInScreen;
    std::vector<long> mChildIds;
    std::string mTitle;

private:
    AccessibilityWindowInfo();
public:
    /**
     * Gets the title of the window.
     *
     * @return The title of the window, or {@code null} if none is available.
     */
    std::string getTitle()const;

    void setTitle(const std::string& title);

    int getType() const;

    void setType(int type);

    int getLayer() const;

    void setLayer(int layer);

    AccessibilityNodeInfo* getRoot();

    void setAnchorId(long anchorId);

    AccessibilityNodeInfo* getAnchor();

    void setPictureInPicture(bool pictureInPicture);

    bool isInPictureInPictureMode() const;

    AccessibilityWindowInfo* getParent();

    void setParentId(int parentId);

    int getId() const;

    void setId(int id);

    void setConnectionId(int connectionId);

    void getBoundsInScreen(Rect& outBounds) const;

    void setBoundsInScreen(const Rect& bounds);

    bool isActive() const;

    void setActive(bool active);

    bool isFocused() const;
   
    void setFocused(bool focused);

    bool isAccessibilityFocused()const;

    void setAccessibilityFocused(bool focused);

    int getChildCount() const;

    AccessibilityWindowInfo* getChild(int index);

    void addChild(int childId);

    static AccessibilityWindowInfo* obtain();

    static AccessibilityWindowInfo* obtain(const AccessibilityWindowInfo& info);

    static void setNumInstancesInUseCounter(int counter);

    void recycle();

    void writeToParcel(Parcel parcel, int flags);
    int hashCode();

    //bool equals(Object obj);
private:
    void initFromParcel(Parcel parcel);
    
    void clear();
  
    bool getBooleanProperty(int property) const;

    void setBooleanProperty(int property, bool value);
    static std::string typeToString(int type);
public:
    bool changed(const AccessibilityWindowInfo& other);

    int differenceFrom(const AccessibilityWindowInfo& other);
};
}/*endof namspace*/
#endif/*__ACCESSIBILITY_WINDOW_INFO_H__*/

