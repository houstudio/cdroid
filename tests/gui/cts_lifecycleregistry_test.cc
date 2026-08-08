// Port of androidx.lifecycle CommonLifecycleRegistryTest (the common/platform-agnostic state-
// machine tests). Pure LifecycleRegistry logic — the framework CTS has no lifecycle tests
// (lifecycle is androidx), but this is the official androidx reference.
//
// CDROID adaptation:
//  - Kotlin's TestObserver implements BOTH DefaultLifecycleObserver AND LifecycleEventObserver and
//    records per-callback call counts + onStateChangedEvents. CDROID's Lifecycling dispatches only
//    onStateChanged for a both-interface observer (not both, like androidx) — the per-callback
//    counts would diverge. So dispatch cases here use a LifecycleEventObserver-only collector and
//    assert the onStateChanged EVENT SEQUENCE (the call-count assertions are dropped). The both-
//    interface dispatch divergence is tracked as the Lifecycling ownership/dispatch P1.
//  - AOSP expects IllegalStateException; CDROID's LifecycleRegistry throws std::runtime_error with
//    the same intent — asserted via EXPECT_THROW(..., std::runtime_error) (message wording differs).
//
// Original: androidx lifecycle/lifecycle-runtime/src/commonTest/.../CommonLifecycleRegistryTest.kt (Apache 2.0)
#include <gtest/gtest.h>
#include <lifecycle/lifecycle.h>
#include <lifecycle/lifecycleowner.h>
#include <lifecycle/lifecycleregistry.h>
#include <lifecycle/lifecycleeventobserver.h>
#include <vector>
#include <memory>
#include <functional>

using namespace cdroid;
using namespace lifecycle;

namespace {
// A LifecycleOwner whose getLifecycle() returns a registry assigned after construction (mirrors
// AOSP's lateinit registry field — the registry is created from the owner, so the back-reference
// is wired post-construction).
class TestLifecycleOwner : public LifecycleOwner {
    LifecycleRegistry* mRegistry = nullptr;
public:
    void setRegistry(LifecycleRegistry* r) { mRegistry = r; }
    Lifecycle& getLifecycle() override { return *mRegistry; }
};

// Collects the onStateChanged event sequence (LifecycleEventObserver only — see header note).
class CollectingObserver : public LifecycleEventObserver {
public:
    std::vector<Lifecycle::Event> events;
    void onStateChanged(LifecycleOwner* /*source*/, Lifecycle::Event event) override {
        events.push_back(event);
    }
};

// A CollectingObserver that re-dispatches a given event from onStateChanged (for the reentrancy
// cases — e.g. constructionDestruction1 dispatches ON_PAUSE from the START callback).
class ReDispatchObserver : public LifecycleEventObserver {
public:
    std::vector<Lifecycle::Event> events;
    ReDispatchObserver(LifecycleRegistry* registry, Lifecycle::Event trigger, Lifecycle::Event fire)
        : mRegistry(registry), mTrigger(trigger), mFire(fire) {}
    void onStateChanged(LifecycleOwner* source, Lifecycle::Event event) override {
        events.push_back(event);
        if (event == mTrigger) mRegistry->handleLifecycleEvent(mFire);
    }
private:
    LifecycleRegistry* mRegistry;
    Lifecycle::Event mTrigger;
    Lifecycle::Event mFire;
};

// --- multi-observer dispatch-order recording (pure LifecycleEventObserver) ---
// The AOSP order cases assert a global (observer, event) sequence; we record (id, event) into a
// per-test shared vector. An optional action fires on each event (addObserver/remove/dispatch),
// mirroring AOSP observers that mutate the registry from a callback.
struct Recorded { int id; Lifecycle::Event e; };
static std::vector<Recorded> g_record;

class RecordingObserver : public LifecycleEventObserver {
    int mId;
    std::function<void(Lifecycle::Event)> mAction;
public:
    RecordingObserver(int id, std::function<void(Lifecycle::Event)> action = {})
        : mId(id), mAction(std::move(action)) {}
    void onStateChanged(LifecycleOwner*, Lifecycle::Event event) override {
        g_record.push_back({mId, event});
        if (mAction) mAction(event);
    }
};

// Asserts g_record matches the given (id, event) sequence exactly, in order.
::testing::AssertionResult recordEquals(const std::vector<Recorded>& expected) {
    if (g_record.size() != expected.size()) {
        return ::testing::AssertionFailure() << "size: got " << g_record.size()
            << " want " << expected.size();
    }
    for (size_t i = 0; i < expected.size(); i++) {
        if (g_record[i].id != expected[i].id || (int)g_record[i].e != (int)expected[i].e) {
            return ::testing::AssertionFailure() << "at " << i << ": got {"
                << g_record[i].id << "," << (int)g_record[i].e << "} want {"
                << expected[i].id << "," << (int)expected[i].e << "}";
        }
    }
    return ::testing::AssertionSuccess();
}
#define EV(id, event) Recorded{id, Lifecycle::Event::event}
} // namespace

