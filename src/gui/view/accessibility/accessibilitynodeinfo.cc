#include <view/accessibility/accessibilitynodeinfo.h>
#include <view/accessibility/accessibilitywindowinfo.h>
#include <view/view.h>
#include <core/bitset.h>
#include <utils/mathutils.h>
#include <widget/R.h>

namespace cdroid{
Pools::SimplePool<AccessibilityNodeInfo> AccessibilityNodeInfo::sPool(MAX_POOL_SIZE);
int AccessibilityNodeInfo::sNumInstancesInUse = 0;
const AccessibilityNodeInfo AccessibilityNodeInfo::DEFAULT;

int AccessibilityNodeInfo::getAccessibilityViewId(long accessibilityNodeId){
    return (int) accessibilityNodeId;
}

int AccessibilityNodeInfo::getVirtualDescendantId(long accessibilityNodeId){
    return (int) ((accessibilityNodeId & VIRTUAL_DESCENDANT_ID_MASK) >> VIRTUAL_DESCENDANT_ID_SHIFT);
}

int64_t AccessibilityNodeInfo::makeNodeId(int accessibilityViewId, int virtualDescendantId){
    return (int64_t(virtualDescendantId) << VIRTUAL_DESCENDANT_ID_SHIFT) | accessibilityViewId;
}

AccessibilityNodeInfo::AccessibilityNodeInfo() {
    /* do nothing */
    mRangeInfo = nullptr;
    mCollectionInfo = nullptr;
    mCollectionItemInfo = nullptr;
}

void AccessibilityNodeInfo::setSource(View* source) {
    setSource(source, AccessibilityNodeProvider::HOST_VIEW_ID);
}

void AccessibilityNodeInfo::setSource(View* root, int virtualDescendantId) {
    enforceNotSealed();
    mWindowId = root ? root->getAccessibilityWindowId() : UNDEFINED_ITEM_ID;
    const int rootAccessibilityViewId = root ? root->getAccessibilityViewId() : UNDEFINED_ITEM_ID;
    mSourceNodeId = makeNodeId(rootAccessibilityViewId, virtualDescendantId);
}

AccessibilityNodeInfo* AccessibilityNodeInfo::findFocus(int focus) {
    enforceSealed();
    enforceValidFocusType(focus);
    if (!canPerformRequestOverConnection(mSourceNodeId)) {
        return nullptr;
    }
    return nullptr;
    //return AccessibilityInteractionClient.getInstance().findFocus(mConnectionId, mWindowId,mSourceNodeId, focus);
}

AccessibilityNodeInfo* AccessibilityNodeInfo::focusSearch(int direction) {
    enforceSealed();
    enforceValidFocusDirection(direction);
    if (!canPerformRequestOverConnection(mSourceNodeId)) {
        return nullptr;
    }
    return nullptr;
    //return AccessibilityInteractionClient.getInstance().focusSearch(mConnectionId, mWindowId,mSourceNodeId, direction);
}

int AccessibilityNodeInfo::getWindowId() const{
    return mWindowId;
}

bool AccessibilityNodeInfo::refresh(Bundle* arguments, bool bypassCache) {
    enforceSealed();
    if (!canPerformRequestOverConnection(mSourceNodeId)) {
        return false;
    }
    AccessibilityNodeInfo* refreshedInfo = nullptr;
    //AccessibilityInteractionClient client = AccessibilityInteractionClient.getInstance();
    //refreshedInfo =client.findAccessibilityNodeInfoByAccessibilityId(mConnectionId, mWindowId, mSourceNodeId, bypassCache, 0, arguments);
    if (refreshedInfo == nullptr) {
        return false;
    }
    // Hard-to-reproduce bugs seem to be due to some tools recycling a node on another
    // thread. If that happens, the init will re-seal the node, which then is in a bad state
    // when it is obtained. Enforce sealing again before we init to fail when a node has been
    // recycled during a refresh to catch such errors earlier.
    enforceSealed();
    init(*refreshedInfo);
    refreshedInfo->recycle();
    return true;
}

bool AccessibilityNodeInfo::refresh() {
    return refresh(nullptr, true);
}

bool AccessibilityNodeInfo::refreshWithExtraData(const std::string& extraDataKey, Bundle* args) {
    //args.putString(EXTRA_DATA_REQUESTED_KEY, extraDataKey);
    return refresh(args, true);
}

std::vector<long> AccessibilityNodeInfo::getChildNodeIds() const{
    return mChildNodeIds;
}

long AccessibilityNodeInfo::getChildId(int index) const{
    if (mChildNodeIds.empty()) {
        throw std::out_of_range("IndexOutOfBoundsException");
    }
    return mChildNodeIds.at(index);
}

int AccessibilityNodeInfo::getChildCount() const{
    return mChildNodeIds.size();
}

AccessibilityNodeInfo* AccessibilityNodeInfo::getChild(int index) {
    enforceSealed();
    if (mChildNodeIds.empty()) {
        return nullptr;
    }
    if (!canPerformRequestOverConnection(mSourceNodeId)) {
        return nullptr;
    }
    const long childId = mChildNodeIds.at(index);
    //AccessibilityInteractionClient client = AccessibilityInteractionClient.getInstance();
    /*return client.findAccessibilityNodeInfoByAccessibilityId(mConnectionId, mWindowId,
            childId, false, FLAG_PREFETCH_DESCENDANTS, nullptr);*/
    return nullptr;
}

void AccessibilityNodeInfo::addChild(View* child) {
    addChildInternal(child, AccessibilityNodeProvider::HOST_VIEW_ID, true);
}

void AccessibilityNodeInfo::addChildUnchecked(View* child) {
    addChildInternal(child, AccessibilityNodeProvider::HOST_VIEW_ID, false);
}

bool AccessibilityNodeInfo::removeChild(View* child) {
    return removeChild(child, AccessibilityNodeProvider::HOST_VIEW_ID);
}

void AccessibilityNodeInfo::addChild(View* root, int virtualDescendantId) {
    addChildInternal(root, virtualDescendantId, true);
}

void AccessibilityNodeInfo::addChildInternal(View* root, int virtualDescendantId, bool checked) {
    enforceNotSealed();
    const int rootAccessibilityViewId =root? root->getAccessibilityViewId() : UNDEFINED_ITEM_ID;
    const long childNodeId = makeNodeId(rootAccessibilityViewId, virtualDescendantId);
    // If we're checking uniqueness and the ID already exists, abort.
    auto it = std::find(mChildNodeIds.begin(),mChildNodeIds.end(),childNodeId);
    if (checked && (it!=mChildNodeIds.end())) {
        return;
    }
    mChildNodeIds.push_back(childNodeId);
}

bool AccessibilityNodeInfo::removeChild(View* root, int virtualDescendantId) {
    enforceNotSealed();
    if (mChildNodeIds.empty()) {
        return false;
    }
    const int rootAccessibilityViewId =root ? root->getAccessibilityViewId() : UNDEFINED_ITEM_ID;
    const long childNodeId = makeNodeId(rootAccessibilityViewId, virtualDescendantId);
    auto it = std::find(mChildNodeIds.begin(),mChildNodeIds.end(),childNodeId);
    if (it==mChildNodeIds.end()) {
        return false;
    }
    mChildNodeIds.erase(it);
    return true;
}

std::vector<AccessibilityNodeInfo::AccessibilityAction*> AccessibilityNodeInfo::getActionList() {
    return mActions;//CollectionUtils.emptyIfNull(mActions);
}

int AccessibilityNodeInfo::getActions() {
    int returnValue = 0;

    if (mActions.empty()) {
        return returnValue;
    }

    const int actionSize = mActions.size();
    for (int i = 0; i < actionSize; i++) {
        const int actionId = mActions.at(i)->getId();
        if (actionId <= LAST_LEGACY_STANDARD_ACTION) {
            returnValue |= actionId;
        }
    }

    return returnValue;
}

void AccessibilityNodeInfo::addAction(AccessibilityAction* action) {
    enforceNotSealed();

    addActionUnchecked(action);
}

void AccessibilityNodeInfo::addActionUnchecked(AccessibilityAction* action) {
    if (action == nullptr) {
        return;
    }
    auto it =std::find(mActions.begin(),mActions.end(),action);
    if(it!=mActions.end())mActions.erase(it);// mActions.remove(action);
    mActions.push_back(action);
}

void AccessibilityNodeInfo::addAction(int action) {
    enforceNotSealed();

    if ((action & ACTION_TYPE_MASK) != 0) {
        throw std::runtime_error("Action is not a combination of the standard actions: ");//,action);
    }

    addStandardActions(action);
}

void AccessibilityNodeInfo::removeAction(int action) {
    enforceNotSealed();

    removeAction(getActionSingleton(action));
}

bool AccessibilityNodeInfo::removeAction(AccessibilityAction* action) {
    enforceNotSealed();

    if (mActions.empty() || (action == nullptr) ){
        return false;
    }
    auto it =std::find(mActions.begin(),mActions.end(),action);
    if(it==mActions.end()){
        mActions.erase(it);
        return true;
    }
    return false;
}

void AccessibilityNodeInfo::removeAllActions() {
    mActions.clear();
}

AccessibilityNodeInfo* AccessibilityNodeInfo::getTraversalBefore() {
    enforceSealed();
    return getNodeForAccessibilityId(mTraversalBefore);
}

void AccessibilityNodeInfo::setTraversalBefore(View* view) {
    setTraversalBefore(view, AccessibilityNodeProvider::HOST_VIEW_ID);
}

void AccessibilityNodeInfo::setTraversalBefore(View* root, int virtualDescendantId) {
    enforceNotSealed();
    const int rootAccessibilityViewId = root ? root->getAccessibilityViewId() : UNDEFINED_ITEM_ID;
    mTraversalBefore = makeNodeId(rootAccessibilityViewId, virtualDescendantId);
}

AccessibilityNodeInfo* AccessibilityNodeInfo::getTraversalAfter() {
    enforceSealed();
    return getNodeForAccessibilityId(mTraversalAfter);
}

void AccessibilityNodeInfo::setTraversalAfter(View* view) {
    setTraversalAfter(view, AccessibilityNodeProvider::HOST_VIEW_ID);
}

void AccessibilityNodeInfo::setTraversalAfter(View* root, int virtualDescendantId) {
    enforceNotSealed();
    const int rootAccessibilityViewId = root ? root->getAccessibilityViewId() : UNDEFINED_ITEM_ID;
    mTraversalAfter = makeNodeId(rootAccessibilityViewId, virtualDescendantId);
}

std::vector<std::string> AccessibilityNodeInfo::getAvailableExtraData() {
    return mExtraDataKeys;
}

void AccessibilityNodeInfo::setAvailableExtraData(const std::vector<std::string>& extraDataKeys) {
    enforceNotSealed();
    mExtraDataKeys = extraDataKeys;
}

void AccessibilityNodeInfo::setMaxTextLength(int max) {
    enforceNotSealed();
    mMaxTextLength = max;
}

int AccessibilityNodeInfo::getMaxTextLength() const{
    return mMaxTextLength;
}

void AccessibilityNodeInfo::setMovementGranularities(int granularities) {
    enforceNotSealed();
    mMovementGranularities = granularities;
}

int AccessibilityNodeInfo::getMovementGranularities() const{
    return mMovementGranularities;
}

bool AccessibilityNodeInfo::performAction(int action) {
    enforceSealed();
    if (!canPerformRequestOverConnection(mSourceNodeId)) {
        return false;
    }
    return true;
    //AccessibilityInteractionClient client = AccessibilityInteractionClient.getInstance();
    //return client.performAccessibilityAction(mConnectionId, mWindowId, mSourceNodeId,action, nullptr);
}

bool AccessibilityNodeInfo::performAction(int action, Bundle* arguments) {
    enforceSealed();
    if (!canPerformRequestOverConnection(mSourceNodeId)) {
        return false;
    }
    return true;
    //AccessibilityInteractionClient client = AccessibilityInteractionClient.getInstance();
    //return client.performAccessibilityAction(mConnectionId, mWindowId, mSourceNodeId,action, arguments);
}

std::vector<AccessibilityNodeInfo*> AccessibilityNodeInfo::findAccessibilityNodeInfosByText(const std::string& text) {
    enforceSealed();
    if (!canPerformRequestOverConnection(mSourceNodeId)) {
        return std::vector<AccessibilityNodeInfo*>();
    }
    return std::vector<AccessibilityNodeInfo*>();
    //AccessibilityInteractionClient client = AccessibilityInteractionClient.getInstance();
    //return client.findAccessibilityNodeInfosByText(mConnectionId, mWindowId, mSourceNodeId,text);
}

std::vector<AccessibilityNodeInfo*> AccessibilityNodeInfo::findAccessibilityNodeInfosByViewId(const std::string& viewId) {
    enforceSealed();
    if (!canPerformRequestOverConnection(mSourceNodeId)) {
        return std::vector<AccessibilityNodeInfo*>();
    }
    return std::vector<AccessibilityNodeInfo*>();
    //AccessibilityInteractionClient client = AccessibilityInteractionClient.getInstance();
    //return client.findAccessibilityNodeInfosByViewId(mConnectionId, mWindowId, mSourceNodeId,viewId);
}

AccessibilityWindowInfo* AccessibilityNodeInfo::getWindow() {
    enforceSealed();
    if (!canPerformRequestOverConnection(mSourceNodeId)) {
        return nullptr;
    }
    return nullptr;
    //AccessibilityInteractionClient client = AccessibilityInteractionClient.getInstance();
    //return client.getWindow(mConnectionId, mWindowId);
}

AccessibilityNodeInfo* AccessibilityNodeInfo::getParent() {
    enforceSealed();
    return getNodeForAccessibilityId(mParentNodeId);
}

long AccessibilityNodeInfo::getParentNodeId() const{
    return mParentNodeId;
}

void AccessibilityNodeInfo::setParent(View* parent) {
    setParent(parent, AccessibilityNodeProvider::HOST_VIEW_ID);
}

void AccessibilityNodeInfo::setParent(View* root, int virtualDescendantId) {
    enforceNotSealed();
    const int rootAccessibilityViewId = root  ? root->getAccessibilityViewId() : UNDEFINED_ITEM_ID;
    mParentNodeId = makeNodeId(rootAccessibilityViewId, virtualDescendantId);
}

void AccessibilityNodeInfo::getBoundsInParent(Rect& outBounds) const{
    outBounds = mBoundsInParent;
}

void AccessibilityNodeInfo::setBoundsInParent(const Rect& bounds) {
    enforceNotSealed();
    mBoundsInParent =bounds;
}

void AccessibilityNodeInfo::getBoundsInScreen(Rect& outBounds) const{
    outBounds = mBoundsInScreen;
}

Rect AccessibilityNodeInfo::getBoundsInScreen() const{
    return mBoundsInScreen;
}

void AccessibilityNodeInfo::setBoundsInScreen(const Rect& bounds) {
    enforceNotSealed();
    mBoundsInScreen = bounds;
}

bool AccessibilityNodeInfo::isCheckable() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_CHECKABLE);
}

