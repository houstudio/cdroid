#include <widgetEx/rlottieview.h>

#if ENABLE(LOTTIE)
#include <rlottie_capi.h>

namespace cdroid{

DECLARE_WIDGET(RLottieView)
RLottieView::RLottieView(Context *ctx, const AttributeSet &attrs) : View(ctx, attrs) {
    initViewData();
    mAutoStart = attrs.getBoolean("autoStart", mAutoStart);
    mOneShot   = attrs.getBoolean("oneShot", mOneShot);
    loadFromFile(attrs.getString("src"));
}

RLottieView::RLottieView(int width, int height) : View(width, height) {
    initViewData();
}

RLottieView::~RLottieView() {
    if (mAnimation) {
        stop();
        lottie_animation_destroy(mAnimation);
        mAnimation = nullptr;
    }
}

void RLottieView::initViewData() {
    mAnimation  = nullptr;
    mFrameNum   = 1;
    mTotalFrame = 1;
    mRunning    = false;
    mAutoStart  = false;
    mOneShot    = false;
    mNextFrame  = [this](){nextFrame();};
}

void RLottieView::loadFromFile(const std::string &path) {
    if (access(path.c_str(), F_OK)) {
        LOGE("path not exists! %s", path.c_str());
        return;
    }
    if (mAnimation) {
        stop();
        lottie_animation_destroy(mAnimation);
        mAnimation = nullptr;
    }
    size_t width, height;
    mAnimation = lottie_animation_from_file(path.c_str());
    lottie_animation_get_size(mAnimation, &width, &height);
    mFrameRate  = lottie_animation_get_framerate(mAnimation);
    mTotalFrame = lottie_animation_get_totalframe(mAnimation);
    LOGV("size=%zux%zu total=%zu frameRate=%.1f", width, height, mTotalFrame, mFrameRate);
    requestLayout();
    if (mAutoStart) start();
}

size_t RLottieView::getTotalFrame() {
    return mTotalFrame;
}

bool RLottieView::setFrameNum(size_t frameNum) {
    if (frameNum > mTotalFrame) return false;
    if (mRunning) {
        LOGE("Running do not set fix frame.");
        return false;
    }
    mFrameNum = frameNum;
    return true;
}

void RLottieView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    LayoutParams *lp = getLayoutParams();
    if ((lp->width > 0 && lp->height > 0) ||
        (lp->width == LayoutParams::MATCH_PARENT && lp->height == LayoutParams::MATCH_PARENT)) {
        View::onMeasure(widthMeasureSpec, heightMeasureSpec);
        return;
    }

    if (!mAnimation) return;

    int widthMode  = MeasureSpec::getMode(widthMeasureSpec);
    int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    int widthSize  = MeasureSpec::getSize(widthMeasureSpec);
    int heightSize = MeasureSpec::getSize(heightMeasureSpec);
    LOGV("%d,%d,%d,%d", widthMode, heightMode, widthSize, heightSize);

    int    width  = widthSize;
    int    height = heightSize;
    size_t rwidth, rheight;
    lottie_animation_get_size(mAnimation, &rwidth, &rheight);
    if (lp->width == LayoutParams::WRAP_CONTENT) width = rwidth;
    if (lp->height == LayoutParams::WRAP_CONTENT) height = rheight;
    if (lp->width == LayoutParams::WRAP_CONTENT || lp->height == LayoutParams::WRAP_CONTENT) {
        setMeasuredDimension(width + getPaddingLeft() + getPaddingRight(),
                             height + getPaddingTop() + getPaddingBottom());
    }
}

void RLottieView::onDraw(Canvas &ctx) {
    View::onDraw(ctx);
    if (!mAnimation) return;

    int width  = getWidth() - getPaddingLeft() - getPaddingRight();
    int height = getHeight() - getPaddingTop() - getPaddingBottom();

    LOGV("(%dx%d) %zu/%zu", width, height, mFrameNum, lottie_animation_get_totalframe(mAnimation));

    Cairo::RefPtr<Cairo::ImageSurface> surface =
        Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, width, height);
    cairo_t *scr = cairo_create(surface->cobj());
    cairo_paint(scr);

    lottie_animation_render(mAnimation, mFrameNum, (uint32_t *)surface->get_data(), width, height,
                            Cairo::ImageSurface::format_stride_for_width(Cairo::Surface::Format::ARGB32, width));

    ctx.set_source(surface, 0, 0);
    ctx.paint();

    cairo_destroy(scr);
}

void RLottieView::start() {
    if (!mAnimation || mRunning) return;
    mRunning  = true;
    mFrameNum = 1;
    invalidate();
    removeCallbacks(mNextFrame);
    postDelayed(mNextFrame, 1000 / mFrameRate);
}

void RLottieView::pause() {
    if (!mAnimation || !mRunning) return;
    mRunning = false;
    removeCallbacks(mNextFrame);
}

void RLottieView::play() {
    if (!mAnimation || mRunning) return;
    mRunning = true;
    postDelayed(mNextFrame, 1000 / mFrameRate);
}

void RLottieView::stop() {
    if (!mRunning) return;
    mRunning = false;
    removeCallbacks(mNextFrame);
}

bool RLottieView::isRunning() {
    return mRunning;
}

void RLottieView::nextFrame() {
    invalidate();
    if (mOneShot && mFrameNum + 1 > mTotalFrame) {
        mRunning = false;
        return;
    }
    if (++mFrameNum > mTotalFrame) { mFrameNum = 1; }
    postDelayed(mNextFrame, 1000 / mFrameRate);
}
}/*endof namespace*/
#endif
