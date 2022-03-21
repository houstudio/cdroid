#ifndef __AIMATOR_SET_H__
#define __AIMATOR_SET_H__
#include <animation/valueanimator.h>
#include <animation/animationhandler.h>
namespace cdroid{

class AnimatorSet:public Animator,public AnimationHandler::AnimationFrameCallback{
private:
    class Node{
    public:
        Animator* mAnimation;
        std::vector<Node*>mChildNodes;
        bool mEnded = false;

        /**Nodes with animations that are defined to play simultaneously with the animation
         * associated with this current node. */
        std::vector<Node*> mSiblings;

        /**Parent nodes are the nodes with animations preceding current node's animation. Parent
         * nodes here are derived from user defined animation sequence. */
        std::vector<Node*> mParents;
        /**Latest parent is the parent node associated with a animation that finishes after all
         * the other parents' animations. */
        Node* mLatestParent = nullptr;

        bool mParentsAdded = false;
        long mStartTime = 0;
        long mEndTime = 0;
        long mTotalDuration = 0;
    public:
        Node(Animator* animation);
        Node* clone();
        void addChild(Node* node);
        void addSibling(Node* node);
        void addParent(Node* node);
        void addParents(const std::vector<Node*>& parents);
    };
    class AnimationEvent{
    public:
        static constexpr int ANIMATION_START = 0;
        static constexpr int ANIMATION_DELAY_ENDED = 1;
        static constexpr int ANIMATION_END = 2;
        Node* mNode;
        int mEvent;
    public:
        AnimationEvent(Node* node, int event);
        long getTime()const;
    };
    class SeekState{
    private:
        AnimatorSet*mAnimSet;
        long mPlayTime;
        bool mSeekingInReverse;
    public:
        SeekState(AnimatorSet*set);
        void reset();
        void setPlayTime(long playTime, bool inReverse);
        void updateSeekDirection(bool inReverse);
        long getPlayTime()const;
        long getPlayTimeNormalized()const;
        bool isActive()const;
    };
public:
    class Builder{
    private:
        AnimatorSet*mAnimSet;
        Node*mCurrentNode;
    public:
        Builder(AnimatorSet*set,Animator* anim);
        Builder& with(Animator* anim);
        Builder& before(Animator* anim);
        Builder& after(Animator* anim);
        Builder& after(long delay);
    };
private:
    std::vector<Node*> mPlayingSet;
    std::map<Animator*, Node*> mNodeMap;
    std::vector<AnimationEvent*> mEvents;
    std::vector<Node*> mNodes;
    bool mDependencyDirty = false;

    /* Indicates whether an AnimatorSet has been start()'d, whether or
     * not there is a nonzero startDelay. */
    bool mStarted = false;

    // The amount of time in ms to delay starting the animation after start() is called
    long mStartDelay = 0;

    // Animator used for a nonzero startDelay
    ValueAnimator* mDelayAnim;// = ValueAnimator.ofFloat(0f, 1f).setDuration(0);

    Node* mRootNode;// = new Node(mDelayAnim);

    long mDuration = -1;

    TimeInterpolator* mInterpolator = nullptr;

    // The total duration of finishing all the Animators in the set.
    long mTotalDuration = 0;

    bool mShouldIgnoreEndWithoutStart;
    bool mShouldResetValuesAtStart;
    bool mEndCanBeCalled;
    long mLastFrameTime = -1;
    long mFirstFrame = -1;
    int mLastEventId = -1;
    // Indicates whether the animation is reversing.
    bool mReversing = false;
    bool mSelfPulse = true;
    SeekState* mSeekState;
    bool mChildrenInitialized = false;
    long mPauseTime = -1;
private:
    void forceToEnd();
    void initAnimation();
    void start(bool inReverse, bool selfPulse);
    static bool isEmptySet(AnimatorSet* set);
    void updateAnimatorsDuration();
    void skipToStartValue(bool inReverse);
    void initChildren();
    void handleAnimationEvents(int startId, int latestId, long playTime);
    void pulseFrame(Node* node, long animPlayTime);
    long getPlayTimeForNode(long overallPlayTime, Node* node) ;
    long getPlayTimeForNode(long overallPlayTime, Node* node, bool inReverse);
    void startAnimation();
    void addDummyListener();
    void removeDummyListener();
     int findLatestEventIdForTime(long currentPlayTime);
    void endAnimation();
    void removeAnimationCallback();
    void addAnimationCallback(long delay);
    void createDependencyGraph();
    void sortAnimationEvents();
    void updatePlayTime(Node* parent,std::vector<Node*>& visited);
    void findSiblings(Node* node,std::vector<Node*>& siblings) ;
    Node* getNodeForAnimation(Animator* anim);
protected:
    void skipToEndValue(bool inReverse);
    void animateBasedOnPlayTime(long currentPlayTime, long lastPlayTime, bool inReverse);
    bool isInitialized();
public:
    AnimatorSet();
    ~AnimatorSet();
    void playTogether(const std::vector<Animator*>&);
    void playSequentially(const std::vector<Animator*>&);
    std::vector<Animator*> getChildAnimations()const;
    void setTarget(void* target);
    void setInterpolator(TimeInterpolator*)override;
    TimeInterpolator*getInterpolator()override;
    Builder* play(Animator* anim);
    void cancel()override;
    void end();
    bool isRunning();
    bool isStarted();
    long getStartDelay();
    void setStartDelay(long startDelay);
    long getDuration();
    Animator& setDuration(long)override;
    void setupStartValues()override;
    void setupEndValues()override;
    void pause()override;
    void resume()override;
    void start()override;
    void setCurrentPlayTime(long playTime);
    long getCurrentPlayTime();
    bool doAnimationFrame(long frameTime)override;
    void commitAnimationFrame(long frameTime)override;
    bool pulseAnimationFrame(long frameTime)override;
    Animator* clone()override;
    bool canReverse()override;
    void reverse()override;
    bool shouldPlayTogether();
    long getTotalDuration()override;
};

}
#endif
