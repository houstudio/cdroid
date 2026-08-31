/*********************************************************************************
 * ScanFragment — scan bed with a MotionLayout-driven beam sweep (scene_scan:
 * scan_start -> scan_end), start/stop, and the scan counter (recordScan).
 *********************************************************************************/
#include <cdroid.h>
#include <functional>
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
    // Self-re-arming sweep callback. A member Runnable recurses through `this` — the old
    // make_shared<std::function> self-capture was a reference cycle that leaked (valgrind
    // 80B/blk in ScanFragment::onViewCreated).
    cdroid::Runnable mSweep;
    // Weak-liveness flag (the PopupWindow/AbsListView mAliveFlag pattern): the sweep's
    // async re-arm rides MotionLayout's transition-end callback, which can fire after
    // this fragment — not just its view — is freed; the mScanning bail would then read
    // the dead fragment. The flag is refcounted with its captured copies and flipped in
    // onDestroyView, so a stale callback no-ops without touching `this`.
    std::shared_ptr<bool> mAliveFlag;
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
            mAliveFlag = std::make_shared<bool>(true);
            // The MotionLayout's app:layoutDescription="@xml/scene_scan" loads the <MotionScene> on
            // first measure (MotionLayout::buildScene), registering the scan_start (beam at top) ->
            // scan_end (beam at bottom) transition with the mid-sweep KeyAttribute. transitionToEnd
            // animates top->bottom; on completion snap back to top (setProgressInstant 0) and re-arm
            // via mSweep (member Runnable — recurses through `this`, no heap self-reference).
            mSweep = [this, ml, flag = mAliveFlag](){
                if(!*flag || !mScanning) return;    // view/fragment gone — flag first (refcounted)
                ml->transitionToEnd([this, ml, flag](){
                    if(!*flag || !mScanning) return;  // stopped / fragment gone — don't touch ml
                    ml->setProgressInstant(0.f);    // snap beam back to the top
                    mSweep();                       // sweep again
                });
            };

            btn->setOnClickListener([this, ml, btn, status](cdroid::View&){
                if(!mScanning){
                    mScanning = true;
                    btn->setText("停止扫描");
                    status->setText("扫描中…");
                    status->setTextColor(0xFF188038);
                    mSweep();
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
        if(mAliveFlag) *mAliveFlag = false;   // ... and that bail itself must not read the dead fragment
        cdroid::fragment::Fragment::onDestroyView();
    }
};
REGISTER_FRAGMENT(ScanFragment);
