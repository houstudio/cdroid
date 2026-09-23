// CDROID-added (not from SupportWearDemos): ArcLayout x CurvedTextView
// verification page — the curved texts ring the circle (widget path through
// dynamic_cast<ArcLayout::Widget*>), exercising measure's thickness pass,
// weighted-free layout, drawChild's rotate-about-center, and the ellipsize
// machinery on the long line. See curved_text_demo.xml for the layout.
#include <core/app.h>
#include <cdroid.h>
#include <core/activityfactory.h>
#include "R.h"

using namespace cdroid;

class CurvedTextDemo : public Window {
public:
    CurvedTextDemo() : Window(&App::getInstance(), 0, 0, -1, -1) {
        addView(LayoutInflater::from(getContext())
                ->inflate(weardemos::R::layout::curved_text_demo, this, false));
    }
};
REGISTER_ACTIVITY(CurvedTextDemo);