class CtsLifecycleRegistryTest : public testing::Test {
protected:
    TestLifecycleOwner mOwner;
    std::unique_ptr<LifecycleRegistry> mRegistry;
    void SetUp() override {
        mRegistry.reset(LifecycleRegistry::createUnsafe(&mOwner));
        mOwner.setRegistry(mRegistry.get());
    }
    void dispatchEvent(Lifecycle::Event e) { mRegistry->handleLifecycleEvent(e); }
    void fullyInitializeRegistry() {
        dispatchEvent(Lifecycle::Event::ON_CREATE);
        dispatchEvent(Lifecycle::Event::ON_START);
        dispatchEvent(Lifecycle::Event::ON_RESUME);
    }
};

// --- state-machine cases (no observer) ---

TEST_F(CtsLifecycleRegistryTest, getCurrentState) {
    mRegistry->handleLifecycleEvent(Lifecycle::Event::ON_RESUME);
    EXPECT_EQ((int)Lifecycle::State::RESUMED, (int)mRegistry->getCurrentState());
    mRegistry->handleLifecycleEvent(Lifecycle::Event::ON_DESTROY);
    EXPECT_EQ((int)Lifecycle::State::DESTROYED, (int)mRegistry->getCurrentState());
}

TEST_F(CtsLifecycleRegistryTest, moveInitializedToDestroyed) {
    // INITIALIZED -> DESTROYED directly is illegal (must be at least CREATED).
    EXPECT_THROW(mRegistry->handleLifecycleEvent(Lifecycle::Event::ON_DESTROY), std::runtime_error);
}

TEST_F(CtsLifecycleRegistryTest, moveDestroyedToAny) {
    mRegistry->handleLifecycleEvent(Lifecycle::Event::ON_CREATE);
    mRegistry->handleLifecycleEvent(Lifecycle::Event::ON_DESTROY);
    // Once DESTROYED, the registry cannot be moved again.
    EXPECT_THROW(mRegistry->handleLifecycleEvent(Lifecycle::Event::ON_CREATE), std::runtime_error);
}

TEST_F(CtsLifecycleRegistryTest, setCurrentState) {
    mRegistry->setCurrentState(Lifecycle::State::RESUMED);
    EXPECT_EQ((int)Lifecycle::State::RESUMED, (int)mRegistry->getCurrentState());
}

// --- observer dispatch cases (event-sequence assertions) ---

TEST_F(CtsLifecycleRegistryTest, addObserverBringsObserverToCurrentState) {
    // AOSP addAndObserve: an observer added after the registry is at a state is brought up to it.
    CollectingObserver obs;
    mRegistry->addObserver(&obs);
    // registry is INITIALIZED — observer is brought to INITIALIZED (no event dispatched for that).
    EXPECT_TRUE(obs.events.empty());

    dispatchEvent(Lifecycle::Event::ON_CREATE);
    ASSERT_EQ(1u, obs.events.size());
    EXPECT_EQ((int)Lifecycle::Event::ON_CREATE, (int)obs.events[0]);
    dispatchEvent(Lifecycle::Event::ON_START);
    ASSERT_EQ(2u, obs.events.size());
    EXPECT_EQ((int)Lifecycle::Event::ON_START, (int)obs.events[1]);
}

