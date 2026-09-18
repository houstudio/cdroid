#include <app/espresso/adapterviewprotocols.h>

#include <stdexcept>

#include <app/espresso/viewmatchers.h>
#include <widget/abslistview.h>
#include <widget/adapterview.h>
#include <widget/adapterviewanimator.h>
#include <widget/adapterviewflipper.h>

namespace cdroid {
namespace espresso {

namespace {

/**
 * Consider views which have over this percentage of their area visible to
 * the user to be fully rendered.
 */
constexpr int FULLY_RENDERED_PERCENTAGE_CUTOFF = 90;

class StandardAdapterViewProtocol : public AdapterViewProtocol {
public:
    /**
     * The AOSP StandardDataFunction re-positions a Cursor on every getData()
     * call; android.database.Cursor is not ported, so only the plain data
     * return survives (see adapterviewprotocol.h).
     */
    class StandardDataFunction : public DataFunction {
    public:
        StandardDataFunction(void* dataAtPosition, int position)
            : mDataAtPosition(dataAtPosition), mPosition(position) {
            if (position < 0) throw std::invalid_argument("position must be >= 0");
        }
        void* getData() override { return mDataAtPosition; }
    private:
        void* mDataAtPosition;
        int mPosition;
    };

    std::vector<AdaptedData> getDataInAdapterView(AdapterView& adapterView) override {
        std::vector<AdaptedData> datas;
        for (int i = 0; i < adapterView.getCount(); i++) {
            const int position = i;
            void* dataAtPosition = adapterView.getItemAtPosition(position);
            datas.push_back(AdaptedData::Builder()
                    .withDataFunction(std::make_shared<StandardDataFunction>(
                            dataAtPosition, position))
                    .withOpaqueToken((void*)(intptr_t)position)
                    .build());
        }
        return datas;
    }

    std::shared_ptr<AdaptedData> getDataRenderedByView2(
            AdapterView& adapterView, View& descendantView) override {
        if (descendantView.getParent() == &adapterView) {
            const int position = adapterView.getPositionForView(&descendantView);
            if (position != AdapterView::INVALID_POSITION) {
                return std::make_shared<AdaptedData>(AdaptedData::Builder()
                        .withDataFunction(std::make_shared<StandardDataFunction>(
                                adapterView.getItemAtPosition(position), position))
                        .withOpaqueToken((void*)(intptr_t)position)
                        .build());
            }
        }
        return nullptr;
    }

    void makeDataRenderedWithinAdapterView(
            AdapterView& adapterView, const AdaptedData& data) override {
        // AOSP: checkArgument(data.opaqueToken instanceof Integer) — the
        // instanceof rejects tokens minted by a different protocol. Tokens
        // here are intptr-encoded positions; range-checking them against
        // this adapter view is the closest ownership test.
        const int position = (int)(intptr_t)data.opaqueToken;
        if (position < 0 || position >= adapterView.getCount()) {
            throw std::invalid_argument("Not my data: " + data.toString());
        }

        bool moved = false;
        // set selection should always work, we can give a little better
        // experience if per subtype though.
        if (AbsListView* absListView = dynamic_cast<AbsListView*>(&adapterView)) {
            absListView->smoothScrollToPositionFromTop(
                    position, adapterView.getPaddingTop(), 0);
            moved = true;
        }

        if (AdapterViewAnimator* animator = dynamic_cast<AdapterViewAnimator*>(&adapterView)) {
            if (dynamic_cast<AdapterViewFlipper*>(animator) != nullptr) {
                ((AdapterViewFlipper*)animator)->stopFlipping();
            }
            animator->setDisplayedChild(position);
            moved = true;
        }
        if (!moved) {
            adapterView.setSelection(position);
        }
    }

    bool isDataRenderedWithinAdapterView(
            AdapterView& adapterView, const AdaptedData& adaptedData) override {
        const int dataPosition = (int)(intptr_t)adaptedData.opaqueToken;
        if (dataPosition < 0 || dataPosition >= adapterView.getCount()) {
            throw std::invalid_argument("Not my data: " + adaptedData.toString());
        }
        bool inView = false;

        if (dataPosition >= adapterView.getFirstVisiblePosition()
                && dataPosition <= adapterView.getLastVisiblePosition()) {
            if (adapterView.getFirstVisiblePosition() == adapterView.getLastVisiblePosition()) {
                // thats a huge element.
                inView = true;
            } else {
                inView = isElementFullyRendered(
                        adapterView, dataPosition - adapterView.getFirstVisiblePosition());
            }
        }
        if (inView) {
            // stops animations - locks in our x/y location.
            adapterView.setSelection(dataPosition);
        }

        return inView;
    }

private:
    static bool isElementFullyRendered(AdapterView& adapterView, int childAt) {
        // Occasionally we'll have to fight with smooth scrolling logic on our
        // definition of when there is extra scrolling to be done. In
        // particular if the element is the first or last element of the list,
        // the smooth scroller may decide that no work needs to be done to
        // scroll to the element if a certain percentage of it is on screen.
        // Ugh. Sigh. Yuck.
        View* element = adapterView.getChildAt(childAt);
        // Deviation guard: AOSP would NPE on a null child; a recycled/mid-
        // scroll child is simply "not rendered yet" here.
        if (element == nullptr) return false;

        return ViewMatchers::isDisplayingAtLeast(FULLY_RENDERED_PERCENTAGE_CUTOFF)
                ->matches(*element);
    }
};

} // namespace

std::shared_ptr<AdapterViewProtocol> AdapterViewProtocols::standardProtocol() {
    static const std::shared_ptr<AdapterViewProtocol> STANDARD_PROTOCOL =
            std::make_shared<StandardAdapterViewProtocol>();
    return STANDARD_PROTOCOL;
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
