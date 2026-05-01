#ifndef __NUMBER_FORMAT_H__
#define __NUMBER_FORMAT_H__
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include <cmath>
#include <climits>
#include <cctype>
#include <algorithm>
#include <regex>
#include <stdexcept>

namespace cdroid{
class NumberFormat {
protected:
    int fMinimumIntegerDigits = 1;
    int fMaximumIntegerDigits = INT_MAX;
    int fMinimumFractionDigits = 0;
    int fMaximumFractionDigits = 3;
    bool fGroupingUsed = true;
    char fDecimalSeparator = '.';
    char fGroupingSeparator = ',';
    int fMultiplier = 1;
    bool fParseIntegerOnly = false;
    
public:
    virtual ~NumberFormat() = default;
    
    void setMinimumIntegerDigits(int newValue) { fMinimumIntegerDigits = newValue; }
    void setMaximumIntegerDigits(int newValue) { fMaximumIntegerDigits = newValue; }
    int getMinimumIntegerDigits() const { return fMinimumIntegerDigits; }
    int getMaximumIntegerDigits() const { return fMaximumIntegerDigits; }
    
    void setMinimumFractionDigits(int newValue) { fMinimumFractionDigits = newValue; }
    void setMaximumFractionDigits(int newValue) { fMaximumFractionDigits = newValue; }
    int getMinimumFractionDigits() const { return fMinimumFractionDigits; }
    int getMaximumFractionDigits() const { return fMaximumFractionDigits; }
    
    void setGroupingUsed(bool newValue) { fGroupingUsed = newValue; }
    bool isGroupingUsed() const { return fGroupingUsed; }
    
    void setMultiplier(int newValue) { fMultiplier = newValue; }
    int getMultiplier() const { return fMultiplier; }
    
    void setParseIntegerOnly(bool value) { fParseIntegerOnly = value; }
    bool isParseIntegerOnly() const { return fParseIntegerOnly; }
    
    virtual std::string format(double number) const;
    
    virtual std::string format(int32_t number) const {
        return format(static_cast<double>(number));
    }
    
    virtual std::string format(int64_t number) const {
        return format(static_cast<double>(number));
    }
    
    virtual std::pair<double, size_t> parse(const std::string& text, size_t pos = 0) const;
    
    static std::unique_ptr<NumberFormat> getInstance();
    static std::unique_ptr<NumberFormat> getCurrencyInstance();
    static std::unique_ptr<NumberFormat> getPercentInstance();
    static std::unique_ptr<NumberFormat> getIntegerInstance();
protected:
    std::string applyGrouping(const std::string& input) const;
};

class DecimalFormat : public NumberFormat {
public:
    enum RoundingMode {
        HALF_UP,
        HALF_DOWN,
        UP,
        DOWN,
        CEILING,
        FLOOR,
        HALF_EVEN,
        UNNECESSARY
    };
    
private:
    RoundingMode fRoundingMode = HALF_UP;
    std::string fPositivePrefix;
    std::string fPositiveSuffix;
    std::string fNegativePrefix;
    std::string fNegativeSuffix;
    std::string fPattern;
    
    struct PatternInfo {
        std::string positivePattern;
        std::string negativePattern;
        int minimumIntegerDigits = 1;
        int maximumIntegerDigits = INT_MAX;
        int minimumFractionDigits = 0;
        int maximumFractionDigits = 0;
        bool groupingUsed = false;
        char decimalSeparator = '.';
        char groupingSeparator = ',';
    };
    double applyRounding(double value, int scale) const;
public:
    DecimalFormat() = default;
    
    explicit DecimalFormat(const std::string& pattern);
    
    void applyPattern(const std::string& pattern);
    
    std::string toPattern() const;
    
    void applyLocalizedPattern(const std::string& pattern);
    
    std::string toLocalizedPattern() const {
        return fPattern;
    }
    
    void setPositivePrefix(const std::string& prefix) { fPositivePrefix = prefix; }
    void setPositiveSuffix(const std::string& suffix) { fPositiveSuffix = suffix; }
    void setNegativePrefix(const std::string& prefix) { fNegativePrefix = prefix; }
    void setNegativeSuffix(const std::string& suffix) { fNegativeSuffix = suffix; }
    
    std::string getPositivePrefix() const { return fPositivePrefix; }
    std::string getPositiveSuffix() const { return fPositiveSuffix; }
    std::string getNegativePrefix() const { return fNegativePrefix; }
    std::string getNegativeSuffix() const { return fNegativeSuffix; }
    
    std::string format(double number) const override;
    
    std::string format(int32_t number) const override {
        return format(static_cast<double>(number));
    }
    
    std::string format(int64_t number) const override {
        return format(static_cast<double>(number));
    }
    
    std::pair<double, size_t> parse(const std::string& text, size_t pos = 0) const override;
    
    void setRoundingMode(RoundingMode mode) {
        // 简化处理，实际实现需要更复杂的四舍五入逻辑
        fRoundingMode = mode;
    }
    
    RoundingMode getRoundingMode() const {
        return fRoundingMode;
    }

private:
    std::string formatNumber(double number) const;
    
    void parsePattern(const std::string& pattern);
    
    void analyzeSubpattern(const std::string& subpattern, bool isPositive);
};
}/*endof namespace*/
#endif/*__NUMBER_FORMAT_H__*/
