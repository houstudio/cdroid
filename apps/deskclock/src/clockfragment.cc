#include <clockfragment.h>

#include <R.h>
#include <widget/internal_R.h>

#include <content/Locale.h>
#include <core/calendar.h>
#include <core/intent.h>
#include <core/systemclock.h>
#include <widget/button.h>
#include <widget/textview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>

#include <datamodel.h>
#include <fragment/fragmentfactory.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

using data::DataModel;

/** RecyclerView.Adapter over the selected cities (+ optional header slots). */
class ClockFragment::SelectedCitiesAdapter : public RecyclerView::Adapter {
public:
    // Item view types.
    static constexpr int MAIN_CLOCK = R::layout::main_clock_frame;
    static constexpr int WORLD_CLOCK = R::layout::world_clock_item;

private:
    Context& mContext;
    LayoutInflater* mInflater;
    const bool mIsPortrait;
    const bool mShowHomeClock;
    std::string mDateFormat;
    std::string mDateFormatForAccessibility;

    /** CityListener facade (EventSet value semantics). */
    data::CityListener mCityListener;

public:
    SelectedCitiesAdapter(Context& context, const std::string& dateFormat,
                          const std::string& dateFormatForAccessibility)
        : mContext(context), mInflater(LayoutInflater::from(&context))
        , mIsPortrait(Utils::isPortrait(context))
        , mShowHomeClock(DataModel::getDataModel().getShowHomeClock())
        , mDateFormat(dateFormat), mDateFormatForAccessibility(dateFormatForAccessibility) {
        mCityListener.citiesChanged = [this](const std::vector<data::City>&,
                                             const std::vector<data::City>&) {
            notifyDataSetChanged();
        };
    }

    data::CityListener& getCityListener() { return mCityListener; }

    int getItemViewType(int position) override {
        return (position == 0 && mIsPortrait) ? MAIN_CLOCK : WORLD_CLOCK;
    }

    RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) override {
        View* view = mInflater->inflate(viewType, parent, false);
        if (viewType == WORLD_CLOCK) {
            return new CityViewHolder(view);
        } else if (viewType == MAIN_CLOCK) {
            return new MainClockViewHolder(view);
        }
        throw std::invalid_argument("View type not recognized");
    }

    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position) override {
        const int viewType = getItemViewType(position);
        if (viewType == WORLD_CLOCK) {
            // Retrieve the city to bind; if showing home clock, put it at the top.
            data::City city("", -1, "", "", "", TimeZone::getDefault());
            const int portraitAdjuster = mIsPortrait ? 1 : 0;
            if (mShowHomeClock && position == portraitAdjuster) {
                city = homeCity();
            } else {
                const int positionAdjuster = portraitAdjuster + (mShowHomeClock ? 1 : 0);
                city = cities().at(position - positionAdjuster);
            }
            static_cast<CityViewHolder&>(holder).bind(mContext, city, position, mIsPortrait);
        } else if (viewType == MAIN_CLOCK) {
            static_cast<MainClockViewHolder&>(holder).bind(mContext, mDateFormat,
                    mDateFormatForAccessibility, getItemCount() > 1);
        } else {
            throw std::invalid_argument("Unexpected view type: " + std::to_string(viewType));
        }
    }

    int getItemCount() override {
        const int mainClockCount = mIsPortrait ? 1 : 0;
        const int homeClockCount = mShowHomeClock ? 1 : 0;
        const int worldClockCount = (int) cities().size();
        return mainClockCount + homeClockCount + worldClockCount;
    }

    void refreshAlarm() {
        if (mIsPortrait && getItemCount() > 0) {
            notifyItemChanged(0);
        }
    }

    data::City homeCity() const { return DataModel::getDataModel().getHomeCity(); }

    const std::vector<data::City>& cities() const {
        return DataModel::getDataModel().getSelectedCities();
    }

    //
    // View holders
    //

    class CityViewHolder : public RecyclerView::ViewHolder {
    private:
        TextView* mName;
        TextClock* mDigitalClock;
        AnalogClock* mAnalogClock;
        TextView* mHoursAhead;

    public:
        explicit CityViewHolder(View* itemView)
            : RecyclerView::ViewHolder(itemView)
            , mName((TextView*) itemView->findViewById(R::id::city_name))
            , mDigitalClock((TextClock*) itemView->findViewById(R::id::digital_clock))
            , mAnalogClock((AnalogClock*) itemView->findViewById(R::id::analog_clock))
            , mHoursAhead((TextView*) itemView->findViewById(R::id::hours_ahead)) {}

        void bind(Context& context, const data::City& city, int position, bool isPortrait) {
            const std::string cityTimeZoneId = city.timeZone.getID();

            // Configure the digital clock or analog clock depending on the user preference.
            if (DataModel::getDataModel().getClockStyle() == data::ClockStyle::ANALOG) {
                mDigitalClock->setVisibility(View::GONE);
                mAnalogClock->setVisibility(View::VISIBLE);
                mAnalogClock->setTimeZone(cityTimeZoneId);
                mAnalogClock->enableSeconds(false);
            } else {
                mAnalogClock->setVisibility(View::GONE);
                mDigitalClock->setVisibility(View::VISIBLE);
                mDigitalClock->setTimeZone(cityTimeZoneId);
                // am/pm-ratio 0.3 span form upstream; TextClock takes the plain pattern.
                Utils::setTimeFormat(mDigitalClock, false);
            }

            // Supply top and bottom padding dynamically.
            Resources& res = context.getResources();
            const int padding = (int) res.getDimension(R::dimen::medium_space_top);
            const int top = (position == 0 && !isPortrait) ? 0 : padding;
            const int left = itemView->getPaddingLeft();
            const int right = itemView->getPaddingRight();
            const int bottom = itemView->getPaddingBottom();
            itemView->setPadding(left, top, right, bottom);

            // Bind the city name.
            mName->setText(city.name);

            // Compute if the city week day matches the weekday of the current timezone.
            std::unique_ptr<Calendar> localCal = Calendar::getInstance();
            std::unique_ptr<Calendar> cityCal = Calendar::getInstance();
            cityCal->setTimeZone(city.timeZone.getRawOffset() / 1000);
            const bool displayDayOfWeek =
                    localCal->get(Calendar::DAY_OF_WEEK) != cityCal->get(Calendar::DAY_OF_WEEK);

            // Compare offset from UTC time on today's date (daylight savings time, etc.)
            const TimeZone currentTimeZone = TimeZone::getDefault();
            const TimeZone cityTimeZone = TimeZone::getTimeZone(cityTimeZoneId);
            const int64_t currentTimeMillis = SystemClock::currentTimeMillis();
            const int64_t currentUtcOffset = currentTimeZone.getOffset(currentTimeMillis);
            const int64_t cityUtcOffset = cityTimeZone.getOffset(currentTimeMillis);
            const int64_t offsetDelta = cityUtcOffset - currentUtcOffset;

            const int hoursDifferent = (int) (offsetDelta / (60LL * 60 * 1000));
            const int minutesDifferent = (int) (offsetDelta / (60LL * 1000)) % 60;
            const bool displayMinutes = offsetDelta % (60LL * 60 * 1000) != 0;
            const bool isAhead = hoursDifferent > 0
                    || (hoursDifferent == 0 && minutesDifferent > 0);
            if (!Utils::isLandscape(context)) {
                // Bind the number of hours ahead or behind, or hide if the time is the same.
                const bool displayDifference = hoursDifferent != 0 || displayMinutes;
                mHoursAhead->setVisibility(displayDifference ? View::VISIBLE : View::GONE);
                const std::string timeString = Utils::createHoursDifferentString(
                        context, displayMinutes, isAhead, hoursDifferent, minutesDifferent);
                if (displayDayOfWeek) {
                    const int stringId = isAhead ? R::string::world_hours_tomorrow
                                                 : R::string::world_hours_yesterday;
                    char buf[128];
                    const std::string fmt = context.getString(stringId);
                    snprintf(buf, sizeof(buf), fmt.c_str(), timeString.c_str());
                    mHoursAhead->setText(buf);
                } else {
                    mHoursAhead->setText(timeString);
                }
            } else {
                // Only tomorrow/yesterday should be shown in landscape view.
                mHoursAhead->setVisibility(displayDayOfWeek ? View::VISIBLE : View::GONE);
                if (displayDayOfWeek) {
                    mHoursAhead->setText(context.getString(isAhead ? R::string::world_tomorrow
                                                                   : R::string::world_yesterday));
                }
            }
        }
    };

    class MainClockViewHolder : public RecyclerView::ViewHolder {
    private:
        View* mHairline;
        TextClock* mDigitalClock;
        AnalogClock* mAnalogClock;

    public:
        explicit MainClockViewHolder(View* itemView)
            : RecyclerView::ViewHolder(itemView)
            , mHairline(itemView->findViewById(R::id::hairline))
            , mDigitalClock((TextClock*) itemView->findViewById(R::id::digital_clock))
            , mAnalogClock((AnalogClock*) itemView->findViewById(R::id::analog_clock)) {}

        void bind(Context& context, const std::string& dateFormat,
                  const std::string& dateFormatForAccessibility, bool showHairline) {
            // Show/hide the top hairline divider.
            mHairline->setVisibility(showHairline ? View::VISIBLE : View::GONE);

            Utils::setClockIconTypeface(itemView);
            Utils::updateDate(dateFormat, dateFormatForAccessibility, itemView);
            Utils::setClockStyle(*mDigitalClock, *mAnalogClock);
            Utils::setClockSecondsEnabled(*mDigitalClock, *mAnalogClock);
            Utils::refreshAlarm(context, itemView);
        }
    };
};

