#include <cityselectionactivity.h>

#include <R.h>

#include <algorithm>
#include <cctype>

#include <core/calendar.h>
#include <core/activityfactory.h>
#include <view/layoutinflater.h>
#include <widget/checkbox.h>
#include <widget/listview.h>
#include <widget/textview.h>

#include <city.h>
#include <datamodel.h>
#include <searchmenuitemcontroller.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace worldclock {

//
// CityAdapter — presents data as [optional Selected-Cities header] + indexed cities.
//

class CitySelectionActivity::CityAdapter : public BaseAdapter {
private:
    Context* mContext;
    actionbarmenu::SearchMenuItemController* mSearchMenuItemController;
    LayoutInflater* mInflater;

    /** 12-hour pattern (approximation of DateFormat.getBestDateTimePattern(locale, "hma")). */
    const std::string mPattern12 = "h:mm a";

    /** 24-hour pattern (approximation of getBestDateTimePattern(locale, "Hm")). */
    const std::string mPattern24 = "HH:mm";

    /** True when time should honor mPattern24; mPattern12 otherwise. */
    bool mIs24HoursMode = false;

    /** A calendar used to format time in a particular timezone. */
    std::unique_ptr<Calendar> mCalendar;

    /** The list of cities which may be filtered by a search term. */
    std::vector<data::City> mFilteredCities;

    /** The set of cities currently selected by the user (insertion-ordered). */
    std::vector<data::City> mUserSelectedCities;

    /** The number of user selections at the top of the adapter to avoid indexing. */
    int mOriginalUserSelectionCount = 0;

    static constexpr int VIEW_TYPE_SELECTED_CITIES_HEADER = 0;
    static constexpr int VIEW_TYPE_CITY = 1;

    /** Cache the child views of each city item view. */
    struct CityItemHolder {
        TextView* index;
        TextView* name;
        TextView* time;
        CheckBox* selected;
    };

public:
    CityAdapter(Context* context, actionbarmenu::SearchMenuItemController* searchMenuItemController)
        : mContext(context), mSearchMenuItemController(searchMenuItemController),
          mInflater(LayoutInflater::from(context)) {
        mCalendar = Calendar::getInstance();
        mCalendar->setTimeInMillis(SystemClock::currentTimeMillis());
    }

    // --- upstream SectionIndexer is not ported (no fast-scroll thumb); the
    // index column still renders per-row via getShowIndex(). ---

    int getCount() const override {
        const int headerCount = hasHeader() ? 1 : 0;
        return headerCount + (int) mFilteredCities.size();
    }

    void* getItem(int position) const override {
        return (void*) &getCity(position);
    }

    long getItemId(int position) const override {
        return position;
    }

    int getViewTypeCount() const override {
        return 2;
    }

    int getItemViewType(int position) const override {
        return (hasHeader() && position == 0) ? VIEW_TYPE_SELECTED_CITIES_HEADER : VIEW_TYPE_CITY;
    }

    View* getView(int position, View* convertView, ViewGroup* parent) override {
        const int itemViewType = getItemViewType(position);
        if (itemViewType == VIEW_TYPE_SELECTED_CITIES_HEADER) {
            return convertView != nullptr ? convertView
                    : mInflater->inflate(R::layout::city_list_header, parent, false);
        }

        const data::City& city = getCity(position);
        const TimeZone& timeZone = city.timeZone;

        // Inflate a new view if necessary.
        if (convertView == nullptr) {
            convertView = mInflater->inflate(R::layout::city_list_item, parent, false);
            CityItemHolder* holder = new CityItemHolder();
            holder->index = (TextView*) convertView->findViewById(R::id::index);
            holder->name = (TextView*) convertView->findViewById(R::id::city_name);
            holder->time = (TextView*) convertView->findViewById(R::id::city_time);
            holder->selected = (CheckBox*) convertView->findViewById(R::id::city_onoff);
            convertView->setTag(holder);
        }

        // Bind data into the child views.
        CityItemHolder* holder = (CityItemHolder*) convertView->getTag();
        holder->selected->setTag((void*) &city);
        holder->selected->setChecked(isSelected(city));
        holder->selected->setContentDescription(city.name);
        holder->selected->setOnCheckedChangeListener([this](CompoundButton& b, bool isChecked) {
            onCheckedChanged(b, isChecked);
        });
        holder->name->setText(city.name);
        holder->time->setText(getTimeString(timeZone));

        const bool showIndex = getShowIndex(position);
        holder->index->setVisibility(showIndex ? View::VISIBLE : View::INVISIBLE);
        if (showIndex) {
            if (data::DataModel::getDataModel().getCitySort() == data::CitySort::NAME) {
                holder->index->setText(city.indexString);
                holder->index->setTextSize(TypedValue::COMPLEX_UNIT_SP, 24.0f);
            } else {
                holder->index->setText(Utils::getGMTHourOffset(timeZone.getRawOffset(), false));
                holder->index->setTextSize(TypedValue::COMPLEX_UNIT_SP, 14.0f);
            }
        }

        // Skip checkbox and other animations.
        convertView->jumpDrawablesToCurrentState();
        convertView->setOnClickListener([this](View& v) { onClick(v); });
        return convertView;
    }

