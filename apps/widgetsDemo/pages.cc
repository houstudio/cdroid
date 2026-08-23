#include "pages.h"
#include <R.h>
#include <widget/R.h>
#include <core/systemclock.h>
#include <text/String.h>
#include <text/spannablestringbuilder.h>
#include <text/style/characterstyles.h>
#include <text/style/metricaffectingspan.h>
#include <widget/ratingbar.h>
#include <widget/chronometer.h>
#include <widget/timepicker.h>
#include <widget/datepicker.h>
#include <widget/textclock.h>
#include <widget/calendarview.h>
#include <widget/spinner.h>
#include <widget/edittext.h>
#include <widget/imageview.h>
#include <drawable/vectordrawable.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/itemtouchhelper.h>
#include <widgetEx/constraintlayout/helpers/carousel.h>
#include <widget/numberpicker.h>
#include <widget/framelayout.h>
#include <widgetEx/qrcodeview.h>
#include <widget/achart/chartfactory.h>
#include <widget/achart/model/xymultipleseriesdataset.h>
#include <widget/achart/model/xyseries.h>
#include <widget/achart/renderer/xymultipleseriesrenderer.h>
#include <widget/achart/renderer/xyseriesrenderer.h>
#include <app/alertdialog.h>
#include <widget/toast.h>
#include <memory>
#include <string>
#include <widgetEx/constraintlayout/constraintlayout.h>
#include <widgetEx/constraintlayout/constraintset.h>
#include <widgetEx/constraintlayout/helpers/barrier.h>
#include <widgetEx/constraintlayout/helpers/flow.h>
#include <widgetEx/constraintlayout/motion/motionlayout.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>

using namespace cdroid;

namespace {
void setResult(TextView* tv, const std::string& msg) {
    if (tv) tv->setText(msg);
}

// Minimal Spinner adapter: one styled TextView row per string.
class StringSpinnerAdapter : public ArrayAdapter<std::string> {
public:
    View* getView(int position, View* convertView, ViewGroup* parent) override {
        return getDropDownView(position, convertView, parent);
    }
    View* getDropDownView(int position, View* convertView, ViewGroup* parent) override {
        TextView* tv = (TextView*)convertView;
        if (tv == nullptr) {
            tv = new TextView(parent->getContext());
            tv->setFocusable(false);
        }
        tv->setText(getItemAt(position));
        tv->setTextColor(0xFFECEFF2);
        tv->setTextSize(18);
        tv->setGravity(Gravity::CENTER_VERTICAL);
        tv->setPadding(16, 0, 16, 0);
        return tv;
    }
};

// RecyclerView adapter backed by a mutable string list. Each row is a rounded
// card with an accent bar and a label; supports remove/move for ItemTouchHelper.
class DemoListAdapter : public RecyclerView::Adapter {
private:
    std::vector<std::string> mItems;
    static constexpr int kColors[4] = {(int)0xFF4EA1FF, (int)0xFF7C5CFF, (int)0xFF22C1A6, (int)0xFFF5A623};
public:
    class VH : public RecyclerView::ViewHolder {
    public:
        View* bar;
        TextView* label;
        VH(View* row, View* bar, TextView* label)
            : RecyclerView::ViewHolder(row), bar(bar), label(label) {}
    };
    void add(const std::string& s) { mItems.push_back(s); }
    void remove(int idx) {
        if (idx < 0 || idx >= (int)mItems.size()) return;
        mItems.erase(mItems.begin() + idx);
        notifyItemRemoved(idx);
    }
    void move(int from, int to) {
        if (from < 0 || to < 0 || from >= (int)mItems.size() || to >= (int)mItems.size()) return;
        std::swap(mItems[from], mItems[to]);
        notifyItemMoved(from, to);
    }
    int getItemCount() override { return (int)mItems.size(); }
    RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) override {
        LinearLayout* row = new LinearLayout(parent->getContext());
        row->setOrientation(LinearLayout::HORIZONTAL);
        row->setLayoutParams(new RecyclerView::LayoutParams(RecyclerView::LayoutParams::MATCH_PARENT, 56));
        row->setGravity(Gravity::CENTER_VERTICAL);
        row->setBackgroundColor(0xFF1E2634);
        row->setPadding(0, 0, 12, 0);
        View* bar = new View(parent->getContext());
        bar->setLayoutParams(new LinearLayout::LayoutParams(6, LinearLayout::LayoutParams::MATCH_PARENT));
        row->addView(bar);
        TextView* label = new TextView(parent->getContext());
        LinearLayout::LayoutParams* lp = new LinearLayout::LayoutParams(0, LinearLayout::LayoutParams::MATCH_PARENT);
        lp->weight = 1;
        lp->setMarginsRelative(14, 0, 0, 0);
        label->setLayoutParams(lp);
        label->setGravity(Gravity::CENTER_VERTICAL);
        label->setTextColor(0xFFECEFF2);
        label->setTextSize(16);
        row->addView(label);
        return new VH(row, bar, label);
    }
    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position) override {
        VH& vh = (VH&)holder;
        vh.bar->setBackgroundColor(kColors[position % 4]);
        vh.label->setText(mItems.at(position));
    }
};
constexpr int DemoListAdapter::kColors[4];

