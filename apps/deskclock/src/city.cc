#include <city.h>

#include <R.h>

#include <cctype>
#include <cstdio>
#include <regex>

#include <content/resources.h>
#include <content/typedarray.h>
#include <content/sharedpreferences.h>
#include <core/context.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace data {

namespace {

/** Collator.getInstance() is not ported; byte-wise compare stands in. */
int stringCompare(const std::string& a, const std::string& b) {
    return a.compare(b);
}

std::vector<std::string> split(const std::string& s, const std::regex& sep) {
    std::sregex_token_iterator it(s.begin(), s.end(), sep, -1), end;
    std::vector<std::string> out;
    for (; it != end; ++it) out.push_back(*it);
    return out;
}

constexpr const char* NUMBER_OF_CITIES = "number_of_cities";
constexpr const char* CITY_ID = "city_id_";

} // namespace

//
// City
//

City::City(const std::string& id, int index, const std::string& indexString,
           const std::string& name, const std::string& phoneticName,
           const deskclock::TimeZone& timeZone)
    : id(id), index(index), indexString(indexString), name(name),
      phoneticName(phoneticName), timeZone(timeZone) {
}

const std::string& City::getNameUpperCase() const {
    if (mNameUpperCase.empty()) {
        mNameUpperCase = name;
        for (char& c : mNameUpperCase) c = toupper((unsigned char) c);
    }
    return mNameUpperCase;
}

const std::string& City::getNameUpperCaseNoSpecialCharacters() const {
    if (mNameUpperCaseNoSpecialCharacters.empty()) {
        mNameUpperCaseNoSpecialCharacters = removeSpecialCharacters(getNameUpperCase());
    }
    return mNameUpperCaseNoSpecialCharacters;
}

bool City::matches(const std::string& upperCaseQueryNoSpecialCharacters) const {
    // By removing all special characters, prefix matching becomes more liberal and it is easier
    // to locate the desired city. e.g. "St. Lucia" is matched by "StL", "St.L", "St L", "St. L"
    return getNameUpperCaseNoSpecialCharacters().compare(0,
            upperCaseQueryNoSpecialCharacters.size(), upperCaseQueryNoSpecialCharacters) == 0;
}

std::string City::toString() const {
    char buf[256];
    snprintf(buf, sizeof(buf), "City {id=%s, index=%d, indexString=%s, name=%s, phonetic=%s, tz=%s}",
             id.c_str(), index, indexString.c_str(), name.c_str(), phoneticName.c_str(),
             timeZone.getID().c_str());
    return buf;
}

std::string City::removeSpecialCharacters(const std::string& token) {
    std::string out;
    for (char c : token) {
        if (c != ' ' && c != '-' && c != '.' && c != '\'') out.push_back(c);
    }
    return out;
}

//
// Comparators
//

int CityUtcOffsetComparator::compare(const City& c1, const City& c2) const {
    const int result = CityUtcOffsetIndexComparator().compare(c1, c2);
    return result != 0 ? result : CityNameComparator().compare(c1, c2);
}

int CityUtcOffsetIndexComparator::compare(const City& c1, const City& c2) const {
    const int utcOffset1 = c1.timeZone.getOffset(mNow);
    const int utcOffset2 = c2.timeZone.getOffset(mNow);
    return utcOffset1 < utcOffset2 ? -1 : (utcOffset1 > utcOffset2 ? 1 : 0);
}

int CityNameComparator::compare(const City& c1, const City& c2) const {
    int result = CityNameIndexComparator().compare(c1, c2);
    if (result == 0) {
        result = stringCompare(c1.phoneticName, c2.phoneticName);
    }
    return result;
}

int CityNameIndexComparator::compare(const City& c1, const City& c2) const {
    int result = c1.index < c2.index ? -1 : (c1.index > c2.index ? 1 : 0);
    if (result == 0) {
        result = stringCompare(c1.indexString, c2.indexString);
    }
    return result;
}

//
// CityDAO
//

std::vector<City> CityDAO::getSelectedCities(SharedPreferences& prefs,
                                             const std::map<std::string, City>& cityMap) {
    const int size = prefs.getInt(NUMBER_OF_CITIES, 0);
    std::vector<City> selectedCities;

    for (int i = 0; i < size; i++) {
        const std::string id = prefs.getString(std::string(CITY_ID) + std::to_string(i), "");
        auto it = cityMap.find(id);
        if (it != cityMap.end()) {
            selectedCities.push_back(it->second);
        }
    }

    return selectedCities;
}

void CityDAO::setSelectedCities(SharedPreferences& prefs, const std::vector<City>& cities) {
    SharedPreferences::Editor& editor = prefs.edit();
    editor.putInt(NUMBER_OF_CITIES, (int) cities.size());

    int count = 0;
    for (const City& city : cities) {
        editor.putString(std::string(CITY_ID) + std::to_string(count), city.id);
        count++;
    }

    editor.apply();
}

std::map<std::string, City> CityDAO::getCities(Context& context) {
    Resources& resources = context.getResources();
    std::unique_ptr<TypedArray> cityStrings = resources.obtainTypedArray(R::array::city_ids);
    const int citiesCount = cityStrings ? (int) cityStrings->length() : 0;

    std::map<std::string, City> cities;
    for (int i = 0; i < citiesCount; i++) {
        // Attempt to locate the resource id defining the city as a string.
        const int cityResourceId = cityStrings->getResourceId(i, 0);
        if (cityResourceId == 0) {
            throw std::logic_error("Unable to locate city resource id for index "
                    + std::to_string(i));
        }

        std::string id;
        resources.getResourceEntryName(cityResourceId, &id);
        const std::string cityString = cityStrings->getString(i);
        if (cityString.empty()) {
            throw std::logic_error("Unable to locate city with id " + id);
        }

        // Attempt to parse the time zone from the city entry.
        const std::vector<std::string> cityParts = split(cityString, std::regex("\\|"));
        if (cityParts.size() != 2) {
            throw std::logic_error("Error parsing malformed city " + cityString);
        }

        City city("", -1, "", "", "", TimeZone::getDefault());
        // Skip cities whose timezone cannot be resolved.
        if (createCity(id, cityParts[0], cityParts[1], city)) {
            cities.emplace(city.id, city);
        }
    }

    return cities;
}

bool CityDAO::createCity(const std::string& id, const std::string& formattedName,
                         const std::string& tzId, City& outCity) {
    const TimeZone tz = TimeZone::getTimeZone(tzId);
    // If the time zone lookup fails, GMT is returned. No cities actually map to GMT.
    // (The app facade keeps unresolved Olson ids with the local-zone offset instead of
    // folding them to GMT, so foreign cities still list — with unshifted times.)
    if (tzId.empty() || tzId == "GMT") {
        return false;
    }

    const std::vector<std::string> parts = split(formattedName, std::regex("[=:]"));
    if (parts.size() < 2) {
        return false;
    }
    const std::string& name = parts[1];
    // Extract index string from input, use the first character of city name as the index string
    // if one is not explicitly provided.
    const std::string indexString = parts[0].empty() ? name.substr(0, 1) : parts[0];
    const std::string phoneticName = parts.size() == 3 ? parts[2] : name;

    static const std::regex NUMERIC_INDEX_REGEX("\\d+");
    std::smatch matcher;
    int index = -1;
    if (std::regex_search(indexString, matcher, NUMERIC_INDEX_REGEX)) {
        index = atoi(matcher.str().c_str());
    }

    outCity = City(id, index, indexString, name, phoneticName, tz);
    return true;
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
