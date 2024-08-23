#ifndef __RLOTTIE_VIEW_H__
#define __RLOTTIE_VIEW_H__
#include <view/view.h>

struct Lottie_Animation;
namespace cdroid{

class RLottieView : public View {
public:
    RLottieView(Context *ctx, const AttributeSet &attrs);
    RLottieView(int width, int height);
    ~RLottieView();
    void   loadFromFile(const std::string &path);
    size_t getTotalFrame();
    bool   setFrameNum(size_t frameNum);
    void   start();
    void   pause();
    void   play();
    void   stop();
    bool   isRunning();

protected:
    virtual void onMeasure(int widthMeasureSpec, int heightMeasureSpec);
    virtual void onDraw(Canvas &ctx);

private:
    void initViewData();
    void nextFrame();

private:
    Lottie_Animation *mAnimation;
    uint32_t *        mBuffer;
    bool              mRunning;
    bool              mOneShot;
    bool              mAutoStart;
    size_t            mFrameNum;
    size_t            mTotalFrame;
    Runnable          mNextFrame;
    float             mFrameRate;
};
}/*endof namespace*/

#endif