class ListTouchCallback : public ItemTouchHelper::SimpleCallback {
private:
    DemoListAdapter* mAdapter;
public:
    ListTouchCallback(DemoListAdapter* a)
        : ItemTouchHelper::SimpleCallback(ItemTouchHelper::UP | ItemTouchHelper::DOWN,
                                          ItemTouchHelper::LEFT | ItemTouchHelper::RIGHT),
          mAdapter(a) {}
    bool onMove(RecyclerView& rv, RecyclerView::ViewHolder& vh, RecyclerView::ViewHolder& target) override {
        mAdapter->move(vh.getAbsoluteAdapterPosition(), target.getAbsoluteAdapterPosition());
        return true;
    }
    void onSwiped(RecyclerView::ViewHolder& vh, int direction) override {
        mAdapter->remove(vh.getAbsoluteAdapterPosition());
    }
};
} // namespace

void setupButtons(View* page) {
    TextView* result = (TextView*)page->findViewById(widgetsDemo::R::id::result_tv);

    Button* btn = (Button*)page->findViewById(widgetsDemo::R::id::btn_default);
    if (btn) btn->setOnClickListener([result](View&) { setResult(result, "Primary button clicked"); });

    btn = (Button*)page->findViewById(widgetsDemo::R::id::btn_outline);
    if (btn) btn->setOnClickListener([result](View&) { setResult(result, "Outline button clicked"); });

    btn = (Button*)page->findViewById(widgetsDemo::R::id::btn_ripple);
    if (btn) btn->setOnClickListener([result](View&) { setResult(result, "Ripple button clicked"); });

    ToggleButton* toggle = (ToggleButton*)page->findViewById(widgetsDemo::R::id::toggle);
    if (toggle) toggle->setOnCheckedChangeListener([result](CompoundButton&, bool c) {
        setResult(result, c ? "Toggle: ON" : "Toggle: OFF");
    });

    Switch* sw = (Switch*)page->findViewById(widgetsDemo::R::id::switch1);
    if (sw) sw->setOnCheckedChangeListener([result](CompoundButton&, bool c) {
        setResult(result, c ? "Switch: ON" : "Switch: OFF");
    });

    CheckBox* chk = (CheckBox*)page->findViewById(widgetsDemo::R::id::checkbox);
    if (chk) chk->setOnCheckedChangeListener([result](CompoundButton&, bool c) {
        setResult(result, c ? "CheckBox: checked" : "CheckBox: unchecked");
    });

    RadioGroup* rg = (RadioGroup*)page->findViewById(widgetsDemo::R::id::radiogroup);
    if (rg) rg->setOnCheckedChangeListener([result](CompoundButton& v, bool c) {
        if (c) setResult(result, "Radio: " + ((RadioButton*)&v)->getText().toString()->toUTF8());
    });

    RatingBar* rating = (RatingBar*)page->findViewById(widgetsDemo::R::id::rating);
    if (rating) rating->setOnRatingBarChangeListener([result](RatingBar&, float r, bool) {
        setResult(result, "Rating: " + std::to_string((int)r) + " star(s)");
    });
}

