#ifndef __DESKCLOCK_CITYMODEL_H__
#define __DESKCLOCK_CITYMODEL_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.CityModel — the world-clock city domain,
 * selection and sort order. Locale-change cache clearing is not ported (no
 * system broadcasts); preference-change clearing is wired.
 *********************************************************************************/
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <city.h>
#include <datalisteners.h>
#include <settingsdao.h>

namespace cdroid {

class Context;
class SharedPreferences;

namespace deskclock {
namespace data {

class CityModel {
private:
    Context& mContext;
    SharedPreferences& mPrefs;
    SettingsModel& mSettingsModel;

    std::vector<CityListener> mCityListeners;

    bool mCityMapLoaded = false;
    std::map<std::string, City> mCityMap;

    bool mAllCitiesLoaded = false;
    std::vector<City> mAllCitiesCache;

    bool mSelectedCitiesLoaded = false;
    std::vector<City> mSelectedCitiesCache;

    bool mUnselectedCitiesLoaded = false;
    std::vector<City> mUnselectedCitiesCache;

    bool mHomeCityLoaded = false;
    City mHomeCity;

public:
    CityModel(Context& context, SharedPreferences& prefs, SettingsModel& settingsModel);

    void addCityListener(const CityListener& cityListener);
    void removeCityListener(const CityListener& cityListener);

    const std::vector<City>& getAllCities();
    const City& getHomeCity();
    const std::vector<City>& getUnselectedCities();
    const std::vector<City>& getSelectedCities();

    void setSelectedCities(const std::vector<City>& cities);

    /** @return the comparator for indexing the chosen sort order. */
    std::function<int(const City&, const City&)> getCityIndexComparator();

    CitySort getCitySort() const;
    void toggleCitySort();

private:
    const std::map<std::string, City>& getCityMap();

    void fireCitiesChanged(const std::vector<City>& oldCities, const std::vector<City>& newCities);
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_CITYMODEL_H__
