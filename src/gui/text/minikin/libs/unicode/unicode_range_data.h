// Unicode 范围数据表头文件
#ifndef __UNICODE_RANGE_DATA_H__
#define __UNICODE_RANGE_DATA_H__

#include "unicode/uchar.h"
#include "unicode/uscript.h"
#include "unicode/ubidi.h"

// Unicode 范围属性结构（16 字节，紧凑对齐）
struct UnicodeRange {
    UChar32 start;          // 4 字节：范围起始
    UChar32 end;            // 4 字节：范围结束
    uint8_t directionality; // 1 字节：UCharDirection
    uint8_t category;       // 1 字节：UCharCategory
    uint8_t gcb;            // 1 字节：UGraphemeClusterBreak
    uint8_t lb;             // 1 字节：ULineBreak
    uint8_t ccc;            // 1 字节：规范组合类
    uint8_t script;         // 1 字节：UScriptCode
    uint16_t binaryProps;   // 2 字节：二进制属性位图
    
    // 辅助函数
    inline bool hasBinaryProperty(int propIndex) const {
        return (binaryProps & (1u << propIndex)) != 0;
    }
    
    inline bool isMirrored() const {
        return hasBinaryProperty(1);  // UCHAR_BIDI_MIRRORED = 1
    }
    
    inline bool isExtendedPic() const {
        return hasBinaryProperty(151);  // UCHAR_EXTENDED_PICTOGRAPHIC = 151
    }
};

// 全局范围表声明
extern const UnicodeRange g_unicodeRanges[];
extern const int g_unicodeRangesCount;

// 查找函数声明
const UnicodeRange* findUnicodeRange(UChar32 c);

#endif // __UNICODE_RANGE_DATA_H__