void setupProgress(View* page) {
    App& app = App::getInstance();

    ProgressBar* ph = (ProgressBar*)page->findViewById(widgetsDemo::R::id::progress_h);
    if (ph) {
        Drawable* pd = app.getDrawable(cdroid::R::drawable::progress_horizontal);
        if (pd) ph->setProgressDrawable(pd);
        ph->setMax(100);
        ph->setProgress(40);
        ph->setSecondaryProgress(60);
    }

    SeekBar* sb = (SeekBar*)page->findViewById(widgetsDemo::R::id::seekbar);
    if (sb) {
        Drawable* pd = app.getDrawable(cdroid::R::drawable::progress_horizontal);
        if (pd) sb->setProgressDrawable(pd);
        //Drawable* thumb = app.getDrawable(cdroid::R::drawable::seek_thumb);
        //if (thumb) sb->setThumb(thumb);
        sb->setMax(100);
        sb->setProgress(40);
        SeekBar::OnSeekBarChangeListener l;
        l.onProgressChanged = [ph](SeekBar&, int progress, bool) {
            if (ph) ph->setProgress(progress);
        };
        sb->setOnSeekBarChangeListener(l);
    }

    ProgressBar* spinner = (ProgressBar*)page->findViewById(widgetsDemo::R::id::spinner);
    if (spinner) {
        //Drawable* ind = app.getDrawable(cdroid::R::drawable::progress_large);
        //if (ind) spinner->setIndeterminateDrawable(ind);
        spinner->setIndeterminate(true);
    }

    Chronometer* chrono = (Chronometer*)page->findViewById(widgetsDemo::R::id::chrono);
    Button* ctoggle = (Button*)page->findViewById(widgetsDemo::R::id::chrono_toggle);
    if (chrono && ctoggle) {
        chrono->setFormat("Elapsed: %s");
        auto running = std::make_shared<bool>(false);
        ctoggle->setOnClickListener([chrono, ctoggle, running](View&) {
            if (*running) {
                chrono->stop();
                ctoggle->setText("Start");
            } else {
                chrono->setBase(SystemClock::uptimeMillis());
                chrono->start();
                ctoggle->setText("Stop");
            }
            *running = !*running;
        });
    }

    RatingBar* rating2 = (RatingBar*)page->findViewById(widgetsDemo::R::id::rating2);
    if (rating2) {
        rating2->setNumStars(5);
        rating2->setRating(3);
    }
}

