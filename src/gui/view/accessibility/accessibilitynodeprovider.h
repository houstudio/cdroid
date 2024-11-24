#ifndef __ACCESSIBILITY_NODE_PROVIDER_H__
#define __ACCESSIBILITY_NODE_PROVIDER_H__
namespace cdroid{
using Bundle=void*;
class AccessibilityNodeInfo;
class AccessibilityNodeProvider {
public:
    /**
     * The virtual id for the hosting View.
     */
    static constexpr int HOST_VIEW_ID = -1;
public:
    /**
     * Returns an {@link AccessibilityNodeInfo} representing a virtual view,
     * such as a descendant of the host View, with the given <code>virtualViewId</code>
     * or the host View itself if <code>virtualViewId</code> equals to {@link #HOST_VIEW_ID}.
     * <p>
     * A virtual descendant is an imaginary View that is reported as a part of the view
     * hierarchy for accessibility purposes. This enables custom views that draw complex
     * content to report them selves as a tree of virtual views, thus conveying their
     * logical structure.
     * </p>
     * <p>
     * The implementer is responsible for obtaining an accessibility node info from the
     * pool of reusable instances and setting the desired properties of the node info
     * before returning it.
     * </p>
     *
     * @param virtualViewId A client defined virtual view id.
     * @return A populated {@link AccessibilityNodeInfo} for a virtual descendant or the
     *     host View.
     *
     * @see View#createAccessibilityNodeInfo()
     * @see AccessibilityNodeInfo
     */
    AccessibilityNodeInfo* createAccessibilityNodeInfo(int virtualViewId) {
        return nullptr;
    }

    /**
     * Adds extra data to an {@link AccessibilityNodeInfo} based on an explicit request for the
     * additional data.
     * <p>
     * This method only needs to be implemented if a virtual view offers to provide additional
     * data.
     * </p>
     *
     * @param virtualViewId The virtual view id used to create the node
     * @param info The info to which to add the extra data
     * @param extraDataKey A key specifying the type of extra data to add to the info. The
     *                     extra data should be added to the {@link Bundle} returned by
     *                     the info's {@link AccessibilityNodeInfo#getExtras} method.
     * @param arguments A {@link Bundle} holding any arguments relevant for this request.
     *
     * @see AccessibilityNodeInfo#setExtraAvailableData
     */
    void addExtraDataToAccessibilityNodeInfo(
            int virtualViewId, AccessibilityNodeInfo* info, const std::string& extraDataKey, Bundle arguments) {
    }

    /**
     * Performs an accessibility action on a virtual view, such as a descendant of the
     * host View, with the given <code>virtualViewId</code> or the host View itself
     * if <code>virtualViewId</code> equals to {@link #HOST_VIEW_ID}.
     *
     * @param virtualViewId A client defined virtual view id.
     * @param action The action to perform.
     * @param arguments Optional action arguments.
     * @return True if the action was performed.
     *
     * @see View#performAccessibilityAction(int, Bundle)
     * @see #createAccessibilityNodeInfo(int)
     * @see AccessibilityNodeInfo
     */
    bool performAction(int virtualViewId, int action, Bundle arguments) {
        return false;
    }

    /**
     * Finds {@link AccessibilityNodeInfo}s by text. The match is case insensitive
     * containment. The search is relative to the virtual view, i.e. a descendant of the
     * host View, with the given <code>virtualViewId</code> or the host View itself
     * <code>virtualViewId</code> equals to {@link #HOST_VIEW_ID}.
     *
     * @param virtualViewId A client defined virtual view id which defined
     *     the root of the tree in which to perform the search.
     * @param text The searched text.
     * @return A list of node info.
     *
     * @see #createAccessibilityNodeInfo(int)
     * @see AccessibilityNodeInfo
     */
    std::vector<AccessibilityNodeInfo*> findAccessibilityNodeInfosByText(const std::string text,
            int virtualViewId) {
        return std::vector<AccessibilityNodeInfo*>();
    }

    /**
     * Find the virtual view, such as a descendant of the host View, that has the
     * specified focus type.
     *
     * @param focus The focus to find. One of
     *            {@link AccessibilityNodeInfo#FOCUS_INPUT} or
     *            {@link AccessibilityNodeInfo#FOCUS_ACCESSIBILITY}.
     * @return The node info of the focused view or null.
     * @see AccessibilityNodeInfo#FOCUS_INPUT
     * @see AccessibilityNodeInfo#FOCUS_ACCESSIBILITY
     */
    AccessibilityNodeInfo* findFocus(int focus) {
        return nullptr;
    }
};
}/*endof namespace*/
#endif/*__ACCESSIBILITY_NODE_PROVIDER_H__*/

