/*********************************************************************************
 * printerdemo: a printer's built-in control-panel UI (MFP HMI), built with
 * Fragment + Navigation + android.transition (Slide). Pure facade — no driver.
 *
 *   HomeFragment     control-panel dashboard: status, ink (K/C/M/Y), paper, function grid
 *   CopyFragment     copier settings: copies stepper, color/paper/quality radios, zoom seekbar
 *   MaintainFragment printhead maintenance actions + supplies status
 *   SettingsFragment network / preferences / system rows
 *
 * Fragment enter/exit use android.transition Slide (set per-Fragment in onCreate), NOT legacy
 * enterAnim/exitAnim — see navdemo_transition. IDs (R.h / ID.xml) are auto-generated.
 *********************************************************************************/
#include <cdroid.h>
#include <core/build.h>
#include <core/activityfactory.h>
#include <core/assetmanager.h>
#include <cdlog.h>
#include <widget/toolbar.h>
#include <widget/textview.h>
#include <widget/button.h>
#include <widget/seekbar.h>
#include <widget/linearlayout.h>
#include <widget/toast.h>
#include <widgetEx/constraintlayout/motion/motionlayout.h>
#include <transition/transitionmanager.h>
#include <transition/fade.h>
#include <animation/valueanimator.h>
#include <drawable/gradientdrawable.h>
#include <drawable/colordrawable.h>
#include <text/html.h>
#include <widget/actionbar.h>
#include <transition/slide.h>
#include <navigation/navhostfragment.h>
#include <navigation/navcontroller.h>
#include <navigation/navdestination.h>
#include <navigation/navigationui.h>
#include <widgetEx/navigationview/bottomnavigationview.h>
#include <fragment/fragment.h>
#include <fragment/fragmentactivity.h>
#include <fragment/fragmentfactory.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <menu/menuinflater.h>
#include <menu/popupmenu.h>
#include <widget/radiogroup.h>
#include <lifecycle/viewmodelprovider.h>
#include "printer_viewmodel.h"
#include "R.h"

static cdroid::NavController* navControllerOf(cdroid::fragment::Fragment* f){
    cdroid::NavHostFragment* host = dynamic_cast<cdroid::NavHostFragment*>(f->getParentFragment());
    return host ? host->getNavController() : nullptr;
}

// ---------------------------------------------------------------------------
// Device data model (androidx.lifecycle.ViewModel).
//   sharedPrinterVM() returns the single activity-scoped PrinterViewModel: HomeFragment
//   renders it, Copy/Scan/Maintain mutate it. Scoped to the FragmentActivity that
//   hosts the NavHostFragment, so the instance survives Fragment view destruction
//   (Home -> Copy -> back rebuilds Home's view and re-reads the updated counters/ink).
//   Mirrors ViewModelProvider(requireActivity()).get(PrinterViewModel::class.java).
// ---------------------------------------------------------------------------
static printerdemo::PrinterViewModel* sharedPrinterVM(cdroid::fragment::Fragment* f){
    static printerdemo::PrinterViewModelFactory sFactory;
    auto* act = dynamic_cast<cdroid::fragment::FragmentActivity*>(f->getActivity());
    if(!act) return nullptr;
    cdroid::lifecycle::ViewModelProvider provider(&act->getViewModelStore(), &sFactory, nullptr);
    return provider.get<printerdemo::PrinterViewModel>("PrinterViewModel");
}

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

// Persisted theme choice — re-read by every new window (AOSP recreate +
// onCreate re-reads the persisted selection).
static bool sDarkTheme = false;
// Persisted locale choice (zh-CN default, matching the pre-locale-switch UI).
// Single source of truth: the settings language picker and the toolbar quick
// toggle both write the tag.
static std::string sLocaleTag = "zh-CN";

