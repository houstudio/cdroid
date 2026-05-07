#ifndef __INDEX_XY_MAP_H__
#define __INDEX_XY_MAP_H__
#include <map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>

namespace cdroid{

template<typename K, typename V>
class XYEntry {
private:
    K key;
    V value;

public:
    XYEntry(const K& k, const V& v) : key(k), value(v) {}
    K getKey() const {
        return key;
    }
    V getValue() const {
        return value;
    }
    V setValue(const V& newValue) {
        value = newValue;
        return value;
    }
};

template<typename K, typename V>
class IndexXYMap : public std::map<K, V> {
private:
    std::vector<K> indexList;
    double maxXDifference;

    void updateMaxXDifference() {
        if (indexList.size() < 2) {
            maxXDifference = 0;
            return;
        }
        double diff = std::abs(static_cast<double>(indexList.back()) - 
                static_cast<double>(indexList[indexList.size() - 2]));
        if (diff > maxXDifference) {
            maxXDifference = diff;
        }
    }
public:
    IndexXYMap() : maxXDifference(0) {}

    V put(const K& key, const V& value) {
        indexList.push_back(key);
        updateMaxXDifference();
        
        auto it = this->find(key);
        V oldValue = it != this->end() ? it->second : V();
        (*this)[key] = value;
        return oldValue;
    }

    V put(size_t index, const K& key, const V& value) {
        if (index > indexList.size()) {
            throw std::out_of_range("Index out of bounds");
        }
        indexList.insert(indexList.begin() + index, key);
        updateMaxXDifference();
        auto it = this->find(key);
        V oldValue = it != this->end() ? it->second : V();
        (*this)[key] = value;
        return oldValue;
    }

    double getMaxXDifference() const {
        return maxXDifference;
    }

    void clear() {
        maxXDifference = 0;
        std::map<K, V>::clear();
        indexList.clear();
    }

    /**
     * Returns X-value according to the given index
     */
    K getXByIndex(int index) const {
        if (index >= indexList.size()) {
            throw std::out_of_range("Index out of bounds");
        }
        return indexList[index];
    }

    /**
     * Returns Y-value according to the given index
     */
    V getYByIndex(int index) const {
        if (index >= (int)indexList.size()) {
            throw std::out_of_range("Index out of bounds");
        }
        K key = indexList[index];
        auto it = this->find(key);
        if (it != this->end()) {
            return it->second;
        } else {
            return V(); // Return default constructed value
        }
    }

    /**
     * Returns XY-entry according to the given index
     */
    XYEntry<K, V> getByIndex(int index) const {
        if (index >= (int)indexList.size()) {
            throw std::out_of_range("Index out of bounds");
        }
        K key = indexList[index];
        auto it = this->find(key);
        V value = (it != this->end()) ? it->second : V();
        return XYEntry<K, V>(key, value);
    }

    /**
     * Removes entry from map by index
     */
    XYEntry<K, V> removeByIndex(size_t index) {
        if (index >= indexList.size()) {
            throw std::out_of_range("Index out of bounds");
        }
        K key = indexList[index];
        indexList.erase(indexList.begin() + index);

        auto it = this->find(key);
        V value = (it != this->end()) ? it->second : V();
        if (it != this->end()) {
            this->erase(it);
        }
        updateMaxXDifference();
        return XYEntry<K, V>(key, value);
    }

    int getIndexForKey(const K& key) const {
        // This assumes K is comparable
        auto it = std::lower_bound(indexList.begin(), indexList.end(), key);
        if (it != indexList.end() && *it == key) {
            return static_cast<int>(std::distance(indexList.begin(), it));
        } else {
            // Return negative value to indicate not found (like Java binarySearch)
            return -(std::distance(indexList.begin(), it) + 1);;
        }
    }
};
}/*endofnamespace*/
#endif/*__INDEX_XY_MAP_H__*/
