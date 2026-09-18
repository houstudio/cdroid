#include <app/espresso/treeiterables.h>

namespace cdroid {
namespace espresso {

std::vector<ViewAndDistance> depthFirstViewTraversalWithDistance(View* root) {
    // AOSP TreeTraversalIterable + DistanceRecordingTreeViewer: a work stack
    // seeded with (root, 0); children are pushed in reverse so the first
    // child pops next (pre-order).
    std::vector<ViewAndDistance> order;
    if (!root) return order;
    std::vector<ViewAndDistance> stack;
    stack.emplace_back(root, 0);
    while (!stack.empty()) {
        ViewAndDistance current = stack.back();
        stack.pop_back();
        order.push_back(current);
        ViewGroup* viewGroup = dynamic_cast<ViewGroup*>(current.getView());
        if (viewGroup) {
            for (int i = viewGroup->getChildCount() - 1; i >= 0; i--) {
                stack.emplace_back(viewGroup->getChildAt(i),
                        current.getDistanceFromRoot() + 1);
            }
        }
    }
    return order;
}

std::vector<View*> depthFirstViewTraversal(View* root) {
    std::vector<View*> order;
    for (const ViewAndDistance& vd : depthFirstViewTraversalWithDistance(root)) {
        order.push_back(vd.getView());
    }
    return order;
}

std::vector<View*> breadthFirstViewTraversal(View* root) {
    // AOSP BREADTH_FIRST strategy: children append to the tail of a work list.
    std::vector<View*> order;
    if (!root) return order;
    std::vector<View*> queue = {root};
    size_t cursor = 0;
    while (cursor < queue.size()) {
        View* current = queue[cursor++];
        order.push_back(current);
        ViewGroup* viewGroup = dynamic_cast<ViewGroup*>(current);
        if (viewGroup) {
            for (int i = 0; i < viewGroup->getChildCount(); i++) {
                queue.push_back(viewGroup->getChildAt(i));
            }
        }
    }
    return order;
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
