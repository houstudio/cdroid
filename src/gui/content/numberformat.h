#ifndef __NUMBER_FORMAT_H__
#define __NUMBER_FORMAT_H__
#include <string>
#include <content/Locale.h>
#include <sstream>
#include <iomanip>
#include <memory>
#include <cmath>
#include <climits>
#include <cctype>
#include <algorithm>
#include <regex>
#include <stdexcept>

namespace i18n { class NumberFormat; }   // vendored engine (global ns) — opaque here

namespace cdroid{
class NumberFormat {
protected:
    int fMinimumIntegerDigits = 1;
    int fMaximumIntegerDigits = INT_MAX;
    int fMinimumFractionDigits = 0;
    int fMaximumFractionDigits = 3;
    bool fGroupingUsed = true;
    // String, not char: locale separators from CLDR can be multi-byte UTF-8
    // (ar decimal "٫" U+066B, fr grouping U+202F). The Locale factory
    // overloads localize these through the i18n engine.
    std::string fDecimalSeparator = ".";
    std::string fGroupingSeparator = ",";
    int fMultiplier = 1;
    bool fParseIntegerOnly = false;
    // The locale this formatter was built for (java factories use the
    // default locale) and the lazily-built engine formatter behind format():
    // CLDR patterns (grouping sizes, localized separators) come from the
    // vendored i18n engine; parse()/integer-digit bounds stay face-local.
    Locale fLocale;
    bool fHasLocale = false;
    mutable i18n::NumberFormat* fEngine = nullptr;
    
public:
    virtual ~NumberFormat();
    
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

    // java.text.NumberFormat locale overloads: same factories, localized
    // decimal/grouping separators from the i18n CLDR data (build with
    // ENABLE_I18N; without it the Locale is accepted but separators stay
    // '.'/','). Signatures match java.text.NumberFormat verbatim.
    static std::unique_ptr<NumberFormat> getInstance(const Locale& inLocale);
    static std::unique_ptr<NumberFormat> getNumberInstance(const Locale& inLocale);
    static std::unique_ptr<NumberFormat> getCurrencyInstance(const Locale& inLocale);
    static std::unique_ptr<NumberFormat> getPercentInstance(const Locale& inLocale);
    static std::unique_ptr<NumberFormat> getIntegerInstance(const Locale& inLocale);
protected:
    std::string applyGrouping(const std::string& input) const;
    static void applyLocaleSeparators(NumberFormat* nf, const Locale& inLocale);
    // Pre-format() engine setup; false when i18n is off/failed (manual path).
    bool ensureEngine() const;
    // The pre-engine hand-rolled algorithm (fallback + DecimalFormat reuse).
    std::string formatManual(double number) const;
    // Zero-pad the integer part to fMinimumIntegerDigits (engine has no knob).
    std::string padIntegerDigits(const std::string& s) const;
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