void AccessibilityNodeInfo::setCheckable(bool checkable) {
    setBooleanProperty(BOOLEAN_PROPERTY_CHECKABLE, checkable);
}

bool AccessibilityNodeInfo::isChecked() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_CHECKED);
}

void AccessibilityNodeInfo::setChecked(bool checked) {
    setBooleanProperty(BOOLEAN_PROPERTY_CHECKED, checked);
}

bool AccessibilityNodeInfo::isFocusable() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_FOCUSABLE);
}

void AccessibilityNodeInfo::setFocusable(bool focusable) {
    setBooleanProperty(BOOLEAN_PROPERTY_FOCUSABLE, focusable);
}

bool AccessibilityNodeInfo::isFocused() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_FOCUSED);
}

void AccessibilityNodeInfo::setFocused(bool focused) {
    setBooleanProperty(BOOLEAN_PROPERTY_FOCUSED, focused);
}

bool AccessibilityNodeInfo::isVisibleToUser() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_VISIBLE_TO_USER);
}

void AccessibilityNodeInfo::setVisibleToUser(bool visibleToUser) {
    setBooleanProperty(BOOLEAN_PROPERTY_VISIBLE_TO_USER, visibleToUser);
}

bool AccessibilityNodeInfo::isAccessibilityFocused() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_ACCESSIBILITY_FOCUSED);
}

void AccessibilityNodeInfo::setAccessibilityFocused(bool focused) {
    setBooleanProperty(BOOLEAN_PROPERTY_ACCESSIBILITY_FOCUSED, focused);
}

bool AccessibilityNodeInfo::isSelected() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_SELECTED);
}

void AccessibilityNodeInfo::setSelected(bool selected) {
    setBooleanProperty(BOOLEAN_PROPERTY_SELECTED, selected);
}

bool AccessibilityNodeInfo::isClickable() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_CLICKABLE);
}

void AccessibilityNodeInfo::setClickable(bool clickable) {
    setBooleanProperty(BOOLEAN_PROPERTY_CLICKABLE, clickable);
}

bool AccessibilityNodeInfo::isLongClickable() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_LONG_CLICKABLE);
}

void AccessibilityNodeInfo::setLongClickable(bool longClickable) {
    setBooleanProperty(BOOLEAN_PROPERTY_LONG_CLICKABLE, longClickable);
}

bool AccessibilityNodeInfo::isEnabled() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_ENABLED);
}

void AccessibilityNodeInfo::setEnabled(bool enabled) {
    setBooleanProperty(BOOLEAN_PROPERTY_ENABLED, enabled);
}

bool AccessibilityNodeInfo::isPassword() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_PASSWORD);
}

