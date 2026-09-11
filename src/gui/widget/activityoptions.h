#ifndef __CDROID_ACTIVITYOPTIONS_H__
#define __CDROID_ACTIVITYOPTIONS_H__
#include <string>
#include <utility>
#include <vector>

namespace cdroid {

class View;
class Window;

// Port of android.app.ActivityOptions (scene-transition subset). Android wraps this into a
// Bundle handed to startActivity; CDROID has no Binder, so the object itself is passed to
// App::startActivity(intent, options), which consumes and deletes it after stamping the
// shared-element sources on the started Window.
//
// Only ANIM_SCENE_TRANSITION is modeled (makeSceneTransitionAnimation): the cross-Window
// shared-element flight. The other flavors (makeScaleUpAnimation, makeThumbnailScaleUp...)
// are window-level animations — route A (Window::setEnterTransition) territory.
class ActivityOptions {
public:
    // android.app.ActivityOptions#makeSceneTransitionAnimation(Activity, View, String).
    // `activity` is the CALLING window (its shared views' bounds are captured when the new
    // window starts); the started window resolves targets by View::getTransitionName() == name.
    static ActivityOptions* makeSceneTransitionAnimation(Window* activity,
            View* sharedElement, const std::string& sharedElementName) {
        return makeSceneTransitionAnimation(activity,
            std::vector<std::pair<View*, std::string>>{{sharedElement, sharedElementName}});
    }

    // android.app.ActivityOptions#makeSceneTransitionAnimation(Activity, Pair<View, String>...).
    // Views/names are borrowed — they must stay attached to `activity` until the new window's
    // first traversal captures them. The vector is moved in.
    static ActivityOptions* makeSceneTransitionAnimation(Window* activity,
            std::vector<std::pair<View*, std::string>> sharedElements) {
        ActivityOptions* o = new ActivityOptions();
        o->mActivity = activity;
        o->mSharedElements = std::move(sharedElements);
        return o;
    }

    // The calling window (android's `activity` argument), borrowed.
    Window* getActivity() const { return mActivity; }
    const std::vector<std::pair<View*, std::string>>& getSharedElements() const { return mSharedElements; }
    bool hasSceneTransition() const { return !mSharedElements.empty(); }

private:
    ActivityOptions() = default;
    Window* mActivity = nullptr;                       // borrowed
    std::vector<std::pair<View*, std::string>> mSharedElements; // borrowed views + names
};

} // namespace cdroid
#endif