TEST_F(CtsLifecycleRegistryTest, fullyInitializedObserverReceivesFullUpSequence) {
    // Adding an observer to a fully-initialized (RESUMED) registry brings it up through
    // ON_CREATE -> ON_START -> ON_RESUME.
    fullyInitializeRegistry();
    CollectingObserver obs;
    mRegistry->addObserver(&obs);
    ASSERT_EQ(3u, obs.events.size());
    EXPECT_EQ((int)Lifecycle::Event::ON_CREATE, (int)obs.events[0]);
    EXPECT_EQ((int)Lifecycle::Event::ON_START,  (int)obs.events[1]);
    EXPECT_EQ((int)Lifecycle::Event::ON_RESUME, (int)obs.events[2]);
}

TEST_F(CtsLifecycleRegistryTest, downEvents) {
    fullyInitializeRegistry();
    CollectingObserver obs;
    mRegistry->addObserver(&obs);
    obs.events.clear();
    dispatchEvent(Lifecycle::Event::ON_PAUSE);
    dispatchEvent(Lifecycle::Event::ON_STOP);
    dispatchEvent(Lifecycle::Event::ON_DESTROY);
    ASSERT_EQ(3u, obs.events.size());
    EXPECT_EQ((int)Lifecycle::Event::ON_PAUSE,  (int)obs.events[0]);
    EXPECT_EQ((int)Lifecycle::Event::ON_STOP,   (int)obs.events[1]);
    EXPECT_EQ((int)Lifecycle::Event::ON_DESTROY,(int)obs.events[2]);
}

TEST_F(CtsLifecycleRegistryTest, removeObserver) {
    // AOSP addRemove: removing an observer stops further dispatch to it.
    CollectingObserver obs;
    mRegistry->addObserver(&obs);
    dispatchEvent(Lifecycle::Event::ON_CREATE);
    ASSERT_EQ(1u, obs.events.size());
    mRegistry->removeObserver(&obs);
    dispatchEvent(Lifecycle::Event::ON_START);
    EXPECT_EQ(1u, obs.events.size());   // no further events after removal
}

TEST_F(CtsLifecycleRegistryTest, add2RemoveOne) {
    // AOSP add2RemoveOne: three observers all receive CREATE; after removing the middle one, only
    // the remaining two receive START.
    CollectingObserver obs1, obs2, obs3;
    mRegistry->addObserver(&obs1);
    mRegistry->addObserver(&obs2);
    mRegistry->addObserver(&obs3);
    dispatchEvent(Lifecycle::Event::ON_CREATE);
    EXPECT_EQ(1u, obs1.events.size());
    EXPECT_EQ(1u, obs2.events.size());
    EXPECT_EQ(1u, obs3.events.size());
    mRegistry->removeObserver(&obs2);
    dispatchEvent(Lifecycle::Event::ON_START);
    EXPECT_EQ(2u, obs1.events.size());  // CREATE + START
    EXPECT_EQ(1u, obs2.events.size());  // removed — no START
    EXPECT_EQ(2u, obs3.events.size());  // CREATE + START
}

TEST_F(CtsLifecycleRegistryTest, subscribeToDead) {
    // AOSP subscribeToDead: an observer added to a DESTROYED registry receives no events.
    dispatchEvent(Lifecycle::Event::ON_CREATE);
    CollectingObserver obs1;
    mRegistry->addObserver(&obs1);
    ASSERT_EQ(1u, obs1.events.size());
    EXPECT_EQ((int)Lifecycle::Event::ON_CREATE, (int)obs1.events[0]);
    dispatchEvent(Lifecycle::Event::ON_DESTROY);
    ASSERT_EQ(2u, obs1.events.size());
    EXPECT_EQ((int)Lifecycle::Event::ON_DESTROY, (int)obs1.events[1]);
    CollectingObserver obs2;
    mRegistry->addObserver(&obs2);
    EXPECT_TRUE(obs2.events.empty());   // added after DESTROY -> no events
}