//
// ClockFragment
//

ClockFragment::ClockFragment() : DeskClockFragment(uidata::Tab::CLOCKS) {
    mQuarterHourUpdater = [this]() {
        // mCityAdapter.notifyDataSetChanged()
        if (mCityAdapter != nullptr) mCityAdapter->notifyDataSetChanged();
    };
}

ClockFragment::~ClockFragment() {
    // The FragmentManager teardown path deletes fragments directly without
    // dispatching onDestroyView (and a queued quarter-hour tick firing after
    // this destructor would touch a dangling adapter), so release the periodic
    // callback and the adapter here as well. Idempotent with onDestroyView —
    // both null out mCityAdapter.
    uidata::UiDataModel::getUiDataModel().removePeriodicCallback(mQuarterHourUpdater);
    if (mCityAdapter != nullptr) {
        DataModel::getDataModel().removeCityListener(mCityAdapter->getCityListener());
        delete mCityAdapter;
        mCityAdapter = nullptr;
    }
}

View* ClockFragment::onCreateView(LayoutInflater* inflater, ViewGroup* container,
                                  Bundle* savedInstanceState) {
    DeskClockFragment::onCreateView(inflater, container, savedInstanceState);

    View* fragmentView = inflater->inflate(R::layout::clock_fragment, container, false);
    Utils::setDefaultBackground(fragmentView);

    mDateFormat = getContext()->getString(R::string::abbrev_wday_month_day_no_year);
    mDateFormatForAccessibility = getContext()->getString(R::string::full_wday_month_day_no_year);

    mCityAdapter = new SelectedCitiesAdapter(*getContext(), mDateFormat,
                                             mDateFormatForAccessibility);

    mCityList = (RecyclerView*) fragmentView->findViewById(R::id::cities);
    mCityList->setLayoutManager(new LinearLayoutManager(getContext()));
    mCityList->setAdapter(mCityAdapter);
    mCityList->setItemAnimator(nullptr);
    DataModel::getDataModel().addCityListener(mCityAdapter->getCityListener());

    mScrollPositionWatcher.onScrolled = [this](RecyclerView&, int, int) {
        setTabScrolledToTop(Utils::isScrolledToTop(*mCityList));
    };
    mCityList->addOnScrollListener(mScrollPositionWatcher);

    // On tablet landscape, the clock frame will be a distinct view. Otherwise, it'll be added
    // on as a header to the main listview.
    mClockFrame = fragmentView->findViewById(R::id::main_clock_left_pane);
    if (mClockFrame != nullptr) {
        mDigitalClock = (TextClock*) mClockFrame->findViewById(R::id::digital_clock);
        mAnalogClock = (AnalogClock*) mClockFrame->findViewById(R::id::analog_clock);
        Utils::setClockIconTypeface(mClockFrame);
        Utils::updateDate(mDateFormat, mDateFormatForAccessibility, mClockFrame);
        Utils::setClockStyle(*mDigitalClock, *mAnalogClock);
        Utils::setClockSecondsEnabled(*mDigitalClock, *mAnalogClock);
    }

    // Schedule a runnable to update the date every quarter hour.
    uidata::UiDataModel::getUiDataModel().addQuarterHourCallback(mQuarterHourUpdater);

    return fragmentView;
}