void AccessibilityNodeInfo::setPassword(bool password) {
    setBooleanProperty(BOOLEAN_PROPERTY_PASSWORD, password);
}

bool AccessibilityNodeInfo::isScrollable() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_SCROLLABLE);
}

void AccessibilityNodeInfo::setScrollable(bool scrollable) {
    setBooleanProperty(BOOLEAN_PROPERTY_SCROLLABLE, scrollable);
}

bool AccessibilityNodeInfo::isEditable() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_EDITABLE);
}

void AccessibilityNodeInfo::setEditable(bool editable){
    setBooleanProperty(BOOLEAN_PROPERTY_EDITABLE, editable);
}

void AccessibilityNodeInfo::setPaneTitle(const std::string& paneTitle) {
    enforceNotSealed();
    mPaneTitle = paneTitle;
}

std::string AccessibilityNodeInfo::getPaneTitle() const{
    return mPaneTitle;
}

int AccessibilityNodeInfo::getDrawingOrder() const{
    return mDrawingOrderInParent;
}

void AccessibilityNodeInfo::setDrawingOrder(int drawingOrderInParent) {
    enforceNotSealed();
    mDrawingOrderInParent = drawingOrderInParent;
}

AccessibilityNodeInfo::CollectionInfo* AccessibilityNodeInfo::getCollectionInfo() const{
    return mCollectionInfo;
}

void AccessibilityNodeInfo::setCollectionInfo(CollectionInfo* collectionInfo) {
    enforceNotSealed();
    mCollectionInfo = collectionInfo;
}

AccessibilityNodeInfo::CollectionItemInfo* AccessibilityNodeInfo::getCollectionItemInfo() const{
    return mCollectionItemInfo;
}

void AccessibilityNodeInfo::setCollectionItemInfo(CollectionItemInfo* collectionItemInfo) {
    enforceNotSealed();
    mCollectionItemInfo = collectionItemInfo;
}

AccessibilityNodeInfo::RangeInfo* AccessibilityNodeInfo::getRangeInfo() const{
    return mRangeInfo;
}

void AccessibilityNodeInfo::setRangeInfo(RangeInfo* rangeInfo) {
    enforceNotSealed();
    mRangeInfo = rangeInfo;
}

bool AccessibilityNodeInfo::isContentInvalid() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_CONTENT_INVALID);
}

void AccessibilityNodeInfo::setContentInvalid(bool contentInvalid) {
    setBooleanProperty(BOOLEAN_PROPERTY_CONTENT_INVALID, contentInvalid);
}

bool AccessibilityNodeInfo::isContextClickable() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_CONTEXT_CLICKABLE);
}

void AccessibilityNodeInfo::setContextClickable(bool contextClickable) {
    setBooleanProperty(BOOLEAN_PROPERTY_CONTEXT_CLICKABLE, contextClickable);
}

int AccessibilityNodeInfo::getLiveRegion() const{
    return mLiveRegion;
}

void AccessibilityNodeInfo::setLiveRegion(int mode) {
    enforceNotSealed();
    mLiveRegion = mode;
}

bool AccessibilityNodeInfo::isMultiLine() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_MULTI_LINE);
}

void AccessibilityNodeInfo::setMultiLine(bool multiLine) {
    setBooleanProperty(BOOLEAN_PROPERTY_MULTI_LINE, multiLine);
}

bool AccessibilityNodeInfo::canOpenPopup() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_OPENS_POPUP);
}

void AccessibilityNodeInfo::setCanOpenPopup(bool opensPopup) {
    enforceNotSealed();
    setBooleanProperty(BOOLEAN_PROPERTY_OPENS_POPUP, opensPopup);
}

bool AccessibilityNodeInfo::isDismissable() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_DISMISSABLE);
}

void AccessibilityNodeInfo::setDismissable(bool dismissable) {
    setBooleanProperty(BOOLEAN_PROPERTY_DISMISSABLE, dismissable);
}

bool AccessibilityNodeInfo::isImportantForAccessibility() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_IMPORTANCE);
}

void AccessibilityNodeInfo::setImportantForAccessibility(bool important) {
    setBooleanProperty(BOOLEAN_PROPERTY_IMPORTANCE, important);
}

bool AccessibilityNodeInfo::isScreenReaderFocusable() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_SCREEN_READER_FOCUSABLE);
}

void AccessibilityNodeInfo::setScreenReaderFocusable(bool screenReaderFocusable) {
    setBooleanProperty(BOOLEAN_PROPERTY_SCREEN_READER_FOCUSABLE, screenReaderFocusable);
}

bool AccessibilityNodeInfo::isShowingHintText() const{
    return getBooleanProperty(BOOLEAN_PROPERTY_IS_SHOWING_HINT);
}

void AccessibilityNodeInfo::setShowingHintText(bool showingHintText) {
    setBooleanProperty(BOOLEAN_PROPERTY_IS_SHOWING_HINT, showingHintText);
}

bool AccessibilityNodeInfo::isHeading() const{
    if (getBooleanProperty(BOOLEAN_PROPERTY_IS_HEADING)) return true;
    CollectionItemInfo* itemInfo = getCollectionItemInfo();
    return (itemInfo && itemInfo->mHeading);
}

void AccessibilityNodeInfo::setHeading(bool isHeading) {
    setBooleanProperty(BOOLEAN_PROPERTY_IS_HEADING, isHeading);
}

std::string AccessibilityNodeInfo::getPackageName() const{
    return mPackageName;
}

void AccessibilityNodeInfo::setPackageName(const std::string& packageName) {
    enforceNotSealed();
    mPackageName = packageName;
}

std::string AccessibilityNodeInfo::getClassName() const{
    return mClassName;
}

void AccessibilityNodeInfo::setClassName(const std::string& className) {
    enforceNotSealed();
    mClassName = className;
}

std::string AccessibilityNodeInfo::getText() const{
    // Attach this node to any spans that need it
#if 0
    if (mText instanceof Spanned) {
        Spanned spanned = (Spanned) mText;
        AccessibilityClickableSpan[] clickableSpans =
                spanned.getSpans(0, mText.length(), AccessibilityClickableSpan.class);
        for (int i = 0; i < clickableSpans.length; i++) {
            clickableSpans[i].copyConnectionDataFrom(this);
        }
        AccessibilityURLSpan[] urlSpans =
                spanned.getSpans(0, mText.length(), AccessibilityURLSpan.class);
        for (int i = 0; i < urlSpans.length; i++) {
            urlSpans[i].copyConnectionDataFrom(this);
        }
    }
#endif
    return mText;
}

std::string AccessibilityNodeInfo::getOriginalText()const {
    return mOriginalText;
}

void AccessibilityNodeInfo::setText(const std::string& text) {
    enforceNotSealed();
    mOriginalText = text;
#if 0
    // Replace any ClickableSpans in mText with placeholders
    if (text instanceof Spanned) {
        ClickableSpan[] spans =
                ((Spanned) text).getSpans(0, text.length(), ClickableSpan.class);
        if (spans.length > 0) {
            Spannable spannable = new Spannablestd::stringBuilder(text);
            for (int i = 0; i < spans.length; i++) {
                ClickableSpan span = spans[i];
                if ((span instanceof AccessibilityClickableSpan)
                        || (span instanceof AccessibilityURLSpan)) {
                    // We've already done enough
                    break;
                }
                int spanToReplaceStart = spannable.getSpanStart(span);
                int spanToReplaceEnd = spannable.getSpanEnd(span);
                int spanToReplaceFlags = spannable.getSpanFlags(span);
                spannable.removeSpan(span);
                ClickableSpan replacementSpan = (span instanceof URLSpan)
                        ? new AccessibilityURLSpan((URLSpan) span)
                        : new AccessibilityClickableSpan(span.getId());
                spannable.setSpan(replacementSpan, spanToReplaceStart, spanToReplaceEnd,
                        spanToReplaceFlags);
            }
            mText = spannable;
            return;
        }
    }
#endif
    mText = text;
}

std::string AccessibilityNodeInfo::getHintText() const{
    return mHintText;
}

void AccessibilityNodeInfo::setHintText(const std::string& hintText) {
    enforceNotSealed();
    mHintText = hintText;
}

void AccessibilityNodeInfo::setError(const std::string& error) {
    enforceNotSealed();
    mError = error;
}

std::string AccessibilityNodeInfo::getError() const{
    return mError;
}

std::string AccessibilityNodeInfo::getContentDescription() const{
    return mContentDescription;
}

void AccessibilityNodeInfo::setContentDescription(const std::string& contentDescription) {
    enforceNotSealed();
    mContentDescription = contentDescription;
}

std::string AccessibilityNodeInfo::getTooltipText() const{
    return mTooltipText;
}

void AccessibilityNodeInfo::setTooltipText(const std::string& tooltipText) {
    enforceNotSealed();
    mTooltipText = tooltipText;
}

