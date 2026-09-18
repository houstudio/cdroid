#ifndef CDROID_ESPRESSO_TREEITERABLES_H
#define CDROID_ESPRESSO_TREEITERABLES_H

/*
 * android.support.test.espresso.util.TreeIterables — view-tree traversal
 * helpers. AOSP returns lazy Iterables; the port collects eagerly into
 * std::vector (same traversal order: pre-order DFS / queue BFS).
 */

#include <vector>

#include <view/view.h>
#include <view/viewgroup.h>

namespace cdroid {
namespace espresso {

/** AOSP TreeIterables.ViewAndDistance. */
class ViewAndDistance {
public:
    ViewAndDistance(View* view, int distanceFromRoot)
        : mView(view), mDistanceFromRoot(distanceFromRoot) {}

    View* getView() const { return mView; }
    int getDistanceFromRoot() const { return mDistanceFromRoot; }

private:
    View* mView;
    int mDistanceFromRoot;
};

/** depthFirstViewTraversalWithDistance(root) — pre-order DFS with depth. */
std::vector<ViewAndDistance> depthFirstViewTraversalWithDistance(View* root);

/** depthFirstViewTraversal(root) — pre-order DFS. */
std::vector<View*> depthFirstViewTraversal(View* root);

/** breadthFirstViewTraversal(root) — queue-based BFS. */
std::vector<View*> breadthFirstViewTraversal(View* root);

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_TREEITERABLES_H*/