// Rich spans, marquee, EditText input types and a Spinner.
void setupText(View* page) {
    TextView* span = (TextView*)page->findViewById(widgetsDemo::R::id::span_tv);
    if (span) {
        SpannableStringBuilder* sb = new SpannableStringBuilder();
        sb->append(u"Bold ", new StyleSpan(Typeface::BOLD), 0);
        sb->append(u"colored ", new ForegroundColorSpan(0xFF4EA1FF), 0);
        sb->append(u"underline ", new UnderlineSpan(), 0);
        sb->append(u"strike ", new StrikethroughSpan(), 0);
        sb->append(u"big ", new AbsoluteSizeSpan(28), 0);
        sb->append(u"x", 0, 1);
        sb->append(u"2", new SuperscriptSpan(), 0);
        sb->append(u" highlight", new BackgroundColorSpan(0xFF7C5CFF), 0);
        span->setText(sb);
    }

    // Marquee only animates on a selected/focused view.
    TextView* marquee = (TextView*)page->findViewById(widgetsDemo::R::id::marquee_tv);
    if (marquee) marquee->setSelected(true);
    if (TextView* marquee2 = (TextView*)page->findViewById(widgetsDemo::R::id::marquee2_tv))
        marquee2->setSelected(true);

    Spinner* spinner = (Spinner*)page->findViewById(widgetsDemo::R::id::spinner1);
    if (spinner && spinner->getAdapter() == nullptr) {
        StringSpinnerAdapter* adapter = new StringSpinnerAdapter();
        adapter->add("Low");
        adapter->add("Medium");
        adapter->add("High");
        adapter->add("Turbo");
        spinner->setAdapter(adapter);
        spinner->setSelection(1);
    }
}
// ImageView scale types, gradient drawables and a tap-to-animate vector.
void setupImages(View* page) {
    ImageView* avd = (ImageView*)page->findViewById(widgetsDemo::R::id::avd_img);
    if (avd) {
        Drawable* d = avd->getDrawable();
        // Kick off the initial animation, then toggle state on each tap.
        if (dynamic_cast<AnimatedVectorDrawable*>(d)) {
            ((AnimatedVectorDrawable*)d)->start();
        }
        auto checked = std::make_shared<bool>(false);
        avd->setOnClickListener([checked](View& v) {
            Drawable* dr = ((ImageView&)v).getDrawable();
            if (!dr) return;
            *checked = !*checked;
            dr->setState(*checked ? StateSet::CHECKED_STATE_SET : StateSet::NOTHING);
            if (dynamic_cast<AnimatedVectorDrawable*>(dr)) {
                ((AnimatedVectorDrawable*)dr)->start();
            }
        });
    }
}
void setupAnimation(View* page) {
    View* target = page->findViewById(widgetsDemo::R::id::anim_target);
    if (!target) return;

    Button* b = (Button*)page->findViewById(widgetsDemo::R::id::anim_rotate);
    if (b) b->setOnClickListener([target](View&) {
        target->animate().setDuration(600).rotationBy(360).start();
    });

    b = (Button*)page->findViewById(widgetsDemo::R::id::anim_scale);
    if (b) b->setOnClickListener([target](View&) {
        target->setScaleX(1.f); target->setScaleY(1.f);
        target->animate().setDuration(300).scaleX(1.6f).scaleY(1.6f)
            .withEndAction([target] {
                target->animate().setDuration(300).scaleX(1.f).scaleY(1.f).start();
            }).start();
    });

    b = (Button*)page->findViewById(widgetsDemo::R::id::anim_move);
    if (b) b->setOnClickListener([target](View&) {
        target->animate().setDuration(400).translationXBy(120).start();
    });

    b = (Button*)page->findViewById(widgetsDemo::R::id::anim_fade);
    if (b) b->setOnClickListener([target](View&) {
        target->animate().setDuration(300).alpha(0.15f)
            .withEndAction([target] {
                target->animate().setDuration(300).alpha(1.f).start();
            }).start();
    });

    b = (Button*)page->findViewById(widgetsDemo::R::id::anim_combo);
    if (b) b->setOnClickListener([target](View&) {
        target->animate().setDuration(700).rotationBy(360)
            .scaleX(1.4f).scaleY(1.4f).alpha(0.5f)
            .withEndAction([target] {
                target->animate().setDuration(400).scaleX(1.f).scaleY(1.f).alpha(1.f).start();
            }).start();
    });

    b = (Button*)page->findViewById(widgetsDemo::R::id::anim_reset);
    if (b) b->setOnClickListener([target](View&) {
        target->animate().setDuration(300)
            .translationX(0).rotation(0).scaleX(1.f).scaleY(1.f).alpha(1.f).start();
    });
}
void setupLists(View* page) {
    RecyclerView* rv = (RecyclerView*)page->findViewById(widgetsDemo::R::id::list_rv);
    if (!rv || rv->getAdapter() != nullptr) return;

    rv->setLayoutManager(std::make_unique<LinearLayoutManager>(&App::getInstance()));

    DemoListAdapter* adapter = new DemoListAdapter();
    static const char* names[] = {
        "Espresso", "Cappuccino", "Latte", "Americano", "Mocha",
        "Macchiato", "Flat White", "Cortado", "Ristretto", "Affogato"
    };
    for (const char* n : names) adapter->add(n);
    rv->setAdapter(adapter);

    ListTouchCallback* cb = new ListTouchCallback(adapter);
    ItemTouchHelper* helper = new ItemTouchHelper(cb);
    helper->attachToRecyclerView(rv);
}
void setupMisc(View* page) {
    // NumberPickers: temperature (16..30) and fan speed (0..5).
    NumberPicker* npTemp = (NumberPicker*)page->findViewById(widgetsDemo::R::id::np_temp);
    NumberPicker* npSpeed = (NumberPicker*)page->findViewById(widgetsDemo::R::id::np_speed);
    TextView* valueTv = (TextView*)page->findViewById(widgetsDemo::R::id::np_value_tv);
    if (npTemp) {
        npTemp->setMinValue(16); npTemp->setMaxValue(30); npTemp->setValue(22);
    }
    if (npSpeed) {
        npSpeed->setMinValue(0); npSpeed->setMaxValue(5); npSpeed->setValue(2);
    }
    auto refresh = [npTemp, npSpeed, valueTv]() {
        if (!valueTv) return;
        int t = npTemp ? npTemp->getValue() : 22;
        int s = npSpeed ? npSpeed->getValue() : 2;
        valueTv->setText(std::to_string(t) + u8"°C · fan " + std::to_string(s));
    };
    if (npTemp) {
        npTemp->setOnValueChangedListener([refresh](NumberPicker&, int, int) { refresh(); });
    }
    if (npSpeed) {
        npSpeed->setOnValueChangedListener([refresh](NumberPicker&, int, int) { refresh(); });
    }

    // AChart line chart — two sine/cosine series rendered into a FrameLayout.
    FrameLayout* container = (FrameLayout*)page->findViewById(widgetsDemo::R::id::chart_container);
    if (container && container->getChildCount() == 0) {
        auto dataset = std::make_shared<XYMultipleSeriesDataset>();
        auto sinSeries = std::make_shared<XYSeries>("sin");
        auto cosSeries = std::make_shared<XYSeries>("cos");
        for (float t = 0.0f; t <= 6.3f; t += 0.1f) {
            sinSeries->add(t, std::sin(t));
            cosSeries->add(t, std::cos(t));
        }
        dataset->addSeries(sinSeries); dataset->addSeries(cosSeries);

        auto renderer = std::make_shared<XYMultipleSeriesRenderer>();
        renderer->setChartTitle("sin & cos");
        renderer->setChartTitleTextSize(14);
        renderer->setXTitle("x"); renderer->setYTitle("y");
        renderer->setAxisTitleTextSize(11);
        renderer->setLabelsTextSize(10);
        renderer->setLegendTextSize(11);
        renderer->setXAxisMin(0); renderer->setXAxisMax(6.3);
        renderer->setYAxisMin(-1.2); renderer->setYAxisMax(1.2);
        renderer->setShowGrid(true);
        renderer->setGridColor(0x33FFFFFF);
        renderer->setAxesColor(0x88FFFFFF);
        renderer->setLabelsColor(0xFFECEFF2);
        renderer->setBackgroundColor(0xFF1E4E7A);
        renderer->setApplyBackgroundColor(true);
        renderer->setShowLegend(true);
        renderer->setPointSize(4);
        renderer->setMargins(std::vector<int>{12, 28, 12, 12});
        renderer->setAntialiasing(true);

        auto r1 = std::make_shared<XYSeriesRenderer>();
        r1->setColor(0xFF4EA1FF); r1->setLineWidth(3.f); r1->setPointSize(5);
        auto r2 = std::make_shared<XYSeriesRenderer>();
        r2->setColor(0xFFF5A623); r2->setLineWidth(3.f); r2->setPointSize(5);
        renderer->addSeriesRenderer(r1); renderer->addSeriesRenderer(r2);

        GraphicalView* chart = ChartFactory::getLineChartView(
            &App::getInstance(), dataset, renderer);
        if (chart) {
            ViewGroup* vg = (ViewGroup*)container;
            vg->addView(chart, new LinearLayout::LayoutParams(
                LinearLayout::LayoutParams::MATCH_PARENT,
                LinearLayout::LayoutParams::MATCH_PARENT));
        }
    }

    // QR code.
    QRCodeView* qr = (QRCodeView*)page->findViewById(widgetsDemo::R::id::qr);
    if (qr) qr->setText("https://github.com/cdroid/cdroid");

    // Feedback buttons.
    Button* toastBtn = (Button*)page->findViewById(widgetsDemo::R::id::btn_toast);
    if (toastBtn) toastBtn->setOnClickListener([](View&) {
        Toast::makeText(&App::getInstance(), "cdroid — hello toast!", 2000)->show();
    });
    Button* dialogBtn = (Button*)page->findViewById(widgetsDemo::R::id::btn_dialog);
    if (dialogBtn) dialogBtn->setOnClickListener([](View&) {
        auto f = [](DialogInterface&, int) {};
        AlertDialog::Builder(&App::getInstance())
            .setTitle("cdroid")
            .setMessage("This is a modal AlertDialog built from AlertDialog::Builder.")
            .setPositiveButton("OK", f)
            .setNegativeButton("Cancel", f)
            .show();
    });
}