void AccessibilityNodeInfo::setLabelFor(View* labeled) {
    setLabelFor(labeled, AccessibilityNodeProvider::HOST_VIEW_ID);
}

void AccessibilityNodeInfo::setLabelFor(View* root, int virtualDescendantId) {
    enforceNotSealed();
    const int rootAccessibilityViewId = root ? root->getAccessibilityViewId() : UNDEFINED_ITEM_ID;
    mLabelForId = makeNodeId(rootAccessibilityViewId, virtualDescendantId);
}

AccessibilityNodeInfo* AccessibilityNodeInfo::getLabelFor() {
    enforceSealed();
    return getNodeForAccessibilityId(mLabelForId);
}

void AccessibilityNodeInfo::setLabeledBy(View* label) {
    setLabeledBy(label, AccessibilityNodeProvider::HOST_VIEW_ID);
}

void AccessibilityNodeInfo::setLabeledBy(View* root, int virtualDescendantId) {
    enforceNotSealed();
    const int rootAccessibilityViewId = root ? root->getAccessibilityViewId() : UNDEFINED_ITEM_ID;
    mLabeledById = makeNodeId(rootAccessibilityViewId, virtualDescendantId);
}

AccessibilityNodeInfo* AccessibilityNodeInfo::getLabeledBy() {
    enforceSealed();
    return getNodeForAccessibilityId(mLabeledById);
}

void AccessibilityNodeInfo::setViewIdResourceName(const std::string& viewIdResName) {
    enforceNotSealed();
    mViewIdResourceName = viewIdResName;
}

std::string AccessibilityNodeInfo::getViewIdResourceName() const{
    return mViewIdResourceName;
}

int AccessibilityNodeInfo::getTextSelectionStart() const{
    return mTextSelectionStart;
}

int AccessibilityNodeInfo::getTextSelectionEnd() const{
    return mTextSelectionEnd;
}

void AccessibilityNodeInfo::setTextSelection(int start, int end) {
    enforceNotSealed();
    mTextSelectionStart = start;
    mTextSelectionEnd = end;
}

int AccessibilityNodeInfo::getInputType() const{
    return mInputType;
}

void AccessibilityNodeInfo::setInputType(int inputType) {
    enforceNotSealed();
    mInputType = inputType;
}

Bundle* AccessibilityNodeInfo::getExtras() {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    return mExtras;
}

bool AccessibilityNodeInfo::hasExtras() const{
    return mExtras != nullptr;
}

bool AccessibilityNodeInfo::getBooleanProperty(int property) const{
    return (mBooleanProperties & property) != 0;
}

void AccessibilityNodeInfo::setBooleanProperty(int property, bool value) {
    enforceNotSealed();
    if (value) {
        mBooleanProperties |= property;
    } else {
        mBooleanProperties &= ~property;
    }
}

void AccessibilityNodeInfo::setConnectionId(int connectionId) {
    enforceNotSealed();
    mConnectionId = connectionId;
}

int AccessibilityNodeInfo::getConnectionId() const{
    return mConnectionId;
}

int AccessibilityNodeInfo::describeContents() {
    return 0;
}

void AccessibilityNodeInfo::setSourceNodeId(long sourceId, int windowId) {
    enforceNotSealed();
    mSourceNodeId = sourceId;
    mWindowId = windowId;
}

long AccessibilityNodeInfo::getSourceNodeId() const{
    return mSourceNodeId;
}

void AccessibilityNodeInfo::setSealed(bool sealed) {
    mSealed = sealed;
}

bool AccessibilityNodeInfo::isSealed() const{
    return mSealed;
}

void AccessibilityNodeInfo::enforceSealed() {
    if (!isSealed()) {
        throw std::runtime_error("Cannot perform this action on a not sealed instance.");
    }
}

void AccessibilityNodeInfo::enforceValidFocusDirection(int direction) {
    switch (direction) {
    case View::FOCUS_DOWN:
    case View::FOCUS_UP:
    case View::FOCUS_LEFT:
    case View::FOCUS_RIGHT:
    case View::FOCUS_FORWARD:
    case View::FOCUS_BACKWARD:
        return;
    default:
        LOGE("Unknown direction: %d",direction);
    }
}

void AccessibilityNodeInfo::enforceValidFocusType(int focusType) {
    switch (focusType) {
    case FOCUS_INPUT:
    case FOCUS_ACCESSIBILITY:
        return;
    default:
        throw std::runtime_error("Unknown focus type: + focusType");
    }
}

void AccessibilityNodeInfo::enforceNotSealed() {
    if (isSealed()) {
        throw std::runtime_error("Cannot perform this action on a sealed instance.");
    }
}

AccessibilityNodeInfo* AccessibilityNodeInfo::obtain(View* source) {
    AccessibilityNodeInfo* info = AccessibilityNodeInfo::obtain();
    info->setSource(source);
    return info;
}

AccessibilityNodeInfo* AccessibilityNodeInfo::obtain(View* root, int virtualDescendantId) {
    AccessibilityNodeInfo* info = AccessibilityNodeInfo::obtain();
    info->setSource(root, virtualDescendantId);
    return info;
}

AccessibilityNodeInfo* AccessibilityNodeInfo::obtain() {
    AccessibilityNodeInfo* info = sPool.acquire();
    sNumInstancesInUse++;
    return info ? info : new AccessibilityNodeInfo();
}

AccessibilityNodeInfo* AccessibilityNodeInfo::obtain(const AccessibilityNodeInfo& info) {
    AccessibilityNodeInfo* infoClone = AccessibilityNodeInfo::obtain();
    infoClone->init(info);
    return infoClone;
}

void AccessibilityNodeInfo::recycle() {
    clear();
    sPool.release(this);
    sNumInstancesInUse--;
}

void AccessibilityNodeInfo::setNumInstancesInUseCounter(int counter) {
    sNumInstancesInUse = counter;
}

void AccessibilityNodeInfo::writeToParcel(Parcel parcel, int flags) {
    writeToParcelNoRecycle(parcel, flags);
    // Since instances of this class are fetched via synchronous i.e. blocking
    // calls in IPCs we always recycle as soon as the instance is marshaled.
    recycle();
}
#define bitAt(i) (1L<<i);
void AccessibilityNodeInfo::writeToParcelNoRecycle(Parcel parcel, int flags) {
    // Write bit set of indices of fields with values differing from default
    int64_t nonDefaultFields = 0;
    int fieldIndex = 0; // index of the current field
    if (isSealed() != DEFAULT.isSealed()) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mSourceNodeId != DEFAULT.mSourceNodeId) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mWindowId != DEFAULT.mWindowId) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mParentNodeId != DEFAULT.mParentNodeId) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mLabelForId != DEFAULT.mLabelForId) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mLabeledById != DEFAULT.mLabeledById) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mTraversalBefore != DEFAULT.mTraversalBefore) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mTraversalAfter != DEFAULT.mTraversalAfter) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mConnectionId != DEFAULT.mConnectionId) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (!std::equal(mChildNodeIds.begin(),mChildNodeIds.end(), DEFAULT.mChildNodeIds.begin())) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (mBoundsInParent!=DEFAULT.mBoundsInParent) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (mBoundsInScreen!=DEFAULT.mBoundsInScreen) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (!std::equal(mActions.begin(),mActions.end(), DEFAULT.mActions.begin()))
        nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mMaxTextLength != DEFAULT.mMaxTextLength) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mMovementGranularities != DEFAULT.mMovementGranularities) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (mBooleanProperties != DEFAULT.mBooleanProperties) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mPackageName!=DEFAULT.mPackageName) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (mClassName!=DEFAULT.mClassName) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mText!=DEFAULT.mText) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mHintText!=DEFAULT.mHintText) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (mError!=DEFAULT.mError) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mContentDescription!=DEFAULT.mContentDescription) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (mPaneTitle!=DEFAULT.mPaneTitle) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (mTooltipText!=DEFAULT.mTooltipText) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (mViewIdResourceName!=DEFAULT.mViewIdResourceName) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (mTextSelectionStart != DEFAULT.mTextSelectionStart) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (mTextSelectionEnd != DEFAULT.mTextSelectionEnd) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (mInputType != DEFAULT.mInputType) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mLiveRegion != DEFAULT.mLiveRegion) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (mDrawingOrderInParent != DEFAULT.mDrawingOrderInParent) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (!std::equal(mExtraDataKeys.begin(),mExtraDataKeys.end(), DEFAULT.mExtraDataKeys.begin())) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (mExtras!=DEFAULT.mExtras) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (!(mRangeInfo==DEFAULT.mRangeInfo)) nonDefaultFields |= bitAt(fieldIndex);
    fieldIndex++;
    if (!(mCollectionInfo==DEFAULT.mCollectionInfo)) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    fieldIndex++;
    if (!(mCollectionItemInfo==DEFAULT.mCollectionItemInfo)) {
        nonDefaultFields |= bitAt(fieldIndex);
    }
    int totalFields = fieldIndex;
    parcel.writeLong(nonDefaultFields);
