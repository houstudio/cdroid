#ifndef __DESKCLOCK_ELLIPSIZELAYOUT_H__
#define __DESKCLOCK_ELLIPSIZELAYOUT_H__
/*********************************************************************************
 * Port of com.android.deskclock.widget.EllipsizeLayout — a horizontal
 * LinearLayout that gives its single ellipsized TextView child a max width so
 * the row fits exactly.
 *********************************************************************************/
#include <widget/linearlayout.h>

namespace cdroid {
namespace deskclock {

class EllipsizeLayout : public LinearLayout {
public:
    EllipsizeLayout(Context* context, const AttributeSet* attrs);

protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ELLIPSIZELAYOUT_H__
