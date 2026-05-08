#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include <cmath>
#include <cctype>
#include <algorithm>
#include <regex>
#include <stdexcept>
#include <core/numberformat.h>
namespace cdroid{

std::string NumberFormat::format(double number) const {
    double scaledNumber = number * fMultiplier / 100.0;
    
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
        
        if (c == fDecimalSeparator) {
            if (hasDecimalPoint || fParseIntegerOnly) break;
            hasDecimalPoint = true;
            ++pos;
        } else if (std::isdigit(c)) {
            int digit = c - '0';
            
            if (!hasDecimalPoint) {
                result = result * 10.0 + digit;
            } else {
                fractionalDivisor *= 10.0;
                result += digit / fractionalDivisor;
            }
            ++pos;
        } else if (c == fGroupingSeparator && fGroupingUsed) {
            ++pos;
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
    result = result * 100.0 / fMultiplier;
    
    return {result, pos - start};
}

std::unique_ptr<NumberFormat>  NumberFormat::getInstance() {
    return std::make_unique<NumberFormat>();
}

std::unique_ptr<NumberFormat>  NumberFormat::getCurrencyInstance() {
    auto nf = std::make_unique<NumberFormat>();
    nf->setMinimumFractionDigits(2);
    nf->setMaximumFractionDigits(2);
    return nf;
}

std::unique_ptr<NumberFormat>  NumberFormat::getPercentInstance() {
    auto nf = std::make_unique<NumberFormat>();
    nf->setMultiplier(100);
    nf->setMinimumFractionDigits(0);
    nf->setMaximumFractionDigits(0);
    return nf;
}

std::unique_ptr<NumberFormat>  NumberFormat::getIntegerInstance() {
    auto nf = std::make_unique<NumberFormat>();
    nf->setMinimumFractionDigits(0);
    nf->setMaximumFractionDigits(0);
    nf->setParseIntegerOnly(true);
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