// AOSP locale switch: route a locale Configuration change through the "system"
// — the arsc language/region gets repacked (values-<locale> variants reselect)
// and the activity recreates under the new locale (Window::recreate posts the
// teardown, so this is safe from inside any menu/click dispatch).
static void applyLocale(const std::string& tag){
    if(tag == sLocaleTag) return;
    sLocaleTag = tag;
    cdroid::Configuration c = cdroid::App::getInstance().getResources().getConfiguration();
    c.setLocales(cdroid::LocaleList(std::vector<cdroid::Locale>{
            cdroid::Locale::forLanguageTag(tag)}));
    cdroid::App::getInstance().handleConfigurationChanged(c);
}

// ---------------------------------------------------------------------------
class HomeFragment : public cdroid::fragment::Fragment{
    cdroid::ValueAnimator* mHeroAnim = nullptr;
public:
    void onCreate(cdroid::Bundle* savedInstanceState) override{
        cdroid::fragment::Fragment::onCreate(savedInstanceState);
        setEnterTransition(new cdroid::Slide(cdroid::Gravity::START));
        setExitTransition(new cdroid::Slide(cdroid::Gravity::START));
    }
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle*) override{
        return inflater->inflate(printerdemo::R::layout::fragment_home, container, false);
    }
    void onViewCreated(cdroid::View* view, cdroid::Bundle*) override{
        cdroid::fragment::Fragment::onViewCreated(view, nullptr);
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
        cdroid::fragment::Fragment::onDestroyView();
    }
};
REGISTER_FRAGMENT(HomeFragment);

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

// ---------------------------------------------------------------------------
class SettingsFragment : public cdroid::fragment::Fragment{
    // Owned here (not self-deleting in onDismiss — that would free the popup
    // while the dismiss notification is still unwinding through
    // MenuPopupHelper::onDismiss); reaped on view teardown, which the
    // locale-switch recreate posts, so it never lands on a callback stack.
    cdroid::PopupMenu* mLangMenu = nullptr;
public:
    void onCreate(cdroid::Bundle* savedInstanceState) override{
        cdroid::fragment::Fragment::onCreate(savedInstanceState);
        setEnterTransition(new cdroid::Slide(cdroid::Gravity::END));
        setExitTransition(new cdroid::Slide(cdroid::Gravity::END));
    }
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle*) override{
        return inflater->inflate(printerdemo::R::layout::fragment_settings, container, false);
    }
    void onDestroyView() override{
        delete mLangMenu;
        mLangMenu = nullptr;
        cdroid::fragment::Fragment::onDestroyView();
    }
    void onViewCreated(cdroid::View* view, cdroid::Bundle*) override{
        cdroid::fragment::Fragment::onViewCreated(view, nullptr);
        cdroid::SeekBar* seek = (cdroid::SeekBar*)view->findViewById(printerdemo::R::id::seek_brightness);
        cdroid::TextView* tv = (cdroid::TextView*)view->findViewById(printerdemo::R::id::tv_brightness);
        if(seek && tv){
            cdroid::SeekBar::OnSeekBarChangeListener l;
            l.onProgressChanged = [tv](cdroid::SeekBar&, int progress, bool){
                tv->setText(std::to_string(progress) + "%");
            };
            l.onStartTrackingTouch = [](cdroid::SeekBar&){};
            l.onStopTrackingTouch  = [](cdroid::SeekBar&){};
            seek->setOnSeekBarChangeListener(l);
        }
        // Language picker: the offered set is what the app's resources actually
        // carry — AssetManager.getNonSystemLocales() enumerates the app pak's
        // values-<locale> tables; the unqualified values/ base (stored in the
        // arsc without a locale tag) is the app's base language, en-US here.
        if(cdroid::View* row = view->findViewById(printerdemo::R::id::row_language)){
            row->setOnClickListener([this, row](cdroid::View& v){
                std::vector<std::string> tags{ "en-US" };   // the values/ base language
                for(const std::string& t : cdroid::App::getInstance().getAssets().getNonSystemLocales())
                    if(!t.empty() && std::find(tags.begin(), tags.end(), t) == tags.end())
                        tags.push_back(t);
                // Gravity.RIGHT aligns the popup's right edge with the row's
                // right edge (the only horizontal alignment PopupWindow
                // special-cases, same as AOSP): the menu drops below the row's
                // right end instead of the easily-missed far-left corner the
                // default bottom-left-of-anchor produces on a 1280px screen.
                delete mLangMenu;   // a previous popup may still be around
                mLangMenu = new cdroid::PopupMenu(v.getContext(), &v, cdroid::Gravity::RIGHT);
                cdroid::Menu* menu = mLangMenu->getMenu();
                for(size_t i = 0; i < tags.size(); i++){
                    const cdroid::Locale l = cdroid::Locale::forLanguageTag(tags[i]);
                    cdroid::MenuItem* mi = menu->add(cdroid::Menu::NONE, (int)i, (int)i,
                            l.getDisplayName(l));   // self-name, the picker convention
                    mi->setCheckable(true);
                    mi->setChecked(tags[i] == sLocaleTag);
                }
                mLangMenu->setOnMenuItemClickListener([tags](cdroid::MenuItem& item){
                    applyLocale(tags[item.getItemId()]);
                    return true;
                });
                mLangMenu->show();
            });
        }
    }
};
REGISTER_FRAGMENT(SettingsFragment);