TEST_F(CtsLifecycleRegistryTest, constructionDestruction1) {
    // AOSP constructionDestruction1: an observer added to a fully-initialized (RESUMED) registry
    // dispatches ON_PAUSE from its START callback. The registry's sync algorithm must handle the
    // reentrant downward event: the observer ends up at [CREATE, START] and never receives RESUME.
    fullyInitializeRegistry();
    ReDispatchObserver obs(mRegistry.get(), Lifecycle::Event::ON_START, Lifecycle::Event::ON_PAUSE);
    mRegistry->addObserver(&obs);
    ASSERT_EQ(2u, obs.events.size());
    EXPECT_EQ((int)Lifecycle::Event::ON_CREATE, (int)obs.events[0]);
    EXPECT_EQ((int)Lifecycle::Event::ON_START,  (int)obs.events[1]);
    // No RESUME dispatched (the reentrant PAUSE pulled the observer back down).
    bool hasResume = false;
    for (auto e : obs.events) if (e == Lifecycle::Event::ON_RESUME) hasResume = true;
    EXPECT_FALSE(hasResume);
}

TEST_F(CtsLifecycleRegistryTest, twoObserversChangingState) {
    // AOSP twoObserversChangingState: observer1 dispatches ON_START from its CREATE callback;
    // both observers end up receiving CREATE then START.
    g_record.clear();
    RecordingObserver obs2(2);
    RecordingObserver obs1(1, [this](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_CREATE) mRegistry->handleLifecycleEvent(Lifecycle::Event::ON_START);
    });
    mRegistry->addObserver(&obs1);
    mRegistry->addObserver(&obs2);
    dispatchEvent(Lifecycle::Event::ON_CREATE);
    auto countId = [](int id, Lifecycle::Event e){
        int c = 0; for (auto& r : g_record) if (r.id == id && (int)r.e == (int)e) c++; return c;
    };
    EXPECT_EQ(1, countId(1, Lifecycle::Event::ON_CREATE));
    EXPECT_EQ(1, countId(2, Lifecycle::Event::ON_CREATE));
    EXPECT_EQ(1, countId(1, Lifecycle::Event::ON_START));
    EXPECT_EQ(1, countId(2, Lifecycle::Event::ON_START));
}

TEST_F(CtsLifecycleRegistryTest, addDuringTraversing) {
    // AOSP addDuringTraversing: observer1 adds observer3 from its START callback (during the
    // CREATE->START forward pass). The new observer is brought up interleaved with the running
    // traversal in a specific order.
    g_record.clear();
    RecordingObserver obs3(3);
    RecordingObserver obs1(1, [this, &obs3](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_START) mRegistry->addObserver(&obs3);
    });
    RecordingObserver obs2(2);
    mRegistry->addObserver(&obs1);
    mRegistry->addObserver(&obs2);
    dispatchEvent(Lifecycle::Event::ON_CREATE);
    dispatchEvent(Lifecycle::Event::ON_START);
    EXPECT_TRUE(recordEquals({
        EV(1, ON_CREATE), EV(2, ON_CREATE),
        EV(1, ON_START),  EV(3, ON_CREATE),
        EV(2, ON_START),  EV(3, ON_START)
    }));
}

