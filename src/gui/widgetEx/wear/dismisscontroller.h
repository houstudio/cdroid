#ifndef __WEAR_DISMISS_CONTROLLER_H__
#define __WEAR_DISMISS_CONTROLLER_H__
#include <core/context.h>
namespace cdroid{
class DismissibleFrameLayout;
class DismissController {
public:
    struct OnDismissListener {
        std::function<void()> onDismissStarted;
        std::function<void()> onDismissCanceled;
        std::function<void()> onDismissed;
    };
protected:
    Context* mContext;
    DismissibleFrameLayout* mLayout;
    OnDismissListener mDismissListener;
public:
    DismissController(Context* context, DismissibleFrameLayout* layout) {
        mContext = context;
        mLayout = layout;
    }
    virtual ~DismissController()=default;
    void setOnDismissListener(const OnDismissListener& listener) {
        mDismissListener = listener;
    }
};
}/*endof namespace*/
#endif/*__WEAR_DISMISS_CONTROLLER_H__*/