// ---------------------------------------------------------------------------
class AboutFragment : public cdroid::fragment::Fragment{
public:
    void onCreate(cdroid::Bundle* savedInstanceState) override{
        cdroid::fragment::Fragment::onCreate(savedInstanceState);
        setEnterTransition(new cdroid::Slide(cdroid::Gravity::END));
        setExitTransition(new cdroid::Slide(cdroid::Gravity::END));
    }
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle*) override{
        return inflater->inflate(printerdemo::R::layout::fragment_about, container, false);
    }
    void onViewCreated(cdroid::View* view, cdroid::Bundle*) override{
        cdroid::fragment::Fragment::onViewCreated(view, nullptr);
        // Rich text via Html.fromHtml (bold headers, brand-colored bullets/emphasis).
        auto set = [view](int id, const std::string& html){
            cdroid::TextView* tv = (cdroid::TextView*)view->findViewById(id);
            if(tv) tv->setText(cdroid::Html::fromHtml(html));
        };
        // Subtitle carries the real framework version (core/build.h), not a
        // hardcoded app version — the demo ships against whatever CDROID builds.
        if(cdroid::TextView* sub = (cdroid::TextView*)view->findViewById(printerdemo::R::id::about_subtitle))
            sub->setText(std::string("CDroid ") + cdroid::Build::VERSION::RELEASE
                         + " · " + (sLocaleTag.compare(0, 2, "zh") == 0 ? "逐行 C++ 移植 · Android UI"
                                                    : "Line-by-line C++ port · Android UI"));
        set(printerdemo::R::id::about_intro,
            "<big><b><font color='#FF4A90E2'>CDroid<sup>™</sup></font></b></big> 是 <b>Android UI 框架</b>"
            "（android.widget / view / text / drawable / animation）的<b>逐行 C++ 移植</b>，"
            "构建于 <i><b>Cairo</b></i> 矢量图形之上，面向<b>嵌入式设备</b>（最低 <tt>32M</tt> 内存即可运行）。"
            "<br/><br/><blockquote><i>「别再 <s>重造 UI 轮子</s>——把 Android 的真东西移植过来。」</i></blockquote>"
            "<br/><br/>类名、方法签名与控制流<b>紧跟 AOSP</b>——可把 Android 参考源码与 C++ 实现并排逐行对照。"
            "在 Android Studio 设计 XML 布局，<u>直接由 Cairo 渲染</u>，<b>全程无需 JVM</b>。");
        set(printerdemo::R::id::about_arch,
            "<b><font color='#FF4A90E2'>Canvas 即 cairo_t</font></b> —— <small>无独立渲染抽象，也无 Bitmap 类（由 ImageSurface 承担）</small>，<tt>onDraw</tt> 直接编程 cairo。"
            "<br/><b><font color='#FF4A90E2'>App = Context/Assets</font></b> —— <small>一套对象回答 <tt>getString</tt> / <tt>getDrawable</tt> / <tt>loadImage</tt>。</small>"
            "<br/><b><font color='#FF4A90E2'>Looper / Choreographer 原样移植</font></b> —— <small>epoll + eventFd 驱动主线程；无硬件 VSYNC 时自节拍。</small>"
            "<br/><b><font color='#FF4A90E2'>脏区 + blit 合成器</font></b> —— <small>按需重绘，非全屏刷新；像素格式 <tt>ARGB<sub>32</sub></tt>。</small>"
            "<br/><b><font color='#FF4A90E2'>多后端</font></b> —— <small>DRM / fb / SDL / XCB / VNC，一套 GUI 跨平台。</small>");
        set(printerdemo::R::id::about_features,
            "<b>• 50+ 控件 · 20+ Drawable</b> <small>API 兼容 Android</small><br/>"
            "<b>• Fragment + Navigation</b> <small>含 saveState / restoreState</small><br/>"
            "<b>• ConstraintLayout + MotionLayout</b> <small>MotionScene / keyframe</small><br/>"
            "<b>• RecyclerView</b> <small>AndroidX 1:1</small><br/>"
            "<b>• Transition 转场</b> <small>Fade / Slide / ChangeBounds</small><br/>"
            "<b>• 文本栈</b> <small>Spans / StaticLayout / minikin</small><br/>"
            "<b>• 输入</b> <small>KeyCharacterMap / 输入法 / 多点触控</small>");
        set(printerdemo::R::id::about_usecases,
            "<b>• 嵌入式 HMI</b> <small>车机 / 机顶盒 / 工业面板——跑不动完整 Android 时的 Android 级 UI。</small><br/>"
            "<b>• 学习 Android 内部</b> <small>最易读的 AOSP framework 镜像，读 C++ 比啃 AOSP 编译通透。</small><br/>"
            "<b>• C++ 实战</b> <small>真实的所有权 / 生命周期挑战，配现代特性（动画 / 转场 / Material）。</small>"
            "<br/><br/><small>详情与源码：</small> <a href='https://gitee.com/houstudio/Cdroid'>https://gitee.com/houstudio/Cdroid</a>");
    }
};
REGISTER_FRAGMENT(AboutFragment);