TEST_F(CtsLifecycleRegistryTest, addDuringAddition) {
    // AOSP addDuringAddition: observer1 adds observer2 from RESUME; observer2 adds observer3 from
    // CREATE. After driving to RESUMED, each observer has received CREATE, START, RESUME in order.
    g_record.clear();
    RecordingObserver obs3(3);
    RecordingObserver obs2(2, [this, &obs3](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_CREATE) mRegistry->addObserver(&obs3);
    });
    RecordingObserver obs1(1, [this, &obs2](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_RESUME) mRegistry->addObserver(&obs2);
    });
    mRegistry->addObserver(&obs1);
    dispatchEvent(Lifecycle::Event::ON_CREATE);
    dispatchEvent(Lifecycle::Event::ON_START);
    dispatchEvent(Lifecycle::Event::ON_RESUME);
    EXPECT_TRUE(recordEquals({
        EV(1, ON_CREATE), EV(1, ON_START), EV(1, ON_RESUME),
        EV(2, ON_CREATE), EV(2, ON_START), EV(2, ON_RESUME),
        EV(3, ON_CREATE), EV(3, ON_START), EV(3, ON_RESUME)
    }));
}

// A both-interfaces observer (DefaultLifecycleObserver + LifecycleEventObserver). CDROID's DLO/LEO
// virtually inherit LifecycleObserver, so the diamond resolves to a single base and this type is
// constructible/addable. Verifies Lifecycling dispatches BOTH the per-callback AND onStateChanged
// (the both-interface branch — previously unreachable due to the diamond, now exercised).
class BothObserver : public DefaultLifecycleObserver, public LifecycleEventObserver {
public:
    int onCreateCount = 0;
    std::vector<Lifecycle::Event> stateEvents;
    void onCreate(LifecycleOwner*) override { onCreateCount++; }
    void onStateChanged(LifecycleOwner*, Lifecycle::Event e) override { stateEvents.push_back(e); }
};

TEST_F(CtsLifecycleRegistryTest, bothInterfaceObserverDispatchesBoth) {
    BothObserver obs;
    mRegistry->addObserver(&obs);
    dispatchEvent(Lifecycle::Event::ON_CREATE);
    EXPECT_EQ(1, obs.onCreateCount);   // DefaultLifecycleObserver per-callback fired
    ASSERT_EQ(1u, obs.stateEvents.size());
    EXPECT_EQ((int)Lifecycle::Event::ON_CREATE, (int)obs.stateEvents[0]);  // onStateChanged fired
}

// Collects its own onStateChanged event sequence AND optionally fires an action per event (for
// cases like constructionDestruction2 that re-dispatch multiple events from a callback).
class ActionCollector : public LifecycleEventObserver {
public:
    std::vector<Lifecycle::Event> events;
    std::function<void(Lifecycle::Event)> action;
    explicit ActionCollector(std::function<void(Lifecycle::Event)> a = {}) : action(std::move(a)) {}
    void onStateChanged(LifecycleOwner*, Lifecycle::Event e) override {
        events.push_back(e);
        if (action) action(e);
    }
};

// True if `expected` appears as an ordered subsequence (not necessarily contiguous) of g_record.
bool containsInOrder(const std::vector<Recorded>& expected) {
    size_t i = 0;
    for (const auto& r : g_record) {
        if (i < expected.size() && r.id == expected[i].id && (int)r.e == (int)expected[i].e) i++;
    }
    return i == expected.size();
}

TEST_F(CtsLifecycleRegistryTest, constructionDestruction2) {
    // AOSP constructionDestruction2: observer dispatches PAUSE+STOP+DESTROY from its START
    // callback. The reentrant down-events pull it from STARTED straight down; RESUME is skipped.
    fullyInitializeRegistry();
    ActionCollector obs([this](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_START) {
            dispatchEvent(Lifecycle::Event::ON_PAUSE);
            dispatchEvent(Lifecycle::Event::ON_STOP);
            dispatchEvent(Lifecycle::Event::ON_DESTROY);
        }
    });
    mRegistry->addObserver(&obs);
    ASSERT_EQ(4u, obs.events.size());
    EXPECT_EQ((int)Lifecycle::Event::ON_CREATE,  (int)obs.events[0]);
    EXPECT_EQ((int)Lifecycle::Event::ON_START,   (int)obs.events[1]);
    EXPECT_EQ((int)Lifecycle::Event::ON_STOP,    (int)obs.events[2]);
    EXPECT_EQ((int)Lifecycle::Event::ON_DESTROY, (int)obs.events[3]);
    bool hasResume = false;
    for (auto e : obs.events) if (e == Lifecycle::Event::ON_RESUME) hasResume = true;
    EXPECT_FALSE(hasResume);
}

