#include "unicode/uchar.h"
#include "unicode/uscript.h"
#include "unicode/ubidi.h"
#include "unicode_range_data.h"
#include <algorithm>

// Unicode 范围数据表（精简版，覆盖常用字符）

const UnicodeRange g_unicodeRanges[] = {
    // 控制字符 (0x0000-0x001F)
    {0x0000, 0x001F, U_OTHER_NEUTRAL, U_CONTROL, U_GCB_CONTROL, U_LB_MANDATORY_BREAK, 0, USCRIPT_COMMON, 0, 0},
    
    // 空格 (0x0020)
    {0x0020, 0x0020, U_WHITE_SPACE_NEUTRAL, U_SPACE_SEPARATOR, U_GCB_OTHER, U_LB_SPACE, 0, USCRIPT_COMMON, (1ULL << URB_WHITE_SPACE), 0},
    
    // 标点符号 (!") (0x0021-0x0022)
    {0x0021, 0x0022, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_EXCLAMATION, 0, USCRIPT_COMMON, (1ULL << URB_PUNCTUATION), 0},
    
    // #$%
    {0x0023, 0x0023, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_COMMON, (1ULL << URB_PUNCTUATION), 0},
    {0x0024, 0x0024, U_EUROPEAN_NUMBER_TERMINATOR, U_CURRENCY_SYMBOL, U_GCB_OTHER, U_LB_PREFIX_NUMERIC, 0, USCRIPT_COMMON, 0, 0},
    {0x0025, 0x0025, U_EUROPEAN_NUMBER_TERMINATOR, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_POSTFIX_NUMERIC, 0, USCRIPT_COMMON, (1ULL << URB_PUNCTUATION), 0},
    
    // &'
    {0x0026, 0x0027, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_COMMON, (1ULL << URB_PUNCTUATION), 0},
    
    // 括号 ()
    {0x0028, 0x0028, U_OTHER_NEUTRAL, U_OPEN_PUNCTUATION, U_GCB_OTHER, U_LB_OPEN_PUNCTUATION, 0, USCRIPT_COMMON, (1ULL << URB_BIDI_MIRRORED) | (1ULL << URB_PUNCTUATION), 0},
    {0x0029, 0x0029, U_OTHER_NEUTRAL, U_CLOSE_PUNCTUATION, U_GCB_OTHER, U_LB_CLOSE_PARENTHESIS, 0, USCRIPT_COMMON, (1ULL << URB_BIDI_MIRRORED) | (1ULL << URB_PUNCTUATION), 0},
    
    // *+
    {0x002A, 0x002A, U_OTHER_NEUTRAL, U_MATH_SYMBOL, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_COMMON, (1ULL << URB_MATH), 0},
    {0x002B, 0x002B, U_EUROPEAN_NUMBER_SEPARATOR, U_MATH_SYMBOL, U_GCB_OTHER, U_LB_INFIX_NUMERIC, 0, USCRIPT_COMMON, (1ULL << URB_MATH), 0},
    
    // ,
    {0x002C, 0x002C, U_COMMON_NUMBER_SEPARATOR, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_INFIX_NUMERIC, 0, USCRIPT_COMMON, (1ULL << URB_PUNCTUATION), 0},
    
    // -
    {0x002D, 0x002D, U_EUROPEAN_NUMBER_SEPARATOR, U_DASH_PUNCTUATION, U_GCB_OTHER, U_LB_HYPHEN, 0, USCRIPT_COMMON, (1ULL << URB_DASH) | (1ULL << URB_HYPHEN), 0},
    
    // .
    {0x002E, 0x002E, U_COMMON_NUMBER_SEPARATOR, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_INFIX_NUMERIC, 0, USCRIPT_COMMON, (1ULL << URB_PUNCTUATION), 0},
    
    // /
    {0x002F, 0x002F, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_BREAK_SYMBOLS, 0, USCRIPT_COMMON, (1ULL << URB_PUNCTUATION), 0},
    
    // 数字 0-9
    {0x0030, 0x0039, U_EUROPEAN_NUMBER, U_DECIMAL_DIGIT_NUMBER, U_GCB_OTHER, U_LB_NUMERIC, 0, USCRIPT_COMMON, (1ULL << URB_NUMERIC) | (1ULL << URB_HEX_DIGIT) | (1ULL << URB_ASCII_HEX_DIGIT) | (1ULL << URB_ALPHANUMERIC), 0},
    
    // :;
    {0x003A, 0x003A, U_EUROPEAN_NUMBER_SEPARATOR, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_INFIX_NUMERIC, 0, USCRIPT_COMMON, (1ULL << URB_PUNCTUATION), 0},
    {0x003B, 0x003B, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_INFIX_NUMERIC, 0, USCRIPT_COMMON, (1ULL << URB_PUNCTUATION), 0},
    
    // <=>
    {0x003C, 0x003E, U_OTHER_NEUTRAL, U_MATH_SYMBOL, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_COMMON, (1ULL << URB_BIDI_MIRRORED) | (1ULL << URB_MATH), 0},
    
    // ?@
    {0x003F, 0x0040, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_COMMON, (1ULL << URB_PUNCTUATION), 0},
    
    // 大写字母 A-Z
    {0x0041, 0x005A, U_LEFT_TO_RIGHT, U_UPPERCASE_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_LATIN, (1ULL << URB_ALPHABETIC) | (1ULL << URB_UPPERCASE) | (1ULL << URB_UPPERCASE_LETTER) | (1ULL << URB_ALPHANUMERIC) | (1ULL << URB_XID_START), 0},
    
    // [\]^_`
    {0x005B, 0x0060, U_OTHER_NEUTRAL, U_OPEN_PUNCTUATION, U_GCB_OTHER, U_LB_OPEN_PUNCTUATION, 0, USCRIPT_COMMON, (1ULL << URB_PUNCTUATION), 0},
    
    // 小写字母 a-z
    {0x0061, 0x007A, U_LEFT_TO_RIGHT, U_LOWERCASE_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_LATIN, (1ULL << URB_ALPHABETIC) | (1ULL << URB_LOWERCASE) | (1ULL << URB_LOWERCASE_LETTER) | (1ULL << URB_ALPHANUMERIC) | (1ULL << URB_XID_START), 0},
    
    // {|}~
    {0x007B, 0x007E, U_OTHER_NEUTRAL, U_CLOSE_PUNCTUATION, U_GCB_OTHER, U_LB_CLOSE_PUNCTUATION, 0, USCRIPT_COMMON, (1ULL << URB_BIDI_MIRRORED) | (1ULL << URB_PUNCTUATION), 0},
    
    // DEL 和控制字符 (0x007F-0x009F)
    {0x007F, 0x009F, U_OTHER_NEUTRAL, U_CONTROL, U_GCB_CONTROL, U_LB_MANDATORY_BREAK, 0, USCRIPT_COMMON, 0, 0},
    
    // 不间断空格 (0x00A0)
    {0x00A0, 0x00A0, U_WHITE_SPACE_NEUTRAL, U_SPACE_SEPARATOR, U_GCB_OTHER, U_LB_BREAK_BOTH, 0, USCRIPT_COMMON, (1ULL << URB_WHITE_SPACE), 0},
    
    // 上标、下标等
    {0x00AA, 0x00AA, U_OTHER_NEUTRAL, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_LATIN, (1ULL << URB_ALPHABETIC) | (1ULL << URB_LOWERCASE_LETTER), 0},
    {0x00B5, 0x00B5, U_OTHER_NEUTRAL, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_LATIN, (1ULL << URB_ALPHABETIC), 0},
    
    // 中间点
    {0x00B7, 0x00B7, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_COMMON, (1ULL << URB_PUNCTUATION), 0},
    
    {0x00BA, 0x00BA, U_OTHER_NEUTRAL, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_LATIN, (1ULL << URB_ALPHABETIC) | (1ULL << URB_LOWERCASE_LETTER), 0},
    
    // 组合变音符号 (0x0300-0x036F)
    {0x0300, 0x036F, U_DIR_NON_SPACING_MARK, U_NON_SPACING_MARK, U_GCB_EXTEND, U_LB_COMBINING_MARK, 0, USCRIPT_INHERITED, (1ULL << URB_DIACRITIC), 0},
    
    // 希腊文 (0x0370-0x03FF)
    {0x0370, 0x03FF, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_GREEK, (1ULL << URB_ALPHABETIC) | (1ULL << URB_XID_START), 0},
    
    // 西里尔文 (0x0400-0x04FF)
    {0x0400, 0x04FF, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_CYRILLIC, (1ULL << URB_ALPHABETIC) | (1ULL << URB_XID_START), 0},
    
    // 希伯来文 (0x0590-0x05FF)
    {0x0590, 0x05FF, U_RIGHT_TO_LEFT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_HEBREW_LETTER, 0, USCRIPT_HEBREW, (1ULL << URB_ALPHABETIC) | (1ULL << URB_XID_START), 0},
    
    // 阿拉伯文 (0x0600-0x06FF)
    {0x0600, 0x06FF, U_RIGHT_TO_LEFT_ARABIC, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ARABIC_LETTER, 0, USCRIPT_ARABIC, (1ULL << URB_ALPHABETIC) | (1ULL << URB_XID_START), 0},
    
    // CJK 统一表意文字 (0x4E00-0x9FFF)
    {0x4E00, 0x9FFF, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_IDEOGRAPHIC, 0, USCRIPT_HAN, (1ULL << URB_ALPHABETIC) | (1ULL << URB_IDEOGRAPHIC) | (1ULL << URB_UNIFIED_IDEOGRAPH) | (1ULL << URB_XID_START), 0},
    
    // 平假名 (0x3040-0x309F)
    {0x3040, 0x309F, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_IDEOGRAPHIC, 0, USCRIPT_HIRAGANA, (1ULL << URB_ALPHABETIC) | (1ULL << URB_IDEOGRAPHIC) | (1ULL << URB_XID_START), 0},
    
    // 片假名 (0x30A0-0x30FF)
    {0x30A0, 0x30FF, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_IDEOGRAPHIC, 0, USCRIPT_KATAKANA, (1ULL << URB_ALPHABETIC) | (1ULL << URB_IDEOGRAPHIC) | (1ULL << URB_XID_START), 0},
    
    // Hangul 音节 (0xAC00-0xD7AF)
    {0xAC00, 0xD7AF, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_LV, U_LB_LV, 0, USCRIPT_HANGUL, (1ULL << URB_ALPHABETIC) | (1ULL << URB_IDEOGRAPHIC) | (1ULL << URB_XID_START), 0},
    
    // Emoji (0x1F600-0x1F64F)
    {0x1F600, 0x1F64F, U_OTHER_NEUTRAL, U_OTHER_SYMBOL, U_GCB_OTHER, U_LB_IDEOGRAPHIC, 0, USCRIPT_COMMON, (1ULL << URB_IDEOGRAPHIC), 0},
    
    // Emoji (0x1F300-0x1F5FF)
    {0x1F300, 0x1F5FF, U_OTHER_NEUTRAL, U_OTHER_SYMBOL, U_GCB_OTHER, U_LB_IDEOGRAPHIC, 0, USCRIPT_COMMON, (1ULL << URB_IDEOGRAPHIC), 0},
    
    // 区域指示符号 (0x1F1E0-0x1F1FF)
    {0x1F1E0, 0x1F1FF, U_OTHER_NEUTRAL, U_OTHER_SYMBOL, U_GCB_REGIONAL_INDICATOR, U_LB_REGIONAL_INDICATOR, 0, USCRIPT_COMMON, 0, 0},
};