// ---------------------------------------------------------------------------
class PrinterDemoWindow : public cdroid::fragment::FragmentActivity{
    cdroid::NavHostFragment* mNavHost = nullptr;
    cdroid::Toolbar* mToolbar = nullptr;
    cdroid::BottomNavigationView* mBottomNavigation = nullptr;
    bool mInited = false;
public:
    PrinterDemoWindow() : FragmentActivity(0, 0, -1, -1){
        cdroid::ViewGroup* root = (cdroid::ViewGroup*)cdroid::LayoutInflater::from(getContext())
            ->inflate(printerdemo::R::layout::main, this, false);
        addView(root);
        // The window surface is transparent by default — any region not covered by an opaque
        // child (e.g. a strip left after a full-screen overlay is hidden, before it repaints)
        // shows black. Give the Window itself an opaque background so uncovered areas are bg_screen.
        setBackground(getContext()->getDrawable(printerdemo::R::drawable::bg_screen));
        mToolbar = (cdroid::Toolbar*)root->findViewById(printerdemo::R::id::toolbar);
        mBottomNavigation = (cdroid::BottomNavigationView*)root->findViewById(
            printerdemo::R::id::bottom_navigation);
    }
    void onCreate(cdroid::Bundle* savedInstanceState) override{
        FragmentActivity::onCreate(savedInstanceState);
        mNavHost = new cdroid::NavHostFragment(printerdemo::R::navigation::nav_graph);
        getSupportFragmentManager()->beginTransaction()
            ->replace(printerdemo::R::id::nav_host_container, mNavHost)
            .commit();
    }
    void onActive() override{
        cdroid::fragment::FragmentActivity::onActive();
        if(mInited) return;
        mInited = true;
        if(!mToolbar || !mNavHost) return;
        cdroid::NavController* nc = mNavHost->getNavController();
        if(!nc) return;
        // NavigationUI owns the destination title, Up behavior, and navigation click. The sample
        // keeps only its app-local icons as a visual fallback because the framework indicator may
        // be absent in the embedded resource pack.
        cdroid::NavigationUI::setupWithNavController(mToolbar, nc);
        if (mBottomNavigation != nullptr) {
            getMenuInflater()->inflate(printerdemo::R::menu::bottom_navigation,
                mBottomNavigation->getMenu());
                mBottomNavigation->refreshMenuView();
            cdroid::NavigationUI::setupWithNavController(mBottomNavigation, nc);
        }

        nc->addOnDestinationChangedListener([this](cdroid::NavController&,
                                                    cdroid::NavDestination& d, cdroid::Bundle*){
            if(!mToolbar) return;
            const bool isHome = (d.getRoute() == "home");
            // Home screen: a HOME icon (branding, non-clickable). Sub-pages: the back arrow.
            mToolbar->setNavigationIcon(getContext()->getDrawable(isHome ? printerdemo::R::drawable::ic_home : printerdemo::R::drawable::ic_back));
            // The nav button (ImageButton) inherits a default button background — override it with
            // an explicitly transparent one so only the icon shows on the gradient toolbar.
            if(cdroid::View* nav = mToolbar->getNavigationView())
                nav->setBackground(new cdroid::ColorDrawable(0));
        });

        // Options menu: "关于 CDroid" overflow item -> intro dialog (the Toolbar's own menu; no
        // ActionBar, so it coexists with the hand-driven nav icon above).
        // Options menu: "关于 CDroid" overflow item -> intro dialog. Toolbar::inflateMenu is a
        // declared-but-undefined stub, so populate the Toolbar's own menu via MenuInflater.
        if(cdroid::Menu* menu = mToolbar->getMenu())
            getMenuInflater()->inflate(printerdemo::R::menu::main, menu);
        mToolbar->setOnMenuItemClickListener([this, nc](cdroid::MenuItem& item)->bool{
            if(item.getItemId() == printerdemo::R::id::action_toggle_theme){
                // AOSP dynamic theming: flip the persisted app-level choice,
                // apply it app-wide, and relaunch this activity so the new
                // instance inflates under it (already-inflated views are never
                // re-themed in place).
                sDarkTheme = !sDarkTheme;
                cdroid::App::getInstance().setTheme(sDarkTheme
                        ? printerdemo::R::style::AppTheme_Dark
                        : printerdemo::R::style::AppTheme);
                recreate();
                return true;
            }
            if(item.getItemId() == printerdemo::R::id::action_toggle_language){
                // Quick toggle between the app's two shipped locales; the
                // settings page offers the resource-driven picker.
                applyLocale(sLocaleTag.compare(0, 2, "zh") == 0 ? "en-US" : "zh-CN");
                return true;
            }
            if(item.getItemId() == printerdemo::R::id::action_about){
                if(nc) nc->navigate("about");   // "关于 CDroid" is its own Fragment destination
                return true;
            }
            return false;
        });
    }
};

REGISTER_ACTIVITY(PrinterDemoWindow);

int main(int argc, const char* argv[]){
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    cdroid::App app(argc, argv);
    // The application theme comes from AndroidManifest.xml (application
    // android:theme="@style/AppTheme"). A persisted dark choice overrides it
    // app-wide before any window is created (AOSP: the stored
    // android:isUiEnabled/night mode is applied at process start).
    if(sDarkTheme) app.setTheme(printerdemo::R::style::AppTheme_Dark);
    // Seed the live Configuration with the startup locale so the resource
    // layer selects the right variants from the start; a language switch later
    // flips this via applyLocale() -> handleConfigurationChanged (recreate).
    {
        cdroid::Configuration c = app.getResources().getConfiguration();
        c.setLocales(cdroid::LocaleList(std::vector<cdroid::Locale>{
                cdroid::Locale::forLanguageTag(sLocaleTag)}));
        app.handleConfigurationChanged(c);   // no windows yet — resources only
    }
    // The launcher activity starts itself: exec() launches the manifest's
    // MAIN/LAUNCHER window when no window is up (App plays the system side).
    return app.exec();
}