void setupDateTime(View* page) {
    TimePicker*   tpClock = (TimePicker*)page->findViewById(widgetsDemo::R::id::dt_tp_clock);
    TimePicker*   tpSpin  = (TimePicker*)page->findViewById(widgetsDemo::R::id::dt_tp_spinner);
    DatePicker*   dp      = (DatePicker*)page->findViewById(widgetsDemo::R::id::dt_dp);
    DatePicker*   dpCal   = (DatePicker*)page->findViewById(widgetsDemo::R::id::dt_dp_cal);
    CalendarView* calView = (CalendarView*)page->findViewById(widgetsDemo::R::id::dt_calview);
    CalendarView* calViewMat = (CalendarView*)page->findViewById(widgetsDemo::R::id::dt_calview_mat);
    // Each picker mirrors its live value into the TextView that shares its row.
    TextView*     tvClock = (TextView*)page->findViewById(widgetsDemo::R::id::dt_tp_clock_val);
    TextView*     tvSpin  = (TextView*)page->findViewById(widgetsDemo::R::id::dt_tp_spinner_val);
    TextView*     tvDp    = (TextView*)page->findViewById(widgetsDemo::R::id::dt_dp_val);
    TextView*     tvDpCal = (TextView*)page->findViewById(widgetsDemo::R::id::dt_dp_cal_val);
    TextView*     tvCal   = (TextView*)page->findViewById(widgetsDemo::R::id::dt_calview_val);
    TextView*     tvCalMat= (TextView*)page->findViewById(widgetsDemo::R::id::dt_calview_mat_val);

    auto two = [](int v) {
        std::string s = std::to_string(v < 0 ? 0 : v);
        while (s.length() < 2) s = "0" + s;
        return s;
    };
    auto fmtDate = [two](DatePicker* d) -> std::string {
        if (!d) return "";
        return std::to_string(d->getYear()) + "-" + two(d->getMonth() + 1) + "-" + two(d->getDayOfMonth());
    };

    if (tpClock) {
        auto update = [tpClock, tvClock, two]() {
            if (tvClock) tvClock->setText(two(tpClock->getHour()) + ":" + two(tpClock->getMinute()));
        };
        update();
        tpClock->setOnTimeChangedListener([update](TimePicker&, int, int) { update(); });
    }
    if (tpSpin) {
        auto update = [tpSpin, tvSpin, two]() {
            if (tvSpin) tvSpin->setText(two(tpSpin->getHour()) + ":" + two(tpSpin->getMinute()));
        };
        update();
        tpSpin->setOnTimeChangedListener([update](TimePicker&, int, int) { update(); });
    }
    if (dp) {
        auto update = [dp, tvDp, fmtDate]() { if (tvDp) tvDp->setText(fmtDate(dp)); };
        update();
        dp->setOnDateChangedListener([update](DatePicker&, int, int, int) { update(); });
    }
    if (dpCal) {
        auto update = [dpCal, tvDpCal, fmtDate]() { if (tvDpCal) tvDpCal->setText(fmtDate(dpCal)); };
        update();
        dpCal->setOnDateChangedListener([update](DatePicker&, int, int, int) { update(); });
    }
    if (calView) {
        auto setCalVal = [tvCal, two](int y, int m, int d) {
            if (tvCal) tvCal->setText(std::to_string(y) + "-" + two(m + 1) + "-" + two(d));
        };
        calView->setOnDateChangeListener([setCalVal](CalendarView&, int y, int m, int d) {
            setCalVal(y, m, d);
        });
        // Show the current selection immediately (matches the other pickers).
        Calendar c;
        c.setTimeInMillis(calView->getDate());
        setCalVal(c.get(Calendar::YEAR), c.get(Calendar::MONTH), c.get(Calendar::DAY_OF_MONTH));
    }
    if (calViewMat) {
        auto setMatVal = [tvCalMat, two](int y, int m, int d) {
            if (tvCalMat) tvCalMat->setText(std::to_string(y) + "-" + two(m + 1) + "-" + two(d));
        };
        calViewMat->setOnDateChangeListener([setMatVal](CalendarView&, int y, int m, int d) {
            setMatVal(y, m, d);
        });
        // Show the current selection immediately — and observe whether a tap
        // reports the tapped date or jumps to 1900 like HOLO does.
        Calendar c;
        c.setTimeInMillis(calViewMat->getDate());
        setMatVal(c.get(Calendar::YEAR), c.get(Calendar::MONTH), c.get(Calendar::DAY_OF_MONTH));
    }
}