    void onCheckedChanged(CompoundButton& buttonView, bool isChecked) {
        const data::City* cityPtr = (const data::City*) buttonView.getTag();
        if (cityPtr == nullptr) return;
        data::City city = *cityPtr;
        if (isChecked) {
            if (!isSelected(city)) mUserSelectedCities.push_back(city);
            buttonView.announceForAccessibility(
                    mContext->getResources().getString(R::string::city_checked, {city.name}));
        } else {
            removeSelection(city);
            buttonView.announceForAccessibility(
                    mContext->getResources().getString(R::string::city_unchecked, {city.name}));
        }
    }

    void onClick(View& v) {
        CheckBox* b = (CheckBox*) v.findViewById(R::id::city_onoff);
        b->setChecked(!b->isChecked());
    }

    /** Rebuilds all internal data structures from scratch. */
    void refresh() {
        // Update the 12/24 hour mode.
        mIs24HoursMode = data::DataModel::getDataModel().is24HourFormat();

        // Refresh the user selections.
        const std::vector<data::City>& selected =
                data::DataModel::getDataModel().getSelectedCities();
        mUserSelectedCities = selected;
        mOriginalUserSelectionCount = (int) selected.size();

        // Recompute filtered cities.
        filter(mSearchMenuItemController->getQueryText());
    }

    /** Filter the cities using the given queryText. */
    void filter(const std::string& queryText) {
        mSearchMenuItemController->setQueryText(queryText);
        std::string query = data::City::removeSpecialCharacters(toUpperCase(queryText));

        // Compute the filtered list of cities.
        if (query.empty()) {
            mFilteredCities = data::DataModel::getDataModel().getAllCities();
        } else {
            mFilteredCities.clear();
            for (const data::City& city : data::DataModel::getDataModel().getUnselectedCities()) {
                if (city.matches(query)) {
                    mFilteredCities.push_back(city);
                }
            }
        }

        // Swap in the filtered list of cities and notify of the data change.
        notifyDataSetChanged();
    }

    bool isFiltering() const {
        return !trim(mSearchMenuItemController->getQueryText()).empty();
    }

    const std::vector<data::City>& getSelectedCities() const {
        return mUserSelectedCities;
    }

private:
    bool hasHeader() const {
        return !isFiltering() && mOriginalUserSelectionCount > 0;
    }

    /** @return the City at the given list position (header-aware). */
    const data::City& getCity(int position) const {
        return mFilteredCities[hasHeader() ? position - 1 : position];
    }

    data::CitySort getCitySort() const {
        return data::DataModel::getDataModel().getCitySort();
    }

    bool isSelected(const data::City& city) const {
        for (const data::City& c : mUserSelectedCities) {
            if (c.id == city.id) return true;
        }
        return false;
    }

    void removeSelection(const data::City& city) {
        for (auto it = mUserSelectedCities.begin(); it != mUserSelectedCities.end(); ++it) {
            if (it->id == city.id) {
                mUserSelectedCities.erase(it);
                return;
            }
        }
    }

    std::string getTimeString(const TimeZone& timeZone) {
        mCalendar->setTimeZone(timeZone.getRawOffset() / 1000);
        const int hour = mCalendar->get(Calendar::HOUR_OF_DAY);
        const int minute = mCalendar->get(Calendar::MINUTE);
        char buf[32];
        if (mIs24HoursMode) {
            snprintf(buf, sizeof(buf), "%02d:%02d", hour, minute);
        } else {
            const int hour12 = hour % 12;
            snprintf(buf, sizeof(buf), "%d:%02d %s", hour12 == 0 ? 12 : hour12, minute,
                     hour < 12 ? "AM" : "PM");
        }
        return buf;
    }

    bool getShowIndex(int position) const {
        // Indexes are never displayed on filtered cities.
        if (isFiltering()) {
            return false;
        }

        if (hasHeader()) {
            // None of the original user selections should show their index.
            if (position <= mOriginalUserSelectionCount) {
                return false;
            }

            // The first item after the original user selections must always show its index.
            if (position == mOriginalUserSelectionCount + 1) {
                return true;
            }
        } else {
            // None of the original user selections should show their index.
            if (position < mOriginalUserSelectionCount) {
                return false;
            }

            // The first item after the original user selections must always show its index.
            if (position == mOriginalUserSelectionCount) {
                return true;
            }
        }

        // Otherwise compare the city with its predecessor to test if it is a header.
        const data::City& priorCity = getCity(position - 1);
        const data::City& city = getCity(position);
        auto comparator = data::DataModel::getDataModel().getCityIndexComparator();
        return comparator(priorCity, city) != 0;
    }

    static std::string toUpperCase(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
            return (char) std::toupper(c);
        });
        return out;
    }

    static std::string trim(const std::string& s) {
        const size_t b = s.find_first_not_of(" \t\r\n");
        if (b == std::string::npos) return std::string();
        const size_t e = s.find_last_not_of(" \t\r\n");
        return s.substr(b, e - b + 1);
    }
};

