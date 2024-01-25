#ifndef __VELOCITY_TRACKER_H__
#define __VELOCITY_TRACKER_H__

#include <queue>
#include <core/uievents.h>
namespace cdroid{
typedef int64_t nsecs_t;
class VelocityTrackerStrategy;

#define MAX_POINTERS 16
#define MAX_POINTER_ID 31
#define ACTIVE_POINTER_ID -1

    
////////////VelocityTracker.java///////////////////////////
//frameworks/base/core/jni/android_view_VelocityTracker.cpp
class VelocityTracker{
private:
    struct Velocity {
        float vx, vy;
    };
    class VelocityTrackerImpl* mVelocityTracker;
    int32_t mActivePointerId;
    BitSet32 mCalculatedIdBits;
    Velocity mCalculatedVelocity[MAX_POINTERS];
    static std::queue<VelocityTracker*>sPool;
public:
    struct Position {
        float x, y;
    };
    struct Estimator {
        static constexpr size_t MAX_DEGREE = 4;
        // Estimator time base.
        nsecs_t time;
        // Polynomial coefficients describing motion in X and Y.
        float xCoeff[MAX_DEGREE + 1], yCoeff[MAX_DEGREE + 1];

        // Polynomial degree (number of coefficients), or zero if no information is
        // available.
        uint32_t degree;
        // Confidence (coefficient of determination), between 0 (no fit) and 1 (perfect fit).
        float confidence;
        inline void clear() {
            time = 0;
            degree = 0;
            confidence = 0;
            for (size_t i = 0; i <= MAX_DEGREE; i++) {
                xCoeff[i] = 0;
                yCoeff[i] = 0;
            }
        }
    };

public:
    explicit VelocityTracker(const char* strategy);
    ~VelocityTracker();
    void clear();
    void addMovement(const MotionEvent& event);
    void computeCurrentVelocity(int32_t units);
    void computeCurrentVelocity(int32_t units, float maxVelocity);
    void getVelocity(int32_t id, float* outVx, float* outVy);
    bool getEstimator(int32_t id, Estimator* outEstimator);
    float getXVelocity();
    float getXVelocity(int);
    float getYVelocity();
    float getYVelocity(int);
    static VelocityTracker*obtain(const char*name=nullptr);
    void recycle(); 
};
    
class VelocityTrackerStrategy {
protected:
    VelocityTrackerStrategy() { }
public:
    virtual ~VelocityTrackerStrategy() { }
    virtual void clear() = 0;
    virtual void clearPointers(BitSet32 idBits) = 0;
    virtual void addMovement(nsecs_t eventTime, BitSet32 idBits,
            const VelocityTracker::Position* positions) = 0;
    virtual bool getEstimator(uint32_t id, VelocityTracker::Estimator* outEstimator) const = 0;
};

class IntegratingVelocityTrackerStrategy : public VelocityTrackerStrategy {
public:
    // Degree must be 1 or 2.
    IntegratingVelocityTrackerStrategy(uint32_t degree);
    ~IntegratingVelocityTrackerStrategy();

    virtual void clear();
    virtual void clearPointers(BitSet32 idBits);
    virtual void addMovement(nsecs_t eventTime, BitSet32 idBits,
            const VelocityTracker::Position* positions);
    virtual bool getEstimator(uint32_t id, VelocityTracker::Estimator* outEstimator) const;

private:
    // Current state estimate for a particular pointer.
    struct State {
        nsecs_t updateTime;
        uint32_t degree;

        float xpos, xvel, xaccel;
        float ypos, yvel, yaccel;
    };

    const uint32_t mDegree;
    BitSet32 mPointerIdBits;
    State mPointerState[MAX_POINTER_ID + 1];

    void initState(State& state, nsecs_t eventTime, float xpos, float ypos) const;
    void updateState(State& state, nsecs_t eventTime, float xpos, float ypos) const;
    void populateEstimator(const State& state, VelocityTracker::Estimator* outEstimator) const;
};

class LeastSquaresVelocityTrackerStrategy : public VelocityTrackerStrategy {
public:
    enum Weighting {
        // No weights applied.  All data points are equally reliable.
        WEIGHTING_NONE,
        // Weight by time delta.  Data points clustered together are weighted less.
        WEIGHTING_DELTA,
        // Weight such that points within a certain horizon are weighed more than those
        // outside of that horizon.
        WEIGHTING_CENTRAL,
        // Weight such that points older than a certain amount are weighed less.
        WEIGHTING_RECENT,
    };
    // Degree must be no greater than Estimator::MAX_DEGREE.
    LeastSquaresVelocityTrackerStrategy(uint32_t degree, Weighting weighting = WEIGHTING_NONE);
    virtual ~LeastSquaresVelocityTrackerStrategy();

    virtual void clear();
    virtual void clearPointers(BitSet32 idBits);
    virtual void addMovement(nsecs_t eventTime, BitSet32 idBits,
            const VelocityTracker::Position* positions);
    virtual bool getEstimator(uint32_t id, VelocityTracker::Estimator* outEstimator) const;
private:
    // Sample horizon.
    // We don't use too much history by default since we want to react to quick
    // changes in direction.
    static const nsecs_t HORIZON = 100*1000000; //100ms
    // Number of samples to keep.
    static const uint32_t HISTORY_SIZE = 20;
    struct Movement {
        nsecs_t eventTime;
        BitSet32 idBits;
        VelocityTracker::Position positions[MAX_POINTERS];

        inline const VelocityTracker::Position& getPosition(uint32_t id) const {
            return positions[idBits.getIndexOfBit(id)];
        }
    };

    float chooseWeight(uint32_t index) const;
    const uint32_t mDegree;
    const Weighting mWeighting;
    uint32_t mIndex;
    Movement mMovements[HISTORY_SIZE];
};

class ImpulseVelocityTrackerStrategy : public VelocityTrackerStrategy {
public:
    ImpulseVelocityTrackerStrategy();
    virtual ~ImpulseVelocityTrackerStrategy();

    virtual void clear();
    virtual void clearPointers(BitSet32 idBits);
    virtual void addMovement(nsecs_t eventTime, BitSet32 idBits,
            const VelocityTracker::Position* positions);
    virtual bool getEstimator(uint32_t id, VelocityTracker::Estimator* outEstimator) const;

private:
    // Sample horizon.
    // We don't use too much history by default since we want to react to quick
    // changes in direction.
    static constexpr nsecs_t HORIZON = 100*1000000; //ms

    // Number of samples to keep.
    static constexpr size_t HISTORY_SIZE = 20;

    struct Movement {
        nsecs_t eventTime;
        BitSet32 idBits;
        VelocityTracker::Position positions[MAX_POINTERS];

        inline const VelocityTracker::Position& getPosition(uint32_t id) const {
            return positions[idBits.getIndexOfBit(id)];
        }
    };

    size_t mIndex;
    Movement mMovements[HISTORY_SIZE];
};
}
#endif
