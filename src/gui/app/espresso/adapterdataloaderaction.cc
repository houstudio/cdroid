#include <app/espresso/adapterdataloaderaction.h>

#include <stdexcept>

#include <app/espresso/espressoexception.h>
#include <app/espresso/humanreadables.h>
#include <app/espresso/uicontroller.h>
#include <app/espresso/viewmatchers.h>
#include <widget/adapterview.h>

namespace cdroid {
namespace espresso {

namespace {

std::string describeDataMatcher(const Matcher<void*>& matcher) {
    StringDescription description;
    matcher.describeTo(description);
    return description.str();
}

std::string describeContainedValues(AdapterView& adapterView,
        AdapterViewProtocol& protocol) {
    // AOSP appends the whole getDataInAdapterView() iterable as one value;
    // Description::appendValue has no iterable overload here, so join the
    // elements' toString()s (documented deviation).
    std::string out = "[";
    bool first = true;
    for (const AdaptedData& data : protocol.getDataInAdapterView(adapterView)) {
        if (!first) out += ", ";
        first = false;
        out += data.toString();
    }
    out += "]";
    return out;
}

} // namespace

AdapterDataLoaderAction::AdapterDataLoaderAction(MatcherPtr<void*> dataToLoadMatcher,
        bool hasAtPosition, int atPosition,
        std::shared_ptr<AdapterViewProtocol> adapterViewProtocol)
    : mDataToLoadMatcher(std::move(dataToLoadMatcher)),
      mHasAtPosition(hasAtPosition),
      mAtPosition(atPosition),
      mAdapterViewProtocol(std::move(adapterViewProtocol)) {
    if (mDataToLoadMatcher == nullptr || mAdapterViewProtocol == nullptr) {
        throw std::invalid_argument("dataToLoadMatcher/adapterViewProtocol cannot be null");
    }
}

const AdaptedData& AdapterDataLoaderAction::getAdaptedData() const {
    if (!mPerformed) {
        // AOSP Checks.checkState(performed, "perform hasn't been called yet!")
        throw std::logic_error("perform hasn't been called yet!");
    }
    return *mAdaptedData;
}

MatcherPtr<View> AdapterDataLoaderAction::getConstraints() {
    return allOf(ViewMatchers::isAssignableFrom<AdapterView>(), ViewMatchers::isDisplayed());
}

void AdapterDataLoaderAction::perform(UiController& uiController, View& view) {
    // The constraints (allOf(isAssignableFrom(AdapterView), isDisplayed()))
    // ran before this; AOSP does an unchecked cast, so a non-AdapterView here
    // is a framework bug, not a test failure.
    AdapterView& adapterView = static_cast<AdapterView&>(view);
    std::vector<AdaptedData> matchedDataItems;

    for (const AdaptedData& data : mAdapterViewProtocol->getDataInAdapterView(adapterView)) {
        void* itemData = data.getData();
        if (mDataToLoadMatcher->matches(itemData)) {
            matchedDataItems.push_back(data);
        }
    }

    if (matchedDataItems.empty()) {
        std::string message = "No data found matching: "
                + describeDataMatcher(*mDataToLoadMatcher)
                + " contained values: "
                + describeContainedValues(adapterView, *mAdapterViewProtocol);
        throw PerformException::Builder()
                .withActionDescription(getDescription())
                .withViewDescription(HumanReadables::describe(&view))
                .withCause(std::make_exception_ptr(std::runtime_error(message)))
                .build();
    }

    // AOSP guards this block with synchronized(dataLock) — single-threaded here.
    if (mPerformed) {
        throw std::logic_error("perform called 2x!");
    }
    mPerformed = true;
    if (mHasAtPosition) {
        const int matchedDataItemsSize = (int)matchedDataItems.size() - 1;
        if (mAtPosition > matchedDataItemsSize) {
            char message[128];
            snprintf(message, sizeof(message),
                    "There are only %d elements that matched but requested %d element.",
                    matchedDataItemsSize, mAtPosition);
            throw PerformException::Builder()
                    .withActionDescription(getDescription())
                    .withViewDescription(HumanReadables::describe(&view))
                    .withCause(std::make_exception_ptr(std::runtime_error(message)))
                    .build();
        } else {
            mAdaptedData = std::make_shared<AdaptedData>(matchedDataItems.at(mAtPosition));
        }
    } else {
        if (matchedDataItems.size() != 1) {
            std::string elements = "[";
            bool first = true;
            for (const AdaptedData& data : matchedDataItems) {
                if (!first) elements += ", ";
                first = false;
                elements += data.toString();
            }
            elements += "]";
            std::string message = "Multiple data elements matched: "
                    + describeDataMatcher(*mDataToLoadMatcher)
                    + ". Elements: " + elements;
            throw PerformException::Builder()
                    .withActionDescription(getDescription())
                    .withViewDescription(HumanReadables::describe(&view))
                    .withCause(std::make_exception_ptr(std::runtime_error(message)))
                    .build();
        } else {
            mAdaptedData = std::make_shared<AdaptedData>(matchedDataItems.at(0));
        }
    }

    int requestCount = 0;
    while (!mAdapterViewProtocol->isDataRenderedWithinAdapterView(adapterView, *mAdaptedData)) {
        if (requestCount > 1) {
            if ((requestCount % 50) == 0) {
                // sometimes an adapter view will receive an event that will
                // block its attempts to scroll.
                adapterView.invalidate();
                mAdapterViewProtocol->makeDataRenderedWithinAdapterView(adapterView, *mAdaptedData);
            }
        } else {
            mAdapterViewProtocol->makeDataRenderedWithinAdapterView(adapterView, *mAdaptedData);
        }
        uiController.loopMainThreadForAtLeast(100);
        requestCount++;
    }
}

std::string AdapterDataLoaderAction::getDescription() {
    return "load adapter data";
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
