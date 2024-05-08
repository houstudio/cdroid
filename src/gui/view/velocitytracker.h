#ifndef __VELOCITY_TRACKER_H__
#define __VELOCITY_TRACKER_H__
#include <map>
#include <array>
#include <memory>
#include <core/pools.h>
#include <view/motionevent.h>
namespace cdroid{
typedef int64_t nsecs_t;
class VelocityTrackerStrategy;
class VelocityTrackerImpl;
#define MAX_POINTERS 16
#define MAX_POINTER_ID 31
#define ACTIVE_POINTER_ID -1

////////////VelocityTracker.java///////////////////////////
//frameworks/base/core/jni/android_view_VelocityTracker.cpp
class VelocityTracker{
public:
    /**
     * Use the default Velocity Tracker Strategy. Different axes may use different default
     * strategies.
     */
    static constexpr int VELOCITY_TRACKER_STRATEGY_DEFAULT = -1;
    /**
     * Velocity Tracker Strategy: Impulse.
     * Physical model of pushing an object.  Quality: VERY GOOD.
     * Works with duplicate coordinates, unclean finger liftoff.
     */
    static constexpr int VELOCITY_TRACKER_STRATEGY_IMPULSE = 0;
    /**
    * Velocity Tracker Strategy: LSQ1.
    * 1st order least squares.  Quality: POOR.
    * Frequently underfits the touch data especially when the finger accelerates
    * or changes direction.  Often underestimates velocity.  The direction
    * is overly influenced by historical touch points.
    */
    static constexpr int VELOCITY_TRACKER_STRATEGY_LSQ1 = 1;
    /**
     * Velocity Tracker Strategy: LSQ2.
     * 2nd order least squares.  Quality: VERY GOOD.
     * Pretty much ideal, but can be confused by certain kinds of touch data,
     * particularly if the panel has a tendency to generate delayed,
     * duplicate or jittery touch coordinates when the finger is released.
     */
    static constexpr int VELOCITY_TRACKER_STRATEGY_LSQ2 = 2;
    /**
     * Velocity Tracker Strategy: LSQ3.
     * 3rd order least squares.  Quality: UNUSABLE.
     * Frequently overfits the touch data yielding wildly divergent estimates
     * of the velocity when the finger is released.
     */
    static constexpr int VELOCITY_TRACKER_STRATEGY_LSQ3 = 3;
    /**
     * Velocity Tracker Strategy: WLSQ2_DELTA.
     * 2nd order weighted least squares, delta weighting.  Quality: EXPERIMENTAL
     */
    static constexpr int VELOCITY_TRACKER_STRATEGY_WLSQ2_DELTA = 4;
    /**
     * Velocity Tracker Strategy: WLSQ2_CENTRAL.
     * 2nd order weighted least squares, central weighting.  Quality: EXPERIMENTALe
     */
    static constexpr int VELOCITY_TRACKER_STRATEGY_WLSQ2_CENTRAL = 5;
    /**
     * Velocity Tracker Strategy: WLSQ2_RECENT.
     * 2nd order weighted least squares, recent weighting.  Quality: EXPERIMENTAL
     */
    static constexpr int VELOCITY_TRACKER_STRATEGY_WLSQ2_RECENT = 6;
    /**
     * Velocity Tracker Strategy: INT1.
     * 1st order integrating filter.  Quality: GOOD.
     * Not as good as 'lsq2' because it cannot estimate acceleration but it is
     * more tolerant of errors.  Like 'lsq1', this strategy tends to underestimate
     * the velocity of a fling but this strategy tends to respond to changes in
     * direction more quickly and accurately.
     */
    static constexpr int VELOCITY_TRACKER_STRATEGY_INT1 = 7;
    /**
     * Velocity Tracker Strategy: INT2.
     * 2nd order integrating filter.  Quality: EXPERIMENTAL.
     * For comparison purposes only.  Unlike 'int1' this strategy can compensate
     * for acceleration but it typically overestimates the effect.
     */
    static constexpr int VELOCITY_TRACKER_STRATEGY_INT2 = 8;
    /**
     * Velocity Tracker Strategy: Legacy.
     * Legacy velocity tracker algorithm.  Quality: POOR.
     * For comparison purposes only.  This algorithm is strongly influenced by
     * old data points, consistently underestimates velocity and takes a very long
     * time to adjust to changes in direction.
     */
    static constexpr int VELOCITY_TRACKER_STRATEGY_LEGACY = 9;
public:
    class VelocityTrackerState;
    struct Estimator;
    struct ComputedVelocity;
private:
    int mStrategy;
    VelocityTrackerState*mTrackerState;
    static std::map<const std::string,int>STRATEGIES;
    static int toStrategyId(const char*);
    static Pools::SimplePool<VelocityTracker>sPool;
    friend Pools::SimplePool<VelocityTracker>;
    VelocityTracker(int strategy=0);
    ~VelocityTracker();
public:
    static VelocityTracker*obtain();
    static VelocityTracker*obtain(const char*);
    static VelocityTracker*obtain(int strategy);
    void recycle();
    void clear();
    static bool isAxisSupported(int axis);
    void addMovement(const MotionEvent& event);
    void computeCurrentVelocity(int units);
    void computeCurrentVelocity(int units,float maxVelocity);
    float getXVelocity();
    float getYVelocity();
    float getXVelocity(int pointerId);
    float getYVelocity(int pointerId);
    float getAxisVelocity(int axis);
    float getAxisVelocity(int axis,int pointerId);
};

struct VelocityTracker::Estimator {
    static constexpr size_t MAX_DEGREE = 4;
    // Estimator time base.
    nsecs_t time;
    // Polynomial coefficients describing motion.
    float coeff[MAX_DEGREE + 1];