// ============================================================================
// ConstraintLayout + MotionLayout page — XML inflation + programmatic motion wiring
// ============================================================================
void setupConstraint(View* page) {
    // Fully XML-driven now:
    //  - ConstraintLayout solves from the layout_constraint* attributes in page_constraint.xml.
    //  - Flow/Barrier resolve their constraint_referenced_ids (bare names) at inflate time via
    //    ConstraintHelper → Context::getId (Android's getIdentifier(name,"id",pkg) equivalent).
    //  - MotionLayout reads its <MotionScene> via app:layoutDescription="@xml/scene_constraint".
    // Nothing left to wire here.
    (void) page;
}

// ============================================================================
// MotionLayout + Carousel page — supply a Carousel.Adapter (everything else is XML-driven).
// ============================================================================
namespace {
constexpr uint32_t kCarouselColors[] = {0xFFEF5350, 0xFF66BB6A, 0xFF42A5F5, 0xFFFFCA28, 0xFFAB47BC};
class DemoCarouselAdapter : public Carousel::Adapter {
  public:
    int count() override { return 10; }
    void populate(View* view, int index) override {
        auto* tv = dynamic_cast<TextView*>(view);
        if (tv == nullptr) return;
        tv->setText(std::to_string(index));
        tv->setBackgroundColor(kCarouselColors[index % (sizeof(kCarouselColors) / sizeof(kCarouselColors[0]))]);
    }
};
} // namespace

void setupMotion(View* page) {
    Carousel* carousel = (Carousel*)page->findViewById(widgetsDemo::R::id::carousel);
    if (carousel != nullptr) {
        carousel->setAdapter(new DemoCarouselAdapter());
    }
}
