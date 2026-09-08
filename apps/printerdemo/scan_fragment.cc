/*********************************************************************************
 * ScanFragment — scan bed with a MotionLayout-driven beam sweep (scene_scan:
 * scan_start -> scan_end), start/stop, and the scan counter (recordScan).
 *********************************************************************************/
#include <cdroid.h>
#include <functional>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>
#include <transition/slide.h>
#include <widget/adapter.h>
#include <widget/spinner.h>
#include <widget/textview.h>
#include <widget/button.h>
#include <widget/toast.h>
#include <widgetEx/constraintlayout/motion/motionlayout.h>
#include "printer_common.h"
#include "R.h"

// DPI choices offered by the scan-settings Spinner (position order).
static const char* kScanDpiChoices[] = {"150 dpi", "300 dpi", "600 dpi", "1200 dpi"};
static const int kScanDpiDefault = 1;   // 300 dpi — matches scan_mode_summary

// ---------------------------------------------------------------------------
class ScanFragment : public cdroid::Fragment{
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
    // DPI Spinner adapter — app-owned (the AdapterView keeps a raw pointer),
    // freed in onDestroy once the view tree (and its popup ListView) is gone.
    cdroid::ArrayAdapter<std::string>* mDpiAdapter = nullptr;
public:
    void onCreate(cdroid::Bundle* savedInstanceState) override{
        cdroid::Fragment::onCreate(savedInstanceState);
        setEnterTransition(new cdroid::Slide(cdroid::Gravity::END));
        setExitTransition(new cdroid::Slide(cdroid::Gravity::END));
    }
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle*) override{
        return inflater->inflate(printerdemo::R::layout::fragment_scan, container, false);
    }
    void onViewCreated(cdroid::View* view, cdroid::Bundle*) override{
        cdroid::Fragment::onViewCreated(view, nullptr);
        // DPI Spinner: dropdown of scan resolutions; the selection re-renders the
        // summary line ("彩色 · 300 dpi · PDF").
        cdroid::Spinner* dpi = (cdroid::Spinner*)view->findViewById(printerdemo::R::id::spinner_dpi);
        cdroid::TextView* summary = (cdroid::TextView*)view->findViewById(printerdemo::R::id::scan_summary);
        if(dpi && summary){
            delete mDpiAdapter;   // view re-created: the previous adapter's tree is gone
            mDpiAdapter = new cdroid::ArrayAdapter<std::string>(
                    getContext(), printerdemo::R::layout::spinner_item,
                    printerdemo::R::id::spinner_item_text);
            // closed row carries the drop-down arrow; popup rows stay plain
            mDpiAdapter->setDropDownViewResource(printerdemo::R::layout::spinner_item_dropdown);
            for(const char* c : kScanDpiChoices) mDpiAdapter->add(c);
            dpi->setAdapter(mDpiAdapter);
            dpi->setSelection(kScanDpiDefault);

            cdroid::AdapterView::OnItemSelectedListener dpiListener;
            dpiListener.onItemSelected = [summary](cdroid::AdapterView&, cdroid::View&, int position, long){
                summary->setText(std::string("彩色 · ") + kScanDpiChoices[position] + " · PDF");
            };
            dpi->setOnItemSelectedListener(dpiListener);
        }
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
        cdroid::Fragment::onDestroyView();
    }
    void onDestroy() override{
        // After the view tree (Spinner + its popup ListView) is destroyed.
        delete mDpiAdapter;
        mDpiAdapter = nullptr;
        cdroid::Fragment::onDestroy();
    }
};
REGISTER_FRAGMENT(ScanFragment);
