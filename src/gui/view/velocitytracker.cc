/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <view/velocitytracker.h>
#include <utils/mathutils.h>
#include <math.h>
#include <string.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <limits.h>
#include <cfloat>
#include <set>
#include <chrono>

namespace cdroid{
constexpr bool DEBUG_VELOCITY = false;
constexpr bool DEBUG_STRATEGY = false;
constexpr bool DEBUG_IMPULSE  = true;
static const nsecs_t NANOS_PER_MS = 1000000LL;
static const nsecs_t ASSUME_POINTER_STOPPED_TIME = 40*NANOS_PER_MS;
// Threshold for determining that a pointer has stopped moving.
// Some input devices do not send ACTION_MOVE events in the case where a pointer has
// stopped.  We need to detect this case so that we can accurately predict the
// velocity after the pointer starts moving again.
// Axes specifying location on a 2D plane (i.e. X and Y).

static const std::unordered_map<int32_t/*axis*/, int/*Strategy*/> DEFAULT_STRATEGY_BY_AXIS = {
    {MotionEvent::AXIS_X, (int)VelocityTracker::VELOCITY_TRACKER_STRATEGY_LSQ2},
    {MotionEvent::AXIS_Y, (int)VelocityTracker::VELOCITY_TRACKER_STRATEGY_LSQ2},
    {MotionEvent::AXIS_SCROLL, (int)VelocityTracker::VELOCITY_TRACKER_STRATEGY_IMPULSE}
};

static const std::set<int32_t> PLANAR_AXES = {MotionEvent::AXIS_X, MotionEvent::AXIS_Y};
static const std::set<int32_t> DIFFERENTIAL_AXES = {MotionEvent::AXIS_SCROLL};

static std::string toString(std::chrono::nanoseconds t) {
    std::stringstream stream;
    stream.precision(1);
    stream << std::fixed << std::chrono::duration<float, std::milli>(t).count() << " ms";
    return stream.str();
}

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
////////////////////////////////////////VelocityTrackerImpl/////////////////////////////////////
//reference frameworks/native/libs/input/VelocityTracker.cpp
class VelocityTrackerImpl{
public:
private:
    nsecs_t mLastEventTime;
    BitSet32 mCurrentPointerIdBits;
    const int mOverrideStrategy;
    int32_t mActivePointerId;
    static Pools::SimplePool<VelocityTracker>sPool;
    std::unordered_map<int32_t /*axis*/, std::unique_ptr<VelocityTrackerStrategy>> mConfiguredStrategies;
private:
    void configureStrategy(int32_t axis);
    static std::unique_ptr<VelocityTrackerStrategy> createStrategy(int strategy,bool deltaValues);
public:
    VelocityTrackerImpl(int strategy = VelocityTracker::VELOCITY_TRACKER_STRATEGY_DEFAULT);
    ~VelocityTrackerImpl();
    static bool isAxisSupported(int32_t axis);
    void clear();
    void clearPointer(int32_t);
     // Adds movement information for a pointer for a specific axis
    void addMovement(nsecs_t eventTime, int32_t pointerId, int32_t axis, float position);
    void addMovement(const MotionEvent& event);
    bool getVelocity(int32_t axis,int32_t pointerId,float*out);
    VelocityTracker::ComputedVelocity getComputedVelocity(int32_t units, float maxVelocity);
    bool getEstimator(int32_t axis,int32_t pointerId, VelocityTracker::Estimator* outEstimator);
    static VelocityTracker*obtain();
    static VelocityTracker*obtain(const char*name);
    inline int32_t getActivePointerId() const { return mActivePointerId; }
    void recycle();
};

VelocityTrackerImpl::VelocityTrackerImpl(int strategy)
     :mLastEventTime(0),mCurrentPointerIdBits(0),mOverrideStrategy(strategy){
    mActivePointerId = -1;
}

VelocityTrackerImpl::~VelocityTrackerImpl(){
}

bool VelocityTrackerImpl::isAxisSupported(int32_t axis) {
    return DEFAULT_STRATEGY_BY_AXIS.find(axis) != DEFAULT_STRATEGY_BY_AXIS.end();
}

void VelocityTrackerImpl::configureStrategy(int32_t axis) {
    const bool isDifferentialAxis = DIFFERENTIAL_AXES.find(axis) != DIFFERENTIAL_AXES.end();

    std::unique_ptr<VelocityTrackerStrategy> createdStrategy;
    if (mOverrideStrategy != VelocityTracker::VELOCITY_TRACKER_STRATEGY_DEFAULT) {
        createdStrategy = createStrategy(mOverrideStrategy, /*deltaValues=*/isDifferentialAxis);
    } else {
        createdStrategy = createStrategy(DEFAULT_STRATEGY_BY_AXIS.at(axis),/*deltaValues=*/isDifferentialAxis);
    }

    FATAL_IF(createdStrategy == nullptr,"Could not create velocity tracker strategy for axis %lld!", axis);
    mConfiguredStrategies[axis] = std::move(createdStrategy);
}


std::unique_ptr<VelocityTrackerStrategy> VelocityTrackerImpl::createStrategy(int strategy, bool deltaValues) {
    switch (strategy) {
    case VelocityTracker::VELOCITY_TRACKER_STRATEGY_IMPULSE:
        LOGI_IF(DEBUG_STRATEGY, "Initializing impulse strategy");
        return std::make_unique<ImpulseVelocityTrackerStrategy>(deltaValues);

    case VelocityTracker::VELOCITY_TRACKER_STRATEGY_LSQ1:
        return std::make_unique<LeastSquaresVelocityTrackerStrategy>(1);

    case VelocityTracker::VELOCITY_TRACKER_STRATEGY_LSQ2:
        LOGI_IF(DEBUG_STRATEGY && !DEBUG_IMPULSE, "Initializing lsq2 strategy");
        return std::make_unique<LeastSquaresVelocityTrackerStrategy>(2);

    case VelocityTracker::VELOCITY_TRACKER_STRATEGY_LSQ3:
        return std::make_unique<LeastSquaresVelocityTrackerStrategy>(3);

    case VelocityTracker::VELOCITY_TRACKER_STRATEGY_WLSQ2_DELTA:
        return std::make_unique<LeastSquaresVelocityTrackerStrategy>(2,
                   LeastSquaresVelocityTrackerStrategy::Weighting::DELTA);
    case VelocityTracker::VELOCITY_TRACKER_STRATEGY_WLSQ2_CENTRAL:
        return std::make_unique<LeastSquaresVelocityTrackerStrategy>(2,
                   LeastSquaresVelocityTrackerStrategy::Weighting::CENTRAL);
    case VelocityTracker::VELOCITY_TRACKER_STRATEGY_WLSQ2_RECENT:
        return std::make_unique<LeastSquaresVelocityTrackerStrategy>(2,
                   LeastSquaresVelocityTrackerStrategy::Weighting::RECENT);

    case VelocityTracker::VELOCITY_TRACKER_STRATEGY_INT1:
        return std::make_unique<IntegratingVelocityTrackerStrategy>(1);

    case VelocityTracker::VELOCITY_TRACKER_STRATEGY_INT2:
        return std::make_unique<IntegratingVelocityTrackerStrategy>(2);

    case VelocityTracker::VELOCITY_TRACKER_STRATEGY_LEGACY:
        return std::make_unique<LegacyVelocityTrackerStrategy>();

    default:
        break;
    }
    return nullptr;
}

void VelocityTrackerImpl::clear() {
    mCurrentPointerIdBits.clear();
    mActivePointerId = -1;
    mConfiguredStrategies.clear();
}

void VelocityTrackerImpl::clearPointer(int32_t pointerId) {
    mCurrentPointerIdBits.clearBit(pointerId);
    if ((mActivePointerId!=-1)&&(mActivePointerId == pointerId)) {
        // The active pointer id is being removed. Mark it invalid and try to find a new one
        // from the remaining pointers.
        mActivePointerId = -1;
        if (!mCurrentPointerIdBits.isEmpty()) {
            mActivePointerId = mCurrentPointerIdBits.firstMarkedBit();
        }
    }
    for (const auto& it: mConfiguredStrategies) {
        it.second->clearPointer(pointerId);
    }
}

void VelocityTrackerImpl::addMovement(nsecs_t eventTime, int32_t pointerId, int32_t axis,
                        float position) {
    if (mCurrentPointerIdBits.hasBit(pointerId) && ((eventTime - mLastEventTime) > ASSUME_POINTER_STOPPED_TIME) ) {
        LOGD_IF(DEBUG_VELOCITY, "VelocityTracker: stopped for %d, clearing state.",int(eventTime - mLastEventTime));

        // We have not received any movements for too long.  Assume that all pointers
        // have stopped.
        mConfiguredStrategies.clear();
    }
    mLastEventTime = eventTime;

    mCurrentPointerIdBits.markBit(pointerId);
    if (mActivePointerId==-1) {
        // Let this be the new active pointer if no active pointer is currently set
        mActivePointerId = pointerId;
    }

    if (mConfiguredStrategies.find(axis) == mConfiguredStrategies.end()) {
        configureStrategy(axis);
    }
    mConfiguredStrategies[axis]->addMovement(eventTime, pointerId, position);

    if (DEBUG_VELOCITY) {
        LOGD("VelocityTracker: addMovement eventTime=%lld, pointerId=%d"
              ", activePointerId=%d", eventTime, pointerId, mActivePointerId);

        VelocityTracker::Estimator estimator;
        getEstimator(axis, pointerId,&estimator);
        LOGD("  %d: axis=%d, position=%0.3f, estimator (degree=%d, coeff=%s, confidence=%f)",
              pointerId, axis, position, int(estimator.degree),
              vectorToString(estimator.coeff, estimator.degree + 1).c_str(),
              estimator.confidence);
    }
}

void VelocityTrackerImpl::addMovement(const MotionEvent& event) {
    // Stores data about which axes to process based on the incoming motion event.
    std::set<int32_t> axesToProcess;
    const int32_t actionMasked = event.getActionMasked();

    switch (actionMasked) {
    case MotionEvent::ACTION_DOWN:
    case MotionEvent::ACTION_HOVER_ENTER:
        // Clear all pointers on down before adding the new movement.
        clear();
        axesToProcess.insert(PLANAR_AXES.begin(), PLANAR_AXES.end());
        break;
    case MotionEvent::ACTION_POINTER_DOWN: {
        // Start a new movement trace for a pointer that just went down.
        // We do this on down instead of on up because the client may want to query the
        // final velocity for a pointer that just went up.
        clearPointer(event.getPointerId(event.getActionIndex()));
        axesToProcess.insert(PLANAR_AXES.begin(), PLANAR_AXES.end());
        break;
        }
    case MotionEvent::ACTION_MOVE:
    case MotionEvent::ACTION_HOVER_MOVE:
        axesToProcess.insert(PLANAR_AXES.begin(), PLANAR_AXES.end());
        break;
    case MotionEvent::ACTION_POINTER_UP:
    case MotionEvent::ACTION_UP: {
        nsecs_t delaySinceLastEvent = (event.getEventTime() - mLastEventTime);
        if (delaySinceLastEvent > ASSUME_POINTER_STOPPED_TIME) {
            LOGD_IF(DEBUG_VELOCITY,
                     "VelocityTracker: stopped for %lld, clearing state upon pointer liftoff.",
                      delaySinceLastEvent);
            // We have not received any movements for too long.  Assume that all pointers
            // have stopped.
            for (int32_t axis : PLANAR_AXES) {
                mConfiguredStrategies.erase(axis);
            }
         }
         // These actions because they do not convey any new information about
         // pointer movement.  We also want to preserve the last known velocity of the pointers.
         // Note that ACTION_UP and ACTION_POINTER_UP always report the last known position
         // of the pointers that went up.  ACTION_POINTER_UP does include the new position of
         // pointers that remained down but we will also receive an ACTION_MOVE with this
         // information if any of them actually moved.  Since we don't know how many pointers
         // will be going up at once it makes sense to just wait for the following ACTION_MOVE
         // before adding the movement.
         return;
        }
    case MotionEvent::ACTION_SCROLL:
        axesToProcess.insert(MotionEvent::AXIS_SCROLL);
        break;
    default:
        // Ignore all other actions.
        return;
    }

    const size_t historySize = event.getHistorySize();
    for (size_t h = 0; h <= historySize; h++) {
        const nsecs_t eventTime = event.getHistoricalEventTime(h)*NANOS_PER_MS;
        for (size_t i = 0; i < event.getPointerCount(); i++) {
            if (event.isResampled(i, h)) {
                continue; // skip resampled samples
            }
            const int32_t pointerId = event.getPointerId(i);
            for (int32_t axis : axesToProcess) {
                const float position = event.getHistoricalAxisValue(axis, i, h);
                addMovement(eventTime, pointerId, axis, position);
            }
        }
    }
}

bool VelocityTrackerImpl::getVelocity(int32_t axis,int32_t pointerId,float*out){
    VelocityTracker::Estimator estimator;
    const bool rc =getEstimator(axis, pointerId,&estimator);
    if (rc && estimator.degree >= 1) {
        *out = estimator.coeff[1];
	return true;
    }
    return false;
}

VelocityTracker::ComputedVelocity VelocityTrackerImpl::getComputedVelocity(int32_t units, float maxVelocity){
    VelocityTracker::ComputedVelocity computedVelocity;
    for (const auto& axis : mConfiguredStrategies) {
        BitSet32 copyIdBits = BitSet32(mCurrentPointerIdBits);
        while (!copyIdBits.isEmpty()) {
            uint32_t id = copyIdBits.clearFirstMarkedBit();
            float velocity;
            if (getVelocity(axis.first, id,&velocity)) {
                const float adjustedVelocity = MathUtils::clamp(velocity * units / 1000.f, -maxVelocity, maxVelocity);
                computedVelocity.addVelocity(axis.first, id, adjustedVelocity);
            }
        }
    }
    return computedVelocity;
}

bool VelocityTrackerImpl::getEstimator(int32_t axis,int32_t pointerId,VelocityTracker::Estimator* outEstimator) {
    const auto& it = mConfiguredStrategies.find(axis);
    if (it == mConfiguredStrategies.end()) {
        return false;
    }
    return it->second->getEstimator(pointerId,outEstimator);
}

//////////////////////////////////////////////////////////////////////////////////////////

VelocityTracker::VelocityTrackerState::VelocityTrackerState(int strategy){
    mVelocityTracker = new VelocityTrackerImpl(strategy);
}

VelocityTracker::VelocityTrackerState::~VelocityTrackerState(){
    delete mVelocityTracker;
}

void VelocityTracker::VelocityTrackerState::clear() {
    mVelocityTracker->clear();
}

void VelocityTracker::VelocityTrackerState::addMovement(const MotionEvent& event) {
    mVelocityTracker->addMovement(event);
}

void VelocityTracker::VelocityTrackerState::computeCurrentVelocity(int32_t units, float maxVelocity) {
    mComputedVelocity = mVelocityTracker->getComputedVelocity(units, maxVelocity);
}

float VelocityTracker::VelocityTrackerState::getVelocity(int32_t axis, int32_t id) {
    if (id == ACTIVE_POINTER_ID) {
        id = mVelocityTracker->getActivePointerId();
    }
    float value =0.f;
    mComputedVelocity.getVelocity(axis, id,value);
    return value;
}

//////////////////////////////////////////////////////////////////////////////////////////
Pools::SimplePool<VelocityTracker>VelocityTracker::sPool(4);

std::unordered_map<std::string,int>VelocityTracker::STRATEGIES = {
     // Strategy string and IDs mapping lookup.
     {"impulse", (int)VELOCITY_TRACKER_STRATEGY_IMPULSE},
     {"lsq1", (int)VELOCITY_TRACKER_STRATEGY_LSQ1},
     {"lsq2", (int)VELOCITY_TRACKER_STRATEGY_LSQ2},
     {"lsq3", (int)VELOCITY_TRACKER_STRATEGY_LSQ3},
     {"wlsq2-delta", (int)VELOCITY_TRACKER_STRATEGY_WLSQ2_DELTA},
     {"wlsq2-central", (int)VELOCITY_TRACKER_STRATEGY_WLSQ2_CENTRAL},
     {"wlsq2-recent", (int)VELOCITY_TRACKER_STRATEGY_WLSQ2_RECENT},
     {"int1", (int)VELOCITY_TRACKER_STRATEGY_INT1},
     {"int2", (int)VELOCITY_TRACKER_STRATEGY_INT2},
     {"legacy", (int)VELOCITY_TRACKER_STRATEGY_LEGACY},
};

int VelocityTracker::toStrategyId(const char*strStrategy){
    auto it = STRATEGIES.find(strStrategy);
    if(it!=STRATEGIES.end())
	return it->second;
    return VELOCITY_TRACKER_STRATEGY_DEFAULT;
}

VelocityTracker::VelocityTracker(int strategy){
    mTrackerState = new VelocityTrackerState(strategy);
    mStrategy = strategy;
}

VelocityTracker::~VelocityTracker(){
    delete mTrackerState;
}

VelocityTracker*VelocityTracker::obtain(){
    VelocityTracker*instance = sPool.acquire();
    return instance?instance:new VelocityTracker(VELOCITY_TRACKER_STRATEGY_DEFAULT);
}

VelocityTracker*VelocityTracker::obtain(const char*strategy){
    if(strategy==nullptr)return obtain();
    return new VelocityTracker(toStrategyId(strategy));
}

void VelocityTracker::recycle(){
    if(mStrategy ==VELOCITY_TRACKER_STRATEGY_DEFAULT){
        clear();
        sPool.release(this);
    }
}

void VelocityTracker::clear(){
    mTrackerState->clear();
}

bool VelocityTracker::isAxisSupported(int axis){
    return VelocityTrackerImpl::isAxisSupported(axis);
}

void VelocityTracker::addMovement(const MotionEvent& event){
    mTrackerState->addMovement(event);
}

void VelocityTracker::computeCurrentVelocity(int units){
    mTrackerState->computeCurrentVelocity(units,FLT_MAX);
}

void VelocityTracker::computeCurrentVelocity(int units,float maxVelocity){
    mTrackerState->computeCurrentVelocity(units,maxVelocity);
}

float VelocityTracker::getXVelocity(){
    return mTrackerState->getVelocity(MotionEvent::AXIS_X,ACTIVE_POINTER_ID);
}

float VelocityTracker::getYVelocity(){
    return mTrackerState->getVelocity(MotionEvent::AXIS_Y,ACTIVE_POINTER_ID);
}

float VelocityTracker::getXVelocity(int pointerId){
    return mTrackerState->getVelocity(MotionEvent::AXIS_X,pointerId);
}

float VelocityTracker::getYVelocity(int pointerId){
    return mTrackerState->getVelocity(MotionEvent::AXIS_Y,pointerId);
}

float VelocityTracker::getAxisVelocity(int axis){
    return mTrackerState->getVelocity(axis,ACTIVE_POINTER_ID);
}

float VelocityTracker::getAxisVelocity(int axis,int pointerId){
    return mTrackerState->getVelocity(axis,pointerId);
}
//////////////////////////////////////////////////////////////////////////////////////////

VelocityTracker::Estimator::Estimator(){
    clear();
}

void VelocityTracker::Estimator::clear(){
    time = 0;
    degree = 0;
    confidence = 0;
    for (size_t i = 0; i <= MAX_DEGREE; i++) {
        coeff[i] = 0;
    }
}

bool VelocityTracker::ComputedVelocity::getVelocity(int32_t axis, int32_t id,float&outVelocity) const {
    const auto& axisVelocities = mVelocities.find(axis);
    if (axisVelocities == mVelocities.end()) {
        return false;
    }
    const auto& axisIdVelocity = axisVelocities->second.find(id);
    if (axisIdVelocity == axisVelocities->second.end()) {
        return false;
    }
    outVelocity = axisIdVelocity->second;
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
IntegratingVelocityTrackerStrategy::IntegratingVelocityTrackerStrategy(uint32_t degree) :
   mDegree(degree) {
}

IntegratingVelocityTrackerStrategy::~IntegratingVelocityTrackerStrategy() {
}

void IntegratingVelocityTrackerStrategy::clearPointer(int32_t pointerId) {
    mPointerIdBits.clearBit(pointerId);
}

void IntegratingVelocityTrackerStrategy::addMovement(nsecs_t eventTime, int32_t pointerId,float position) {
    State& state = mPointerState[pointerId];
    if (mPointerIdBits.hasBit(pointerId)) {
        updateState(state, eventTime, position);
    } else {
        initState(state, eventTime, position);
    }
    mPointerIdBits.markBit(pointerId);
}

bool IntegratingVelocityTrackerStrategy::getEstimator(int32_t pointerId,
        VelocityTracker::Estimator* outEstimator) const {
    outEstimator->clear();

    if (mPointerIdBits.hasBit(pointerId)) {
        const State& state = mPointerState[pointerId];
        populateEstimator(state, outEstimator);
        return true;
    }

    return false;
}

void IntegratingVelocityTrackerStrategy::initState(State& state,nsecs_t eventTime, float pos) const {
    state.updateTime = eventTime;
    state.degree = 0;

    state.pos = pos;
    state.vel = 0;
    state.accel = 0;
}

void IntegratingVelocityTrackerStrategy::updateState(State& state,nsecs_t eventTime, float pos) const {
    const nsecs_t MIN_TIME_DELTA = 2*NANOS_PER_MS;
    const float FILTER_TIME_CONSTANT = 0.010f; // 10 milliseconds

    if (eventTime <= state.updateTime + MIN_TIME_DELTA) {
        return;
    }

    float dt = (eventTime - state.updateTime) *0.000000001f;
    state.updateTime = eventTime;

    float vel = (pos - state.pos) / dt;
    if (state.degree == 0) {
        state.vel = vel;
        state.degree = 1;
    } else {
        float alpha = dt / (FILTER_TIME_CONSTANT + dt);
        if (mDegree == 1) {
            state.vel += (vel - state.vel) * alpha;
        } else {
            float accel = (vel - state.vel) / dt;
            if (state.degree == 1) {
                state.accel = accel;
                state.degree = 2;
            } else {
                state.accel += (accel - state.accel) * alpha;
            }
            state.vel += (state.accel * dt) * alpha;
        }
    }
    state.pos = pos;
}

void IntegratingVelocityTrackerStrategy::populateEstimator(const State& state,
        VelocityTracker::Estimator* outEstimator) const {
    outEstimator->time = state.updateTime;
    outEstimator->confidence = 1.0f;
    outEstimator->degree = state.degree;
    outEstimator->coeff[0] = state.pos;
    outEstimator->coeff[1] = state.vel;
    outEstimator->coeff[2] = state.accel / 2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// --- LegacyVelocityTrackerStrategy ---

LegacyVelocityTrackerStrategy::LegacyVelocityTrackerStrategy() {}

LegacyVelocityTrackerStrategy::~LegacyVelocityTrackerStrategy() {
}

void LegacyVelocityTrackerStrategy::clearPointer(int32_t pointerId) {
    mIndex.erase(pointerId);
    mMovements.erase(pointerId);
}

void LegacyVelocityTrackerStrategy::addMovement(nsecs_t eventTime, int32_t pointerId,
                                                float position) {
    // If data for this pointer already exists, we have a valid entry at the position of
    // mIndex[pointerId] and mMovements[pointerId]. In that case, we need to advance the index
    // to the next position in the circular buffer and write the new Movement there. Otherwise,
    // if this is a first movement for this pointer, we initialize the maps mIndex and mMovements
    // for this pointer and write to the first position.
    auto movementIt= mMovements.insert({pointerId, {}});
    auto indexIt = mIndex.insert({pointerId, 0});
    size_t& index = indexIt.first->second;
    const bool inserted = movementIt.second;
    if (!inserted && movementIt.first->second[index].eventTime != eventTime) {
        // When ACTION_POINTER_DOWN happens, we will first receive ACTION_MOVE with the coordinates
        // of the existing pointers, and then ACTION_POINTER_DOWN with the coordinates that include
        // the new pointer. If the eventtimes for both events are identical, just update the data
        // for this time.
        // We only compare against the last value, as it is likely that addMovement is called
        // in chronological order as events occur.
        index++;
    }
    if (index == HISTORY_SIZE) {
        index = 0;
    }

    Movement& movement = movementIt.first->second[index];
    movement.eventTime = eventTime;
    movement.position = position;
}

bool LegacyVelocityTrackerStrategy::getEstimator(int32_t pointerId,VelocityTracker::Estimator*outEstimator) const {
    const auto movementIt = mMovements.find(pointerId);
    if (movementIt == mMovements.end()) {
        return false; // no data
    }
    const Movement& newestMovement = movementIt->second[mIndex.at(pointerId)];

    // Find the oldest sample that contains the pointer and that is not older than HORIZON.
    nsecs_t minTime = newestMovement.eventTime - HORIZON;
    uint32_t oldestIndex = mIndex.at(pointerId);
    uint32_t numTouches = 1;
    do {
        uint32_t nextOldestIndex = (oldestIndex == 0 ? HISTORY_SIZE : oldestIndex) - 1;
        const Movement& nextOldestMovement = mMovements.at(pointerId)[nextOldestIndex];
        if (nextOldestMovement.eventTime < minTime) {
            break;
        }
        oldestIndex = nextOldestIndex;
    } while (++numTouches < HISTORY_SIZE);

    // Calculate an exponentially weighted moving average of the velocity estimate
    // at different points in time measured relative to the oldest sample.
    // This is essentially an IIR filter.  Newer samples are weighted more heavily
    // than older samples.  Samples at equal time points are weighted more or less
    // equally.
    //
    // One tricky problem is that the sample data may be poorly conditioned.
    // Sometimes samples arrive very close together in time which can cause us to
    // overestimate the velocity at that time point.  Most samples might be measured
    // 16ms apart but some consecutive samples could be only 0.5sm apart because
    // the hardware or driver reports them irregularly or in bursts.
    float accumV = 0;
    uint32_t index = oldestIndex;
    uint32_t samplesUsed = 0;
    const Movement& oldestMovement = mMovements.at(pointerId)[oldestIndex];
    float oldestPosition = oldestMovement.position;
    nsecs_t lastDuration = 0;

    while (numTouches-- > 1) {
        if (++index == HISTORY_SIZE) {
            index = 0;
        }
        const Movement& movement = mMovements.at(pointerId)[index];
        nsecs_t duration = movement.eventTime - oldestMovement.eventTime;

        // If the duration between samples is small, we may significantly overestimate
        // the velocity.  Consequently, we impose a minimum duration constraint on the
        // samples that we include in the calculation.
        if (duration >= MIN_DURATION) {
            float position = movement.position;
            float scale = 1000000000.0f / duration; // one over time delta in seconds
            float v = (position - oldestPosition) * scale;
            accumV = (accumV * lastDuration + v * duration) / (duration + lastDuration);
            lastDuration = duration;
            samplesUsed += 1;
        }
    }

    // Report velocity.
    float newestPosition = newestMovement.position;
    outEstimator->time = newestMovement.eventTime;
    outEstimator->confidence = 1;
    outEstimator->coeff[0] = newestPosition;
    if (samplesUsed) {
        outEstimator->coeff[1] = accumV;
        outEstimator->degree = 1;
    } else {
        outEstimator->degree = 0;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

ImpulseVelocityTrackerStrategy::ImpulseVelocityTrackerStrategy(bool deltaValues)
      :mDeltaValues(deltaValues) {
}

ImpulseVelocityTrackerStrategy::~ImpulseVelocityTrackerStrategy() {
    LOGV("ImpulseVelocityTrackerStrategy");
}


void ImpulseVelocityTrackerStrategy::clearPointer(int32_t pointerId) {
    mIndex.erase(pointerId);
    mMovements.erase(pointerId);
}

void ImpulseVelocityTrackerStrategy::addMovement(nsecs_t eventTime, int32_t pointerId,float position) {
    // If data for this pointer already exists, we have a valid entry at the position of
    // mIndex[pointerId] and mMovements[pointerId]. In that case, we need to advance the index
    // to the next position in the circular buffer and write the new Movement there. Otherwise,
    // if this is a first movement for this pointer, we initialize the maps mIndex and mMovements
    // for this pointer and write to the first position.
    auto movementIt = mMovements.insert({pointerId, {}});
    auto indexIt = mIndex.insert({pointerId, 0});
    size_t& index = indexIt.first->second;
    const bool inserted = movementIt.second;
    if (!inserted && movementIt.first->second[index].eventTime != eventTime) {
        // When ACTION_POINTER_DOWN happens, we will first receive ACTION_MOVE with the coordinates
        // of the existing pointers, and then ACTION_POINTER_DOWN with the coordinates that include
        // the new pointer. If the eventtimes for both events are identical, just update the data
        // for this time.
        // We only compare against the last value, as it is likely that addMovement is called
        // in chronological order as events occur.
        index++;
    }
    if (index == HISTORY_SIZE) {
        index = 0;
    }

    Movement& movement = movementIt.first->second[index];
    movement.eventTime = eventTime;
    movement.position = position;
}

static float kineticEnergyToVelocity(float work) {
    static constexpr float sqrt2 = 1.41421356237f;
    return (work < 0 ? -1.0 : 1.0) * sqrtf(fabsf(work)) * sqrt2;
}

static float calculateImpulseVelocity(const nsecs_t* t, const float* x, size_t count,bool deltaValues) {
    // The input should be in reversed time order (most recent sample at index i=0)
    // t[i] is in nanoseconds, but due to FP arithmetic, convert to seconds inside this function
    static constexpr float SECONDS_PER_NANO = 1E-9;

    if (count < 2) {
        return 0; // if 0 or 1 points, velocity is zero
    }
    if (t[1] > t[0]) { // Algorithm will still work, but not perfectly
        LOGE("Samples(%lld,%lld) provided to calculateImpulseVelocity in the wrong order",t[1],t[0]);
    }
    if (count == 2) { // if 2 points, basic linear calculation
        if (t[1] == t[0]) {
            LOGE("Events have identical time stamps t=%lld, setting velocity = 0", t[0]);
            return 0;
        }
	const float deltaX = deltaValues ? -x[0] : x[1] - x[0];
        return deltaX / (SECONDS_PER_NANO * (t[1] - t[0]));
    }
    // Guaranteed to have at least 3 points here
    float work = 0;
    for (size_t i = count - 1; i > 0 ; i--) { // start with the oldest sample and go forward in time
        if (t[i] == t[i-1]) {
            LOGV("Events have identical time stamps t=%lld, skipping sample", t[i]);
            continue;
        }
        float vprev = kineticEnergyToVelocity(work); // v[i-1]
        const float deltaX = deltaValues ? -x[i-1] : x[i] - x[i-1];
        float vcurr = deltaX / (SECONDS_PER_NANO * (t[i] - t[i-1])); // v[i]
        work += (vcurr - vprev) * fabsf(vcurr);
        if (i == count - 1) {
            work *= 0.5; // initial condition, case 2) above
        }
    }
    return kineticEnergyToVelocity(work);
}

bool ImpulseVelocityTrackerStrategy::getEstimator(int32_t pointerId,VelocityTracker::Estimator* outEstimator) const {
    const auto movementIt = mMovements.find(pointerId);
    if (movementIt == mMovements.end()) {
        return false; // no data
    }

    // Iterate over movement samples in reverse time order and collect samples.
    float positions[HISTORY_SIZE];
    nsecs_t time[HISTORY_SIZE];
    size_t m = 0; // number of points that will be used for fitting
    size_t index = mIndex.at(pointerId);
    const Movement& newestMovement = movementIt->second[index];
    do {
        const Movement& movement = movementIt->second[index];
        nsecs_t age = newestMovement.eventTime - movement.eventTime;
        if (age > HORIZON) {
            break;
        }
        if(movement.eventTime == 0 && index!=0){
            // All eventTime's are initialized to 0. If we encounter a time of 0 in a position
            // that's >0, it means that we hit the block where the data wasn't initialized.
            // It's also possible that the sample at 0 would be invalid, but there's no harm in
            // processing it, since it would be just a single point, and will only be encountered
            // in artificial circumstances (in tests).
            break;
	}
        positions[m] = movement.position;
        time[m] = movement.eventTime;
        index = (index == 0 ? HISTORY_SIZE : index) - 1;
    } while (++m < HISTORY_SIZE);

    if (m == 0) {
        return false; // no data
    }
    outEstimator->coeff[0] = 0;
    outEstimator->coeff[1] = calculateImpulseVelocity(time, positions, m ,mDeltaValues);
    outEstimator->coeff[2] = 0;

    outEstimator->time = newestMovement.eventTime;
    outEstimator->degree = 2; // similar results to 2nd degree fit
    outEstimator->confidence = 1;

    LOGD_IF(DEBUG_STRATEGY,"velocity: %f", outEstimator->coeff[1]);
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// --- LeastSquaresVelocityTrackerStrategy ---

LeastSquaresVelocityTrackerStrategy::LeastSquaresVelocityTrackerStrategy(
        uint32_t degree, Weighting weighting) :
        mDegree(degree), mWeighting(weighting) {
}

LeastSquaresVelocityTrackerStrategy::~LeastSquaresVelocityTrackerStrategy() {
}


void LeastSquaresVelocityTrackerStrategy::clearPointer(int32_t pointerId) {
    mIndex.erase(pointerId);
    mMovements.erase(pointerId);
}

void LeastSquaresVelocityTrackerStrategy::addMovement(nsecs_t eventTime, int32_t pointerId,float position) {
    // If data for this pointer already exists, we have a valid entry at the position of
    // mIndex[pointerId] and mMovements[pointerId]. In that case, we need to advance the index
    // to the next position in the circular buffer and write the new Movement there. Otherwise,
    // if this is a first movement for this pointer, we initialize the maps mIndex and mMovements
    // for this pointer and write to the first position.
    auto movementIt  = mMovements.insert({pointerId, {}});
    auto indexIt = mIndex.insert({pointerId, 0});
    const bool inserted = movementIt.second;
    size_t& index = indexIt.first->second;//second;
    if (!inserted && movementIt.first->second[index].eventTime != eventTime) {
        // When ACTION_POINTER_DOWN happens, we will first receive ACTION_MOVE with the coordinates
        // of the existing pointers, and then ACTION_POINTER_DOWN with the coordinates that include
        // the new pointer. If the eventtimes for both events are identical, just update the data
        // for this time.
        // We only compare against the last value, as it is likely that addMovement is called
        // in chronological order as events occur.
        index++;
    }
    if (index == HISTORY_SIZE) {
        index = 0;
    }

    Movement& movement = movementIt.first->second[index];
    movement.eventTime = eventTime;
    movement.position = position;
}
static bool solveLeastSquares(const std::vector<float>& x, const std::vector<float>& y,const std::vector<float>& w,
	uint32_t n, float* outB, float* outDet) {
    const size_t m = x.size();
    LOGD_IF(DEBUG_STRATEGY,"solveLeastSquares: m=%d, n=%d, x=%s, y=%s, w=%s", int(m), int(n),
            vectorToString(x.data(), m).c_str(), vectorToString(y.data(), m).c_str(),vectorToString(w.data(), m).c_str());

    // Expand the X vector to a matrix A, pre-multiplied by the weights.
    std::vector<std::vector<float>> a(n,std::vector<float>(m)); // column-major order
    for (uint32_t h = 0; h < m; h++) {
        a[0][h] = w[h];
        for (uint32_t i = 1; i < n; i++) {
            a[i][h] = a[i - 1][h] * x[h];
        }
    }
    LOGD_IF(DEBUG_STRATEGY,"  - a=%s", matrixToString(&a[0][0], m, n, false /*rowMajor*/).c_str());

    // Apply the Gram-Schmidt process to A to obtain its QR decomposition.
    std::vector<std::vector<float>> q(n,std::vector<float>(m)); // orthonormal basis, column-major order
    std::vector<std::vector<float>> r(n,std::vector<float>(n)); // upper triangular matrix, row-major order
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
            LOGD_IF(DEBUG_STRATEGY,"  - no solution, norm=%f", norm);
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
    std::vector<float> wy(m);
    for (uint32_t h = 0; h < m; h++) {
        wy[h] = y[h] * w[h];
    }
    for (uint32_t i = n; i != 0; ) {
        i--;
        outB[i] = vectorDot(&q[i][0], wy.data(), m);
        for (uint32_t j = n - 1; j > i; j--) {
            outB[i] -= r[i][j] * outB[j];
        }
        outB[i] /= r[i][i];
    }
    LOGD_IF(DEBUG_STRATEGY,"  - b=%s", vectorToString(outB, n).c_str());

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
    LOGD_IF(DEBUG_STRATEGY,"  - sserr=%f", sserr);
    LOGD_IF(DEBUG_STRATEGY,"  - sstot=%f", sstot);
    LOGD_IF(DEBUG_STRATEGY,"  - det=%f", *outDet);
    return true;
}

/*
 * Optimized unweighted second-order least squares fit. About 2x speed improvement compared to
 * the default implementation
 */
static bool solveUnweightedLeastSquaresDeg2(const std::vector<float>& x, const std::vector<float>& y,
	    std::array<float,3>*out) {
    const size_t count = x.size();
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

    float denominator = Sxx*Sx2x2 - Sxx2*Sxx2;
    if (denominator == 0) {
        LOGW("division by 0 when computing velocity, Sxx=%f, Sx2x2=%f, Sxx2=%f", Sxx, Sx2x2, Sxx2);
        return false;
    }
     // Compute a
    float numerator = Sx2y*Sxx - Sxy*Sxx2;
    const float a = numerator / denominator;

    // Compute b
    numerator = Sxy*Sx2x2 - Sx2y*Sxx2;
    const float b = numerator / denominator;

    // Compute c
    const float c = syi/count - b * sxi/count - a * sxi2/count;
    *out={a,b,c};
    return true;
}

bool LeastSquaresVelocityTrackerStrategy::getEstimator(int32_t pointerId,VelocityTracker::Estimator* outEstimator) const {
    const auto movementIt = mMovements.find(pointerId);
    if (movementIt == mMovements.end()) {
        return false; // no data
    }
    outEstimator->clear();

    // Iterate over movement samples in reverse time order and collect samples.
    std::vector<float> positions;
    std::vector<float> w;
    std::vector<float> time;

    size_t index = mIndex.at(pointerId);
    const Movement& newestMovement = movementIt->second[index];
    do {
        const Movement& movement = movementIt->second[index];
        nsecs_t age = newestMovement.eventTime - movement.eventTime;
        if (age > HORIZON) {
            break;
        }
        if (movement.eventTime == 0 && index != 0) {
            // All eventTime's are initialized to 0. In this fixed-width circular buffer, it's
            // possible that not all entries are valid. We use a time=0 as a signal for those
            // uninitialized values. If we encounter a time of 0 in a position
            // that's > 0, it means that we hit the block where the data wasn't initialized.
            // We still don't know whether the value at index=0, with eventTime=0 is valid.
            // However, that's only possible when the value is by itself. So there's no hard in
            // processing it anyways, since the velocity for a single point is zero, and this
            // situation will only be encountered in artificial circumstances (in tests).
            // In practice, time will never be 0.
            break;
        }
        positions.push_back(movement.position);
        w.push_back(chooseWeight(pointerId, index));
        time.push_back(-age * 0.000000001f);
        index = (index == 0 ? HISTORY_SIZE : index) - 1;    
    } while (positions.size() < HISTORY_SIZE);

    const size_t m = positions.size();
    if (m == 0) {
        return false; // no data
    }

    // Calculate a least squares polynomial fit.
    uint32_t degree = mDegree;
    if (degree > m - 1) {
        degree = m - 1;
    }

    if (degree == 2 && mWeighting == Weighting::NONE) {
        // Optimize unweighted, quadratic polynomial fit
        std::array<float, 3> coeff;
        const bool solved =solveUnweightedLeastSquaresDeg2(time, positions,&coeff);
        if (solved) {
            VelocityTracker::Estimator estimator;
            estimator.time = newestMovement.eventTime;
            estimator.degree = 2;
            estimator.confidence = 1;
            for (size_t i = 0; i <= estimator.degree; i++) {
                estimator.coeff[i] = coeff[i];
            }
            *outEstimator = estimator;
            return true;
        }
    } else if (degree >= 1) {
        // General case for an Nth degree polynomial fit
        float det;
        uint32_t n = degree + 1;
	VelocityTracker::Estimator estimator;
        if (solveLeastSquares(time, positions, w, n, estimator.coeff, &det)) {
            estimator.time = newestMovement.eventTime;
            estimator.degree = degree;
            estimator.confidence = det;
            *outEstimator = estimator;
            LOGD_IF(DEBUG_STRATEGY, "estimate: degree=%d, coeff=%s, confidence=%f",
                     int(estimator.degree), vectorToString(estimator.coeff, n).c_str(),
                     estimator.confidence);

            return true;
        }
    }

    // No velocity data available for this pointer, but we do have its current position.
    VelocityTracker::Estimator estimator;
    estimator.coeff[0] = positions[0];
    estimator.time = newestMovement.eventTime;
    estimator.degree = 0;
    estimator.confidence = 1;
    *outEstimator=estimator;
    return true;
}

float LeastSquaresVelocityTrackerStrategy::chooseWeight(int32_t pointerId,uint32_t index) const {
    const std::array<Movement, HISTORY_SIZE>& movements = mMovements.at(pointerId);
    switch (mWeighting) {
    case DELTA: {
        // Weight points based on how much time elapsed between them and the next
        // point so that points that "cover" a shorter time span are weighed less.
        //   delta  0ms: 0.5
        //   delta 10ms: 1.0
        if (index == mIndex.at(pointerId)) return 1.0f;

        uint32_t nextIndex = (index + 1) % HISTORY_SIZE;
        float deltaMillis = (movements[nextIndex].eventTime- movements[index].eventTime)
                *0.000001f;
        if (deltaMillis < 0 ) return 0.5f;
        if (deltaMillis < 10) return 0.5f + deltaMillis * 0.05f;
        return 1.0f;
    }

    case CENTRAL: {
        // Weight points based on their age, weighing very recent and very old points less.
        //   age  0ms: 0.5
        //   age 10ms: 1.0
        //   age 50ms: 1.0
        //   age 60ms: 0.5
        float ageMillis = (movements[mIndex.at(pointerId)].eventTime - movements[index].eventTime) * 0.001f;/*android us ns 0.000001f*/;
        if (ageMillis < 0 ) return 0.5f;
        if (ageMillis < 10) return 0.5f + ageMillis * 0.05f;
        if (ageMillis < 50) return 1.0f;
        if (ageMillis < 60) return 0.5f + (60 - ageMillis) * 0.05f;
        return 0.5f;
    }

    case RECENT: {
        // Weight points based on their age, weighing older points less.
        // age   0ms: 1.0  , age  50ms: 1.0 ,  age 100ms: 0.5
        float ageMillis = (movements[mIndex.at(pointerId)].eventTime - movements[index].eventTime)
                *0.000001f;
        if (ageMillis < 50) {
            return 1.0f;
        }
        if (ageMillis < 100) {
            return 0.5f + (100 - ageMillis) * 0.01f;
        }
        return 0.5f;
    }

    case NONE:
    default:
        return 1.0f;
    }
}

}//endof namespace