TEST_F(CtsLifecycleRegistryTest, downEventsAddition) {
    // AOSP downEventsAddition: during the STARTED->STOPPED down-pass, observer2's onStop adds
    // observer3; observer3 receives CREATE, then everyone receives DESTROY.
    g_record.clear();
    dispatchEvent(Lifecycle::Event::ON_CREATE);
    dispatchEvent(Lifecycle::Event::ON_START);
    RecordingObserver obs3(3);
    RecordingObserver obs2(2, [this, &obs3](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_STOP) mRegistry->addObserver(&obs3);
    });
    RecordingObserver obs1(1);
    mRegistry->addObserver(&obs1);
    mRegistry->addObserver(&obs2);
    dispatchEvent(Lifecycle::Event::ON_STOP);
    EXPECT_TRUE(containsInOrder({EV(2, ON_STOP), EV(3, ON_CREATE), EV(1, ON_STOP)}));
    dispatchEvent(Lifecycle::Event::ON_DESTROY);
    EXPECT_TRUE(containsInOrder({EV(3, ON_DESTROY), EV(2, ON_DESTROY), EV(1, ON_DESTROY)}));
}

TEST_F(CtsLifecycleRegistryTest, deadParentInAddition) {
    // AOSP deadParentInAddition: observer1 removes itself from onStart (during the up-pass to a
    // RESUMED registry) and adds observer2/3, which are then brought up to RESUMED.
    g_record.clear();
    RecordingObserver obs2(2), obs3(3);
    RecordingObserver* p1 = nullptr;
    RecordingObserver obs1(1, [this, &p1, &obs2, &obs3](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_START) {
            mRegistry->removeObserver(p1);
            EXPECT_EQ(0, mRegistry->getObserverCount());
            mRegistry->addObserver(&obs2);
            mRegistry->addObserver(&obs3);
        }
    });
    p1 = &obs1;
    fullyInitializeRegistry();
    mRegistry->addObserver(&obs1);
    EXPECT_TRUE(recordEquals({
        EV(1, ON_CREATE), EV(1, ON_START),
        EV(2, ON_CREATE), EV(3, ON_CREATE),
        EV(2, ON_START), EV(2, ON_RESUME),
        EV(3, ON_START), EV(3, ON_RESUME)
    }));
}

TEST_F(CtsLifecycleRegistryTest, deadParentWhileTraversing) {
    // AOSP deadParentWhileTraversing: same as deadParentInAddition but the registry is driven to
    // STARTED (not RESUMED), so observer2/3 are only brought up to STARTED.
    g_record.clear();
    RecordingObserver obs2(2), obs3(3);
    RecordingObserver* p1 = nullptr;
    RecordingObserver obs1(1, [this, &p1, &obs2, &obs3](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_START) {
            mRegistry->removeObserver(p1);
            mRegistry->addObserver(&obs2);
            mRegistry->addObserver(&obs3);
        }
    });
    p1 = &obs1;
    mRegistry->addObserver(&obs1);
    dispatchEvent(Lifecycle::Event::ON_CREATE);
    dispatchEvent(Lifecycle::Event::ON_START);
    EXPECT_TRUE(recordEquals({
        EV(1, ON_CREATE), EV(1, ON_START),
        EV(2, ON_CREATE), EV(3, ON_CREATE),
        EV(2, ON_START), EV(3, ON_START)
    }));
}

