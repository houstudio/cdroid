/*********************************************************************************
 * HomeFragment — control-panel dashboard: status, ink (K/C/M/Y), paper, job
 * counters, and the function grid. Renders the activity-scoped PrinterViewModel
 * (bindPrinter — the layouts hold no hardcoded telemetry); cards navigate via
 * the NavHostFragment's NavController.
 *********************************************************************************/
#include <cdroid.h>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>
#include <navigation/navcontroller.h>
#include <transition/transitionmanager.h>
#include <transition/fade.h>
#include <transition/slide.h>
#include <animation/valueanimator.h>
#include <drawable/gradientdrawable.h>
#include <widget/textview.h>
#include "printer_common.h"
#include "R.h"

static void setWeight(cdroid::View* v, float w){
    if(!v) return;
    auto* lp = (cdroid::LinearLayout::LayoutParams*)v->getLayoutParams();
    if(lp){ lp->weight = w; v->requestLayout(); }
}

// Pushes the ViewModel's state into the Home dashboard views (ink bars, paper,
// status, job counters). Called from HomeFragment.onViewCreated so the first frame
// already reflects the model; re-runs each time Home's view is recreated.
static void bindPrinter(cdroid::View* root, printerdemo::PrinterViewModel& vm){
    namespace R = printerdemo::R;
    // Ink: each cartridge column = top spacer (weight 100-level) + fill (weight level) + % label.
    struct Slot{ int space, fill, pct; };
    const Slot inkSlots[4] = {
        {R::id::ink_k_space, R::id::ink_k_fill, R::id::ink_k_pct},
        {R::id::ink_c_space, R::id::ink_c_fill, R::id::ink_c_pct},
        {R::id::ink_m_space, R::id::ink_m_fill, R::id::ink_m_pct},
        {R::id::ink_y_space, R::id::ink_y_fill, R::id::ink_y_pct},
    };
    const auto& inks = vm.getInks();
    for(size_t i = 0; i < 4 && i < inks.size(); i++){
        const auto& ink = inks[i];
        float p = ink.level / 10.0f;             // 0.1-resolution percent: 0.5% steps are visible in the bar
        setWeight(root->findViewById(inkSlots[i].space), 100.0f - p);
        cdroid::View* fill = root->findViewById(inkSlots[i].fill);
        setWeight(fill, p);                      // fill color comes from the ink_fill_* pill drawable
        if(cdroid::TextView* pct = (cdroid::TextView*)root->findViewById(inkSlots[i].pct)){
            // Show one decimal only when nonzero, so 850 -> "85%", 845 -> "84.5%".
            int whole = ink.level / 10, tenths = ink.level % 10;
            std::string s = std::to_string(whole);
            if(tenths) s += "." + std::to_string(tenths);
            pct->setText(s + "%");
            pct->setTextColor(ink.percent() < ink.lowThreshold ? (int)0xFFEF6C00 : (int)0xFF8A929C);
        }
    }

    // Paper tray.
    const auto& paper = vm.getPaper();
    int ppct = paper.percent();
    setWeight(root->findViewById(R::id::paper_fill), (float)ppct);
    setWeight(root->findViewById(R::id::paper_space), (float)(100 - ppct));
    if(cdroid::TextView* t = (cdroid::TextView*)root->findViewById(R::id::paper_pct)) t->setText(std::to_string(ppct) + "%");
    if(cdroid::TextView* t = (cdroid::TextView*)root->findViewById(R::id::paper_sheets)) t->setText("约剩 " + std::to_string(paper.remaining) + " 张 · 250 张纸盒");
    if(cdroid::TextView* t = (cdroid::TextView*)root->findViewById(R::id::paper_name)) t->setText(paper.name);

    // Status + network.
    if(cdroid::TextView* t = (cdroid::TextView*)root->findViewById(R::id::status_text)) t->setText(vm.getStatus());
    if(cdroid::TextView* t = (cdroid::TextView*)root->findViewById(R::id::network_text)) t->setText(vm.getNetwork());

    // Job counters.
    auto setNum = [root](int id, int n){
        if(cdroid::TextView* t = (cdroid::TextView*)root->findViewById(id)) t->setText(std::to_string(n));
    };
    setNum(R::id::stat_prints, vm.getTotalPrints());
    setNum(R::id::stat_scans,  vm.getTotalScans());
    setNum(R::id::stat_copies, vm.getTotalCopies());
    setNum(R::id::stat_maint,  vm.getTotalMaintenance());
}

