#!/usr/bin/env python3
"""
Unicode 范围数据表生成器
从 Unicode 官方数据文件生成 C++ 范围表
"""

import re
from dataclasses import dataclass
from typing import List, Tuple, Dict

@dataclass
class UnicodeRange:
    start: int
    end: int
    directionality: int
    category: int
    gcb: int
    lb: int
    ccc: int
    script: int
    binaryProps: int

# UCharCategory 映射
CATEGORY_MAP = {
    'Cc': 15, 'Cf': 16, 'Cn': 0, 'Co': 17, 'Cs': 18,
    'Ll': 2, 'Lm': 4, 'Lo': 5, 'Lt': 3, 'Lu': 1,
    'Mc': 8, 'Me': 7, 'Mn': 6,
    'Nd': 9, 'Nl': 10, 'No': 11,
    'Pc': 19, 'Pd': 20, 'Pe': 21, 'Pf': 22, 'Pi': 23, 'Po': 24, 'Ps': 25,
    'Sc': 26, 'Sk': 27, 'Sm': 28, 'So': 29,
    'Zl': 13, 'Zp': 14, 'Zs': 12,
}

# UCharDirection 映射
DIRECTION_MAP = {
    'L': 0, 'R': 1, 'EN': 2, 'ES': 3, 'ET': 4, 'AN': 5, 'CS': 6,
    'B': 7, 'S': 8, 'WS': 9, 'ON': 10, 'LRE': 11, 'LRO': 12,
    'AL': 13, 'RLE': 14, 'RLO': 15, 'PDF': 16, 'NSM': 17, 'BN': 18,
    'FSI': 19, 'LRI': 20, 'RLI': 21, 'PDI': 22,
}

# UGraphemeClusterBreak 映射
GCB_MAP = {
    'Other': 0, 'Control': 1, 'CR': 2, 'LF': 5, 'Extend': 3,
    'Regional_Indicator': 12, 'Prepend': 11, 'SpacingMark': 10,
    'L': 4, 'V': 9, 'T': 8, 'LV': 6, 'LVT': 7,
    'E_Base': 13, 'E_Modifier': 15, 'Glue_After_ZWJ': 16, 'E_Base_GAZ': 14, 'ZWJ': 17,
}

# ULineBreak 映射
LB_MAP = {
    'XX': 0, 'AI': 1, 'AL': 2, 'B2': 3, 'BA': 4, 'BB': 5, 'BK': 6,
    'CB': 7, 'CL': 8, 'CM': 9, 'CR': 10, 'EX': 11, 'GL': 12, 'HY': 13,
    'ID': 14, 'IN': 15, 'IS': 16, 'LF': 17, 'NS': 18, 'NU': 19, 'OP': 20,
    'PO': 21, 'PR': 22, 'QU': 23, 'SA': 24, 'SG': 25, 'SP': 26, 'SY': 27,
    'ZW': 28, 'NL': 29, 'WJ': 30, 'H2': 31, 'H3': 32, 'JL': 33, 'JT': 34,
    'JV': 35, 'CP': 36, 'CJ': 37, 'HL': 38, 'RI': 39, 'EB': 40, 'EM': 41,
    'ZWJ': 42,
}

# UScriptCode 映射（简化版）
SCRIPT_MAP = {
    'Common': 0, 'Inherited': 1, 'Unknown': 2,
    'Arabic': 3, 'Armenian': 4, 'Bengali': 5, 'Bopomofo': 6,
    'Cherokee': 7, 'Coptic': 8, 'Cyrillic': 9, 'Deseret': 10,
    'Devanagari': 11, 'Ethiopic': 12, 'Georgian': 13, 'Gothic': 14,
    'Greek': 15, 'Gujarati': 16, 'Gurmukhi': 17, 'Han': 18,
    'Hangul': 19, 'Hebrew': 20, 'Hiragana': 21, 'Kannada': 22,
    'Katakana': 23, 'Khmer': 24, 'Lao': 25, 'Latin': 26,
    'Malayalam': 27, 'Mongolian': 28, 'Myanmar': 29, 'Ogham': 30,
    'Old_Italic': 31, 'Oriya': 32, 'Runic': 33,
}