TEST_F(CtsLifecycleRegistryTest, removeCascade) {
    // AOSP removeCascade: observer1 adds observer2/3/4 from RESUME; observer2 removes itself from
    // START (so it never reaches RESUME). The remaining observers finish the up-pass.
    g_record.clear();
    RecordingObserver obs3(3), obs4(4);
    RecordingObserver* p2 = nullptr;
    RecordingObserver obs2(2, [this, &p2](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_START) mRegistry->removeObserver(p2);
    });
    RecordingObserver obs1(1, [this, &obs2, &obs3, &obs4](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_RESUME) {
            mRegistry->addObserver(&obs2);
            mRegistry->addObserver(&obs3);
            mRegistry->addObserver(&obs4);
        }
    });
    p2 = &obs2;
    fullyInitializeRegistry();
    mRegistry->addObserver(&obs1);
    EXPECT_TRUE(recordEquals({
        EV(1, ON_CREATE), EV(1, ON_START), EV(1, ON_RESUME),
        EV(2, ON_CREATE), EV(2, ON_START),
        EV(3, ON_CREATE), EV(3, ON_START),
        EV(4, ON_CREATE), EV(4, ON_START),
        EV(3, ON_RESUME), EV(4, ON_RESUME)
    }));
}

TEST_F(CtsLifecycleRegistryTest, changeStateDuringDescending) {
    // AOSP changeStateDuringDescending: observer1 re-dispatches RESUME and adds observer2 from its
    // PAUSE callback (during the RESUMED->CREATED down-pass), reversing direction mid-traversal.
    g_record.clear();
    RecordingObserver obs2(2);
    RecordingObserver obs1(1, [this, &obs2](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_PAUSE) {
            dispatchEvent(Lifecycle::Event::ON_RESUME);
            mRegistry->addObserver(&obs2);
        }
    });
    fullyInitializeRegistry();
    mRegistry->addObserver(&obs1);
    mRegistry->handleLifecycleEvent(Lifecycle::Event::ON_PAUSE);
    EXPECT_TRUE(containsInOrder({
        EV(1, ON_PAUSE), EV(2, ON_CREATE), EV(2, ON_START),
        EV(1, ON_RESUME), EV(2, ON_RESUME)
    }));
}

TEST_F(CtsLifecycleRegistryTest, siblingLimitationCheck) {
    // AOSP siblingLimitationCheck: observer1 adds observer2 from START and observer3 from RESUME.
    g_record.clear();
    RecordingObserver obs2(2), obs3(3);
    RecordingObserver obs1(1, [this, &obs2, &obs3](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_START)  mRegistry->addObserver(&obs2);
        if (e == Lifecycle::Event::ON_RESUME) mRegistry->addObserver(&obs3);
    });
    fullyInitializeRegistry();
    mRegistry->addObserver(&obs1);
    EXPECT_TRUE(recordEquals({
        EV(1, ON_CREATE), EV(1, ON_START), EV(2, ON_CREATE),
        EV(1, ON_RESUME), EV(3, ON_CREATE),
        EV(2, ON_START), EV(2, ON_RESUME),
        EV(3, ON_START), EV(3, ON_RESUME)
    }));
}

TEST_F(CtsLifecycleRegistryTest, siblingRemovalLimitationCheck1) {
    // AOSP siblingRemovalLimitationCheck1: observer1 adds observer2 from START, then from RESUME
    // removes observer2 and adds observer3/4 (observer2 stops at CREATED).
    g_record.clear();
    RecordingObserver obs3(3), obs4(4);
    RecordingObserver obs2(2);
    RecordingObserver obs1(1, [this, &obs2, &obs3, &obs4](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_START) {
            mRegistry->addObserver(&obs2);
        } else if (e == Lifecycle::Event::ON_RESUME) {
            mRegistry->removeObserver(&obs2);
            mRegistry->addObserver(&obs3);
            mRegistry->addObserver(&obs4);
        }
    });
    fullyInitializeRegistry();
    mRegistry->addObserver(&obs1);
    EXPECT_TRUE(recordEquals({
        EV(1, ON_CREATE), EV(1, ON_START), EV(2, ON_CREATE),
        EV(1, ON_RESUME),
        EV(3, ON_CREATE), EV(3, ON_START),
        EV(4, ON_CREATE), EV(4, ON_START),
        EV(3, ON_RESUME), EV(4, ON_RESUME)
    }));
}