// ---------------------------------------------------------------------------
class HomeFragment : public cdroid::Fragment{
    cdroid::ValueAnimator* mHeroAnim = nullptr;
public:
    void onCreate(cdroid::Bundle* savedInstanceState) override{
        cdroid::Fragment::onCreate(savedInstanceState);
        setEnterTransition(new cdroid::Slide(cdroid::Gravity::START));
        setExitTransition(new cdroid::Slide(cdroid::Gravity::START));
    }
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle*) override{
        return inflater->inflate(printerdemo::R::layout::fragment_home, container, false);
    }
    void onViewCreated(cdroid::View* view, cdroid::Bundle*) override{
        cdroid::Fragment::onViewCreated(view, nullptr);
        auto go = [this, view](int cardId, int actionId){
            cdroid::View* card = view->findViewById(cardId);
            if(card) card->setOnClickListener([this, actionId](cdroid::View&){
                if(cdroid::NavController* nc = navControllerOf(this)) nc->navigate(actionId);
            });
        };
        go(printerdemo::R::id::card_copy,     printerdemo::R::id::action_home_to_copy);
        go(printerdemo::R::id::card_scan,     printerdemo::R::id::action_home_to_scan);
        go(printerdemo::R::id::card_maintain, printerdemo::R::id::action_home_to_maintain);
        go(printerdemo::R::id::card_settings, printerdemo::R::id::action_home_to_settings);

        // Function-card entrance via the transition framework (Fade) — the cards start invisible
        // in XML; beginDelayedTransition animates them fading in. No legacy view animations.
        cdroid::View* entrance[] = {
            view->findViewById(printerdemo::R::id::card_copy),
            view->findViewById(printerdemo::R::id::card_scan),
            view->findViewById(printerdemo::R::id::card_maintain),
            view->findViewById(printerdemo::R::id::card_settings)
        };
        cdroid::Fade fade;
        cdroid::TransitionManager::beginDelayedTransition((cdroid::ViewGroup*)view, &fade);
        for(cdroid::View* c : entrance) if(c) c->setVisibility(cdroid::View::VISIBLE);

        // Hero "breathing" gradient: cycle the gradient start color through a palette via
        // ValueAnimator(ofArgb). GradientDrawable.setGradientColors drives its own repaint each
        // tick, so no manual invalidate is needed. ValueAnimator is the property-animation
        // framework (android.animation), not a legacy view animation.
        cdroid::View* hero = view->findViewById(printerdemo::R::id::hero);
        if(hero)/*{
            cdroid::GradientDrawable* grad = dynamic_cast<cdroid::GradientDrawable*>(hero->getBackground());
            if(grad){
                mHeroAnim = cdroid::ValueAnimator::ofArgb({(int)0xFF0F72E5, (int)0xFF1565C0, (int)0xFF6A1B9A, (int)0xFF00695C});
                mHeroAnim->setDuration(50000);
                mHeroAnim->setRepeatCount(cdroid::ValueAnimator::INFINITE);
                mHeroAnim->setRepeatMode(cdroid::ValueAnimator::REVERSE);
                mHeroAnim->addUpdateListener(cdroid::ValueAnimator::AnimatorUpdateListener([grad](cdroid::ValueAnimator& va){
                    int c = va.getAnimatedValue().get<int>();
                    grad->setColors({c, (int)0xFF00B8D4});
                }));
                mHeroAnim->start();
            }
        }*/

        // Render the shared device model (ink, paper, status, counters) — the layouts
        // hold no hardcoded telemetry; the ViewModel is the single source.
        if(auto* vm = sharedPrinterVM(this)) bindPrinter(view, *vm);
    }
    void onDestroyView() override{
        if(mHeroAnim){ mHeroAnim->cancel(); mHeroAnim = nullptr; }
        cdroid::Fragment::onDestroyView();
    }
};
REGISTER_FRAGMENT(HomeFragment);