# 二进制属性索引
BINARY_PROPS = {
    'White_Space': 40,
    'Bidi_Mirrored': 1,
    'Emoji': 8,
    'Emoji_Modifier': 149,
    'Emoji_Modifier_Base': 150,
    'Extended_Pictographic': 151,
    'Hex_Digit': 18,
    'ID_Start': 20,
    'ID_Continue': 21,
    'Ideographic': 22,
    'Lowercase': 27,
    'Uppercase': 38,
    'Grapheme_Extend': 16,
}

def parse_unicode_data(filename: str) -> List[UnicodeRange]:
    """解析 UnicodeData.txt"""
    ranges = []
    current_range = None
    
    with open(filename, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            
            parts = line.split(';')
            if len(parts) < 3:
                continue
            
            code_point = int(parts[0], 16)
            name = parts[1]
            category_str = parts[2]
            category = CATEGORY_MAP.get(category_str, 0)
            
            # 处理范围（首尾标记）
            if name.endswith('First>') or name.endswith('Last>'):
                if name.endswith('First>'):
                    current_range = {
                        'start': code_point,
                        'category': category,
                    }
                else:
                    if current_range:
                        ranges.append(UnicodeRange(
                            start=current_range['start'],
                            end=code_point,
                            directionality=0,
                            category=category,
                            gcb=0,
                            lb=0,
                            ccc=0,
                            script=0,
                            binaryProps=0,
                        ))
                        current_range = None
            else:
                # 单个字符
                ranges.append(UnicodeRange(
                    start=code_point,
                    end=code_point,
                    directionality=0,
                    category=category,
                    gcb=0,
                    lb=0,
                    ccc=0,
                    script=0,
                    binaryProps=0,
                ))
    
    return ranges

def merge_ranges(ranges: List[UnicodeRange]) -> List[UnicodeRange]:
    """合并相邻的相同属性范围"""
    if not ranges:
        return []
    
    merged = []
    current = ranges[0]
    
    for r in ranges[1:]:
        if (r.start == current.end + 1 and
            r.directionality == current.directionality and
            r.category == current.category and
            r.gcb == current.gcb and
            r.lb == current.lb and
            r.script == current.script):
            # 可以合并
            current.end = r.end
        else:
            merged.append(current)
            current = r
    
    merged.append(current)
    return merged

def generate_cpp_code(ranges: List[UnicodeRange]) -> str:
    """生成 C++ 代码"""
    lines = []
    lines.append('// Unicode 范围数据表（由 generate_unicode_data.py 自动生成）')
    lines.append('static const UnicodeRange g_unicodeRanges[] = {')
    
    for r in ranges:
        lines.append(f'    {{0x{r.start:04X}, 0x{r.end:04X}, {r.directionality}, {r.category}, '
                    f'{r.gcb}, 0, {r.lb}, {r.ccc}, {r.script}, 0, 0, 0, 0, 0x{r.binaryProps:04X}}},')
    
    lines.append('};')
    lines.append(f'static const int g_unicodeRangesCount = sizeof(g_unicodeRanges) / sizeof(g_unicodeRanges[0]);')
    
    return '\n'.join(lines)

def main():
    # 示例：生成基本拉丁文范围
    ranges = [
        UnicodeRange(0x0000, 0x001F, 10, 15, 1, 6, 0, 0, 0),  # 控制字符
        UnicodeRange(0x0020, 0x0020, 9, 12, 0, 26, 0, 0, 1 << 40),  # 空格
        UnicodeRange(0x0030, 0x0039, 2, 9, 0, 19, 0, 0, (1 << 18) | (1 << 20) | (1 << 21)),  # 数字
        UnicodeRange(0x0041, 0x005A, 0, 1, 0, 2, 0, 26, (1 << 20) | (1 << 21) | (1 << 38)),  # 大写
        UnicodeRange(0x0061, 0x007A, 0, 2, 0, 2, 0, 26, (1 << 20) | (1 << 21) | (1 << 27)),  # 小写
    ]
    
    merged = merge_ranges(ranges)
    cpp_code = generate_cpp_code(merged)
    print(cpp_code)

if __name__ == '__main__':
    main()
