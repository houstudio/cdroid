#include <view/velocitytracker.h>
#include <math.h>
#include <string.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <limits.h>

namespace cdroid{

// Threshold for determining that a pointer has stopped moving.
// Some input devices do not send ACTION_MOVE events in the case where a pointer has
// stopped.  We need to detect this case so that we can accurately predict the
// velocity after the pointer starts moving again.

static const nsecs_t ASSUME_POINTER_STOPPED_TIME = 40;/*Millisecond*/

static float vectorDot(const float* a, const float* b, uint32_t m) {
    float r = 0;
    for (size_t i = 0; i < m; i++) {
        r += *(a++) * *(b++);
    }
    return r;
}

static float vectorNorm(const float* a, uint32_t m) {
    float r = 0;
    for (size_t i = 0; i < m; i++) {
        float t = *(a++);
        r += t * t;
    }
    return sqrtf(r);
}

#if DEBUG_STRATEGY || DEBUG_VELOCITY
static std::string vectorToString(const float* a, uint32_t m) {
    std::ostringstream ostr;
    ostr << "[";
    for (size_t i = 0; i < m; i++) {
        if (i) {
            ostr << ",";
        }
        ostr << *(a++);
    }
    ostr << " ]";
    return ostr.str();
}
#endif

#if DEBUG_STRATEGY
static std::string matrixToString(const float* a, uint32_t m, uint32_t n, bool rowMajor) {
    std::ostringstream ostr;
    ostr << "[";
    for (size_t i = 0; i < m; i++) {
        if (i) {
            ostr << ",";
        }
        ostr << " [";
        for (size_t j = 0; j < n; j++) {
            if (j) {
                ostr << ",";
            }
            ostr << a[rowMajor ? i * n + j : j * m + i];
        }
        ostr << " ]";
    }
    ostr << " ]";
    return ostr.str();
}
#endif
class VelocityTrackerImpl {//from VelocityTracker.cpp
public:

    // Creates a velocity tracker using the specified strategy.
    // If strategy is NULL, uses the default strategy for the platform.
    VelocityTrackerImpl(const char* strategy = nullptr);

    ~VelocityTrackerImpl();

    // Resets the velocity tracker state.
    void clear();

    // Resets the velocity tracker state for specific pointers.
    // Call this method when some pointers have changed and may be reusing
    // an id that was assigned to a different pointer earlier.
    void clearPointers(BitSet32 idBits);

    // Adds movement information for a set of pointers.
    // The idBits bitfield specifies the pointer ids of the pointers whose positions
    // are included in the movement.
    // The positions array contains position information for each pointer in order by
    // increasing id.  Its size should be equal to the number of one bits in idBits.
    void addMovement(nsecs_t eventTime, BitSet32 idBits, const VelocityTracker::Position* positions);

    // Adds movement information for all pointers in a MotionEvent, including historical samples.
    void addMovement(const MotionEvent& event);

    // Gets the velocity of the specified pointer id in position units per second.
    // Returns false and sets the velocity components to zero if there is
    // insufficient movement information for the pointer.
    bool getVelocity(uint32_t id, float* outVx, float* outVy) const;

    // Gets an estimator for the recent movements of the specified pointer id.
    // Returns false and clears the estimator if there is no information available
    // about the pointer.
    bool getEstimator(uint32_t id, VelocityTracker::Estimator* outEstimator) const;

    // Gets the active pointer id, or -1 if none.
    inline int32_t getActivePointerId() const { return mActivePointerId; }

    // Gets a bitset containing all pointer ids from the most recent movement.
    inline BitSet32 getCurrentPointerIdBits() const { return mCurrentPointerIdBits; }
private:
    static const char* DEFAULT_STRATEGY;
    nsecs_t mLastEventTime;
    BitSet32 mCurrentPointerIdBits;
    int32_t mActivePointerId;
    VelocityTrackerStrategy* mStrategy;

    bool configureStrategy(const char* strategy);

