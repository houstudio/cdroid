/* Demo: side-by-side comparison of the two Flexbox implementations in cdroid.
 *   Left  : FlexboxLayout                (ViewGroup based)
 *   Right : RecyclerView + FlexboxLayoutManager (RecyclerView based)
 * Both sides get the same items and the same flex settings, so you can visually
 * verify that flex-grow / wrap / align produce identical results. Items whose
 * index is a multiple of 3 have flexGrow = 1, so each row's free space is
 * distributed across them — this exercises the grow path that was previously
 * broken (FlexLine was copied by value). */
#include <cdroid.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/flexbox/flexboxlayout.h>
#include <widgetEx/flexbox/flexboxlayoutmanager.h>
#include <widgetEx/flexbox/aligndefs.h>
#include <memory>
#include <vector>
#include <string>

static const int PALETTE[] = {
    (int) 0xFFE57373, (int) 0xFFBA68C8, (int) 0xFF64B5F6,
    (int) 0xFF4DB6AC, (int) 0xFFFFB74D, (int) 0xFFAED581
};

/* A small "chip" used by both sides so they render identically. */
static TextView* makeChip(const std::string& text, int bg) {
    TextView* tv = new TextView(text, LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
    tv->setBackgroundColor(bg);
    tv->setTextColor(0xFFFFFFFF);
    tv->setTextSize(18);
    tv->setGravity(Gravity::CENTER);
    tv->setPadding(28, 18, 28, 18);
    return tv;
}

/* Adapter for the RecyclerView side. flexGrow is (re)applied on every bind so it
 * stays correct even when views are recycled across positions. */
class FlexAdapter : public RecyclerView::Adapter {
private:
    std::vector<std::string> mItems;
    std::vector<int> mBgs;
public:
    class VH : public RecyclerView::ViewHolder {
    public:
        TextView* tv;
        VH(View* v) : RecyclerView::ViewHolder(v) { tv = (TextView*) v; }
    };

    FlexAdapter(const std::vector<std::string>& items, const std::vector<int>& bgs)
        : mItems(items), mBgs(bgs) {}

    RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* /*parent*/, int /*viewType*/) override {
        TextView* tv = new TextView("", LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
        tv->setTextColor(0xFFFFFFFF);
        tv->setTextSize(18);
        tv->setGravity(Gravity::CENTER);
        tv->setPadding(28, 18, 28, 18);
        tv->setLayoutParams(new FlexboxLayoutManager::LayoutParams(
                LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT));
        return new VH(tv);
    }

    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position) override {
        VH& h = (VH&) holder;
        h.tv->setText(mItems[position]);
        h.tv->setBackgroundColor(mBgs[position]);
        FlexboxLayoutManager::LayoutParams* lp =
                (FlexboxLayoutManager::LayoutParams*) h.tv->getLayoutParams();
        if (lp) lp->setFlexGrow((position % 3 == 0) ? 1.f : 0.f);
    }

    int getItemCount() override { return (int) mItems.size(); }
};

static LinearLayout* titledColumn(const std::string& title, int titleBg) {
    LinearLayout* col = new LinearLayout(-1, -1);
    col->setOrientation(LinearLayout::VERTICAL);
    TextView* t = new TextView(title, -1, 40);
    t->setGravity(Gravity::CENTER);
    t->setTextColor(0xFFFFFFFF);
    t->setBackgroundColor(titleBg);
    t->setTextSize(20);
    col->addView(t, new LinearLayout::LayoutParams(-1, 40));
    return col;
}

int main(int argc, const char* argv[]) {
    App app(argc, argv);
    Window* w = new Window(0, 0, 1280, 720);
    w->setBackgroundColor(0xFF202028);
    w->setId(0);

    /* ---- shared data ---- */
    std::vector<std::string> items;
    std::vector<int> bgs;
    const int N = 30;
    for (int i = 0; i < N; i++) {
        items.push_back((i % 7 == 0) ? ("#" + std::to_string(i) + " long")
                                     : ("#" + std::to_string(i)));
        bgs.push_back(PALETTE[i % 6]);
    }

    /* ---- root: two equal columns ---- */
    LinearLayout* root = new LinearLayout(-1, -1);
    root->setOrientation(LinearLayout::HORIZONTAL);

    /* ===== left: FlexboxLayout ===== */
    LinearLayout* left = titledColumn("FlexboxLayout", 0xFF3949AB);
    left->setBackgroundColor(0xFF181820);
    FlexboxLayout* fb = new FlexboxLayout(-1, -1);
    fb->setFlexDirection((int) FlexDirection::ROW);
    fb->setFlexWrap((int) FlexWrap::WRAP);
    fb->setAlignItems((int) AlignItems::FLEX_START);
    fb->setPadding(16, 16, 16, 16);
    for (int i = 0; i < N; i++) {
        TextView* tv = makeChip(items[i], bgs[i]);
        FlexboxLayout::LayoutParams* lp = new FlexboxLayout::LayoutParams(
                LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
        lp->setFlexGrow((i % 3 == 0) ? 1.f : 0.f);
        fb->addView(tv, -1, lp);
    }
    left->addView(fb, new LinearLayout::LayoutParams(-1, -1));

    /* ===== right: RecyclerView + FlexboxLayoutManager ===== */
    LinearLayout* right = titledColumn("RecyclerView + FlexboxLayoutManager", 0xFF00897B);
    right->setBackgroundColor(0xFF202028);
    RecyclerView* rv = new RecyclerView(-1, -1);
    FlexboxLayoutManager* lm = new FlexboxLayoutManager(&app,
            (int) FlexDirection::ROW, (int) FlexWrap::WRAP);
    lm->setAlignItems((int) AlignItems::FLEX_START);
    rv->setLayoutManager(std::unique_ptr<RecyclerView::LayoutManager>(lm));
    rv->setAdapter(new FlexAdapter(items, bgs));
    rv->setVerticalScrollBarEnabled(true);
    rv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    right->addView(rv, new LinearLayout::LayoutParams(-1, -1));

    root->addView(left, new LinearLayout::LayoutParams(640, -1));
    root->addView(right, new LinearLayout::LayoutParams(640, -1));
    w->addView(root);
    app.exec();
}