void ClockFragment::onResume() {
    DeskClockFragment::onResume();

    mDateFormat = getContext()->getString(R::string::abbrev_wday_month_day_no_year);
    mDateFormatForAccessibility = getContext()->getString(R::string::full_wday_month_day_no_year);

    // Resume can be invoked after changing the clock style or seconds display.
    if (mDigitalClock != nullptr && mAnalogClock != nullptr) {
        Utils::setClockStyle(*mDigitalClock, *mAnalogClock);
        Utils::setClockSecondsEnabled(*mDigitalClock, *mAnalogClock);
    }

    View* view = getView();
    if (view != nullptr && view->findViewById(R::id::main_clock_left_pane) != nullptr) {
        // Center the main clock frame by hiding the world clocks when none are selected.
        mCityList->setVisibility(mCityAdapter->getItemCount() == 0 ? View::GONE : View::VISIBLE);
    }

    refreshAlarm();
}

void ClockFragment::onDestroyView() {
    DeskClockFragment::onDestroyView();
    uidata::UiDataModel::getUiDataModel().removePeriodicCallback(mQuarterHourUpdater);
    if (mCityAdapter != nullptr) {
        DataModel::getDataModel().removeCityListener(mCityAdapter->getCityListener());
        delete mCityAdapter;
        mCityAdapter = nullptr;
    }
}

void ClockFragment::onFabClick(ImageView& /*fab*/) {
    Intent intent;
    intent.setClassName("cdroid.deskclock", "CitySelectionActivity")
          .setAction(Intent::ACTION_MAIN);
    getContext()->startActivity(intent);
}

void ClockFragment::onUpdateFab(ImageView& fab) {
    fab.setVisibility(View::VISIBLE);
    fab.setImageResource(R::drawable::ic_public);
    fab.setContentDescription(fab.getContext()->getString(R::string::button_cities));
}

void ClockFragment::onUpdateFabButtons(Button& left, Button& right) {
    left.setVisibility(View::INVISIBLE);
    right.setVisibility(View::INVISIBLE);
}

void ClockFragment::refreshAlarm() {
    if (mClockFrame != nullptr) {
        Utils::refreshAlarm(*getContext(), mClockFrame);
    } else if (mCityAdapter != nullptr) {
        mCityAdapter->refreshAlarm();
    }
}

} // namespace deskclock

// Registered under the bare name the UiDataModel CLOCKS tab references.
static const int _cdroid_frag_reg_clock =
    (::cdroid::FragmentFactory::registerFragment("ClockFragment",
        []() -> ::cdroid::Fragment* {
            return new ::cdroid::deskclock::ClockFragment();
        }), 0);

} // namespace cdroid
