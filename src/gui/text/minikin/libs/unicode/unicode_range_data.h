// Unicode 范围数据表头文件
#ifndef __UNICODE_RANGE_DATA_H__
#define __UNICODE_RANGE_DATA_H__

#include "unicode/uchar.h"
#include "unicode/uscript.h"
#include "unicode/ubidi.h"

// Unicode 属性索引常量（与 ICU 兼容）
// 注意：这里定义的是内部位图索引（0-63），与 uchar.h 中的 UProperty 值不同
// 使用时需要通过 hasBinaryProperty 方法进行转换
#define URB_WHITE_SPACE           0
#define URB_BIDI_MIRRORED         1
#define URB_ALPHABETIC            2
#define URB_NUMERIC               3
#define URB_BIDI_CONTROL          4
#define URB_JOIN_CONTROL          5
#define URB_DASH                  6
#define URB_HYPHEN                7
#define URB_LOWERCASE             8
#define URB_UPPERCASE             9
#define URB_TITLECASE             10
#define URB_IDEOGRAPHIC           11
#define URB_DIACRITIC             12
#define URB_EXTENDER              13
#define URB_BREAK_WHITESPACE      14
#define URB_HEX_DIGIT             15
#define URB_ASCII_HEX_DIGIT       16
#define URB_ALPHANUMERIC          17
#define URB_PUNCTUATION           18
#define URB_MATH                  19
#define URB_LOWERCASE_LETTER      20
#define URB_UPPERCASE_LETTER      21
#define URB_IDEOGRAPHIC_LETTER    22
#define URB_NONCHARACTER_CODE_POINT 23
#define URB_DEFAULT_IGNORABLE_CODE_POINT 24
#define URB_DEPRECATED            25
#define URB_SOFTHYPHEN            26
#define URB_QUOTATION_MARK        27
#define URB_TERMINAL_PUNCTUATION   28
#define URB_SEGMENT_STARTER       29
#define URB_XID_START             30
#define URB_XID_CONTINUE          31

// 扩展属性 (32-63)
#define URB_GRAPHEME_EXTEND       32
#define URB_GRAPHEME_LINK         33
#define URB_IDS_BINARY_OPERATOR    34
#define URB_IDS_TRINARY_OPERATOR   35
#define URB_RADICAL                36
#define URB_UNIFIED_IDEOGRAPH      37
#define URB_VARIATION_SELECTOR    38
#define URB_PATTERN_SYNTAX         39
#define URB_PATTERN_WHITE_SPACE    40
#define URB_PREPENDED_CONCATENATION_MARK 41

// 属性映射宏（将 ICU UProperty 值转换为内部位图索引）
#define UPROPERTY_TO_INDEX(prop)  ((prop) - 0x100)

// Unicode 范围属性结构（24 字节，紧凑对齐）
struct UnicodeRange {
    UChar32 start;          // 4 字节：范围起始
    UChar32 end;            // 4 字节：范围结束
    uint8_t directionality; // 1 字节：UCharDirection
    uint8_t category;       // 1 字节：UCharCategory
    uint8_t gcb;            // 1 字节：UGraphemeClusterBreak
    uint8_t lb;             // 1 字节：ULineBreak
    uint8_t ccc;            // 1 字节：规范组合类
    uint8_t script;         // 1 字节：UScriptCode
    uint64_t binaryProps;   // 8 字节：二进制属性位图（支持 0-63）
    uint64_t binaryProps2;  // 8 字节：扩展属性位图（支持 64-127）
    
    // 辅助函数
    inline bool hasBinaryProperty(int propIndex) const {
        if (propIndex < 64) {
            return (binaryProps & (1ULL << propIndex)) != 0;
        } else if (propIndex < 128) {
            return (binaryProps2 & (1ULL << (propIndex - 64))) != 0;
        }
        return false;
    }
    
    inline bool isMirrored() const {
        return hasBinaryProperty(UCHAR_BIDI_MIRRORED);
    }
    
    inline bool isWhiteSpace() const {
        return hasBinaryProperty(UCHAR_WHITE_SPACE);
    }
    
    inline bool isExtendedPic() const {
        // UCHAR_EXTENDED_PICTOGRAPHIC = 151，超出当前范围，返回 false
        // 需要支持的话需要扩展到第三块
        return false;
    }
};

// 全局范围表声明
extern const UnicodeRange g_unicodeRanges[];
extern const int g_unicodeRangesCount;

// 查找函数声明
const UnicodeRange* findUnicodeRange(UChar32 c);

// ========== 大小写转换功能 ==========

// ASCII 快速转换（内联函数，零空间开销）
inline UChar32 toLower(UChar32 c) {
    if (c >= 'A' && c <= 'Z') return c | 0x20;
    return c;
}

inline UChar32 toUpper(UChar32 c) {
    if (c >= 'a' && c <= 'z') return c & ~0x20;
    return c;
}

// 特殊字符大小写映射结构（用于非ASCII字符）
struct CaseMapping {
    uint16_t from;   // 原始字符（U+0080 以上）
    uint16_t lower;  // 小写形式
    uint16_t upper;  // 大写形式
};

// 大小写映射数据表声明
extern const CaseMapping g_caseMappings[];
extern const int g_caseMappingsCount;

// 完整大小写转换函数
UChar32 toLowerFull(UChar32 c);
UChar32 toUpperFull(UChar32 c);

#endif // __UNICODE_RANGE_DATA_H__
