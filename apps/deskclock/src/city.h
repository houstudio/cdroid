#ifndef __DESKCLOCK_CITY_H__
#define __DESKCLOCK_CITY_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.{City,CityDAO} — a read-only city domain
 * object (with the UtcOffset/Name comparator family) and its storage in
 * Resources (values/cities.xml arrays) + SharedPreferences.
 *
 * CDROID notes: java.text.Collator is not ported — the locale-sensitive
 * index-string/phonetic-name comparisons degrade to byte-wise compares; Olson
 * zone ids resolve through the app TimeZone facade (no DST).
 *********************************************************************************/
#include <map>
#include <string>
#include <vector>

#include <timezone.h>

namespace cdroid {

class Context;
class SharedPreferences;

namespace deskclock {
namespace data {

/** A read-only domain object representing a city of the world. */
class City {
public:
    /** A unique identifier for the city. */
    std::string id;
    /** An optional numeric index used to order cities for display; -1 if none. */
    int index;
    /** An index string used to order cities for display. */
    std::string indexString;
    /** The display name of the city. */
    std::string name;
    /** The phonetic name of the city used to order cities for display. */
    std::string phoneticName;
    /** The TimeZone corresponding to the city. */
    deskclock::TimeZone timeZone;

    City(const std::string& id, int index, const std::string& indexString,
         const std::string& name, const std::string& phoneticName,
         const deskclock::TimeZone& timeZone);

    /** @return the city name converted to upper case. */
    const std::string& getNameUpperCase() const;

    /** @return true iff the name of this city starts with the given query. */
    bool matches(const std::string& upperCaseQueryNoSpecialCharacters) const;

    std::string toString() const;

    /** Strips out characters considered optional for matching: spaces, dashes, periods, quotes. */
    static std::string removeSpecialCharacters(const std::string& token);

private:
    /** Cached upper-case name forms (lazy). */
    mutable std::string mNameUpperCase;
    mutable std::string mNameUpperCaseNoSpecialCharacters;

    const std::string& getNameUpperCaseNoSpecialCharacters() const;
};

/** Orders by UTC offset, then index, then index string, then phonetic name. */
class CityUtcOffsetComparator {
public:
    int compare(const City& c1, const City& c2) const;
};

/** Orders by UTC offset only. */
class CityUtcOffsetIndexComparator {
public:
    CityUtcOffsetIndexComparator() : mNow(0) {}
    explicit CityUtcOffsetIndexComparator(int64_t now) : mNow(now) {}
    int compare(const City& c1, const City& c2) const;
private:
    // Snapshot the current time at construction to obtain consistent offsets.
    int64_t mNow;
};

/** Orders by index, index string, then (locale-collated) phonetic name. */
class CityNameComparator {
public:
    int compare(const City& c1, const City& c2) const;
};

/** Orders by numeric index then index string. */
class CityNameIndexComparator {
public:
    int compare(const City& c1, const City& c2) const;
};

/** Transfer of data between City domain objects and Resources/SharedPreferences. */
class CityDAO {
public:
    /** @return the list of city ids selected for display by the user. */
    static std::vector<City> getSelectedCities(SharedPreferences& prefs,
                                               const std::map<std::string, City>& cityMap);

    /** @param cities the collection of cities selected for display by the user. */
    static void setSelectedCities(SharedPreferences& prefs, const std::vector<City>& cities);

    /** @return the domain of cities from which the user may choose a world clock. */
    static std::map<std::string, City> getCities(Context& context);

    /**
     * @param id unique identifier for city
     * @param formattedName "[index]=[name]" or "[index]=[name]:[phonetic]"
     * @param tzId the string id of the timezone a given city is located in
     */
    static bool createCity(const std::string& id, const std::string& formattedName,
                           const std::string& tzId, City& outCity);
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_CITY_H__