    static VelocityTrackerStrategy* createStrategy(const char* strategy);
};
const char* VelocityTrackerImpl::DEFAULT_STRATEGY = "impulse";//"lsq2";

VelocityTrackerImpl::VelocityTrackerImpl(const char* strategy) :
    mLastEventTime(0), mCurrentPointerIdBits(0), mActivePointerId(-1) {

    // Allow the default strategy to be overridden using a system property for debugging.
    if (!strategy) strategy = DEFAULT_STRATEGY;

    // Configure the strategy.
    if (!configureStrategy(strategy)) {
        LOGW("Unrecognized velocity tracker strategy name '%s'.", strategy);
        if (!configureStrategy(DEFAULT_STRATEGY)) {
            LOGW("Could not create the default velocity tracker strategy '%s'!", strategy);
        }
    }
}

VelocityTrackerImpl::~VelocityTrackerImpl() {
    delete mStrategy;
}

bool VelocityTrackerImpl::configureStrategy(const char* strategy) {
    mStrategy = createStrategy(strategy);
    return mStrategy != NULL;
}

VelocityTrackerStrategy* VelocityTrackerImpl::createStrategy(const char* strategy) {
    if (!strcmp("impulse", strategy)) {
        // Physical model of pushing an object.  Quality: VERY GOOD.
        // Works with duplicate coordinates, unclean finger liftoff.
        return new ImpulseVelocityTrackerStrategy();
    }
    if (!strcmp("lsq1", strategy)) {
        // 1st order least squares.  Quality: POOR.
        // Frequently underfits the touch data especially when the finger accelerates
        // or changes direction.  Often underestimates velocity.  The direction
        // is overly influenced by historical touch points.
        return new LeastSquaresVelocityTrackerStrategy(1);
    }
    if (!strcmp("lsq2", strategy)) {
        // 2nd order least squares.  Quality: VERY GOOD.
        // Pretty much ideal, but can be confused by certain kinds of touch data,
        // particularly if the panel has a tendency to generate delayed,
        // duplicate or jittery touch coordinates when the finger is released.
        return new LeastSquaresVelocityTrackerStrategy(2);
    }
    if (!strcmp("lsq3", strategy)) {
        // 3rd order least squares.  Quality: UNUSABLE.
        // Frequently overfits the touch data yielding wildly divergent estimates
        // of the velocity when the finger is released.
        return new LeastSquaresVelocityTrackerStrategy(3);
    }
    if (!strcmp("wlsq2-delta", strategy)) {
        // 2nd order weighted least squares, delta weighting.  Quality: EXPERIMENTAL
        return new LeastSquaresVelocityTrackerStrategy(2, LeastSquaresVelocityTrackerStrategy::WEIGHTING_DELTA);
    }
    if (!strcmp("wlsq2-central", strategy)) {
        // 2nd order weighted least squares, central weighting.  Quality: EXPERIMENTAL
        return new LeastSquaresVelocityTrackerStrategy(2, LeastSquaresVelocityTrackerStrategy::WEIGHTING_CENTRAL);
    }
    if (!strcmp("wlsq2-recent", strategy)) {
        // 2nd order weighted least squares, recent weighting.  Quality: EXPERIMENTAL
        return new LeastSquaresVelocityTrackerStrategy(2, LeastSquaresVelocityTrackerStrategy::WEIGHTING_RECENT);
    }
    if (!strcmp("int1", strategy)) {
        // 1st order integrating filter.  Quality: GOOD.
        // Not as good as 'lsq2' because it cannot estimate acceleration but it is
        // more tolerant of errors.  Like 'lsq1', this strategy tends to underestimate
        // the velocity of a fling but this strategy tends to respond to changes in
        // direction more quickly and accurately.
        return new IntegratingVelocityTrackerStrategy(1);
    }
    if (!strcmp("int2", strategy)) {
        // 2nd order integrating filter.  Quality: EXPERIMENTAL.
        // For comparison purposes only.  Unlike 'int1' this strategy can compensate
        // for acceleration but it typically overestimates the effect.
        return new IntegratingVelocityTrackerStrategy(2);
    }
    return NULL;
}

void VelocityTrackerImpl::clear() {
    mCurrentPointerIdBits.clear();
    mActivePointerId = -1;

    mStrategy->clear();
}

void VelocityTrackerImpl::clearPointers(BitSet32 idBits) {
    BitSet32 remainingIdBits(mCurrentPointerIdBits.value & ~idBits.value);
    mCurrentPointerIdBits = remainingIdBits;

    if (mActivePointerId >= 0 && idBits.hasBit(mActivePointerId)) {
        mActivePointerId = !remainingIdBits.isEmpty() ? remainingIdBits.firstMarkedBit() : -1;
    }

    mStrategy->clearPointers(idBits);
}

void VelocityTrackerImpl::addMovement(nsecs_t eventTime, BitSet32 idBits, const VelocityTracker::Position* positions) {
    while (idBits.count() > MAX_POINTERS) {
        idBits.clearLastMarkedBit();
    }

    if ((mCurrentPointerIdBits.value & idBits.value)
            && eventTime >= mLastEventTime + ASSUME_POINTER_STOPPED_TIME) {
#if DEBUG_VELOCITY
        LOGD("VelocityTracker: stopped for %0.3f ms, clearing state. eventTime=%lld mLastEventTime=%lld",
                (eventTime - mLastEventTime) * 0.001f,eventTime,mLastEventTime);
#endif
        // We have not received any movements for too long.  Assume that all pointers
        // have stopped.
        mStrategy->clear();
    }
    mLastEventTime = eventTime;

    mCurrentPointerIdBits = idBits;
    if (mActivePointerId < 0 || !idBits.hasBit(mActivePointerId)) {
        mActivePointerId = idBits.isEmpty() ? -1 : idBits.firstMarkedBit();
    }

    mStrategy->addMovement(eventTime, idBits, positions);

#if DEBUG_VELOCITY
    LOGD("VelocityTracker: addMovement eventTime=%lld, idBits=0x%08x, activePointerId=%d  xy=%f,%f",
            eventTime, idBits.value, mActivePointerId,positions[0].x,positions[0].y);
    for (BitSet32 iterBits(idBits); !iterBits.isEmpty(); ) {
        uint32_t id = iterBits.firstMarkedBit();
        uint32_t index = idBits.getIndexOfBit(id);
        iterBits.clearBit(id);
        VelocityTracker::Estimator estimator;
        getEstimator(id, &estimator);
        LOGD("  %d: position (%0.3f, %0.3f),time=%lld estimator (degree=%d, xCoeff=%s, yCoeff=%s, confidence=%f)",
                id, positions[index].x, positions[index].y, eventTime,int(estimator.degree),
                vectorToString(estimator.xCoeff, estimator.degree + 1).c_str(),
                vectorToString(estimator.yCoeff, estimator.degree + 1).c_str(),
                estimator.confidence);
    }
#endif
}

void VelocityTrackerImpl::addMovement(const MotionEvent& event) {
    int32_t actionMasked = event.getActionMasked();

    switch (actionMasked) {
    case MotionEvent::ACTION_DOWN:
    case MotionEvent::ACTION_HOVER_ENTER:
        // Clear all pointers on down before adding the new movement.
        clear();
        break;
    case MotionEvent::ACTION_POINTER_DOWN: {
        // Start a new movement trace for a pointer that just went down.
        // We do this on down instead of on up because the client may want to query the
        // final velocity for a pointer that just went up.
        BitSet32 downIdBits;
        downIdBits.markBit(event.getPointerId(event.getActionIndex()));
        clearPointers(downIdBits);
        break;
    }
    case MotionEvent::ACTION_MOVE:
    case MotionEvent::ACTION_HOVER_MOVE:
        break;
    default:
        // Ignore all other actions because they do not convey any new information about
        // pointer movement.  We also want to preserve the last known velocity of the pointers.
        // Note that ACTION_UP and ACTION_POINTER_UP always report the last known position
        // of the pointers that went up.  ACTION_POINTER_UP does include the new position of
        // pointers that remained down but we will also receive an ACTION_MOVE with this
        // information if any of them actually moved.  Since we don't know how many pointers
        // will be going up at once it makes sense to just wait for the following ACTION_MOVE
        // before adding the movement.
        return;
    }

    size_t pointerCount = event.getPointerCount();
    if (pointerCount > MAX_POINTERS) {
        pointerCount = MAX_POINTERS;
    }

    BitSet32 idBits;
    for (size_t i = 0; i < pointerCount; i++) {
        idBits.markBit(event.getPointerId(i));
    }

    uint32_t pointerIndex[MAX_POINTERS]={0};
    for (size_t i = 0; i < pointerCount; i++) {
        pointerIndex[i] = idBits.getIndexOfBit(event.getPointerId(i));
    }

    nsecs_t eventTime;
    VelocityTracker::Position positions[pointerCount];

    size_t historySize = event.getHistorySize();
    memset(positions,0,sizeof(positions));
    for (size_t h = 0; h < historySize; h++) {
        eventTime = event.getHistoricalEventTime(h);
        for (size_t i = 0; i < pointerCount; i++) {
            uint32_t index = pointerIndex[i];
            positions[index].x = event.getHistoricalRawX(i, h);
            positions[index].y = event.getHistoricalRawY(i, h);
        }
        addMovement(eventTime, idBits, positions);
    }

    eventTime = event.getEventTime();
    for (size_t i = 0; i < pointerCount; i++) {
        uint32_t index = pointerIndex[i];
        positions[index].x = event.getRawX(i);
        positions[index].y = event.getRawY(i);
    }
    addMovement(eventTime, idBits, positions);
}

bool VelocityTrackerImpl::getVelocity(uint32_t id, float* outVx, float* outVy) const {
    VelocityTracker::Estimator estimator;
    if (getEstimator(id, &estimator) && estimator.degree >= 1) {
        *outVx = estimator.xCoeff[1];
        *outVy = estimator.yCoeff[1];
        return true;
    }
    *outVx = 0;
    *outVy = 0;
    return false;
}

bool VelocityTrackerImpl::getEstimator(uint32_t id,VelocityTracker::Estimator* outEstimator) const {
    return mStrategy->getEstimator(id, outEstimator);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IntegratingVelocityTrackerStrategy::IntegratingVelocityTrackerStrategy(uint32_t degree) :
   mDegree(degree) {
}

IntegratingVelocityTrackerStrategy::~IntegratingVelocityTrackerStrategy() {
}

void IntegratingVelocityTrackerStrategy::clear() {
    mPointerIdBits.clear();
}

void IntegratingVelocityTrackerStrategy::clearPointers(BitSet32 idBits) {
    mPointerIdBits.value &= ~idBits.value;
}

void IntegratingVelocityTrackerStrategy::addMovement(nsecs_t eventTime, BitSet32 idBits,
        const VelocityTracker::Position* positions) {
    uint32_t index = 0;
    for (BitSet32 iterIdBits(idBits); !iterIdBits.isEmpty();) {
        uint32_t id = iterIdBits.clearFirstMarkedBit();
        State& state = mPointerState[id];
        const VelocityTracker::Position& position = positions[index++];
        if (mPointerIdBits.hasBit(id)) {
            updateState(state, eventTime, position.x, position.y);
        } else {
            initState(state, eventTime, position.x, position.y);
        }
    }

    mPointerIdBits = idBits;
}

bool IntegratingVelocityTrackerStrategy::getEstimator(uint32_t id,
        VelocityTracker::Estimator* outEstimator) const {
    outEstimator->clear();

    if (mPointerIdBits.hasBit(id)) {
        const State& state = mPointerState[id];
        populateEstimator(state, outEstimator);
        return true;
    }

    return false;
}

void IntegratingVelocityTrackerStrategy::initState(State& state,
        nsecs_t eventTime, float xpos, float ypos) const {
    state.updateTime = eventTime;
    state.degree = 0;

    state.xpos = xpos;
    state.xvel = 0;
    state.xaccel = 0;
    state.ypos = ypos;
    state.yvel = 0;
    state.yaccel = 0;
}

void IntegratingVelocityTrackerStrategy::updateState(State& state,
        nsecs_t eventTime, float xpos, float ypos) const {
    const nsecs_t MIN_TIME_DELTA = 2 ;
    const float FILTER_TIME_CONSTANT = 0.010f; // 10 milliseconds

    if (eventTime <= state.updateTime + MIN_TIME_DELTA) {
        return;
    }

    float dt = (eventTime - state.updateTime) *0.001f;//0.000000001f;
    state.updateTime = eventTime;

    float xvel = (xpos - state.xpos) / dt;
    float yvel = (ypos - state.ypos) / dt;
    if (state.degree == 0) {
        state.xvel = xvel;
        state.yvel = yvel;
        state.degree = 1;
    } else {
        float alpha = dt / (FILTER_TIME_CONSTANT + dt);
        if (mDegree == 1) {
            state.xvel += (xvel - state.xvel) * alpha;
            state.yvel += (yvel - state.yvel) * alpha;
        } else {
            float xaccel = (xvel - state.xvel) / dt;
            float yaccel = (yvel - state.yvel) / dt;
            if (state.degree == 1) {
                state.xaccel = xaccel;
                state.yaccel = yaccel;
                state.degree = 2;
            } else {
                state.xaccel += (xaccel - state.xaccel) * alpha;
                state.yaccel += (yaccel - state.yaccel) * alpha;
            }
            state.xvel += (state.xaccel * dt) * alpha;
            state.yvel += (state.yaccel * dt) * alpha;
        }
    }
    state.xpos = xpos;
    state.ypos = ypos;
}

void IntegratingVelocityTrackerStrategy::populateEstimator(const State& state,
        VelocityTracker::Estimator* outEstimator) const {
    outEstimator->time = state.updateTime;
    outEstimator->confidence = 1.0f;
    outEstimator->degree = state.degree;
    outEstimator->xCoeff[0] = state.xpos;
    outEstimator->xCoeff[1] = state.xvel;
    outEstimator->xCoeff[2] = state.xaccel / 2;
    outEstimator->yCoeff[0] = state.ypos;
    outEstimator->yCoeff[1] = state.yvel;
    outEstimator->yCoeff[2] = state.yaccel / 2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

ImpulseVelocityTrackerStrategy::ImpulseVelocityTrackerStrategy() {
    clear();
    memset(mMovements,0,sizeof(mMovements));
}

ImpulseVelocityTrackerStrategy::~ImpulseVelocityTrackerStrategy() {
}

void ImpulseVelocityTrackerStrategy::clear() {
    mIndex = 0;
    mMovements[0].idBits.clear();
}

void ImpulseVelocityTrackerStrategy::clearPointers(BitSet32 idBits) {
    BitSet32 remainingIdBits(mMovements[mIndex].idBits.value & ~idBits.value);
    mMovements[mIndex].idBits = remainingIdBits;
}

void ImpulseVelocityTrackerStrategy::addMovement(nsecs_t eventTime, BitSet32 idBits,
        const VelocityTracker::Position* positions) {
    if (++mIndex == HISTORY_SIZE) {
        mIndex = 0;
    }

    Movement& movement = mMovements[mIndex];
    movement.eventTime = eventTime;
    movement.idBits = idBits;
    uint32_t count = idBits.count();
    for (uint32_t i = 0; i < count; i++) {
        movement.positions[i] = positions[i];
    }
}

static float kineticEnergyToVelocity(float work) {
    static constexpr float sqrt2 = 1.41421356237;
    return (work < 0 ? -1.0 : 1.0) * sqrtf(fabsf(work)) * sqrt2;
}

static float calculateImpulseVelocity(const nsecs_t* t, const float* x, size_t count) {
    // The input should be in reversed time order (most recent sample at index i=0)
    // t[i] is in nanoseconds, but due to FP arithmetic, convert to seconds inside this function
    static constexpr float SECONDS_PER_NANO = 1E-3;//android use nanosecond used value 1E-9;

    if (count < 2) {
        return 0; // if 0 or 1 points, velocity is zero
    }
    if (t[1] > t[0]) { // Algorithm will still work, but not perfectly
        LOGE("Samples provided to calculateImpulseVelocity in the wrong order");
    }
    if (count == 2) { // if 2 points, basic linear calculation
        if (t[1] == t[0]) {
            LOGE("Events have identical time stamps t=%lld, setting velocity = 0", t[0]);
            return 0;
        }
        return (x[1] - x[0]) / (SECONDS_PER_NANO * (t[1] - t[0]));
    }
    // Guaranteed to have at least 3 points here
    float work = 0;
    for (size_t i = count - 1; i > 0 ; i--) { // start with the oldest sample and go forward in time
        if (t[i] == t[i-1]) {
            LOGV("Events have identical time stamps t=%lld, skipping sample", t[i]);
            continue;
        }
        float vprev = kineticEnergyToVelocity(work); // v[i-1]
        float vcurr = (x[i] - x[i-1]) / (SECONDS_PER_NANO * (t[i] - t[i-1])); // v[i]
        work += (vcurr - vprev) * fabsf(vcurr);
        if (i == count - 1) {
            work *= 0.5; // initial condition, case 2) above
        }
    }
    return kineticEnergyToVelocity(work);
}

bool ImpulseVelocityTrackerStrategy::getEstimator(uint32_t id,
        VelocityTracker::Estimator* outEstimator) const {
    outEstimator->clear();

    // Iterate over movement samples in reverse time order and collect samples.
    float x[HISTORY_SIZE];
    float y[HISTORY_SIZE];
    nsecs_t time[HISTORY_SIZE];
    size_t m = 0; // number of points that will be used for fitting
    size_t index = mIndex;
    const Movement& newestMovement = mMovements[mIndex];
    do {
        const Movement& movement = mMovements[index];
        if (!movement.idBits.hasBit(id)) {
            break;
        }

        nsecs_t age = newestMovement.eventTime - movement.eventTime;
        if (age > HORIZON) {
            break;
        }

        const VelocityTracker::Position& position = movement.getPosition(id);
        x[m] = position.x;
        y[m] = position.y;
        time[m] = movement.eventTime;
        index = (index == 0 ? HISTORY_SIZE : index) - 1;
    } while (++m < HISTORY_SIZE);

    if (m == 0) {
        return false; // no data
    }
    outEstimator->xCoeff[0] = 0;
    outEstimator->yCoeff[0] = 0;
    outEstimator->xCoeff[1] = calculateImpulseVelocity(time, x, m);
    outEstimator->yCoeff[1] = calculateImpulseVelocity(time, y, m);
    outEstimator->xCoeff[2] = 0;
    outEstimator->yCoeff[2] = 0;
    outEstimator->time = newestMovement.eventTime;
    outEstimator->degree = 2; // similar results to 2nd degree fit
    outEstimator->confidence = 1;
#if DEBUG_STRATEGY
    LOGD("velocity: (%f, %f)", outEstimator->xCoeff[1], outEstimator->yCoeff[1]);
#endif
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// --- LeastSquaresVelocityTrackerStrategy ---

LeastSquaresVelocityTrackerStrategy::LeastSquaresVelocityTrackerStrategy(
        uint32_t degree, Weighting weighting) :
        mDegree(degree), mWeighting(weighting) {
    clear();
    memset(mMovements,0,sizeof(mMovements));
}

LeastSquaresVelocityTrackerStrategy::~LeastSquaresVelocityTrackerStrategy() {
}

void LeastSquaresVelocityTrackerStrategy::clear() {
    mIndex = 0;
    mMovements[0].idBits.clear();
}

void LeastSquaresVelocityTrackerStrategy::clearPointers(BitSet32 idBits) {
    BitSet32 remainingIdBits(mMovements[mIndex].idBits.value & ~idBits.value);
    mMovements[mIndex].idBits = remainingIdBits;
}

void LeastSquaresVelocityTrackerStrategy::addMovement(nsecs_t eventTime, BitSet32 idBits,
        const VelocityTracker::Position* positions) {
    if (++mIndex == HISTORY_SIZE) {
        mIndex = 0;
    }

    Movement& movement = mMovements[mIndex];
    movement.eventTime = eventTime;
    movement.idBits = idBits;
    uint32_t count = idBits.count();
    for (uint32_t i = 0; i < count; i++) {
        movement.positions[i] = positions[i];
    }
}
static bool solveLeastSquares(const float* x, const float* y,
        const float* w, uint32_t m, uint32_t n, float* outB, float* outDet) {
#if DEBUG_STRATEGY
    LOGD("solveLeastSquares: m=%d, n=%d, x=%s, y=%s, w=%s", int(m), int(n),
            vectorToString(x, m).c_str(), vectorToString(y, m).c_str(),
            vectorToString(w, m).c_str());
#endif

    // Expand the X vector to a matrix A, pre-multiplied by the weights.
    float a[n][m]; // column-major order
    for (uint32_t h = 0; h < m; h++) {
        a[0][h] = w[h];
        for (uint32_t i = 1; i < n; i++) {
            a[i][h] = a[i - 1][h] * x[h];
        }
    }
#if DEBUG_STRATEGY
    LOGD("  - a=%s", matrixToString(&a[0][0], m, n, false /*rowMajor*/).c_str());
#endif

    // Apply the Gram-Schmidt process to A to obtain its QR decomposition.
    float q[n][m]; // orthonormal basis, column-major order
    float r[n][n]; // upper triangular matrix, row-major order
    for (uint32_t j = 0; j < n; j++) {
        for (uint32_t h = 0; h < m; h++) {
            q[j][h] = a[j][h];
        }
        for (uint32_t i = 0; i < j; i++) {
            float dot = vectorDot(&q[j][0], &q[i][0], m);
            for (uint32_t h = 0; h < m; h++) {
                q[j][h] -= dot * q[i][h];
            }
        }

        float norm = vectorNorm(&q[j][0], m);
        if (norm < 0.000001f) {
            // vectors are linearly dependent or zero so no solution
#if DEBUG_STRATEGY
            LOGD("  - no solution, norm=%f", norm);
#endif
            return false;
        }

        float invNorm = 1.0f / norm;
        for (uint32_t h = 0; h < m; h++) {
            q[j][h] *= invNorm;
        }
        for (uint32_t i = 0; i < n; i++) {
            r[j][i] = i < j ? 0 : vectorDot(&q[j][0], &a[i][0], m);
        }
    }
#if DEBUG_STRATEGY
    LOGD("  - q=%s", matrixToString(&q[0][0], m, n, false /*rowMajor*/).c_str());
    LOGD("  - r=%s", matrixToString(&r[0][0], n, n, true /*rowMajor*/).c_str());

    // calculate QR, if we factored A correctly then QR should equal A
    float qr[n][m];
    for (uint32_t h = 0; h < m; h++) {
        for (uint32_t i = 0; i < n; i++) {
            qr[i][h] = 0;
            for (uint32_t j = 0; j < n; j++) {
                qr[i][h] += q[j][h] * r[j][i];
            }
        }
    }
    LOGD("  - qr=%s", matrixToString(&qr[0][0], m, n, false /*rowMajor*/).c_str());
#endif

    // Solve R B = Qt W Y to find B.  This is easy because R is upper triangular.
    // We just work from bottom-right to top-left calculating B's coefficients.
    float wy[m];
    for (uint32_t h = 0; h < m; h++) {
        wy[h] = y[h] * w[h];
    }
    for (uint32_t i = n; i != 0; ) {
        i--;
        outB[i] = vectorDot(&q[i][0], wy, m);
        for (uint32_t j = n - 1; j > i; j--) {
            outB[i] -= r[i][j] * outB[j];
        }
        outB[i] /= r[i][i];
    }
#if DEBUG_STRATEGY
    LOGD("  - b=%s", vectorToString(outB, n).c_str());
#endif

    // Calculate the coefficient of determination as 1 - (SSerr / SStot) where
    // SSerr is the residual sum of squares (variance of the error),
    // and SStot is the total sum of squares (variance of the data) where each
    // has been weighted.
    float ymean = 0;
    for (uint32_t h = 0; h < m; h++) {
        ymean += y[h];
    }
    ymean /= m;

    float sserr = 0;
    float sstot = 0;
    for (uint32_t h = 0; h < m; h++) {
        float err = y[h] - outB[0];
        float term = 1;
        for (uint32_t i = 1; i < n; i++) {
            term *= x[h];
            err -= term * outB[i];
        }
        sserr += w[h] * w[h] * err * err;
        float var = y[h] - ymean;
        sstot += w[h] * w[h] * var * var;
    }
    *outDet = sstot > 0.000001f ? 1.0f - (sserr / sstot) : 1;
#if DEBUG_STRATEGY
    LOGD("  - sserr=%f", sserr);
    LOGD("  - sstot=%f", sstot);
    LOGD("  - det=%f", *outDet);
#endif
    return true;
}

/*
 * Optimized unweighted second-order least squares fit. About 2x speed improvement compared to
 * the default implementation
 */
static float solveUnweightedLeastSquaresDeg2(const float* x, const float* y, size_t count) {
    float sxi = 0, sxiyi = 0, syi = 0, sxi2 = 0, sxi3 = 0, sxi2yi = 0, sxi4 = 0;

    for (size_t i = 0; i < count; i++) {
        float xi = x[i];
        float yi = y[i];
        float xi2 = xi*xi;
        float xi3 = xi2*xi;
        float xi4 = xi3*xi;
        float xi2yi = xi2*yi;
        float xiyi = xi*yi;

        sxi += xi;
        sxi2 += xi2;
        sxiyi += xiyi;
        sxi2yi += xi2yi;
        syi += yi;
        sxi3 += xi3;
        sxi4 += xi4;
    }

    float Sxx = sxi2 - sxi*sxi / count;
    float Sxy = sxiyi - sxi*syi / count;
    float Sxx2 = sxi3 - sxi*sxi2 / count;
    float Sx2y = sxi2yi - sxi2*syi / count;
    float Sx2x2 = sxi4 - sxi2*sxi2 / count;

    float numerator = Sxy*Sx2x2 - Sx2y*Sxx2;
    float denominator = Sxx*Sx2x2 - Sxx2*Sxx2;
    if (denominator == 0) {
        LOGW("division by 0 when computing velocity, Sxx=%f, Sx2x2=%f, Sxx2=%f", Sxx, Sx2x2, Sxx2);
        return 0;
    }
    return numerator/denominator;
}

bool LeastSquaresVelocityTrackerStrategy::getEstimator(uint32_t id,
        VelocityTracker::Estimator* outEstimator) const {
    outEstimator->clear();

    // Iterate over movement samples in reverse time order and collect samples.
    float x[HISTORY_SIZE]={.0};
    float y[HISTORY_SIZE]={.0};
    float w[HISTORY_SIZE]={.0};
    float time[HISTORY_SIZE]={.0};
    uint32_t m = 0;
    uint32_t index = mIndex;
    const Movement& newestMovement = mMovements[mIndex];
    do {
        const Movement& movement = mMovements[index];
        if (!movement.idBits.hasBit(id)) {
            break;
        }

        nsecs_t age = newestMovement.eventTime - movement.eventTime;
        if (age > HORIZON) {
            break;
        }

        const VelocityTracker::Position& position = movement.getPosition(id);
        x[m] = position.x;
        y[m] = position.y;
        w[m] = chooseWeight(index);
        time[m] = -age * 0.001f;//android nanosecond use 0.000000001f;
        index = (index == 0 ? HISTORY_SIZE : index) - 1;
    } while (++m < HISTORY_SIZE);

    if (m == 0) {
        return false; // no data
    }

    // Calculate a least squares polynomial fit.
    uint32_t degree = mDegree;
    if (degree > m - 1) {
        degree = m - 1;
    }
    if (degree >= 1) {
        if (degree == 2 && mWeighting == WEIGHTING_NONE) { // optimize unweighted, degree=2 fit
            outEstimator->time = newestMovement.eventTime;
            outEstimator->degree = 2;
            outEstimator->confidence =1;
            outEstimator->xCoeff[0] = 0; // only slope is calculated, set rest of coefficients = 0
            outEstimator->yCoeff[0] = 0;
            outEstimator->xCoeff[1] = solveUnweightedLeastSquaresDeg2(time, x, m);
            outEstimator->yCoeff[1] = solveUnweightedLeastSquaresDeg2(time, y, m);
            outEstimator->xCoeff[2] = 0;
            outEstimator->yCoeff[2] = 0;
            return true;
        }

        float xdet, ydet;
        uint32_t n = degree + 1;
        if (solveLeastSquares(time, x, w, m, n, outEstimator->xCoeff, &xdet)
                && solveLeastSquares(time, y, w, m, n, outEstimator->yCoeff, &ydet)) {
            outEstimator->time = newestMovement.eventTime;
            outEstimator->degree = degree;
            outEstimator->confidence = xdet * ydet;
#if DEBUG_STRATEGY
            LOGD("estimate: degree=%d, xCoeff=%s, yCoeff=%s, confidence=%f",
                    int(outEstimator->degree),
                    vectorToString(outEstimator->xCoeff, n).c_str(),
                    vectorToString(outEstimator->yCoeff, n).c_str(),
                    outEstimator->confidence);
#endif
            return true;
        }
    }

    // No velocity data available for this pointer, but we do have its current position.
    outEstimator->xCoeff[0] = x[0];
    outEstimator->yCoeff[0] = y[0];
    outEstimator->time = newestMovement.eventTime;
    outEstimator->degree = 0;
    outEstimator->confidence = 1;
    return true;
}

float LeastSquaresVelocityTrackerStrategy::chooseWeight(uint32_t index) const {
    switch (mWeighting) {
    case WEIGHTING_DELTA: {
        // Weight points based on how much time elapsed between them and the next
        // point so that points that "cover" a shorter time span are weighed less.
        //   delta  0ms: 0.5
        //   delta 10ms: 1.0
        if (index == mIndex) return 1.0f;

        uint32_t nextIndex = (index + 1) % HISTORY_SIZE;
        float deltaMillis = (mMovements[nextIndex].eventTime- mMovements[index].eventTime)
                * 0.000001f;
        if (deltaMillis < 0 ) return 0.5f;
        if (deltaMillis < 10) return 0.5f + deltaMillis * 0.05;
        return 1.0f;
    }

    case WEIGHTING_CENTRAL: {
        // Weight points based on their age, weighing very recent and very old points less.
        //   age  0ms: 0.5
        //   age 10ms: 1.0
        //   age 50ms: 1.0
        //   age 60ms: 0.5
        float ageMillis = (mMovements[mIndex].eventTime - mMovements[index].eventTime) * 0.000001f;
        if (ageMillis < 0 ) return 0.5f;
        if (ageMillis < 10) return 0.5f + ageMillis * 0.05;
        if (ageMillis < 50) return 1.0f;
        if (ageMillis < 60) return 0.5f + (60 - ageMillis) * 0.05;
        return 0.5f;
    }

    case WEIGHTING_RECENT: {
        // Weight points based on their age, weighing older points less.
        // age   0ms: 1.0  , age  50ms: 1.0 ,  age 100ms: 0.5
        float ageMillis = (mMovements[mIndex].eventTime - mMovements[index].eventTime)
                * 0.000001f;
        if (ageMillis < 50) {
            return 1.0f;
        }
        if (ageMillis < 100) {
            return 0.5f + (100 - ageMillis) * 0.01f;
        }
        return 0.5f;
    }

    case WEIGHTING_NONE:
    default:
        return 1.0f;
    }
}

///////////////////////////////////////from VelocityTracker.java/////////////////////////////////////

std::queue<VelocityTracker*>VelocityTracker::sPool;
VelocityTracker::VelocityTracker(const char* strategy):mActivePointerId(-1) {
    mVelocityTracker=new VelocityTrackerImpl(strategy);
    mCalculatedIdBits.clear();
    bzero(mCalculatedVelocity,sizeof(mCalculatedVelocity));
}

void VelocityTracker::clear() {
    mVelocityTracker->clear();
    mActivePointerId = -1;
    mCalculatedIdBits.clear();
}

void VelocityTracker::addMovement(const MotionEvent& event) {
    mVelocityTracker->addMovement(event);
    LOGV("%f,%f",event.getX(),event.getY());
}

void VelocityTracker::computeCurrentVelocity(int32_t units){
    computeCurrentVelocity(units,INT_MAX);
}

void VelocityTracker::computeCurrentVelocity(int32_t units, float maxVelocity) {
    BitSet32 idBits(mVelocityTracker->getCurrentPointerIdBits());
    mCalculatedIdBits = idBits;
    for (uint32_t index = 0; !idBits.isEmpty(); index++) {
        uint32_t id = idBits.clearFirstMarkedBit();

        float vx, vy;
        mVelocityTracker->getVelocity(id, &vx, &vy);

        vx = vx * units / 1000;
        vy = vy * units / 1000;

        if (vx > maxVelocity)       vx = maxVelocity;
        else if (vx < -maxVelocity) vx = -maxVelocity;

        if (vy > maxVelocity)       vy = maxVelocity;
        else if (vy < -maxVelocity) vy = -maxVelocity;

        Velocity& velocity = mCalculatedVelocity[index];
        velocity.vx = vx;
        velocity.vy = vy;
    }
}

void VelocityTracker::getVelocity(int32_t id, float* outVx, float* outVy) {
    if (id == ACTIVE_POINTER_ID) {
        id = mVelocityTracker->getActivePointerId();
    }

    float vx, vy;
    if (id >= 0 && id <= MAX_POINTER_ID && mCalculatedIdBits.hasBit(id)) {
        uint32_t index = mCalculatedIdBits.getIndexOfBit(id);
        const Velocity& velocity = mCalculatedVelocity[index];
        vx = velocity.vx;
        vy = velocity.vy;
    } else {
        vx = 0;
        vy = 0;
    }
    if (outVx) {
        *outVx = vx;
    }
    if (outVy) {
        *outVy = vy;
    }
}

float VelocityTracker::getXVelocity(){
    return getXVelocity(ACTIVE_POINTER_ID);
}

float VelocityTracker::getXVelocity(int id){
    float vx;
    getVelocity(id, &vx,nullptr);
    return vx;
}

float VelocityTracker::getYVelocity(){
    return getYVelocity(ACTIVE_POINTER_ID);
}

float VelocityTracker::getYVelocity(int id){
    float vy;
    getVelocity(id, nullptr, &vy);
    return vy;
}

bool VelocityTracker::getEstimator(int32_t id,Estimator* outEstimator) {
    return mVelocityTracker->getEstimator(id, outEstimator);
}

void VelocityTracker::recycle(){
    sPool.push(this);
}

VelocityTracker*VelocityTracker::obtain(const char*strategy){
    if(sPool.size()){
        VelocityTracker*vt=sPool.front();
        sPool.pop();
        return vt; 
    }
    return new VelocityTracker(strategy);
}

}//endof namespace
