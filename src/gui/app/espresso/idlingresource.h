#ifndef CDROID_ESPRESSO_IDLINGRESOURCE_H
#define CDROID_ESPRESSO_IDLINGRESOURCE_H

/*
 * android.support.test.espresso.IdlingResource — represents a resource that
 * can be busy while the test is running; Espresso syncs on it before every
 * action/assertion.
 *
 * Plus the slice of base.IdlingResourceRegistry the framework needs: the
 * registry Espresso.registerIdlingResources() feeds. CDROID has no
 * AsyncTask/Loader ecosystem today, so the async/compat monitors of
 * UiControllerImpl are dropped and this registry is the only dynamic idle
 * signal. Deviation (documented): AOSP's notifyWhenAllResourcesAreIdle drives
 * the DYNAMIC_TASKS_HAVE_IDLED signal through callbacks; the port polls
 * allResourcesAreIdle() from inside loopUntil — same observable semantics
 * (condition not satisfied until every resource reports idle), no thread hop.
 */

#include <memory>
#include <string>
#include <vector>

namespace cdroid {
namespace espresso {

class IdlingResource {
public:
    /** Called by Espresso on a background thread; keep it cheap. */
    class ResourceCallback {
    public:
        virtual ~ResourceCallback() = default;
        /** Called when the resource transitions to idle. */
        virtual void onTransitionToIdle() = 0;
    };

    virtual ~IdlingResource() = default;

    /** A unique, non-null, printable name for the resource. */
    virtual std::string getName() const = 0;

    /** False while the resource is still doing work. */
    virtual bool isIdleNow() const = 0;

    /** Registers the given callback. May be a no-op if isIdleNow polling suffices. */
    virtual void registerIdleTransitionCallback(ResourceCallback* callback) = 0;
};

using IdlingResourcePtr = std::shared_ptr<IdlingResource>;

/** base.IdlingResourceRegistry — Espresso's REGISTRY (poll-based port). */
class IdlingResourceRegistry {
public:
    static IdlingResourceRegistry& getInstance();

    void registerResources(const std::vector<IdlingResourcePtr>& resources);
    bool unregisterResources(const std::vector<IdlingResourcePtr>& resources);
    std::vector<IdlingResourcePtr> getResources() const;

    /** All registered resources are idle (true when none are registered). */
    bool allResourcesAreIdle() const;

    /** Names of the currently busy resources (timeout diagnostics). */
    std::vector<std::string> getBusyResourceNames() const;

    void cancelIdleMonitor() {}

private:
    IdlingResourceRegistry() = default;
    std::vector<IdlingResourcePtr> mResources;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_IDLINGRESOURCE_H*/
