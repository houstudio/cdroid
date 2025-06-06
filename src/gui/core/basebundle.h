#ifndef __CDROID_BASEBUNDLE_H__
#define __CDROID_BASEBUNDLE_H__

#include <string>
#include <stdexcept>
#include <typeinfo>
#include <vector>
#include <unordered_map>
#include <core/any.h>

namespace cdroid {
using namespace nonstd;
class BaseBundle {
private:
    std::unordered_map<std::string, any> data_;

public:
    // Put methods for single values

    void putByte(const std::string& key, uint8_t value){
        data_[key] = value;
    }

    void putShort(const std::string& key, int16_t value){
        data_[key] = value;
    }

    void putInt(const std::string& key, int value) {
        data_[key] = value;
    }

    void putLong(const std::string& key,int64_t value){
        data_[key] = value;
    }

    void putFloat(const std::string& key, float value) {
        data_[key] = value;
    }

    void putDouble(const std::string& key, double value) {
        data_[key] = value;
    }

    void putString(const std::string& key, const std::string& value) {
        data_[key] = value;
    }

    void putBoolean(const std::string& key, bool value) {
        data_[key] = value;
    }

    // Put methods for arrays
    void putByteArray(const std::string& key, const std::vector<int8_t>& value){
        data_[key] = value;
    }

    void putShortArray(const std::string& key, const std::vector<int16_t>& value){
        data_[key] = value;
    }

    void putIntArray(const std::string& key, const std::vector<int>& value) {
        data_[key] = value;
    }

    void putLongArray(const std::string& key, const std::vector<int64_t>& value) {
        data_[key] = value;
    }

    void putFloatArray(const std::string& key, const std::vector<float>& value) {
        data_[key] = value;
    }

    void putDoubleArray(const std::string& key, const std::vector<double>& value) {
        data_[key] = value;
    }

    void putStringArray(const std::string& key, const std::vector<std::string>& value) {
        data_[key] = value;
    }

    void putBooleanArray(const std::string& key, const std::vector<bool>& value) {
        data_[key] = value;
    }

    // Get methods for single values
    int8_t getByte(const std::string& key) const{
        return getValue<int8_t>(key);
    }

    int16_t getShort(const std::string&key) const{
        return getValue<int16_t>(key);
    }
    int getInt(const std::string& key) const {
        return getValue<int>(key);
    }

    int64_t getLong(const std::string& key) const {
        return getValue<int64_t>(key);
    }

    float getFloat(const std::string& key) const {
        return getValue<float>(key);
    }

    double getDouble(const std::string& key) const {
        return getValue<double>(key);
    }

    std::string getString(const std::string& key) const {
        return getValue<std::string>(key);
    }

    bool getBoolean(const std::string& key) const {
        return getValue<bool>(key);
    }

    // Get methods for arrays
     std::vector<int8_t> getByteArray(const std::string& key) const {
        return getValue<std::vector<int8_t>>(key);
    }

    std::vector<int16_t> getShortArray(const std::string& key) const {
        return getValue<std::vector<int16_t>>(key);
    }

    std::vector<int> getIntArray(const std::string& key) const {
        return getValue<std::vector<int>>(key);
    }

    std::vector<int64_t> getLongArray(const std::string& key) const {
        return getValue<std::vector<int64_t>>(key);
    }

    std::vector<float> getFloatArray(const std::string& key) const {
        return getValue<std::vector<float>>(key);
    }

    std::vector<double> getDoubleArray(const std::string& key) const {
        return getValue<std::vector<double>>(key);
    }

    std::vector<std::string> getStringArray(const std::string& key) const {
        return getValue<std::vector<std::string>>(key);
    }

    std::vector<bool> getBooleanArray(const std::string& key) const {
        return getValue<std::vector<bool>>(key);
    }

    // Check if a key exists
    bool containsKey(const std::string& key) const {
        return data_.find(key) != data_.end();
    }

    // Remove a key-value pair
    void remove(const std::string& key) {
        data_.erase(key);
    }

    // Clear all key-value pairs
    void clear() {
        data_.clear();
    }

    // Get the size of the bundle
    size_t size() const {
        return data_.size();
    }

    // Check if the bundle is empty
    bool isEmpty() const {
        return data_.empty();
    }

    template<typename T>
    T getValue(const std::string& key) const {
        auto it = data_.find(key);
        if (it == data_.end()) {
            throw std::out_of_range("Key not found: " + key);
        }
        try {
            return any_cast<T>(it->second);
        } catch (const bad_any_cast& e) {
            throw std::runtime_error("Type mismatch for key: " + key);
        }
    }

    template<typename T>
    T getValue(const std::string& key,const T&defaultValue) const {
        auto it = data_.find(key);
        if (it == data_.end()) {
            return defaultValue;
        }
        try {
            return any_cast<T>(it->second);
        } catch (const bad_any_cast& e) {
            throw std::runtime_error("Type mismatch for key: " + key);
        }
    }

};

} // namespace cdroid

#endif // __CDROID_BASEBUNDLE_H__
