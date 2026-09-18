#include <citymodel.h>

#include <algorithm>

#include <R.h>

#include <content/sharedpreferences.h>
#include <core/context.h>
#include <core/systemclock.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace data {

CityModel::CityModel(Context& context, SharedPreferences& prefs, SettingsModel& settingsModel)
    : mContext(context), mPrefs(prefs), mSettingsModel(settingsModel)
    , mHomeCity("", -1, "", "", "", TimeZone::getDefault()) {
}

void CityModel::addCityListener(const CityListener& cityListener) {
    mCityListeners.push_back(cityListener);
}

void CityModel::removeCityListener(const CityListener& cityListener) {
    for (auto it = mCityListeners.begin(); it != mCityListeners.end(); ++it) {
        if (*it == cityListener) {
            mCityListeners.erase(it);
            return;
        }
    }
}

const std::vector<City>& CityModel::getAllCities() {
    if (!mAllCitiesLoaded) {
        // Create a set of selections to identify the unselected cities.
        std::vector<City> selected = getSelectedCities();

        // Sort the selected cities alphabetically by name.
        CityNameComparator nameComparator;
        std::stable_sort(selected.begin(), selected.end(),
                [&](const City& a, const City& b) { return nameComparator.compare(a, b) < 0; });

        // Combine selected and unselected cities into a single list.
        mAllCitiesCache = selected;
        const std::vector<City>& unselected = getUnselectedCities();
        mAllCitiesCache.insert(mAllCitiesCache.end(), unselected.begin(), unselected.end());
        mAllCitiesLoaded = true;
    }
    return mAllCitiesCache;
}

const City& CityModel::getHomeCity() {
    if (!mHomeCityLoaded) {
        const std::string name = mContext.getString(R::string::home_label);
        const TimeZone timeZone = mSettingsModel.getHomeTimeZone();
        mHomeCity = City("", -1, "", name, name, timeZone);
        mHomeCityLoaded = true;
    }
    return mHomeCity;
}

const std::vector<City>& CityModel::getUnselectedCities() {
    if (!mUnselectedCitiesLoaded) {
        // Create a set of selections to identify the unselected cities.
        const std::vector<City>& selected = getSelectedCities();
        auto isSelected = [&](const City& city) {
            for (const City& s : selected) {
                if (s.id == city.id) return true;
            }
            return false;
        };

        const std::map<std::string, City>& all = getCityMap();
        for (const auto& entry : all) {
            if (!isSelected(entry.second)) {
                mUnselectedCitiesCache.push_back(entry.second);
            }
        }

        // Sort the unselected cities according by the user's preferred sort.
        const auto comparator = getCityIndexComparator();
        std::stable_sort(mUnselectedCitiesCache.begin(), mUnselectedCitiesCache.end(),
                [&](const City& a, const City& b) { return comparator(a, b) < 0; });
        mUnselectedCitiesLoaded = true;
    }
    return mUnselectedCitiesCache;
}

const std::vector<City>& CityModel::getSelectedCities() {
    if (!mSelectedCitiesLoaded) {
        mSelectedCitiesCache = CityDAO::getSelectedCities(mPrefs, getCityMap());
        CityUtcOffsetComparator comparator;
        std::stable_sort(mSelectedCitiesCache.begin(), mSelectedCitiesCache.end(),
                [&](const City& a, const City& b) { return comparator.compare(a, b) < 0; });
        mSelectedCitiesLoaded = true;
    }
    return mSelectedCitiesCache;
}

void CityModel::setSelectedCities(const std::vector<City>& cities) {
    const std::vector<City> oldCities = getAllCities();
    CityDAO::setSelectedCities(mPrefs, cities);

    // Clear caches affected by this update.
    mAllCitiesLoaded = false;
    mSelectedCitiesLoaded = false;
    mUnselectedCitiesLoaded = false;

    // Broadcast the change to the selected cities for the benefit of widgets.
    fireCitiesChanged(oldCities, getAllCities());
}

std::function<int(const City&, const City&)> CityModel::getCityIndexComparator() {
    if (mSettingsModel.getCitySort() == CitySort::UTC_OFFSET) {
        return [](const City& a, const City& b) {
            return CityUtcOffsetIndexComparator(SystemClock::currentTimeMillis()).compare(a, b);
        };
    }
    return [](const City& a, const City& b) {
        return CityNameIndexComparator().compare(a, b);
    };
}

CitySort CityModel::getCitySort() const {
    return mSettingsModel.getCitySort();
}

void CityModel::toggleCitySort() {
    mSettingsModel.toggleCitySort();

    // Clear caches affected by this update.
    mAllCitiesLoaded = false;
    mUnselectedCitiesLoaded = false;
}

const std::map<std::string, City>& CityModel::getCityMap() {
    if (!mCityMapLoaded) {
        mCityMap = CityDAO::getCities(mContext);
        mCityMapLoaded = true;
    }
    return mCityMap;
}

void CityModel::fireCitiesChanged(const std::vector<City>& oldCities,
                                  const std::vector<City>& newCities) {
    for (CityListener& cityListener : mCityListeners) {
        if (cityListener.citiesChanged) cityListener.citiesChanged(oldCities, newCities);
    }
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
