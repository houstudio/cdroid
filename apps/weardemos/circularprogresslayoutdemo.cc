// Port of SupportWearDemos CircularProgressLayoutDemo: a 10 s
// CircularProgressLayout timer — tap to cancel (red), let it run out
// (green + "Finished!").
#include <core/app.h>
#include <cdroid.h>
#include <core/activityfactory.h>
#include <widget/textview.h>
#include <widgetEx/wear/circularprogresslayout.h>
#include "R.h"

using namespace cdroid;

namespace {
// Upstream TOTAL_TIME = TimeUnit.SECONDS.toMillis(10).
constexpr int64_t TOTAL_TIME = 10000;
}

class CircularProgressLayoutDemo : public Window {
private:
    CircularProgressLayout* mCircularProgressLayout;
    TextView* mChildView;

public:
    CircularProgressLayoutDemo() : Window(&App::getInstance(), 0, 0, -1, -1) {
        ViewGroup* root = (ViewGroup*)LayoutInflater::from(getContext())
                ->inflate(weardemos::R::layout::cpl_demo, this, false);
        addView(root);
        mCircularProgressLayout = (CircularProgressLayout*)root->findViewById(
                weardemos::R::id::circularProgressLayout_layout);
        mChildView = (TextView*)root->findViewById(
                weardemos::R::id::circularProgressLayout_child);

        mCircularProgressLayout->setOnClickListener([this](View& v) { onClick(v); });
        mCircularProgressLayout->setOnTimerFinishedListener(
                [this](CircularProgressLayout& layout) { onTimerFinished(layout); });

        mCircularProgressLayout->setTotalTime(TOTAL_TIME);
        mCircularProgressLayout->startTimer();
    }

private:
    void onTimerFinished(CircularProgressLayout& layout) {
        if (&layout == mCircularProgressLayout) {
            mChildView->setText(getContext()->getString(weardemos::R::string::cpl_finished));
            mCircularProgressLayout->setBackgroundColor(
                    getContext()->getColor(weardemos::R::color::cpl_light_green));
        }
    }
    void onClick(View& view) {
        if (&view == mCircularProgressLayout && mCircularProgressLayout->isTimerRunning()) {
            mCircularProgressLayout->stopTimer();
            mChildView->setText(getContext()->getString(weardemos::R::string::cpl_clicked));
            mCircularProgressLayout->setBackgroundColor(
                    getContext()->getColor(weardemos::R::color::cpl_light_red));
        }
    }
};
REGISTER_ACTIVITY(CircularProgressLayoutDemo);
