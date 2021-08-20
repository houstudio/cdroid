#include <widget/edgeeffect.h>
#include <systemclock.h>
#include <cdtypes.h>
#include <cdlog.h>


namespace cdroid{

constexpr float EdgeEffect::MAX_ALPHA ;
constexpr float EdgeEffect::PULL_GLOW_BEGIN;
constexpr int EdgeEffect::MIN_VELOCITY;
constexpr int EdgeEffect::MAX_VELOCITY;

EdgeEffect::EdgeEffect(Context* context){
    mInterpolator = new DecelerateInterpolator();
    mDisplacement=0.5f;
    mBounds.set(0,0,0,0);
    mPullDistance =0;
    mState=STATE_IDLE;
    mGlowAlpha=0;
    mGlowAlphaStart=mGlowAlphaFinish=.0f;
    mBaseGlowScale=0;
    mGlowScaleY=0;
    mGlowScaleYStart=mGlowScaleYFinish=.0f;
    mStartTime=0;
    mDuration =PULL_DECAY_TIME;
}

EdgeEffect::~EdgeEffect(){
    delete mInterpolator;
}

void EdgeEffect::setSize(int width, int height){
    float r = width * RADIUS_FACTOR / SIN;
    float y = COS * r;
    float h = r - y;
    float oR = height * RADIUS_FACTOR / SIN;
    float oy = COS * oR;
    float oh = oR - oy;

    mRadius = r;
    mBaseGlowScale = h > 0 ? std::min(oh / h, 1.f) : 1.f;
    mBounds.set(mBounds.x, mBounds.y, width, (int) std::min((float)height, h));
}
bool EdgeEffect::isFinished()const{
    return mState == STATE_IDLE;
}
void EdgeEffect::finish(){
    mState = STATE_IDLE;
}

void EdgeEffect::onPull(float deltaDistance){
    onPull(deltaDistance, 0.5f);
}

void EdgeEffect::onPull(float deltaDistance, float displacement){
    long now = SystemClock::uptimeMillis();
    mTargetDisplacement = displacement;
    if (mState == STATE_PULL_DECAY && now - mStartTime < mDuration) {
        return;
    }
    if (mState != STATE_PULL) {
        mGlowScaleY = std::max(PULL_GLOW_BEGIN, mGlowScaleY);
    }
    mState = STATE_PULL;

    mStartTime = now;
    mDuration = PULL_TIME;

    mPullDistance += deltaDistance;

    float absdd = std::abs(deltaDistance);
    mGlowAlpha = mGlowAlphaStart = std::min(MAX_ALPHA,
                mGlowAlpha + (absdd * PULL_DISTANCE_ALPHA_GLOW_FACTOR));

    if (mPullDistance == 0) {
        mGlowScaleY = mGlowScaleYStart = 0;
    } else {
        float scale = (float) (std::max(0.f, 1.f - 1.f /
               (float)std::sqrt(std::abs(mPullDistance) * mBounds.height) - 0.3f) / 0.7f);

        mGlowScaleY = mGlowScaleYStart = scale;
    }

    mGlowAlphaFinish = mGlowAlpha;
    mGlowScaleYFinish = mGlowScaleY;
}

void EdgeEffect::onRelease(){
    mPullDistance = 0;

    if (mState != STATE_PULL && mState != STATE_PULL_DECAY) {
        return;
    }

    mState = STATE_RECEDE;
    mGlowAlphaStart = mGlowAlpha;
    mGlowScaleYStart = mGlowScaleY;

    mGlowAlphaFinish = 0.f;
    mGlowScaleYFinish = 0.f;

    mStartTime = SystemClock::uptimeMillis();
    mDuration = RECEDE_TIME;
}

void EdgeEffect::onAbsorb(int velocity){
    mState = STATE_ABSORB;
    velocity = std::min(std::max(MIN_VELOCITY, std::abs(velocity)), MAX_VELOCITY);

    mStartTime = SystemClock::uptimeMillis();
    mDuration = 0.15f + (velocity * 0.02f);

    // The glow depends more on the velocity, and therefore starts out
    // nearly invisible.
    mGlowAlphaStart = GLOW_ALPHA_START;
    mGlowScaleYStart = std::max(mGlowScaleY, 0.f);


    // Growth for the size of the glow should be quadratic to properly
    // respond
    // to a user's scrolling speed. The faster the scrolling speed, the more
    // intense the effect should be for both the size and the saturation.
    mGlowScaleYFinish = std::min(0.025f + (velocity * (velocity / 100) * 0.00015f) / 2, 1.f);
    // Alpha should change for the glow as well as size.
    mGlowAlphaFinish = std::max(
            mGlowAlphaStart, std::min(velocity * VELOCITY_GLOW_FACTOR * .00001f, MAX_ALPHA));
    mTargetDisplacement = 0.5f;
}

void EdgeEffect::setColor(int color){
    mColor=color;
}

int EdgeEffect::getColor()const{
    return mColor;
}

bool EdgeEffect::draw(Canvas& canvas){
    update();
    float centerX = mBounds.centerX();
    float centerY = mBounds.height - mRadius;
    canvas.save();

    float displacement = std::max(0.f, std::min(mDisplacement, 1.f)) - 0.5f;
    float translateX = mBounds.width * displacement / 2;
   
    mColor=0xFFFFFFFF;

    canvas.rectangle(mBounds);
    canvas.clip();
    //mPaint.setAlpha((int) (0xff * mGlowAlpha));
    canvas.set_color(mColor);
    canvas.curve_to(mBounds.x,mBounds.y,mBounds.width/2+translateX,mBounds.height*mGlowScaleY,mBounds.width,0);
    canvas.fill();

    canvas.restore();

    bool oneLastFrame = false;
    if (mState == STATE_RECEDE && mGlowScaleY == 0) {
        mState = STATE_IDLE;
        oneLastFrame = true;
    }
    return mState != STATE_IDLE || oneLastFrame;
}

int EdgeEffect::getMaxHeight()const{
   return (int) (mBounds.height * MAX_GLOW_SCALE + 0.5f);
}

void EdgeEffect::update() {
    long time = SystemClock::uptimeMillis();
    float t = std::min((time - mStartTime) / mDuration, 1.f);

    float interp = mInterpolator->getInterpolation(t);

    mGlowAlpha = mGlowAlphaStart + (mGlowAlphaFinish - mGlowAlphaStart) * interp;
    mGlowScaleY = mGlowScaleYStart + (mGlowScaleYFinish - mGlowScaleYStart) * interp;
    mDisplacement = (mDisplacement + mTargetDisplacement) / 2;

    if (t >= 1.f - EPSILON) {
        switch (mState) {
        case STATE_ABSORB:
            mState = STATE_RECEDE;
            mStartTime = SystemClock::uptimeMillis();
            mDuration = RECEDE_TIME;

            mGlowAlphaStart = mGlowAlpha;
            mGlowScaleYStart = mGlowScaleY;

            // After absorb, the glow should fade to nothing.
            mGlowAlphaFinish = 0.f;
            mGlowScaleYFinish = 0.f;
            break;
        case STATE_PULL:
            mState = STATE_PULL_DECAY;
            mStartTime = SystemClock::uptimeMillis();
            mDuration = PULL_DECAY_TIME;

            mGlowAlphaStart = mGlowAlpha;
            mGlowScaleYStart = mGlowScaleY;

            // After pull, the glow should fade to nothing.
            mGlowAlphaFinish = 0.f;
            mGlowScaleYFinish = 0.f;
            break;
        case STATE_PULL_DECAY:
            mState = STATE_RECEDE;
            break;
        case STATE_RECEDE:
            mState = STATE_IDLE;
            break;
        }
    }
}

}//namespace