    // Polynomial degree (number of coefficients), or zero if no information is
    // available.
    uint32_t degree;
    // Confidence (coefficient of determination), between 0 (no fit) and 1 (perfect fit).
    float confidence;
    Estimator();
    void clear();
};

/*Contains all available velocity data from a VelocityTracker.*/
struct VelocityTracker::ComputedVelocity {
    bool getVelocity(int32_t axis, int32_t id,float&outVelocity) const;
    inline void addVelocity(int32_t axis, int32_t id, float velocity) {
        mVelocities[axis][id] = velocity;
    }
private:
    std::map<int32_t /*axis*/, std::map<int32_t /*pointerId*/, float /*velocity*/>> mVelocities;
};

class VelocityTracker::VelocityTrackerState {
public:
    explicit VelocityTrackerState(int strategy);
    ~VelocityTrackerState();
    void clear();
    void addMovement(const MotionEvent& event);
    // TODO(b/32830165): consider supporting an overload that supports computing velocity only for
    // a subset of the supported axes.
    void computeCurrentVelocity(int32_t units, float maxVelocity);
    float getVelocity(int32_t axis, int32_t id);

private:
    VelocityTrackerImpl *mVelocityTracker;
    VelocityTracker::ComputedVelocity mComputedVelocity;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class VelocityTrackerStrategy {
protected:
    VelocityTrackerStrategy() { }
public:
    virtual ~VelocityTrackerStrategy() { }
    virtual void clearPointer(int32_t pointerId) = 0;
    virtual void addMovement(nsecs_t eventTime, int32_t pointerId,float positions) = 0;
    virtual bool getEstimator(int32_t pointerId, VelocityTracker::Estimator* outEstimator) const = 0;
};

class LeastSquaresVelocityTrackerStrategy : public VelocityTrackerStrategy {
public:
    enum Weighting {
        // No weights applied.  All data points are equally reliable.
        NONE,
        // Weight by time delta.  Data points clustered together are weighted less.
        DELTA,
        // Weight such that points within a certain horizon are weighed more than those
        // outside of that horizon.
        CENTRAL,
        // Weight such that points older than a certain amount are weighed less.
        RECENT,
    };
    // Degree must be no greater than Estimator::MAX_DEGREE.
    LeastSquaresVelocityTrackerStrategy(uint32_t degree, Weighting weighting = NONE);
    virtual ~LeastSquaresVelocityTrackerStrategy();