#define isBitSet(v,b) BitSet64::hasBit(v,b)
    fieldIndex = 0;
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeInt(isSealed() ? 1 : 0);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeLong(mSourceNodeId);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeInt(mWindowId);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeLong(mParentNodeId);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeLong(mLabelForId);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeLong(mLabeledById);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeLong(mTraversalBefore);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeLong(mTraversalAfter);

    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeInt(mConnectionId);

    if (isBitSet(nonDefaultFields, fieldIndex++)) {
        auto& childIds = mChildNodeIds;
        if (childIds.empty()) {
            parcel.writeInt(0);
        } else {
            const int childIdsSize = childIds.size();
            parcel.writeInt(childIdsSize);
            for (int i = 0; i < childIdsSize; i++) {
                parcel.writeLong(childIds.at(i));
            }
        }
    }

    if (isBitSet(nonDefaultFields, fieldIndex++)) {
        parcel.writeInt(mBoundsInParent.top);
        parcel.writeInt(mBoundsInParent.height);
        parcel.writeInt(mBoundsInParent.left);
        parcel.writeInt(mBoundsInParent.width);
    }

    if (isBitSet(nonDefaultFields, fieldIndex++)) {
        parcel.writeInt(mBoundsInScreen.top);
        parcel.writeInt(mBoundsInScreen.height);
        parcel.writeInt(mBoundsInScreen.left);
        parcel.writeInt(mBoundsInScreen.width);
    }

    if (isBitSet(nonDefaultFields, fieldIndex++)) {
        if (mActions.empty()) {
            const int actionCount = mActions.size();

            int nonStandardActionCount = 0;
            long defaultStandardActions = 0;
            for (int i = 0; i < actionCount; i++) {
                AccessibilityAction* action = mActions.at(i);
                if (isDefaultStandardAction(action)) {
                    defaultStandardActions |= action->mSerializationFlag;
                } else {
                    nonStandardActionCount++;
                }
            }
            parcel.writeLong(defaultStandardActions);

            parcel.writeInt(nonStandardActionCount);
            for (int i = 0; i < actionCount; i++) {
                AccessibilityAction* action = mActions.at(i);
                if (!isDefaultStandardAction(action)) {
                    parcel.writeInt(action->getId());
                    parcel.writeCharSequence(action->getLabel());
                }
            }
        } else {
            parcel.writeLong(0);
            parcel.writeInt(0);
        }
    }

    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeInt(mMaxTextLength);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeInt(mMovementGranularities);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeInt(mBooleanProperties);

    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeCharSequence(mPackageName);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeCharSequence(mClassName);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeCharSequence(mText);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeCharSequence(mHintText);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeCharSequence(mError);
    if (isBitSet(nonDefaultFields, fieldIndex++)) {
        parcel.writeCharSequence(mContentDescription);
    }
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeCharSequence(mPaneTitle);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeCharSequence(mTooltipText);

    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeString(mViewIdResourceName);

    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeInt(mTextSelectionStart);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeInt(mTextSelectionEnd);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeInt(mInputType);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeInt(mLiveRegion);
    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeInt(mDrawingOrderInParent);

    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeStringArrayList(mExtraDataKeys);

    if (isBitSet(nonDefaultFields, fieldIndex++)) parcel.writeBundle(*mExtras);

    if (isBitSet(nonDefaultFields, fieldIndex++)) {
        parcel.writeInt(mRangeInfo->getType());
        parcel.writeFloat(mRangeInfo->getMin());
        parcel.writeFloat(mRangeInfo->getMax());
        parcel.writeFloat(mRangeInfo->getCurrent());
    }

    if (isBitSet(nonDefaultFields, fieldIndex++)) {
        parcel.writeInt(mCollectionInfo->getRowCount());
        parcel.writeInt(mCollectionInfo->getColumnCount());
        parcel.writeInt(mCollectionInfo->isHierarchical() ? 1 : 0);
        parcel.writeInt(mCollectionInfo->getSelectionMode());
    }

    if (isBitSet(nonDefaultFields, fieldIndex++)) {
        parcel.writeInt(mCollectionItemInfo->getRowIndex());
        parcel.writeInt(mCollectionItemInfo->getRowSpan());
        parcel.writeInt(mCollectionItemInfo->getColumnIndex());
        parcel.writeInt(mCollectionItemInfo->getColumnSpan());
        parcel.writeInt(mCollectionItemInfo->isHeading() ? 1 : 0);
        parcel.writeInt(mCollectionItemInfo->isSelected() ? 1 : 0);
    }

    if (Debug) {
        fieldIndex--;
        if (totalFields != fieldIndex) {
            throw std::runtime_error("Number of fields mismatch:  + totalFields vs  + fieldIndex");
        }
    }
}

void AccessibilityNodeInfo::init(const AccessibilityNodeInfo& other) {
    mSealed = other.mSealed;
    mSourceNodeId = other.mSourceNodeId;
    mParentNodeId = other.mParentNodeId;
    mLabelForId = other.mLabelForId;
    mLabeledById = other.mLabeledById;
    mTraversalBefore = other.mTraversalBefore;
    mTraversalAfter = other.mTraversalAfter;
    mWindowId = other.mWindowId;
    mConnectionId = other.mConnectionId;
    mBoundsInParent = other.mBoundsInParent;
    mBoundsInScreen = other.mBoundsInScreen;
    mPackageName = other.mPackageName;
    mClassName = other.mClassName;
    mText = other.mText;
    mOriginalText = other.mOriginalText;
    mHintText = other.mHintText;
    mError = other.mError;
    mContentDescription = other.mContentDescription;
    mPaneTitle = other.mPaneTitle;
    mTooltipText = other.mTooltipText;
    mViewIdResourceName = other.mViewIdResourceName;

    mActions.clear();
    auto& otherActions = other.mActions;
    if (otherActions.size() > 0) {
        if (mActions.empty()) {
            mActions = otherActions;
        } else {
            mActions.insert(mActions.end(),otherActions.begin(),otherActions.end());
        }
    }

    mBooleanProperties = other.mBooleanProperties;
    mMaxTextLength = other.mMaxTextLength;
    mMovementGranularities = other.mMovementGranularities;


    mChildNodeIds.clear();
    auto& otherChildNodeIds = other.mChildNodeIds;
    if ( otherChildNodeIds.size() > 0) {
        if (mChildNodeIds.empty()) {
            mChildNodeIds = otherChildNodeIds;
        } else {
            mChildNodeIds.insert(mChildNodeIds.end(),otherChildNodeIds.begin(),otherChildNodeIds.end());
        }
    }

    mTextSelectionStart = other.mTextSelectionStart;
    mTextSelectionEnd = other.mTextSelectionEnd;
    mInputType = other.mInputType;
    mLiveRegion = other.mLiveRegion;
    mDrawingOrderInParent = other.mDrawingOrderInParent;

    mExtraDataKeys = other.mExtraDataKeys;

    mExtras = other.mExtras != nullptr ? new Bundle(*other.mExtras) : nullptr;

    if (mRangeInfo != nullptr) mRangeInfo->recycle();
    mRangeInfo = (other.mRangeInfo != nullptr)
            ? RangeInfo::obtain(*other.mRangeInfo) : nullptr;
    if (mCollectionInfo != nullptr) mCollectionInfo->recycle();
    mCollectionInfo = (other.mCollectionInfo != nullptr)
            ? CollectionInfo::obtain(*other.mCollectionInfo) : nullptr;
    if (mCollectionItemInfo != nullptr) mCollectionItemInfo->recycle();
    mCollectionItemInfo =  (other.mCollectionItemInfo != nullptr)
            ? CollectionItemInfo::obtain(*other.mCollectionItemInfo) : nullptr;
}

