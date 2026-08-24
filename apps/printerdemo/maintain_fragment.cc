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
        cdroid::View* overlay = view->findViewById(printerdemo::R::id::maint_overlay);
        cdroid::View* fill = view->findViewById(printerdemo::R::id::maint_fill);
        cdroid::TextView* status = (cdroid::TextView*)view->findViewById(printerdemo::R::id::maint_status);
        auto act = [this, view, overlay, fill, status](int id, const std::string& msg){
            cdroid::View* row = view->findViewById(id);
            if(!row || !overlay || !fill || !status) return;
            row->setOnClickListener([this, overlay, fill, status, msg](cdroid::View&){
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
                overlay->postDelayed([setPct](){ setPct(60); }, 600);
                overlay->postDelayed([setPct](){ setPct(100); }, 1200);
                overlay->postDelayed([this, overlay, setPct, msg](){
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
};
REGISTER_FRAGMENT(MaintainFragment);
