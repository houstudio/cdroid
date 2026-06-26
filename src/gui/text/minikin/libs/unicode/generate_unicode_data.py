#!/usr/bin/env python3
"""
unicode_data.cpp generator (root-cure rewrite).

Reads authoritative UCD files (UnicodeData.txt, Scripts.txt, LineBreak.txt,
GraphemeBreakProperty.txt, DerivedCoreProperties.txt) and emits a SORTED,
NON-OVERLAPPING, property-merged g_unicodeRanges table matching myicu's
UnicodeRange struct. Enum values are parsed from myicu's own headers so the
mapping cannot drift.

Data size is driven by the number of property *runs*, not codepoint count —
so even full Unicode (incl. astral planes) stays compact: big blocks like CJK
Extension B (U+20000..U+2A6DF, ~55k Han chars) collapse to a single range.

Env:
  UCD_DIR        UCD .txt directory          (default /usr/share/unicode)
  UNICODE_MAX_CP cap (0x10FFFF = all planes) (default 0x10FFFF; 0xFFFF = BMP only)
"""
import os, re

HERE = os.path.dirname(os.path.abspath(__file__))
INC  = os.path.join(HERE, "../../", "include", "myicu", "unicode")
UCD  = os.environ.get("UCD_DIR", "/usr/share/unicode")
OUT  = os.path.join(HERE, "unicode_data_generated.cpp")
MAX_CP = int(os.environ.get("UNICODE_MAX_CP", "0x10FFFF"), 0)

# UCharCategory: UCD 2-letter -> myicu int (matches uchar.h UCharCategory enum)
CAT_MAP = {'Lu':1,'Ll':2,'Lt':3,'Lm':4,'Lo':5,'Mn':6,'Me':7,'Mc':8,'Nd':9,'Nl':10,
           'No':11,'Zs':12,'Zl':13,'Zp':14,'Cc':15,'Cf':16,'Co':17,'Cs':18,'Pd':19,
           'Ps':20,'Pe':21,'Pc':22,'Po':23,'Sm':24,'Sc':25,'Sk':26,'So':27,'Pi':28,
           'Pf':29,'Cn':0}
# UCharDirection (bidi class): UCD letter -> myicu int
DIR_MAP = {'L':0,'R':1,'EN':2,'ES':3,'ET':4,'AN':5,'CS':6,'B':7,'S':8,'WS':9,'ON':10,
           'LRE':11,'LRO':12,'AL':13,'RLE':14,'RLO':15,'PDF':16,'NSM':17,'BN':18,
           'FSI':19,'LRI':20,'RLI':21,'PDI':22}


def parse_enum(path, prefix, want_code=False):
    """Parse PREFIX_NAME = int  ...  [CODE] -> dict (by NAME, and by [CODE] if present)."""
    out = {}
    pat = re.compile(r'\b' + re.escape(prefix) + r'([A-Z0-9_]+)\s*=\s*(\d+)(?:.*?\[(\w+)\])?')
    for line in open(path, encoding='utf-8'):
        m = pat.search(line)
        if m:
            out[m.group(1).upper()] = int(m.group(2))
            if m.group(3):
                # [code] keys may collide (e.g. U_LB_ALPHABETIC and U_LB_ARABIC_LETTER
                # both carry [AL] in myicu's header). First wins — the UAX short code
                # maps to the primary class (AL=Alphabetic=2, not Arabic_Letter=51).
                out.setdefault(m.group(3).upper(), int(m.group(2)))
    return out


GCB_ENUM = parse_enum(os.path.join(INC, "uchar.h"), "U_GCB_")
LB_ENUM  = parse_enum(os.path.join(INC, "uchar.h"), "U_LB_")
SCRIPT_ENUM = {m.group(1).upper(): int(m.group(2))
               for m in (re.compile(r'\bUSCRIPT_([A-Z0-9_]+)\s*=\s*(\d+)').search(l)
                         for l in open(os.path.join(INC, "uscript.h"), encoding='utf-8')) if m}
USCRIPT_UNKNOWN = SCRIPT_ENUM.get('UNKNOWN', 103)