TEST_F(CtsLifecycleRegistryTest, siblingRemovalLimitationCheck2) {
    // AOSP siblingRemovalLimitationCheck2: observer1 adds observer2 from START; from RESUME adds
    // observer3 (whose onCreate removes observer2) and observer4.
    g_record.clear();
    RecordingObserver obs4(4);
    RecordingObserver* p2 = nullptr;
    RecordingObserver obs3(3, [this, &p2](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_CREATE && p2) mRegistry->removeObserver(p2);
    });
    RecordingObserver obs2(2);
    RecordingObserver obs1(1, [this, &obs2, &obs3, &obs4](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_START)  mRegistry->addObserver(&obs2);
        if (e == Lifecycle::Event::ON_RESUME) { mRegistry->addObserver(&obs3); mRegistry->addObserver(&obs4); }
    });
    p2 = &obs2;
    fullyInitializeRegistry();
    mRegistry->addObserver(&obs1);
    EXPECT_TRUE(recordEquals({
        EV(1, ON_CREATE), EV(1, ON_START), EV(2, ON_CREATE),
        EV(1, ON_RESUME),
        EV(3, ON_CREATE), EV(3, ON_START),
        EV(4, ON_CREATE), EV(4, ON_START),
        EV(3, ON_RESUME), EV(4, ON_RESUME)
    }));
}

TEST_F(CtsLifecycleRegistryTest, downEventsRemoveAll) {
    // AOSP downEventsRemoveAll: during the STOPPED down-pass, observer2's onStop removes everyone
    // (incl. itself); observer1 never receives STOP. PAUSE is delivered to all in reverse order.
    g_record.clear();
    fullyInitializeRegistry();
    RecordingObserver obs1(1), obs3(3);
    RecordingObserver* p2 = nullptr;
    RecordingObserver obs2(2, [this, &p1 = p2, &obs1, &obs3](Lifecycle::Event e){
        if (e == Lifecycle::Event::ON_STOP) {
            mRegistry->removeObserver(&obs3);
            mRegistry->removeObserver(p1);
            mRegistry->removeObserver(&obs1);
            EXPECT_EQ(0, mRegistry->getObserverCount());
        }
    });
    p2 = &obs2;
    mRegistry->addObserver(&obs1);
    mRegistry->addObserver(&obs2);
    mRegistry->addObserver(&obs3);
    dispatchEvent(Lifecycle::Event::ON_PAUSE);
    EXPECT_TRUE(containsInOrder({EV(3, ON_PAUSE), EV(2, ON_PAUSE), EV(1, ON_PAUSE)}));
    dispatchEvent(Lifecycle::Event::ON_STOP);
    // observer1 was removed before its STOP fired.
    int obs1Stop = 0; for (auto& r : g_record) if (r.id == 1 && (int)r.e == (int)Lifecycle::Event::ON_STOP) obs1Stop++;
    EXPECT_EQ(0, obs1Stop);
}

TEST_F(CtsLifecycleRegistryTest, sameObserverReAddition) {
    // AOSP sameObserverReAddition: add -> remove -> RE-ADD the same observer; the subsequent
    // CREATE dispatch delivers ON_CREATE to it (the re-added observer is back in the map).
    CollectingObserver obs;
    mRegistry->addObserver(&obs);
    mRegistry->removeObserver(&obs);
    mRegistry->addObserver(&obs);   // re-add the same observer
    dispatchEvent(Lifecycle::Event::ON_CREATE);
    bool gotCreate = false;
    for (auto e : obs.events) if (e == Lifecycle::Event::ON_CREATE) gotCreate = true;
    EXPECT_TRUE(gotCreate);
}
