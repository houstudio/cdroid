// CurvedTextDemo's outer-alignment twin (CDROID-added): identical texts at
// app:layout_valign="outer" — see curved_text_outer_demo.xml. Compare side by
// side with CurvedTextDemo (default center alignment) to see the vertical
// alignment shift ArcLayout gives widget children.
#include <core/app.h>
#include <cdroid.h>
#include <core/activityfactory.h>
#include "R.h"

using namespace cdroid;

class CurvedTextOuterDemo : public Window {
public:
    CurvedTextOuterDemo() : Window(&App::getInstance(), 0, 0, -1, -1) {
        addView(LayoutInflater::from(getContext())
                ->inflate(weardemos::R::layout::curved_text_outer_demo, this, false));
    }
};
REGISTER_ACTIVITY(CurvedTextOuterDemo);