def parse_enum_names(path, prefix):
    """name-only parse: {value: NAME} (first-wins), ignoring [code] aliases —
    used to emit the symbolic ICU constant for each value."""
    out = {}
    pat = re.compile(r'\b' + re.escape(prefix) + r'([A-Z0-9_]+)\s*=\s*(\d+)')
    for line in open(path, encoding='utf-8'):
        m = pat.search(line)
        if m:
            out.setdefault(int(m.group(2)), m.group(1))  # first (canonical) name wins
    return out


# value -> symbolic ICU constant, for emitting readable tables
DIR_REV = {0:'U_LEFT_TO_RIGHT',1:'U_RIGHT_TO_LEFT',2:'U_EUROPEAN_NUMBER',3:'U_EUROPEAN_NUMBER_SEPARATOR',
 4:'U_EUROPEAN_NUMBER_TERMINATOR',5:'U_ARABIC_NUMBER',6:'U_COMMON_NUMBER_SEPARATOR',7:'U_BLOCK_SEPARATOR',
 8:'U_SEGMENT_SEPARATOR',9:'U_WHITE_SPACE_NEUTRAL',10:'U_OTHER_NEUTRAL',11:'U_LEFT_TO_RIGHT_EMBEDDING',
 12:'U_LEFT_TO_RIGHT_OVERRIDE',13:'U_RIGHT_TO_LEFT_ARABIC',14:'U_RIGHT_TO_LEFT_EMBEDDING',
 15:'U_RIGHT_TO_LEFT_OVERRIDE',16:'U_POP_DIRECTIONAL_FORMAT',17:'U_DIR_NON_SPACING_MARK',
 18:'U_BOUNDARY_NEUTRAL',19:'U_FIRST_STRONG_ISOLATE',20:'U_LEFT_TO_RIGHT_ISOLATE',21:'U_RIGHT_TO_LEFT_ISOLATE',
 22:'U_POP_DIRECTIONAL_ISOLATE'}
CAT_REV = {0:'U_UNASSIGNED',1:'U_UPPERCASE_LETTER',2:'U_LOWERCASE_LETTER',3:'U_TITLECASE_LETTER',4:'U_MODIFIER_LETTER',
 5:'U_OTHER_LETTER',6:'U_NON_SPACING_MARK',7:'U_ENCLOSING_MARK',8:'U_COMBINING_SPACING_MARK',9:'U_DECIMAL_DIGIT_NUMBER',
 10:'U_LETTER_NUMBER',11:'U_OTHER_NUMBER',12:'U_SPACE_SEPARATOR',13:'U_LINE_SEPARATOR',14:'U_PARAGRAPH_SEPARATOR',
 15:'U_CONTROL_CHAR',16:'U_FORMAT_CHAR',17:'U_PRIVATE_USE_CHAR',18:'U_SURROGATE',19:'U_DASH_PUNCTUATION',
 20:'U_START_PUNCTUATION',21:'U_END_PUNCTUATION',22:'U_CONNECTOR_PUNCTUATION',23:'U_OTHER_PUNCTUATION',
 24:'U_MATH_SYMBOL',25:'U_CURRENCY_SYMBOL',26:'U_MODIFIER_SYMBOL',27:'U_OTHER_SYMBOL',28:'U_INITIAL_PUNCTUATION',29:'U_FINAL_PUNCTUATION'}
GCB_REV    = {k: 'U_GCB_' + v for k, v in parse_enum_names(os.path.join(INC, "uchar.h"), "U_GCB_").items()}
LB_REV     = {k: 'U_LB_'  + v for k, v in parse_enum_names(os.path.join(INC, "uchar.h"), "U_LB_").items()}
SCRIPT_REV = {v: 'USCRIPT_' + k for k, v in SCRIPT_ENUM.items() if 'RESERVED' not in k}
USCRIPT_COMMON  = SCRIPT_ENUM.get('COMMON', 0)

