#include <gtest/gtest.h>
#include <core/app.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>
#include <widget/progressbar.h>
#include <guienvironment.h>

using namespace cdroid;

// Horizontal LinearLayout must lay nested vertical blocks side by side.
// Reproduces the widgetsDemo progress page where the three spinner blocks
// (vertical wrap blocks containing a 44dp ProgressBar + caption) all stacked
// on top of each other at the row head.
static void layoutAndLog(View* row) {
    row->measure(MeasureSpec::makeMeasureSpec(1080, MeasureSpec::EXACTLY),
                 MeasureSpec::makeMeasureSpec(200, MeasureSpec::AT_MOST));
    row->layout(0, 0, 1080, 200);
}

TEST(LINEARLAYOUT, HorizontalRowOfNestedVerticalBlocks) {
    Context* ctx = &App::getInstance();

    LinearLayout row(ctx);
    row.setOrientation(LinearLayout::HORIZONTAL);

    for (int i = 0; i < 3; i++) {
        auto* block = new LinearLayout(ctx);
        block->setOrientation(LinearLayout::VERTICAL);
        auto* dot = new View(ctx);
        dot->setMinimumWidth(44);
        dot->setMinimumHeight(44);
        auto* label = new TextView(ctx);
        label->setText(std::to_string(i));
        block->addView(dot, new LinearLayout::LayoutParams(44, 44));
        block->addView(label, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT, ViewGroup::LayoutParams::WRAP_CONTENT));
        row.addView(block, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT, ViewGroup::LayoutParams::WRAP_CONTENT));
    }

    layoutAndLog(&row);

    int left0 = row.getChildAt(0)->getLeft();
    int left1 = row.getChildAt(1)->getLeft();
    int left2 = row.getChildAt(2)->getLeft();
    int w0 = row.getChildAt(0)->getWidth();
    LOGD("block lefts %d %d %d w0=%d", left0, left1, left2, w0);
    EXPECT_GT(w0, 0);
    EXPECT_GT(left1, left0);
    EXPECT_GT(left2, left1);
    EXPECT_GT(left1 - left0, 30);   // blocks must not overlap (>= block width)
}
