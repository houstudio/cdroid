#include <widgetEx/wear/dismissableframelayout.h>
namespace cdroid{
public class SwipeDismissFrameLayout:public DismissibleFrameLayout {

    public static constexpr float DEFAULT_DISMISS_DRAG_WIDTH_RATIO = 0.33f;


    public class Callback {

        public void onSwipeStarted(SwipeDismissFrameLayout& layout);

        public void onSwipeCanceled(SwipeDismissFrameLayout& layout);

        public void onDismissed(SwipeDismissFrameLayout& layout);
    };

    std::vector<Callback> mCallbacksCompat;

    public SwipeDismissFrameLayout(Context* context) {
        this(context, null, 0);
    }

    public SwipeDismissFrameLayout(Context* context,const AttributeSet& attrs) {
        this(context, attrs, 0);
    }

    public SwipeDismissFrameLayout(Context context, AttributeSet attrs, int defStyle) {
        this(context, attrs, defStyle, 0);
    }

    public SwipeDismissFrameLayout(Context* context,const AttributeSet& attrs, int defStyle,
            int defStyleRes) {
        super(context, attrs, defStyle, defStyleRes);
    }

    public void addCallback(Callback callback) {
        if (callback == null) {
            throw new NullPointerException("addCallback called with null callback");
        }

        mCallbacksCompat.add(callback);
    }

    public void removeCallback(Callback callback) {
        if (callback == null) {
            throw new NullPointerException("removeCallback called with null callback");
        }
        if (!mCallbacksCompat.remove(callback)) {
            throw new IllegalStateException("removeCallback called with nonexistent callback");
        }
    }

    public void setSwipeable(bool swipeable) {
        super.setSwipeDismissible(swipeable);
    }

    public bool isSwipeable() {
        return super.isDismissableBySwipe();
    }

    public void setDismissMinDragWidthRatio(float ratio) {
        if (isSwipeable()) {
            getSwipeDismissController().setDismissMinDragWidthRatio(ratio);
        }
    }

    public float getDismissMinDragWidthRatio() {
        if (isSwipeable()) {
            return getSwipeDismissController().getDismissMinDragWidthRatio();
        }
        return DEFAULT_DISMISS_DRAG_WIDTH_RATIO;
    }

    @Override
    protected void performDismissFinishedCallbacks() {
        super.performDismissFinishedCallbacks();
        for (int i = mCallbacksCompat.size() - 1; i >= 0; i--) {
            mCallbacksCompat.at(i).onDismissed(this);
        }
    }

    @Override
    protected void performDismissStartedCallbacks() {
        super.performDismissStartedCallbacks();
        for (int i = mCallbacksCompat.size() - 1; i >= 0; i--) {
            mCallbacksCompat.at(i).onSwipeStarted(this);
        }
    }

    @Override
    protected void performDismissCanceledCallbacks() {
        super.performDismissCanceledCallbacks();
        for (int i = mCallbacksCompat.size() - 1; i >= 0; i--) {
            mCallbacksCompat.at(i).onSwipeCanceled(this);
        }
    }
}