const int g_unicodeRangesCount = sizeof(g_unicodeRanges) / sizeof(g_unicodeRanges[0]);

const UnicodeRange* findUnicodeRange(UChar32 c) {
    // 处理无效码点
    if (c < 0 || c > 0x10FFFF) {
        static const UnicodeRange invalidRange = {
            0, 0, U_OTHER_NEUTRAL, U_UNASSIGNED, U_GCB_OTHER, U_LB_AMBIGUOUS, 0, USCRIPT_UNKNOWN, 0
        };
        return &invalidRange;
    }
    
    // 二分查找
    int left = 0;
    int right = g_unicodeRangesCount - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        const auto& range = g_unicodeRanges[mid];
        
        if (c >= range.start && c <= range.end) {
            return &range;
        } else if (c < range.start) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    
    // 返回默认范围
    static const UnicodeRange defaultRange = {
        0, 0, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_UNKNOWN, 0
    };
    return &defaultRange;
}

// ========== 大小写转换数据和函数 ==========

// 非ASCII字符大小写映射表（按字符值排序，用于二分查找）
// 覆盖常用的拉丁扩展字符、希腊字母、西里尔字母等
const CaseMapping g_caseMappings[] = {
    // Latin-1 补充 (0x00C0-0x00FF)
    {0x00C0, 0x00E0, 0x00C0},  // À -> à
    {0x00C1, 0x00E1, 0x00C1},  // Á -> á
    {0x00C2, 0x00E2, 0x00C2},  // Â -> â
    {0x00C3, 0x00E3, 0x00C3},  // Ã -> ã
    {0x00C4, 0x00E4, 0x00C4},  // Ä -> ä
    {0x00C5, 0x00E5, 0x00C5},  // Å -> å
    {0x00C6, 0x00E6, 0x00C6},  // Æ -> æ
    {0x00C7, 0x00E7, 0x00C7},  // Ç -> ç
    {0x00C8, 0x00E8, 0x00C8},  // È -> è
    {0x00C9, 0x00E9, 0x00C9},  // É -> é
    {0x00CA, 0x00EA, 0x00CA},  // Ê -> ê
    {0x00CB, 0x00EB, 0x00CB},  // Ë -> ë
    {0x00CC, 0x00EC, 0x00CC},  // Ì -> ì
    {0x00CD, 0x00ED, 0x00CD},  // Í -> í
    {0x00CE, 0x00EE, 0x00CE},  // Î -> î
    {0x00CF, 0x00EF, 0x00CF},  // Ï -> ï
    {0x00D0, 0x00F0, 0x00D0},  // Ð -> ð
    {0x00D1, 0x00F1, 0x00D1},  // Ñ -> ñ
    {0x00D2, 0x00F2, 0x00D2},  // Ò -> ò
    {0x00D3, 0x00F3, 0x00D3},  // Ó -> ó
    {0x00D4, 0x00F4, 0x00D4},  // Ô -> ô
    {0x00D5, 0x00F5, 0x00D5},  // Õ -> õ
    {0x00D6, 0x00F6, 0x00D6},  // Ö -> ö
    {0x00D8, 0x00F8, 0x00D8},  // Ø -> ø
    {0x00D9, 0x00F9, 0x00D9},  // Ù -> ù
    {0x00DA, 0x00FA, 0x00DA},  // Ú -> ú
    {0x00DB, 0x00FB, 0x00DB},  // Û -> û
    {0x00DC, 0x00FC, 0x00DC},  // Ü -> ü
    {0x00DD, 0x00FD, 0x00DD},  // Ý -> ý
    {0x00DE, 0x00FE, 0x00DE},  // Þ -> þ
    {0x00DF, 0x00DF, 0x1E9E},  // ß -> ß/ẞ
    
    // 希腊大写字母 (0x0391-0x03A9) -> 小写 (0x03B1-0x03C9)
    {0x0391, 0x03B1, 0x0391},  // Α -> α
    {0x0392, 0x03B2, 0x0392},  // Β -> β
    {0x0393, 0x03B3, 0x0393},  // Γ -> γ
    {0x0394, 0x03B4, 0x0394},  // Δ -> δ
    {0x0395, 0x03B5, 0x0395},  // Ε -> ε
    {0x0396, 0x03B6, 0x0396},  // Ζ -> ζ
    {0x0397, 0x03B7, 0x0397},  // Η -> η
    {0x0398, 0x03B8, 0x0398},  // Θ -> θ
    {0x0399, 0x03B9, 0x0399},  // Ι -> ι
    {0x039A, 0x03BA, 0x039A},  // Κ -> κ
    {0x039B, 0x03BB, 0x039B},  // Λ -> λ
    {0x039C, 0x03BC, 0x039C},  // Μ -> μ
    {0x039D, 0x03BD, 0x039D},  // Ν -> ν
    {0x039E, 0x03BE, 0x039E},  // Ξ -> ξ
    {0x039F, 0x03BF, 0x039F},  // Ο -> ο
    {0x03A0, 0x03C0, 0x03A0},  // Π -> π
    {0x03A1, 0x03C1, 0x03A1},  // Ρ -> ρ
    {0x03A3, 0x03C3, 0x03A3},  // Σ -> σ
    {0x03A4, 0x03C4, 0x03A4},  // Τ -> τ
    {0x03A5, 0x03C5, 0x03A5},  // Υ -> υ
    {0x03A6, 0x03C6, 0x03A6},  // Φ -> φ
    {0x03A7, 0x03C7, 0x03A7},  // Χ -> χ
    {0x03A8, 0x03C8, 0x03A8},  // Ψ -> ψ
    {0x03A9, 0x03C9, 0x03A9},  // Ω -> ω
    
    // 西里尔大写字母 (0x0410-0x042F) -> 小写 (0x0430-0x044F)
    {0x0410, 0x0430, 0x0410},  // А -> а
    {0x0411, 0x0431, 0x0411},  // Б -> б
    {0x0412, 0x0432, 0x0412},  // В -> в
    {0x0413, 0x0433, 0x0413},  // Г -> г
    {0x0414, 0x0434, 0x0414},  // Д -> д
    {0x0415, 0x0435, 0x0415},  // Е -> е
    {0x0416, 0x0436, 0x0416},  // Ж -> ж
    {0x0417, 0x0437, 0x0417},  // З -> з
    {0x0418, 0x0438, 0x0418},  // И -> и
    {0x0419, 0x0439, 0x0419},  // Й -> й
    {0x041A, 0x043A, 0x041A},  // К -> к
    {0x041B, 0x043B, 0x041B},  // Л -> л
    {0x041C, 0x043C, 0x041C},  // М -> м
    {0x041D, 0x043D, 0x041D},  // Н -> н
    {0x041E, 0x043E, 0x041E},  // О -> о
    {0x041F, 0x043F, 0x041F},  // П -> п
    {0x0420, 0x0440, 0x0420},  // Р -> р
    {0x0421, 0x0441, 0x0421},  // С -> с
    {0x0422, 0x0442, 0x0422},  // Т -> т
    {0x0423, 0x0443, 0x0423},  // У -> у
    {0x0424, 0x0444, 0x0424},  // Ф -> ф
    {0x0425, 0x0445, 0x0425},  // Х -> х
    {0x0426, 0x0446, 0x0426},  // Ц -> ц
    {0x0427, 0x0447, 0x0427},  // Ч -> ч
    {0x0428, 0x0448, 0x0428},  // Ш -> ш
    {0x0429, 0x0449, 0x0429},  // Щ -> щ
    {0x042A, 0x044A, 0x042A},  // Ъ -> ъ
    {0x042B, 0x044B, 0x042B},  // Ы -> ы
    {0x042C, 0x044C, 0x042C},  // Ь -> ь
    {0x042D, 0x044D, 0x042D},  // Э -> э
    {0x042E, 0x044E, 0x042E},  // Ю -> ю
    {0x042F, 0x044F, 0x042F},  // Я -> я
};

const int g_caseMappingsCount = sizeof(g_caseMappings) / sizeof(g_caseMappings[0]);

// 完整小写转换函数
UChar32 toLowerFull(UChar32 c) {
    // ASCII 快速路径
    if (c >= 'A' && c <= 'Z') {
        return c | 0x20;
    }
    
    // 非ASCII字符，二分查找映射表
    if (c > 0x7F) {
        int left = 0;
        int right = g_caseMappingsCount - 1;
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (g_caseMappings[mid].from == c) {
                return g_caseMappings[mid].lower;
            } else if (g_caseMappings[mid].from < c) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
    }
    
    // 未找到映射，返回原字符
    return c;
}

// 完整大写转换函数
UChar32 toUpperFull(UChar32 c) {
    // ASCII 快速路径
    if (c >= 'a' && c <= 'z') {
        return c & ~0x20;
    }
    
    // 非ASCII字符，二分查找映射表
    if (c > 0x7F) {
        int left = 0;
        int right = g_caseMappingsCount - 1;
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (g_caseMappings[mid].from == c) {
                return g_caseMappings[mid].upper;
            } else if (g_caseMappings[mid].from < c) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
    }
    
    // 未找到映射，返回原字符
    return c;
}