void AccessibilityNodeInfo::initFromParcel(Parcel parcel) {
    // Bit mask of non-default-valued field indices
    long nonDefaultFields = parcel.readLong();
    int fieldIndex = 0;
    const bool sealed = isBitSet(nonDefaultFields, fieldIndex++)
            ? (parcel.readInt() == 1): DEFAULT.mSealed;
    if (isBitSet(nonDefaultFields, fieldIndex++)) mSourceNodeId = parcel.readLong();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mWindowId = parcel.readInt();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mParentNodeId = parcel.readLong();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mLabelForId = parcel.readLong();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mLabeledById = parcel.readLong();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mTraversalBefore = parcel.readLong();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mTraversalAfter = parcel.readLong();

    if (isBitSet(nonDefaultFields, fieldIndex++)) mConnectionId = parcel.readInt();

    if (isBitSet(nonDefaultFields, fieldIndex++)) {
        const int childrenSize = parcel.readInt();
        if (childrenSize <= 0) {
            mChildNodeIds.clear();// = null;
        } else {
            //mChildNodeIds = new LongArray(childrenSize);
            for (int i = 0; i < childrenSize; i++) {
                const long childId = parcel.readLong();
                mChildNodeIds.push_back(childId);
            }
        }
    }

    if (isBitSet(nonDefaultFields, fieldIndex++)) {
        mBoundsInParent.top = parcel.readInt();
        mBoundsInParent.height = parcel.readInt();
        mBoundsInParent.left = parcel.readInt();
        mBoundsInParent.width = parcel.readInt();
    }

    if (isBitSet(nonDefaultFields, fieldIndex++)) {
        mBoundsInScreen.top = parcel.readInt();
        mBoundsInScreen.height = parcel.readInt();
        mBoundsInScreen.left = parcel.readInt();
        mBoundsInScreen.width = parcel.readInt();
    }

    if (isBitSet(nonDefaultFields, fieldIndex++)) {
        const long standardActions = parcel.readLong();
        addStandardActions(standardActions);
        const int nonStandardActionCount = parcel.readInt();
        for (int i = 0; i < nonStandardActionCount; i++) {
            AccessibilityAction* action = new AccessibilityAction(
                    parcel.readInt(), parcel.readCharSequence());
            addActionUnchecked(action);
        }
    }

    if (isBitSet(nonDefaultFields, fieldIndex++)) mMaxTextLength = parcel.readInt();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mMovementGranularities = parcel.readInt();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mBooleanProperties = parcel.readInt();

    if (isBitSet(nonDefaultFields, fieldIndex++)) mPackageName = parcel.readCharSequence();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mClassName = parcel.readCharSequence();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mText = parcel.readCharSequence();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mHintText = parcel.readCharSequence();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mError = parcel.readCharSequence();
    if (isBitSet(nonDefaultFields, fieldIndex++)) {
        mContentDescription = parcel.readCharSequence();
    }
    if (isBitSet(nonDefaultFields, fieldIndex++)) mPaneTitle = parcel.readCharSequence();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mTooltipText = parcel.readCharSequence();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mViewIdResourceName = parcel.readString();

    if (isBitSet(nonDefaultFields, fieldIndex++)) mTextSelectionStart = parcel.readInt();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mTextSelectionEnd = parcel.readInt();

    if (isBitSet(nonDefaultFields, fieldIndex++)) mInputType = parcel.readInt();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mLiveRegion = parcel.readInt();
    if (isBitSet(nonDefaultFields, fieldIndex++)) mDrawingOrderInParent = parcel.readInt();

    mExtraDataKeys = isBitSet(nonDefaultFields, fieldIndex++)
            ? parcel.createStringArrayList()
            : std::vector<std::string>();

    mExtras = isBitSet(nonDefaultFields, fieldIndex++)
            ? parcel.readBundle()
            : nullptr;

    if (mRangeInfo != nullptr) mRangeInfo->recycle();
    mRangeInfo = isBitSet(nonDefaultFields, fieldIndex++)
            ? RangeInfo::obtain(
                    parcel.readInt(),
                    parcel.readFloat(),
                    parcel.readFloat(),
                    parcel.readFloat())
            : nullptr;

    if (mCollectionInfo != nullptr) mCollectionInfo->recycle();
    mCollectionInfo = isBitSet(nonDefaultFields, fieldIndex++)
            ? CollectionInfo::obtain(
                    parcel.readInt(),
                    parcel.readInt(),
                    parcel.readInt() == 1,
                    parcel.readInt())
            : nullptr;

    if (mCollectionItemInfo != nullptr) mCollectionItemInfo->recycle();
    mCollectionItemInfo = isBitSet(nonDefaultFields, fieldIndex++)
            ? CollectionItemInfo::obtain(
                    parcel.readInt(),
                    parcel.readInt(),
                    parcel.readInt(),
                    parcel.readInt(),
                    parcel.readInt() == 1,
                    parcel.readInt() == 1)
            : nullptr;

    mSealed = sealed;
}

void AccessibilityNodeInfo::clear() {
    init(DEFAULT);
}

bool AccessibilityNodeInfo::isDefaultStandardAction(const AccessibilityAction* action) {
    return (action->mSerializationFlag != -1L) && action->getLabel().empty();//TextUtils::isEmpty(action->getLabel());
}

template<typename T>
static T valueAt(const std::set<T>& mySet, size_t index) {
    if (index >= mySet.size()) {
        throw std::out_of_range("Index out of range");
    }

    auto it = mySet.begin();
    std::advance(it, index);  // 将迭代器向前移动 index 步
    return *it;
}

AccessibilityNodeInfo::AccessibilityAction* AccessibilityNodeInfo::getActionSingleton(int actionId) {
    const int actions = AccessibilityAction::sStandardActions.size();
    for (int i = 0; i < actions; i++) {
        AccessibilityAction* currentAction = valueAt(AccessibilityAction::sStandardActions,i);
        if (actionId == currentAction->getId()) {
            return currentAction;
        }
    }

    return nullptr;
}

AccessibilityNodeInfo::AccessibilityAction* AccessibilityNodeInfo::getActionSingletonBySerializationFlag(long flag) {
    const int actions = AccessibilityAction::sStandardActions.size();
    for (int i = 0; i < actions; i++) {
        AccessibilityAction* currentAction = valueAt(AccessibilityAction::sStandardActions,i);
        if (flag == currentAction->mSerializationFlag) {
            return currentAction;
        }
    }

    return nullptr;
}

void AccessibilityNodeInfo::addStandardActions(long serializationIdMask) {
    long remainingIds = serializationIdMask;
    while (remainingIds > 0) {
        const long id = 1L << MathUtils::numberOfTrailingZeros(remainingIds);
        remainingIds &= ~id;
        AccessibilityAction* action = getActionSingletonBySerializationFlag(id);
        addAction(action);
    }
}

std::string AccessibilityNodeInfo::getActionSymbolicName(int action){
    switch (action) {
    case ACTION_FOCUS:        return "ACTION_FOCUS";
    case ACTION_CLEAR_FOCUS:  return "ACTION_CLEAR_FOCUS";
    case ACTION_SELECT:       return "ACTION_SELECT";
    case ACTION_CLEAR_SELECTION:  return "ACTION_CLEAR_SELECTION";
    case ACTION_CLICK:        return "ACTION_CLICK";
    case ACTION_LONG_CLICK:   return "ACTION_LONG_CLICK";
    case ACTION_ACCESSIBILITY_FOCUS:         return "ACTION_ACCESSIBILITY_FOCUS";
    case ACTION_CLEAR_ACCESSIBILITY_FOCUS:   return "ACTION_CLEAR_ACCESSIBILITY_FOCUS";
    case ACTION_NEXT_AT_MOVEMENT_GRANULARITY:return "ACTION_NEXT_AT_MOVEMENT_GRANULARITY";
    case ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY:return "ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY";
    case ACTION_NEXT_HTML_ELEMENT:        return "ACTION_NEXT_HTML_ELEMENT";
    case ACTION_PREVIOUS_HTML_ELEMENT:    return "ACTION_PREVIOUS_HTML_ELEMENT";
    case ACTION_SCROLL_FORWARD:        return "ACTION_SCROLL_FORWARD";
    case ACTION_SCROLL_BACKWARD:       return "ACTION_SCROLL_BACKWARD";
    case ACTION_CUT:        return "ACTION_CUT";
    case ACTION_COPY:       return "ACTION_COPY";
    case ACTION_PASTE:      return "ACTION_PASTE";
    case ACTION_SET_SELECTION:  return "ACTION_SET_SELECTION";
    case ACTION_EXPAND:     return "ACTION_EXPAND";
    case ACTION_COLLAPSE:   return "ACTION_COLLAPSE";
    case ACTION_DISMISS:    return "ACTION_DISMISS";
    case ACTION_SET_TEXT:   return "ACTION_SET_TEXT";

    case R::id::accessibilityActionShowOnScreen:  return "ACTION_SHOW_ON_SCREEN";
    case R::id::accessibilityActionScrollToPosition: return "ACTION_SCROLL_TO_POSITION";
    case R::id::accessibilityActionScrollUp:      return "ACTION_SCROLL_UP";
    case R::id::accessibilityActionScrollLeft:    return "ACTION_SCROLL_LEFT";
    case R::id::accessibilityActionScrollDown:    return "ACTION_SCROLL_DOWN";
    case R::id::accessibilityActionScrollRight:   return "ACTION_SCROLL_RIGHT";
    case R::id::accessibilityActionSetProgress:   return "ACTION_SET_PROGRESS";
    case R::id::accessibilityActionContextClick:  return "ACTION_CONTEXT_CLICK";
    case R::id::accessibilityActionShowTooltip:   return "ACTION_SHOW_TOOLTIP";
    case R::id::accessibilityActionHideTooltip:   return "ACTION_HIDE_TOOLTIP";

    default:        return "ACTION_UNKNOWN";
    }
}