# GraphemeBreakProperty.txt uses long labels -> GCB enum value
GCB_LABEL_MAP = {k: GCB_ENUM.get(k.upper().replace('-', '_'),
                                 GCB_ENUM.get({'Regional_Indicator': 'REGIONAL_INDICATOR'}.get(k, k.upper()), 0))
                 for k in ['Other','Control','CR','LF','Extend','Regional_Indicator','Prepend',
                           'SpacingMark','L','V','T','LV','LVT','ZWJ']}

# Binary properties we care about -> their UCHAR_* bit index (parsed from uchar.h)
BIN_BIT = {m.group(1): int(m.group(2))
           for m in (re.compile(r'\bUCHAR_([A-Z0-9_]+)\s*=\s*(\d+)').search(l)
                     for l in open(os.path.join(INC, "uchar.h"), encoding='utf-8')) if m}
BIN_ALIASES = {'Alphabetic':'ALPHABETIC','Uppercase':'UPPERCASE','Lowercase':'LOWERCASE',
               'White_Space':'WHITE_SPACE','Hex_Digit':'HEX_DIGIT','ID_Start':'ID_START',
               'ID_Continue':'ID_CONTINUE','Ideographic':'IDEOGRAPHIC',
               'Grapheme_Extend':'GRAPHEME_EXTEND','Math':'MATH','Dash':'DASH',
               'Extended_Pictographic':'EXTENDED_PICTOGRAPHIC'}


def cp_span(field):
    if '..' in field:
        a, b = field.split('..'); return int(a, 16), int(b, 16)
    v = int(field, 16); return v, v


def each_prop_range(path):
    print(path)
    with open(path, encoding='utf-8') as f:
        for line in f:
            line = line.split('#', 1)[0].strip()
            if not line: continue
            p = line.split(';')
            if len(p) < 2: continue
            yield (*cp_span(p[0].strip()), p[1].strip())


