/*********************************************************************************
 * CopyFragment — copier settings: copies stepper, color/paper/quality radios,
 * zoom seekbar, and the simulated print job (weight-driven progress overlay;
 * completion commits to the shared model via recordCopy).
 *********************************************************************************/
#include <cdroid.h>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>
#include <transition/slide.h>
#include <widget/textview.h>
#include <widget/button.h>
#include <widget/seekbar.h>
#include <widget/linearlayout.h>
#include <widget/radiogroup.h>
#include <widget/toast.h>
#include "printer_common.h"
#include "R.h"

// ---------------------------------------------------------------------------
class CopyFragment : public cdroid::fragment::Fragment{
    int mCopies = 1;
public:
    void onCreate(cdroid::Bundle* savedInstanceState) override{
        cdroid::fragment::Fragment::onCreate(savedInstanceState);
        setEnterTransition(new cdroid::Slide(cdroid::Gravity::END));
        setExitTransition(new cdroid::Slide(cdroid::Gravity::END));
    }
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle*) override{
        return inflater->inflate(printerdemo::R::layout::fragment_copy, container, false);
    }
    void onViewCreated(cdroid::View* view, cdroid::Bundle*) override{
        cdroid::fragment::Fragment::onViewCreated(view, nullptr);

        cdroid::TextView* tvCopies = (cdroid::TextView*)view->findViewById(printerdemo::R::id::tv_copies);
        cdroid::Button* minus = (cdroid::Button*)view->findViewById(printerdemo::R::id::btn_minus);
        cdroid::Button* plus   = (cdroid::Button*)view->findViewById(printerdemo::R::id::btn_plus);
        if(minus && tvCopies) minus->setOnClickListener([this, tvCopies](cdroid::View&){
            if(mCopies > 1){ mCopies--; tvCopies->setText(std::to_string(mCopies)); }
        });
        if(plus && tvCopies) plus->setOnClickListener([this, tvCopies](cdroid::View&){
            if(mCopies < 99){ mCopies++; tvCopies->setText(std::to_string(mCopies)); }
        });

        cdroid::SeekBar* seek = (cdroid::SeekBar*)view->findViewById(printerdemo::R::id::seek_zoom);
        cdroid::TextView* tvZoom = (cdroid::TextView*)view->findViewById(printerdemo::R::id::tv_zoom);
        if(seek && tvZoom){
            cdroid::SeekBar::OnSeekBarChangeListener l;
            l.onProgressChanged = [tvZoom](cdroid::SeekBar&, int progress, bool){
                tvZoom->setText(std::to_string(progress) + "%");
            };
            l.onStartTrackingTouch = [](cdroid::SeekBar&){};
            l.onStopTrackingTouch  = [](cdroid::SeekBar&){};
            seek->setOnSeekBarChangeListener(l);
        }

        cdroid::Button* start = (cdroid::Button*)view->findViewById(printerdemo::R::id::btn_start);
        cdroid::View* overlay = view->findViewById(printerdemo::R::id::print_overlay);
        cdroid::TextView* status = (cdroid::TextView*)view->findViewById(printerdemo::R::id::print_status);
        cdroid::View* fill = view->findViewById(printerdemo::R::id::print_fill);
        if(start && overlay && status){
            start->setOnClickListener([this, view, overlay, status, fill](cdroid::View&){
                const int N = mCopies;
                // Color mode drives ink drain in the shared model (K only for B/W, CMY too for color).
                bool color = false;
                if(cdroid::RadioGroup* rg = (cdroid::RadioGroup*)view->findViewById(printerdemo::R::id::rg_color))
                    color = (rg->getCheckedRadioButtonId() == printerdemo::R::id::rb_color);
                // Manual progress bar: advance the blue fill's weight inside a weightSum=100 track.
                auto setPct = [fill](int w){
                    if(fill){
                        auto* lp = (cdroid::LinearLayout::LayoutParams*)fill->getLayoutParams();
                        lp->weight = (float)w;
                        fill->requestLayout();
                    }
                };
                overlay->setVisibility(cdroid::View::VISIBLE);
                setPct(10);
                status->setText("正在预热…");
                overlay->postDelayed([status, N, setPct](){ setPct(35); status->setText("正在复印 1 / " + std::to_string(N)); }, 600);
                overlay->postDelayed([status, N, setPct](){ setPct(70); status->setText("正在复印 " + std::to_string(N) + " / " + std::to_string(N)); }, 1600);
                overlay->postDelayed([status, setPct](){ setPct(100); status->setText("正在收尾…"); }, 2600);
                overlay->postDelayed([this, overlay, N, setPct, color](){
                    setPct(0);
                    overlay->setVisibility(cdroid::View::GONE);
                    cdroid::Toast::makeText(getContext(), "复印完成 · " + std::to_string(N) + " 张")->show();
                    // Commit the job to the shared device model: bump counters, feed paper, drain ink.
                    if(auto* vm = sharedPrinterVM(this)) vm->recordCopy(N, color);
                }, 4000);
            });
        }
    }
};
REGISTER_FRAGMENT(CopyFragment);