std::string AccessibilityNodeInfo::getMovementGranularitySymbolicName(int granularity){
    switch (granularity) {
    case MOVEMENT_GRANULARITY_CHARACTER:  return "MOVEMENT_GRANULARITY_CHARACTER";
    case MOVEMENT_GRANULARITY_WORD:       return "MOVEMENT_GRANULARITY_WORD";
    case MOVEMENT_GRANULARITY_LINE:       return "MOVEMENT_GRANULARITY_LINE";
    case MOVEMENT_GRANULARITY_PARAGRAPH:  return "MOVEMENT_GRANULARITY_PARAGRAPH";
    case MOVEMENT_GRANULARITY_PAGE:       return "MOVEMENT_GRANULARITY_PAGE";
    default:
        throw  std::runtime_error("Unknown movement granularity: ");// + granularity);
    }
}

bool AccessibilityNodeInfo::canPerformRequestOverConnection(long accessibilityNodeId) {
    return ((mWindowId != AccessibilityWindowInfo::UNDEFINED_WINDOW_ID)
            && (getAccessibilityViewId(accessibilityNodeId) != UNDEFINED_ITEM_ID)
            && (mConnectionId != UNDEFINED_CONNECTION_ID));
}

bool AccessibilityNodeInfo::operator==(const AccessibilityNodeInfo*other) const{
    if (this == other) {
        return true;
    }
    if (other == nullptr) {
        return false;
    }
    if (mSourceNodeId != other->mSourceNodeId) {
        return false;
    }
    if (mWindowId != other->mWindowId) {
        return false;
    }
    return true;
}

int AccessibilityNodeInfo::hashCode() {
    const int prime = 31;
    int result = 1;
    result = prime * result + getAccessibilityViewId(mSourceNodeId);
    result = prime * result + getVirtualDescendantId(mSourceNodeId);
    result = prime * result + mWindowId;
    return result;
}

std::string AccessibilityNodeInfo::toString() {
    std::ostringstream builder;

    if (Debug) {
        builder<<"; sourceNodeId: " <<mSourceNodeId;
        builder<<"; windowId: "<< mWindowId;
        builder<<"; accessibilityViewId: "<<getAccessibilityViewId(mSourceNodeId);
        builder<<"; virtualDescendantId: "<<getVirtualDescendantId(mSourceNodeId);
        builder<<"; mParentNodeId: "<< mParentNodeId;
        builder<<"; traversalBefore: "<<mTraversalBefore;
        builder<<"; traversalAfter: "<<mTraversalAfter;

        int granularities = mMovementGranularities;
        builder<<"; MovementGranularities: [";
        while (granularities != 0) {
            int granularity = 1 << MathUtils::numberOfTrailingZeros(granularities);
            granularities &= ~granularity;
            builder<<getMovementGranularitySymbolicName(granularity);
            if (granularities != 0) {
                builder<<", ";
            }
        }
        builder<<"]";

        builder<<"; childAccessibilityIds: [";
        auto& childIds = mChildNodeIds;
        if (childIds.size()) {
            for (int i = 0, count = childIds.size(); i < count; i++) {
                builder<<childIds.at(i);
                if (i < count - 1) {
                    builder<<", ";
                }
            }
        }
        builder<<"]";
    }

    //builder<<"; boundsInParent: "<<mBoundsInParent;
    //builder<<"; boundsInScreen: "<<mBoundsInScreen;

    builder<<"; packageName: "<<mPackageName;
    builder<<"; className: "<<mClassName;
    builder<<"; text: "<<mText;
    builder<<"; error: "<<mError;
    builder<<"; maxTextLength: "<<mMaxTextLength;
    builder<<"; contentDescription: "<<mContentDescription;
    builder<<"; tooltipText: "<<mTooltipText;
    builder<<"; viewIdResName: "<<mViewIdResourceName;

    builder<<"; checkable: "<<isCheckable();
    builder<<"; checked: "<<isChecked();
    builder<<"; focusable: "<<isFocusable();
    builder<<"; focused: "<<isFocused();
    builder<<"; selected: "<<isSelected();
    builder<<"; clickable: "<<isClickable();
    builder<<"; longClickable: "<<isLongClickable();
    builder<<"; contextClickable: "<<isContextClickable();
    builder<<"; enabled: "<<isEnabled();
    builder<<"; password: "<<isPassword();
    builder<<"; scrollable: "<<isScrollable();
    builder<<"; importantForAccessibility: "<<isImportantForAccessibility();
    builder<<"; visible: "<<isVisibleToUser();
    //builder<<"; actions: "<<mActions;

    return builder.str();
}

AccessibilityNodeInfo* AccessibilityNodeInfo::getNodeForAccessibilityId(long accessibilityId) {
    if (!canPerformRequestOverConnection(accessibilityId)) {
        return nullptr;
    }
    return nullptr;
    //AccessibilityInteractionClient client = AccessibilityInteractionClient.getInstance();
    //return client.findAccessibilityNodeInfoByAccessibilityId(mConnectionId,mWindowId, accessibilityId,
    //    false, FLAG_PREFETCH_PREDECESSORS | FLAG_PREFETCH_DESCENDANTS | FLAG_PREFETCH_SIBLINGS, nullptr);
}

std::string AccessibilityNodeInfo::idToString(long accessibilityId) {
    int accessibilityViewId = getAccessibilityViewId(accessibilityId);
    int virtualDescendantId = getVirtualDescendantId(accessibilityId);
    return virtualDescendantId == AccessibilityNodeProvider::HOST_VIEW_ID
            ? idItemToString(accessibilityViewId)
            : idItemToString(accessibilityViewId) + ":" + idItemToString(virtualDescendantId);
}

std::string AccessibilityNodeInfo::idItemToString(int item) {
    switch (item) {
        case ROOT_ITEM_ID: return "ROOT";
        case UNDEFINED_ITEM_ID: return "UNDEFINED";
        case AccessibilityNodeProvider::HOST_VIEW_ID: return "HOST";
        default: return std::to_string(item);
    }
}

/////////////////static final class AccessibilityAction {