def main():
    # props[cp] = [category, ccc, dir, gcb, lb, script, binaryProps]
    props = {}

    def setprop(s, e, fn):
        s = max(s, 0); e = min(e, MAX_CP)
        for cp in range(s, e + 1):
            p = props.get(cp) or [0, 0, 0, 0, 0, USCRIPT_COMMON, 0]  # Cn/0/L/Other/XX/Common
            fn(p); props[cp] = p

    # 1) UnicodeData.txt — category, ccc, bidi (handles First/Last ranges)
    pending = None
    for line in open(os.path.join(UCD, "UnicodeData.txt"), encoding='utf-8'):
        line = line.strip()
        if not line or line.startswith('#'): continue
        f = line.split(';')
        cp = int(f[0], 16); name = f[1]
        if cp > MAX_CP and not name.endswith(', First>'): continue
        cat = CAT_MAP.get(f[2], 0); ccc = int(f[3]) if f[3].isdigit() else 0; d = DIR_MAP.get(f[4], 0)
        if name.endswith(', First>'): pending = (cp, cat, ccc, d)
        elif name.endswith(', Last>'):
            if pending:
                s, c, cc, dd = pending
                setprop(s, cp, lambda p, c=c, cc=cc, dd=dd: (p.__setitem__(0, c), p.__setitem__(1, cc), p.__setitem__(2, dd)))
                pending = None
        else:
            setprop(cp, cp, lambda p, c=cat, cc=ccc, dd=d: (p.__setitem__(0, c), p.__setitem__(1, cc), p.__setitem__(2, dd)))

    # 2) Scripts.txt
    for s, e, val in each_prop_range(os.path.join(UCD, "Scripts.txt")):
        sc = SCRIPT_ENUM.get(val.upper().replace('-', '_'), USCRIPT_UNKNOWN)
        setprop(s, e, lambda p, sc=sc: p.__setitem__(5, sc))
    # 3) LineBreak.txt
    for s, e, val in each_prop_range(os.path.join(UCD, "LineBreak.txt")):
        lb = LB_ENUM.get(val.upper(), LB_ENUM.get('UNKNOWN', 0))
        setprop(s, e, lambda p, lb=lb: p.__setitem__(4, lb))
    # 4) GraphemeBreakProperty.txt
    for s, e, val in each_prop_range(os.path.join(UCD, "GraphemeBreakProperty.txt")):
        g = GCB_LABEL_MAP.get(val, 0)
        setprop(s, e, lambda p, g=g: p.__setitem__(3, g))
    # 5) DerivedCoreProperties.txt — selected binary props
    for s, e, val in each_prop_range(os.path.join(UCD, "DerivedCoreProperties.txt")):
        key = BIN_ALIASES.get(val)
        if not key or key not in BIN_BIT: continue
        bit = BIN_BIT[key]
        setprop(s, e, lambda p, bit=bit: p.__setitem__(6, p[6] | (1 << bit)))

    # 5b) PropList.txt — the rest of the binary props (White_Space, Hex_Digit,
    #     Ideographic, Bidi_Mirrored, ...). These are NOT in DerivedCoreProperties.
    for s, e, val in each_prop_range(os.path.join(UCD, "PropList.txt")):
        key = BIN_ALIASES.get(val)
        if not key or key not in BIN_BIT: continue
        bit = BIN_BIT[key]
        setprop(s, e, lambda p, bit=bit: p.__setitem__(6, p[6] | (1 << bit)))

    # 6) Merge adjacent codepoints with identical property tuples -> ranges
    cps = sorted(props)
    ranges = []
    cur_s = cps[0]; cur = props[cur_s]; prev = cur_s
    for cp in cps[1:]:
        if cp == prev + 1 and props[cp] == cur:
            prev = cp
        else:
            ranges.append((cur_s, prev, cur)); cur_s = cp; cur = props[cp]; prev = cp
    ranges.append((cur_s, prev, cur))

    # 7) Emit — fully symbolic: all fields use ICU enum constants; binaryProps
    #    use BP_<NAME> shorthand macros (defined below) for maximum readability.
    IDX_TO_NAME = {}
    for _k, _v in BIN_BIT.items():
        if 'RESERVED' not in _k and _v not in IDX_TO_NAME:
            IDX_TO_NAME[_v] = _k

    L = ["// Unicode range table — AUTO-GENERATED by generate_unicode_data.py from UCD.",
         "// Sorted, non-overlapping, property-merged. Do not edit by hand.", "",
         '#include "unicode/uchar.h"', '#include "unicode/uscript.h"',
         '#include "unicode_range_data.h"', ""]
    # BP_<NAME> shorthand: one name per binary property bit.
    for idx in sorted(IDX_TO_NAME):
        nm = IDX_TO_NAME[idx]
        L.append("#define BP_%-26s (1ULL<<UCHAR_%s)" % (nm, nm))
    L += ["", "const UnicodeRange g_unicodeRanges[] = {"]
    for s, e, p in ranges:
        cat, ccc, d, gcb, lb, sc, bp = p
        names = ["BP_%s" % IDX_TO_NAME[b]
                 for b in range(64) if (bp >> b) & 1 and b in IDX_TO_NAME]
        bp_expr = "|".join(names) if names else "0"
        hi = (bp >> 64) & ((1 << 64) - 1)
        bp2_expr = ("0x%016XULL" % hi) if hi else "0"
        L.append("    {0x%04X, 0x%04X, %s, %s, %s, %s, %d, %s, %s, %s},"
                 % (s, e, DIR_REV.get(d, str(d)), CAT_REV.get(cat, str(cat)),
                    GCB_REV.get(gcb, str(gcb)), LB_REV.get(lb, str(lb)), ccc,
                    SCRIPT_REV.get(sc, str(sc)), bp_expr, bp2_expr))
    L += ["};", "", "const int g_unicodeRangesCount = sizeof(g_unicodeRanges) / sizeof(g_unicodeRanges[0]);", ""]
    open(OUT, 'w', encoding='utf-8').write('\n'.join(L))

    bad = sum(1 for i in range(1, len(ranges)) if ranges[i][0] <= ranges[i-1][1])
    print("MAX_CP=0x%X  ranges=%d  codepoints=%d  out=%s" % (MAX_CP, len(ranges), len(cps), OUT))
    print("first=0x%04X last=0x%04X  sort/overlap violations=%d  src~%.0fKB"
          % (ranges[0][0], ranges[-1][1], bad, os.path.getsize(OUT)/1024))


if __name__ == '__main__':
    main()