//
// SortOrderMenuItemController
//

namespace {
/** Toggles NAME <-> UTC_OFFSET and refreshes the adapter. */
class SortOrderMenuItemController : public actionbarmenu::MenuItemController {
private:
    CitySelectionActivity* mActivity;

public:
    explicit SortOrderMenuItemController(CitySelectionActivity* activity) : mActivity(activity) {}

    int getId() const override { return R::id::menu_item_sort; }

    void onCreateOptionsItem(Menu& menu) override {
        menu.add(Menu::NONE, R::id::menu_item_sort, Menu::NONE,
                 mActivity->getResources().getString(R::string::menu_item_sort_by_gmt_offset))
                ->setShowAsAction(MenuItem::SHOW_AS_ACTION_NEVER);
    }

    void onPrepareOptionsItem(MenuItem& item) override {
        item.setTitle(mActivity->getResources().getString(
                data::DataModel::getDataModel().getCitySort() == data::CitySort::NAME
                        ? R::string::menu_item_sort_by_gmt_offset
                        : R::string::menu_item_sort_by_name));
    }

    bool onOptionsItemSelected(MenuItem& item) override {
        // Save the new sort order.
        data::DataModel::getDataModel().toggleCitySort();

        // Honor the new sort order in the adapter (section headers recomputed inside).
        mActivity->onSortOrderChanged();
        return true;
    }
};
} // namespace

//
// CitySelectionActivity
//

CitySelectionActivity::CitySelectionActivity() : Window(0, 0, -1, -1) {
}

CitySelectionActivity::~CitySelectionActivity() {
    delete mCitiesAdapter;
    delete mSearchMenuItemController;
    delete mDropShadowController;
}

void CitySelectionActivity::onCreate(Bundle* savedInstanceState) {
    Window::onCreate(savedInstanceState);

    View* content = LayoutInflater::from(getContext())
            ->inflate(R::layout::cities_activity, nullptr, false);
    // Opaque base layer: without one, SRC_OVER smears whatever was under the window.
    Utils::setDefaultBackground(content);
    addView(content);

    mSearchMenuItemController = new actionbarmenu::SearchMenuItemController(getContext(),
            [this](const std::string& query) {
                mCitiesAdapter->filter(query);
                updateFastScrolling();
                return true;
            });
    mCitiesAdapter = new CityAdapter(getContext(), mSearchMenuItemController);
    mOptionsMenuManager.addMenuItemController(
            {new actionbarmenu::NavUpMenuItemController(this),
             mSearchMenuItemController,
             new SortOrderMenuItemController(this),
             new actionbarmenu::SettingsMenuItemController(this)});
    mCitiesList = (ListView*) findViewById(R::id::cities_list);
    mCitiesList->setAdapter(mCitiesAdapter);

    updateFastScrolling();
}

void CitySelectionActivity::onResume() {
    Window::onResume();

    // Recompute the contents of the adapter before displaying on screen.
    mCitiesAdapter->refresh();

    mDropShadowController = new DropShadowController(*findViewById(R::id::drop_shadow),
            uidata::UiDataModel::getUiDataModel(), *mCitiesList);
}

void CitySelectionActivity::onPause() {
    Window::onPause();

    mDropShadowController->stop();

    // Save the selected cities.
    data::DataModel::getDataModel().setSelectedCities(mCitiesAdapter->getSelectedCities());
}

bool CitySelectionActivity::onCreateOptionsMenu(Menu& menu) {
    mOptionsMenuManager.onCreateOptionsMenu(menu);
    return true;
}

bool CitySelectionActivity::onPrepareOptionsMenu(Menu& menu) {
    mOptionsMenuManager.onPrepareOptionsMenu(menu);
    return true;
}

bool CitySelectionActivity::onOptionsItemSelected(MenuItem& item) {
    return mOptionsMenuManager.onOptionsItemSelected(item) ||
           Window::onOptionsItemSelected(item);
}

void CitySelectionActivity::updateFastScrolling() {
    const bool enabled = !mCitiesAdapter->isFiltering();
    mCitiesList->setFastScrollAlwaysVisible(enabled);
    mCitiesList->setFastScrollEnabled(enabled);
}

void CitySelectionActivity::onSortOrderChanged() {
    // Upstream also clears section headers here; the adapter's filter recomputes them.
    mCitiesAdapter->filter(mSearchMenuItemController->getQueryText());
}

} // namespace worldclock
} // namespace deskclock

// Registered under the bare class name ClockFragment's Intent ComponentName carries
// (REGISTER_ACTIVITY would stringify the qualified name).
static const int _cdroid_act_reg_cityselection =
    (::cdroid::ActivityFactory::registerActivity("CitySelectionActivity",
        []() -> ::cdroid::Window* {
            ::cdroid::Window* w = new ::cdroid::deskclock::worldclock::CitySelectionActivity();
            w->setActivityName("CitySelectionActivity");
            return w;
        }), 0);

} // namespace cdroid