std::set<AccessibilityNodeInfo::AccessibilityAction*> AccessibilityNodeInfo::AccessibilityAction::sStandardActions;

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_FOCUS(AccessibilityNodeInfo::ACTION_FOCUS);
const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_CLEAR_FOCUS(AccessibilityNodeInfo::ACTION_CLEAR_FOCUS);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_SELECT (AccessibilityNodeInfo::ACTION_SELECT);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_CLEAR_SELECTION (AccessibilityNodeInfo::ACTION_CLEAR_SELECTION);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_CLICK(AccessibilityNodeInfo::ACTION_CLICK);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_LONG_CLICK (AccessibilityNodeInfo::ACTION_LONG_CLICK);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_ACCESSIBILITY_FOCUS (AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_CLEAR_ACCESSIBILITY_FOCUS (AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_NEXT_AT_MOVEMENT_GRANULARITY (AccessibilityNodeInfo::ACTION_NEXT_AT_MOVEMENT_GRANULARITY);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY(AccessibilityNodeInfo::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_NEXT_HTML_ELEMENT (AccessibilityNodeInfo::ACTION_NEXT_HTML_ELEMENT);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_PREVIOUS_HTML_ELEMENT (AccessibilityNodeInfo::ACTION_PREVIOUS_HTML_ELEMENT);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_FORWARD (AccessibilityNodeInfo::ACTION_SCROLL_FORWARD);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_BACKWARD (AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_COPY (AccessibilityNodeInfo::ACTION_COPY);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_PASTE(AccessibilityNodeInfo::ACTION_PASTE);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_CUT(AccessibilityNodeInfo::ACTION_CUT);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_SET_SELECTION(AccessibilityNodeInfo::ACTION_SET_SELECTION);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_EXPAND(AccessibilityNodeInfo::ACTION_EXPAND);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_COLLAPSE(AccessibilityNodeInfo::ACTION_COLLAPSE);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_DISMISS (AccessibilityNodeInfo::ACTION_DISMISS);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_SET_TEXT(AccessibilityNodeInfo::ACTION_SET_TEXT);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_SHOW_ON_SCREEN(R::id::accessibilityActionShowOnScreen);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_TO_POSITION(R::id::accessibilityActionScrollToPosition);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_UP(R::id::accessibilityActionScrollUp);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_LEFT (R::id::accessibilityActionScrollLeft);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_DOWN (R::id::accessibilityActionScrollDown);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_RIGHT(R::id::accessibilityActionScrollRight);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_CONTEXT_CLICK(R::id::accessibilityActionContextClick);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_SET_PROGRESS (R::id::accessibilityActionSetProgress);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_MOVE_WINDOW(R::id::accessibilityActionMoveWindow);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_SHOW_TOOLTIP(R::id::accessibilityActionShowTooltip);

const AccessibilityNodeInfo::AccessibilityAction  AccessibilityNodeInfo::AccessibilityAction::ACTION_HIDE_TOOLTIP (R::id::accessibilityActionHideTooltip);

AccessibilityNodeInfo::AccessibilityAction::AccessibilityAction(int actionId, const std::string& label) {
    if ((actionId & ACTION_TYPE_MASK) == 0 && BitSet32::count(actionId) != 1) {
        //throw std::runtime_error("Invalid standard action id");
        LOGD("Invalid standard action id:%d",actionId);
    }

    mActionId = actionId;
    mLabel = label;
}

AccessibilityNodeInfo::AccessibilityAction::AccessibilityAction(int standardActionId)
    :AccessibilityAction(standardActionId, ""){

    mSerializationFlag = bitAt(sStandardActions.size());
    sStandardActions.insert(this);
}

int AccessibilityNodeInfo::AccessibilityAction::getId() const{
    return mActionId;
}

std::string AccessibilityNodeInfo::AccessibilityAction::getLabel() const{
    return mLabel;
}

int AccessibilityNodeInfo::AccessibilityAction::hashCode() const{
    return mActionId;
}

bool AccessibilityNodeInfo::AccessibilityAction::operator==(const AccessibilityAction* other)const{
    if (other == nullptr) {
        return false;
    }

    if (other == this) {
        return true;
    }

    return mActionId == other->mActionId;
}

std::string AccessibilityNodeInfo::AccessibilityAction::toString() const{
    return "AccessibilityAction: " + getActionSymbolicName(mActionId) + " - " + mLabel;
}

////////////public static final class RangeInfo {

Pools::SimplePool<AccessibilityNodeInfo::RangeInfo> AccessibilityNodeInfo::RangeInfo::sPool(AccessibilityNodeInfo::RangeInfo::MAX_POOL_SIZE);

AccessibilityNodeInfo::RangeInfo* AccessibilityNodeInfo::RangeInfo::obtain(const RangeInfo& other) {
    return obtain(other.mType, other.mMin, other.mMax, other.mCurrent);
}

AccessibilityNodeInfo::RangeInfo* AccessibilityNodeInfo::RangeInfo::obtain(int type, float min, float max, float current) {
    RangeInfo* info = sPool.acquire();
    if (info == nullptr) {
        return new RangeInfo(type, min, max, current);
    }

    info->mType = type;
    info->mMin = min;
    info->mMax = max;
    info->mCurrent = current;
    return info;
}

AccessibilityNodeInfo::RangeInfo::RangeInfo(int type, float min, float max, float current) {
    mType = type;
    mMin = min;
    mMax = max;
    mCurrent = current;
}

int AccessibilityNodeInfo::RangeInfo::getType()const{
    return mType;
}

float AccessibilityNodeInfo::RangeInfo::getMin()const{
    return mMin;
}

float AccessibilityNodeInfo::RangeInfo::getMax()const{
    return mMax;
}

float AccessibilityNodeInfo::RangeInfo::getCurrent()const{
    return mCurrent;
}

void AccessibilityNodeInfo::RangeInfo::recycle() {
    clear();
    sPool.release(this);
}

void AccessibilityNodeInfo::RangeInfo::clear() {
    mType = 0;
    mMin = 0;
    mMax = 0;
    mCurrent = 0;
}

bool AccessibilityNodeInfo::RangeInfo::operator==(const RangeInfo* other)const{
    if (other == nullptr) {
        return false;
    }

    if (other == this) {
        return true;
    }

    return (mType==other->mType)&&(mMin==other->mMin)&&(mMax==other->mMax)&&(mCurrent==other->mCurrent);
}

/////////////////public static final class CollectionInfo
/** Selection mode where items are not selectable. */

Pools::SimplePool<AccessibilityNodeInfo::CollectionInfo> AccessibilityNodeInfo::CollectionInfo::sPool(AccessibilityNodeInfo::CollectionInfo::MAX_POOL_SIZE);

AccessibilityNodeInfo::CollectionInfo* AccessibilityNodeInfo::CollectionInfo::obtain(const CollectionInfo& other) {
    return CollectionInfo::obtain(other.mRowCount, other.mColumnCount, other.mHierarchical,
            other.mSelectionMode);
}

AccessibilityNodeInfo::CollectionInfo* AccessibilityNodeInfo::CollectionInfo::obtain(int rowCount, int columnCount,
        bool hierarchical) {
    return obtain(rowCount, columnCount, hierarchical, SELECTION_MODE_NONE);
}

AccessibilityNodeInfo::CollectionInfo* AccessibilityNodeInfo::CollectionInfo::obtain(int rowCount, int columnCount,
        bool hierarchical, int selectionMode) {
    CollectionInfo* info = sPool.acquire();
    if (info == nullptr) {
        return new CollectionInfo(rowCount, columnCount, hierarchical, selectionMode);
    }

    info->mRowCount = rowCount;
    info->mColumnCount = columnCount;
    info->mHierarchical = hierarchical;
    info->mSelectionMode = selectionMode;
    return info;
}

AccessibilityNodeInfo::CollectionInfo::CollectionInfo(int rowCount, int columnCount, bool hierarchical, int selectionMode) {
    mRowCount = rowCount;
    mColumnCount = columnCount;
    mHierarchical = hierarchical;
    mSelectionMode = selectionMode;
}

int AccessibilityNodeInfo::CollectionInfo::getRowCount() const{
    return mRowCount;
}

int AccessibilityNodeInfo::CollectionInfo::getColumnCount() const{
    return mColumnCount;
}

bool AccessibilityNodeInfo::CollectionInfo::isHierarchical() const{
    return mHierarchical;
}

int AccessibilityNodeInfo::CollectionInfo::getSelectionMode() const{
    return mSelectionMode;
}

void AccessibilityNodeInfo::CollectionInfo::recycle() {
    clear();
    sPool.release(this);
}

void AccessibilityNodeInfo::CollectionInfo::clear() {
    mRowCount = 0;
    mColumnCount = 0;
    mHierarchical = false;
    mSelectionMode = SELECTION_MODE_NONE;
}


//////////////////public static final class CollectionItemInfo

Pools::SimplePool<AccessibilityNodeInfo::CollectionItemInfo> AccessibilityNodeInfo::CollectionItemInfo::sPool(AccessibilityNodeInfo::CollectionItemInfo::MAX_POOL_SIZE);

AccessibilityNodeInfo::CollectionItemInfo* AccessibilityNodeInfo::CollectionItemInfo::obtain(const CollectionItemInfo& other) {
    return CollectionItemInfo::obtain(other.mRowIndex, other.mRowSpan, other.mColumnIndex,
            other.mColumnSpan, other.mHeading, other.mSelected);
}

AccessibilityNodeInfo::CollectionItemInfo* AccessibilityNodeInfo::CollectionItemInfo::obtain(int rowIndex, int rowSpan,
        int columnIndex, int columnSpan, bool heading) {
    return obtain(rowIndex, rowSpan, columnIndex, columnSpan, heading, false);
}

AccessibilityNodeInfo::CollectionItemInfo* AccessibilityNodeInfo::CollectionItemInfo::obtain(int rowIndex, int rowSpan,
        int columnIndex, int columnSpan, bool heading, bool selected) {
    CollectionItemInfo* info = sPool.acquire();
    if (info == nullptr) {
        return new CollectionItemInfo(
                rowIndex, rowSpan, columnIndex, columnSpan, heading, selected);
    }

    info->mRowIndex = rowIndex;
    info->mRowSpan = rowSpan;
    info->mColumnIndex = columnIndex;
    info->mColumnSpan = columnSpan;
    info->mHeading = heading;
    info->mSelected = selected;
    return info;
}

AccessibilityNodeInfo::CollectionItemInfo::CollectionItemInfo(int rowIndex, int rowSpan, int columnIndex, int columnSpan,
        bool heading, bool selected) {
    mRowIndex = rowIndex;
    mRowSpan = rowSpan;
    mColumnIndex = columnIndex;
    mColumnSpan = columnSpan;
    mHeading  = heading;
    mSelected = selected;
}

int AccessibilityNodeInfo::CollectionItemInfo::getColumnIndex() const{
    return mColumnIndex;
}

int AccessibilityNodeInfo::CollectionItemInfo::getRowIndex() const{
    return mRowIndex;
}

int AccessibilityNodeInfo::CollectionItemInfo::getColumnSpan() const{
    return mColumnSpan;
}

int AccessibilityNodeInfo::CollectionItemInfo::getRowSpan() const{
    return mRowSpan;
}

bool AccessibilityNodeInfo::CollectionItemInfo::isHeading() const{
    return mHeading;
}

bool AccessibilityNodeInfo::CollectionItemInfo::isSelected() const{
    return mSelected;
}

void AccessibilityNodeInfo::CollectionItemInfo::recycle() {
    clear();
    sPool.release(this);
}

void AccessibilityNodeInfo::CollectionItemInfo::clear() {
    mColumnIndex = 0;
    mColumnSpan = 0;
    mRowIndex = 0;
    mRowSpan = 0;
    mHeading = false;
    mSelected = false;
}

}/*endof namespace*/
