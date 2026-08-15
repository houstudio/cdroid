// Binary AXML inflation test sample.
// Loads a layout from the SDK framework pak (binary AXML) and displays it.
//
//   make axmltest
//   ./outX64-Debug/axmltest
#include <core/app.h>
#include <widget/cdwindow.h>
#include <widget/internal_R.h>
#include <widget/textview.h>
#include <widget/linearlayout.h>
#include <view/layoutinflater.h>
#include <porting/cdlog.h>

int main(int argc, const char* argv[]) {
    App app(argc, argv);
    Window* w = new Window(0, 0, -1, -1);

    // Inflate a framework layout (binary AXML in cdroid.pak).
    LayoutInflater* inflater = LayoutInflater::from(&app);
    View* view = inflater->inflate(cdroid::internal::R::layout::simple_list_item_1, nullptr);
    if (view) {
        LOGD("Binary AXML layout inflated OK");
        TextView* tv = dynamic_cast<TextView*>(view);
        if (tv) tv->setText("Binary AXML inflation OK!");
        w->addView(view);
    } else {
        LOGE("Layout inflation FAILED — text fallback");
        TextView* tv = new TextView("Inflation failed (text fallback)", 0, 0);
        w->addView(tv);
    }

    w->requestLayout();
    return app.exec();
}
