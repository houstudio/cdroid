/*********************************************************************************
 * ScanFragment — scan bed with a MotionLayout-driven beam sweep (scene_scan:
 * scan_start -> scan_end), start/stop, and the scan counter (recordScan).
 *********************************************************************************/
#include <cdroid.h>
#include <functional>
#include <memory>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>
#include <transition/slide.h>
#include <widget/textview.h>
#include <widget/button.h>
#include <widget/toast.h>
#include <widgetEx/constraintlayout/motion/motionlayout.h>
#include "printer_common.h"
#include "R.h"

// ---------------------------------------------------------------------------
class ScanFragment : public cdroid::fragment::Fragment{
    bool mScanning = false;
public:
    void onCreate(cdroid::Bundle* savedInstanceState) override{
        cdroid::fragment::Fragment::onCreate(savedInstanceState);
        setEnterTransition(new cdroid::Slide(cdroid::Gravity::END));
        setExitTransition(new cdroid::Slide(cdroid::Gravity::END));
    }
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle*) override{
        return inflater->inflate(printerdemo::R::layout::fragment_scan, container, false);
    }
    void onViewCreated(cdroid::View* view, cdroid::Bundle*) override{
        cdroid::fragment::Fragment::onViewCreated(view, nullptr);
        cdroid::MotionLayout* ml = (cdroid::MotionLayout*)view->findViewById(printerdemo::R::id::scan_preview);
        cdroid::TextView* status = (cdroid::TextView*)view->findViewById(printerdemo::R::id::scan_status);
        cdroid::Button* btn = (cdroid::Button*)view->findViewById(printerdemo::R::id::btn_scan);
        if(ml && btn && status){
            // The MotionLayout's app:layoutDescription="@xml/scene_scan" loads the <MotionScene> on
            // first measure (MotionLayout::buildScene), registering the scan_start (beam at top) ->
            // scan_end (beam at bottom) transition with the mid-sweep KeyAttribute. transitionToEnd
            // animates top->bottom; on completion snap back to top (setProgressInstant 0) and re-arm,
            // chained via the completion callback.
            auto sweep = std::make_shared<std::function<void()>>();
            *sweep = [this, ml, sweep](){
                if(!mScanning) return;
                ml->transitionToEnd([this, ml, sweep](){
                    if(!mScanning) return;          // stopped / fragment gone — don't touch ml
                    ml->setProgressInstant(0.f);    // snap beam back to the top
                    (*sweep)();                      // sweep again
                });
            };

            btn->setOnClickListener([this, ml, btn, status, sweep](cdroid::View&){
                if(!mScanning){
                    mScanning = true;
                    btn->setText("停止扫描");
                    status->setText("扫描中…");
                    status->setTextColor(0xFF188038);
                    (*sweep)();
                } else {
                    mScanning = false;
                    ml->setProgressInstant(0.f);
                    btn->setText("开始扫描");
                    status->setText("就绪 · Ready");
                    status->setTextColor(0xFF8A929C);
                    cdroid::Toast::makeText(getContext(), "扫描完成 · 已保存为 PDF")->show();
                    if(auto* vm = sharedPrinterVM(this)) vm->recordScan(); // bump scan counter
                }
            });
        }
    }
    void onDestroyView() override{
        mScanning = false;   // halt the sweep so any pending callback bails before touching ml
        cdroid::fragment::Fragment::onDestroyView();
    }
};
REGISTER_FRAGMENT(ScanFragment);
