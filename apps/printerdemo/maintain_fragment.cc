/*********************************************************************************
 * MaintainFragment — printhead maintenance actions (clean / nozzle check /
 * alignment / self-test) sharing one progress overlay; each completed run bumps
 * the maintenance counter (recordMaintenance).
 *********************************************************************************/
#include <cdroid.h>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>
#include <transition/slide.h>
#include <widget/textview.h>
#include <widget/linearlayout.h>
#include <widget/toast.h>
#include "printer_common.h"
#include "R.h"

// ---------------------------------------------------------------------------
class MaintainFragment : public cdroid::fragment::Fragment{
    // Weak-liveness flag (the PopupWindow/AbsListView mAliveFlag pattern), same
    // reason as CopyFragment: the progress posts capture raw view pointers and
    // can outlive the view (navigate away / recreate within 1.7s of the click —
    // AOSP survives this on GC alone). Flipped in onDestroyView; captured flag
    // copies turn stale posts into no-ops.
    std::shared_ptr<bool> mAliveFlag;
public:
    void onCreate(cdroid::Bundle* savedInstanceState) override{
        cdroid::fragment::Fragment::onCreate(savedInstanceState);
        setEnterTransition(new cdroid::Slide(cdroid::Gravity::END));
        setExitTransition(new cdroid::Slide(cdroid::Gravity::END));
    }
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle*) override{
        return inflater->inflate(printerdemo::R::layout::fragment_maintain, container, false);
    }
    void onViewCreated(cdroid::View* view, cdroid::Bundle*) override{
        cdroid::fragment::Fragment::onViewCreated(view, nullptr);
        mAliveFlag = std::make_shared<bool>(true);
        cdroid::View* overlay = view->findViewById(printerdemo::R::id::maint_overlay);
        cdroid::View* fill = view->findViewById(printerdemo::R::id::maint_fill);
        cdroid::TextView* status = (cdroid::TextView*)view->findViewById(printerdemo::R::id::maint_status);
        auto act = [this, view, overlay, fill, status, flag = mAliveFlag](int id, const std::string& msg){
            cdroid::View* row = view->findViewById(id);
            if(!row || !overlay || !fill || !status) return;
            row->setOnClickListener([this, overlay, fill, status, msg, flag](cdroid::View&){
                auto setPct = [fill](int w){
                    if(fill){
                        auto* lp = (cdroid::LinearLayout::LayoutParams*)fill->getLayoutParams();
                        lp->weight = (float)w;
                        fill->requestLayout();
                    }
                };
                overlay->setVisibility(cdroid::View::VISIBLE);
                setPct(15);
                status->setText(msg + "…");
                overlay->postDelayed([setPct, flag](){ if(!*flag) return; setPct(60); }, 600);
                overlay->postDelayed([setPct, flag](){ if(!*flag) return; setPct(100); }, 1200);
                overlay->postDelayed([this, overlay, setPct, msg, flag](){
                    if(!*flag) return;
                    setPct(0);
                    overlay->setVisibility(cdroid::View::GONE);
                    cdroid::Toast::makeText(getContext(), msg + " 已完成")->show();
                    if(auto* vm = sharedPrinterVM(this)) vm->recordMaintenance(); // bump maintenance counter
                }, 1700);
            });
        };
        act(printerdemo::R::id::m_clean,    "清洗打印头");
        act(printerdemo::R::id::m_nozzle,   "喷嘴检查");
        act(printerdemo::R::id::m_align,    "打印头校准");
        act(printerdemo::R::id::m_selftest, "设备自检");
    }
    void onDestroyView() override{
        if(mAliveFlag) *mAliveFlag = false;   // pending progress posts become no-ops
        cdroid::fragment::Fragment::onDestroyView();
    }
};
REGISTER_FRAGMENT(MaintainFragment);
