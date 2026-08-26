#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include <cmath>
#include <content/Locale.h>
#include <gui_features.h>
#ifdef ENABLE_I18N
#include <content/i18nbridge.h>
#include <content/i18n/locale_info.h>
#include <content/i18n/number_format.h>
#include <content/i18n/types.h>
#endif
#include <cctype>
#include <algorithm>
#include <regex>
#include <stdexcept>
#include <content/numberformat.h>
namespace cdroid{
// Lazily build the engine formatter for fLocale (default-locale factories
// stamp Locale::getDefault() at creation, java.text style). Returns false
// when i18n is compiled out or the engine has no data — callers then use
// the manual algorithm.
bool NumberFormat::ensureEngine() const {
#ifdef ENABLE_I18N
    if (fEngine != nullptr) return true;
    Locale loc = fHasLocale ? fLocale : Locale::getDefault();
    i18n::LocaleInfo info = I18nBridge::toLocaleInfo(loc);
    int status = 0;
    // LocaleInfo & — engine wants an lvalue
    i18n::NumberFormat* engine = new i18n::NumberFormat(info, status);
    if (status != 0 || !engine->Init()) {
        delete engine;
        return false;
    }
    fEngine = engine;
    return true;
#else
    return false;
#endif
}

// Zero-pad the integer part to fMinimumIntegerDigits: the engine exposes no
// integer-digit knob, so the face applies java.text's rule on its output
// (split on the localized decimal separator, pad, re-join).
std::string NumberFormat::padIntegerDigits(const std::string& s) const {
    if (fMinimumIntegerDigits <= 1) return s;
    size_t dotPos = s.find(fDecimalSeparator);
    const std::string integerPart = (dotPos == std::string::npos) ? s : s.substr(0, dotPos);
    const std::string rest = (dotPos == std::string::npos) ? std::string() : s.substr(dotPos);
    bool negative = (!integerPart.empty() && integerPart[0] == '-');
    size_t digits = integerPart.size() - (negative ? 1 : 0);
    if ((int)digits >= fMinimumIntegerDigits) return s;
    std::string padded = integerPart.substr(0, negative ? 1 : 0);
    padded.append(fMinimumIntegerDigits - (int)digits, '0');
    padded += integerPart.substr(negative ? 1 : 0);
    return padded + rest;
}

// java.text.NumberFormat.format: multiplier applies on the face (the engine's
// PERCENT type would double-scale), then the engine renders the CLDR pattern
// (localized separators AND real grouping sizes — hi-IN 2;2;3 etc.), with the
// fraction bounds pushed through Set{Min,Max}DecimalLength.
std::string NumberFormat::format(double number) const {
#ifdef ENABLE_I18N
    if (ensureEngine()) {
        fEngine->SetMinDecimalLength(fMinimumFractionDigits);
        fEngine->SetMaxDecimalLength(fMaximumFractionDigits);
        int status = 0;
        const double scaled = number * fMultiplier;
        std::string out = fGroupingUsed
                ? fEngine->Format(scaled, i18n::DECIMAL, status)
                : fEngine->FormatNoGroup(scaled, i18n::DECIMAL, status);
        if (status == 0 && !out.empty()) {
            // Zero-fraction configs: the engine keeps a trailing decimal
            // separator ("50,") — java.text integer output has none.
            if (fMaximumFractionDigits == 0 && out.size() >= fDecimalSeparator.size()
                    && out.compare(out.size() - fDecimalSeparator.size(),
                                   fDecimalSeparator.size(), fDecimalSeparator) == 0) {
                out.erase(out.size() - fDecimalSeparator.size());
            }
            return padIntegerDigits(std::move(out));
        }
    }
#endif
    return formatManual(number);
}

NumberFormat::~NumberFormat() {
#ifdef ENABLE_I18N
    delete fEngine;
#endif
}


// The pre-engine hand-rolled algorithm: DecimalFormat's path and the
// engine-failure fallback.
std::string NumberFormat::formatManual(double number) const {
    // java.text: format multiplies by the multiplier (percent=100 → "50" from
    // 0.5). The old unconditional "/100" scaled EVERY number down — plain
    // format(5) produced "0.050" — and paired with a compensating "*100" in
    // parse(). Zero in-tree consumers, so realigned to java.text semantics.
    double scaledNumber = number * fMultiplier;
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    int precision = (fMaximumFractionDigits == fMinimumFractionDigits) ? 
                    fMinimumFractionDigits : fMaximumFractionDigits;
    oss << std::fixed << std::setprecision(precision) << scaledNumber;
    
    std::string result = oss.str();
    
    // 处理小数部分长度
    size_t dotPos = result.find('.');
    if (dotPos != std::string::npos) {
        std::string integerPart = result.substr(0, dotPos);
        std::string fractionPart = result.substr(dotPos + 1);
        
        // 调整小数位数到最小要求
        while (static_cast<int>(fractionPart.length()) < fMinimumFractionDigits) {
            fractionPart += '0';
        }
        
        // 截断到最大允许的小数位数
        if (static_cast<int>(fractionPart.length()) > fMaximumFractionDigits) {
            fractionPart = fractionPart.substr(0, fMaximumFractionDigits);
        }
        
        result = integerPart + fDecimalSeparator + fractionPart;
    } else if (fMinimumFractionDigits > 0) {
        // 如果没有小数点但要求小数位
        result += fDecimalSeparator;
        for (int i = 0; i < fMinimumFractionDigits; ++i) {
            result += '0';
        }
    }
    
    // 应用千分位分隔符
    if (fGroupingUsed) {
        result = applyGrouping(result);
    }
    
    return result;
}

std::pair<double, size_t> NumberFormat::parse(const std::string& text, size_t pos) const {
    if (text.empty() || pos >= text.length()) {
        return {0.0, 0};
    }
    
    size_t start = pos;
    
    // 跳过前导空格
    while (pos < text.length() && std::isspace(text[pos])) {
        ++pos;
    }
    
    if (pos >= text.length()) {
        return {0.0, 0};
    }
    
    // 检查符号
    bool negative = false;
    if (text[pos] == '-' || text[pos] == '+') {
        negative = (text[pos] == '-');
        ++pos;
    }
    
    // 解析数字
    double result = 0.0;
    bool hasDecimalPoint = false;
    double fractionalDivisor = 1.0;
    
    while (pos < text.length()) {
        char c = text[pos];

        // Separators may be multi-byte UTF-8 (locale factories localize them);
        // match at the byte level instead of comparing single chars.
        auto sepAt = [&text, pos](const std::string& sep) -> bool {
            return !sep.empty() && text.compare(pos, sep.size(), sep) == 0;
        };

        if (sepAt(fDecimalSeparator)) {
            if (hasDecimalPoint || fParseIntegerOnly) break;
            hasDecimalPoint = true;
            pos += fDecimalSeparator.size();
        } else if (std::isdigit(c)) {
            int digit = c - '0';
            
            if (!hasDecimalPoint) {
                result = result * 10.0 + digit;
            } else {
                fractionalDivisor *= 10.0;
                result += digit / fractionalDivisor;
            }
            ++pos;
        } else if (sepAt(fGroupingSeparator) && fGroupingUsed) {
            pos += fGroupingSeparator.size();
        } else if (std::isspace(c)) {
            ++pos; // 跳过空格
        } else {
            break;
        }
    }
    
    if (pos == start) {
        return {0.0, 0};
    }
    
    result *= (negative ? -1.0 : 1.0);
    // java.text: parse divides by the multiplier (inverse of format's multiply).
    result = result / fMultiplier;
    
    return {result, pos - start};
}

std::unique_ptr<NumberFormat>  NumberFormat::getInstance() {
    return std::make_unique<NumberFormat>();
}

std::unique_ptr<NumberFormat>  NumberFormat::getCurrencyInstance() {
    auto nf = std::make_unique<NumberFormat>();
    nf->fLocale = Locale::getDefault();
    nf->fHasLocale = true;
    nf->setMinimumFractionDigits(2);
    nf->setMaximumFractionDigits(2);
    return nf;
}

std::unique_ptr<NumberFormat>  NumberFormat::getPercentInstance() {
    auto nf = std::make_unique<NumberFormat>();
    nf->fLocale = Locale::getDefault();
    nf->fHasLocale = true;
    nf->setMultiplier(100);
    nf->setMinimumFractionDigits(0);
    nf->setMaximumFractionDigits(0);
    return nf;
}

std::unique_ptr<NumberFormat>  NumberFormat::getIntegerInstance() {
    auto nf = std::make_unique<NumberFormat>();
    nf->fLocale = Locale::getDefault();
    nf->fHasLocale = true;
    nf->setMinimumFractionDigits(0);
    nf->setMaximumFractionDigits(0);
    nf->setParseIntegerOnly(true);
    return nf;
}

// Locale overloads (java.text.NumberFormat): build the same instance as the
// no-locale factory, then localize the separators from the i18n CLDR data
// through I18nBridge (the vendored i18n engine stays untouched).
// ENABLE_I18N off → separators stay '.'/',''.
void NumberFormat::applyLocaleSeparators(NumberFormat* nf, const Locale& inLocale)
{
    nf->fLocale = inLocale;
    nf->fHasLocale = true;
#ifdef ENABLE_I18N
    const std::string dec = I18nBridge::decimalSeparator(inLocale);
    const std::string grp = I18nBridge::groupingSeparator(inLocale);
    if (!dec.empty()) nf->fDecimalSeparator = dec;
    if (!grp.empty()) nf->fGroupingSeparator = grp;
#else
    (void)nf; (void)inLocale;
#endif
}

std::unique_ptr<NumberFormat>  NumberFormat::getInstance(const Locale& inLocale) {
    auto nf = getInstance();
    applyLocaleSeparators(nf.get(), inLocale);
    return nf;
}

std::unique_ptr<NumberFormat>  NumberFormat::getNumberInstance(const Locale& inLocale) {
    // java.text: getNumberInstance(Locale) ≡ getInstance(Locale).
    return getInstance(inLocale);
}

std::unique_ptr<NumberFormat>  NumberFormat::getCurrencyInstance(const Locale& inLocale) {
    auto nf = getCurrencyInstance();
    applyLocaleSeparators(nf.get(), inLocale);
    return nf;
}

std::unique_ptr<NumberFormat>  NumberFormat::getPercentInstance(const Locale& inLocale) {
    auto nf = getPercentInstance();
    applyLocaleSeparators(nf.get(), inLocale);
    return nf;
}

std::unique_ptr<NumberFormat>  NumberFormat::getIntegerInstance(const Locale& inLocale) {
    auto nf = getIntegerInstance();
    applyLocaleSeparators(nf.get(), inLocale);
    return nf;
}

std::string  NumberFormat::applyGrouping(const std::string& input) const {
    size_t dotPos = input.find(fDecimalSeparator);
    std::string integerPart = (dotPos != std::string::npos) ? 
                              input.substr(0, dotPos) : input;
    std::string fractionPart = (dotPos != std::string::npos) ? 
                               input.substr(dotPos) : "";
    
    std::reverse(integerPart.begin(), integerPart.end());
    
    std::string grouped;
    int count = 0;
    
    for (char c : integerPart) {
        if (count > 0 && count % 3 == 0) {
            grouped += fGroupingSeparator;
        }
        grouped += c;
        count++;
    }
    
    std::reverse(grouped.begin(), grouped.end());
    
    return grouped + fractionPart;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//class DecimalFormat : public NumberFormat {
    
DecimalFormat::DecimalFormat(const std::string& pattern) {
    applyPattern(pattern);
}

void DecimalFormat::applyPattern(const std::string& pattern) {
    fPattern = pattern;
    parsePattern(pattern);
}

std::string DecimalFormat::toPattern() const {
    return fPattern;
}

void DecimalFormat::applyLocalizedPattern(const std::string& pattern) {
    applyPattern(pattern);
}

double DecimalFormat::applyRounding(double value, int scale) const {
    if (scale < 0) return value;
    
    double multiplier = std::pow(10.0, scale);
    double shifted = value * multiplier;
    
    switch (fRoundingMode) {
        case HALF_UP: {
            double remainder = shifted - std::floor(shifted);
            return (remainder >= 0.5) ? std::ceil(shifted) / multiplier : std::floor(shifted) / multiplier;
        }
        case HALF_DOWN: {
            double remainder = shifted - std::floor(shifted);
            return (remainder > 0.5) ? std::ceil(shifted) / multiplier : std::floor(shifted) / multiplier;
        }
        case UP: {
            return (value >= 0) ? std::ceil(shifted) / multiplier : std::floor(shifted) / multiplier;
        }
        case DOWN: {
            return (value >= 0) ? std::floor(shifted) / multiplier : std::ceil(shifted) / multiplier;
        }
        case CEILING: {
            return std::ceil(shifted) / multiplier;
        }
        case FLOOR: {
            return std::floor(shifted) / multiplier;
        }
        case HALF_EVEN: {
            double floor_val = std::floor(shifted);
            double remainder = shifted - floor_val;
            if (remainder > 0.5 || (remainder == 0.5 && static_cast<long long>(floor_val) % 2 != 0)) {
                return std::ceil(shifted) / multiplier;
            } else {
                return floor_val / multiplier;
            }
        }
        case UNNECESSARY: {
            double rounded = std::round(shifted);
            if (std::abs(shifted - rounded) > 1e-10) {
                throw std::runtime_error("Rounding necessary but UNNECESSARY rounding mode specified");
            }
            return rounded / multiplier;
        }
    }
    return shifted / multiplier;
}

std::string DecimalFormat::format(double number) const{
    std::string formatted;
    const int precision = (fMaximumFractionDigits == fMinimumFractionDigits) ? 
        fMinimumFractionDigits : fMaximumFractionDigits;
    number = applyRounding(number, precision);
    if (number < 0) {
        formatted = fNegativePrefix + formatNumber(-number) + fNegativeSuffix;
    } else {
        formatted = fPositivePrefix + formatNumber(number) + fPositiveSuffix;
    }
    
    return formatted;
}

std::pair<double, size_t> DecimalFormat::parse(const std::string& text, size_t pos) const{
    if (text.empty() || pos >= text.length()) {
        return {0.0, 0};
    }
    
    size_t originalPos = pos;
    
    // 检查前缀
    std::string prefixToCheck = (text[pos] == '-') ? fNegativePrefix : fPositivePrefix;
    if (text.substr(pos, prefixToCheck.length()) == prefixToCheck) {
        pos += prefixToCheck.length();
    }
    
    // 使用父类解析数字
    auto v/*[value, consumed]*/ = NumberFormat::parse(text, pos);
    pos += v.second/*consumed*/;
    
    // 检查后缀
    std::string suffixToCheck = (v.first/*value*/ < 0) ? fNegativeSuffix : fPositiveSuffix;
    if (pos + suffixToCheck.length() <= text.length() &&
        text.substr(pos, suffixToCheck.length()) == suffixToCheck) {
        pos += suffixToCheck.length();
    }
    
    return {v.first/*value*/, pos - originalPos};
}
    
std::string DecimalFormat::formatNumber(double number) const {
    double scaledNumber = number * fMultiplier / 100.0;
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    int precision = (fMaximumFractionDigits == fMinimumFractionDigits) ? 
                    fMinimumFractionDigits : fMaximumFractionDigits;
    oss << std::fixed << std::setprecision(precision) << scaledNumber;
    
    std::string result = oss.str();
    
    // 处理小数部分
    size_t dotPos = result.find('.');
    if (dotPos != std::string::npos) {
        std::string integerPart = result.substr(0, dotPos);
        std::string fractionPart = result.substr(dotPos + 1);
        
        // 调整小数位数
        while (static_cast<int>(fractionPart.length()) < fMinimumFractionDigits) {
            fractionPart += '0';
        }
        if (static_cast<int>(fractionPart.length()) > fMaximumFractionDigits) {
            fractionPart = fractionPart.substr(0, fMaximumFractionDigits);
        }
        
        result = integerPart + fDecimalSeparator + fractionPart;
    } else if (fMinimumFractionDigits > 0) {
        result += fDecimalSeparator;
        for (int i = 0; i < fMinimumFractionDigits; ++i) {
            result += '0';
        }
    }
    
    // 应用千分位分隔符
    if (fGroupingUsed) {
        result = applyGrouping(result);
    }
    
    return result;
}
    
void DecimalFormat::parsePattern(const std::string& pattern) {
    // 简化的模式解析
    size_t semicolonPos = pattern.find(';');
    std::string positivePattern = (semicolonPos != std::string::npos) ? 
                                 pattern.substr(0, semicolonPos) : pattern;
    std::string negativePattern = (semicolonPos != std::string::npos) ? 
                                 pattern.substr(semicolonPos + 1) : "-" + positivePattern;
    
    // 分析正数模式
    analyzeSubpattern(positivePattern, true);
    analyzeSubpattern(negativePattern, false);
}

void DecimalFormat::analyzeSubpattern(const std::string& subpattern, bool isPositive) {
    // 简化的子模式分析
    int integerDigits = 0, fractionDigits = 0;
    bool inFraction = false;
    bool hasGrouping = false;
    
    for (char c : subpattern) {
        if (c == '#') {
            if (!inFraction) {
                integerDigits++;
            } else {
                fractionDigits++;
            }
        } else if (c == '0') {
            if (!inFraction) {
                integerDigits++;
            } else {
                fractionDigits++;
            }
        } else if (c == '.') {
            inFraction = true;
        } else if (c == ',') {
            hasGrouping = true;
        }
    }
    
    if (isPositive) {
        fMinimumIntegerDigits = integerDigits;
        fMinimumFractionDigits = fractionDigits;
        fGroupingUsed = hasGrouping;
    }
}

}