    void clearPointer(int32_t pointerId)override;
    void addMovement(nsecs_t eventTime, int32_t pointerId,float positions) override;
    bool getEstimator(int32_t pointerId, VelocityTracker::Estimator* outEstimator) const override;
private:
    // Sample horizon.
    // We don't use too much history by default since we want to react to quick
    // changes in direction.
    static const nsecs_t HORIZON = 100*1000000; //100ms
    // Number of samples to keep.
    static const uint32_t HISTORY_SIZE = 20;
    struct Movement {
        nsecs_t eventTime;
        float position ;
    };

    float chooseWeight(int32_t pointerId,uint32_t index) const;
    const uint32_t mDegree;
    const Weighting mWeighting;
    std::map<int32_t/*pointerId*/,size_t/*position inArray*/> mIndex;
    std::map<int32_t/*pointerId*/,std::array<Movement,HISTORY_SIZE>>mMovements;
};

class IntegratingVelocityTrackerStrategy : public VelocityTrackerStrategy {
public:
    // Degree must be 1 or 2.
    IntegratingVelocityTrackerStrategy(uint32_t degree);
    ~IntegratingVelocityTrackerStrategy();

    void clearPointer(int32_t pointerId) override;
    void addMovement(nsecs_t eventTime, int32_t pointerId,float positions) override;
    bool getEstimator(int32_t pointerId, VelocityTracker::Estimator* outEstimator) const override;

private:
    // Current state estimate for a particular pointer.
    struct State {
        nsecs_t updateTime;
        uint32_t degree;
        float pos, vel, accel;
    };

    const uint32_t mDegree;
    BitSet32 mPointerIdBits;
    State mPointerState[MAX_POINTER_ID + 1];

    void initState(State& state, nsecs_t eventTime, float pos) const;
    void updateState(State& state, nsecs_t eventTime, float pos) const;
    void populateEstimator(const State& state, VelocityTracker::Estimator* outEstimator) const;
};

/*
 * Velocity tracker strategy used prior to ICS.
 */
class LegacyVelocityTrackerStrategy : public VelocityTrackerStrategy {
public:
    LegacyVelocityTrackerStrategy();
    ~LegacyVelocityTrackerStrategy() override;

    void clearPointer(int32_t pointerId) override;
    void addMovement(nsecs_t eventTime, int32_t pointerId, float position) override;
    bool getEstimator(int32_t pointerId,VelocityTracker::Estimator*) const override;

private:
    // Oldest sample to consider when calculating the velocity.
    static const nsecs_t HORIZON = 200 * 1000000; // 100 ms

    // Number of samples to keep.
    static const uint32_t HISTORY_SIZE = 20;

    // The minimum duration between samples when estimating velocity.
    static const nsecs_t MIN_DURATION = 10 * 1000000; // 10 ms

    struct Movement {
        nsecs_t eventTime;
        float position;
    };
    std::map<int32_t /*pointerId*/, size_t /*positionInArray*/> mIndex;
    std::map<int32_t /*pointerId*/, std::array<Movement, HISTORY_SIZE>> mMovements;
};

class ImpulseVelocityTrackerStrategy : public VelocityTrackerStrategy {
public:
    ImpulseVelocityTrackerStrategy(bool deltaValues);
    virtual ~ImpulseVelocityTrackerStrategy();

    void clearPointer(int32_t pointerId) override;
    void addMovement(nsecs_t eventTime, int32_t pointer,float positions) override;
    bool getEstimator(int32_t pointerId, VelocityTracker::Estimator* outEstimator) const override;
private:
    // Sample horizon.
    // We don't use too much history by default since we want to react to quick
    // changes in direction.
    static constexpr nsecs_t HORIZON = 100*1000000; //ms

    // Number of samples to keep.
    static constexpr size_t HISTORY_SIZE = 20;

    struct Movement {
        nsecs_t eventTime;
        float position;
    };
    const bool mDeltaValues;
    std::map<int32_t /*pointerId*/, size_t /*positionInArray*/> mIndex;
    std::map<int32_t /*pointerId*/, std::array<Movement, HISTORY_SIZE>> mMovements;
};
}
#endif
