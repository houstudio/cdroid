#ifndef __UNICODE_RANGE_DATA_H__
#define __UNICODE_RANGE_DATA_H__

#include "unicode/uchar.h"
#include "unicode/uscript.h"
#include "unicode/ubidi.h"

// 属性映射宏（将 ICU UProperty 值转换为内部位图索引）
// 二进制属性从 0 开始，直接使用属性值作为索引
#define UPROPERTY_TO_INDEX(prop)  ((prop) < UCHAR_BINARY_LIMIT ? (prop) : -1)

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
    uint8_t wbp;            // 1 字节：UAX#29 Word_Break property (matches ubrk.cpp WBProperty enum)
    uint64_t binaryProps;   // 8 字节：二进制属性位图（支持 UCHAR_BINARY_START 到 UCHAR_BINARY_LIMIT-1）
    uint64_t binaryProps2;  // 8 字节：扩展属性位图（保留）
    
    // 辅助函数
    inline bool hasBinaryProperty(int propIndex) const {
        if (propIndex >= 0 && propIndex < 64) {
            return (binaryProps & (1ULL << propIndex)) != 0;
        } else if (propIndex >= 64 && propIndex < 128) {
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
        return hasBinaryProperty(UCHAR_EXTENDED_PICTOGRAPHIC);
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

// ========== Canonical (NFD) Decomposition ==========

// 单个 NFD 分解条目：把一个源码点分解为 1~4 个 BMP code unit。
// decompLen 为 0 表示该码点无规范分解。
struct NfdMapping {
    uint32_t cp;          // 源码点
    uint16_t decomp[4];   // 分解结果（BMP code unit）
    uint8_t  decompLen;   // 有效 code unit 个数（0..4）
};

// 全局 NFD 分解表声明（按 cp 升序排列，支持二分查找）
extern const NfdMapping g_nfdMappings[];
extern const int g_nfdMappingsCount;

// 二分查找：返回匹配条目指针，未找到返回 nullptr
const NfdMapping* findNfdMapping(UChar32 c);

#endif // __UNICODE_RANGE_DATA_H__