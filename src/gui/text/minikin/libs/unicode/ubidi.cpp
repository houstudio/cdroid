// SPDX-License-Identifier: Apache-2.0
//
// Self-contained implementation of the ICU ubidi_* C API on top of a from-scratch
// Unicode Bidirectional Algorithm (UAX #9). Replaces the previous fribidi-backed
// shim. The bidi character class table is supplied by MYICU's u_charDirection();
// only the algorithm (P/X/W/N/I/L) lives here.
//
// Reference: https://www.unicode.org/reports/tr9/ (revision 51, Unicode 17.0).
#include "unicode/ubidi.h"
#include "unicode/uchar.h"
#include "unicode/utf16.h"

#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include <algorithm>

#ifndef U_INDEX_OUTOFBOUNDS_ERROR
#define U_INDEX_OUTOFBOUNDS_ERROR U_ILLEGAL_ARGUMENT_ERROR
#endif

namespace {

// Bidirectional character types. Values deliberately match UCharDirection so
// that the raw result of u_charDirection() can be stored directly.
enum BidiType : uint8_t {
    kL = 0,   kR = 1,  kEN = 2,  kES = 3,  kET = 4,  kAN = 5,  kCS = 6,
    kB = 7,   kS = 8,  kWS = 9,  kON = 10,
    kLRE = 11, kLRO = 12, kAL = 13, kRLE = 14, kRLO = 15, kPDF = 16,
    kNSM = 17, kBN = 18,
    kFSI = 19, kLRI = 20, kRLI = 21, kPDI = 22
};

// Directional override status (BD6).
enum Override : uint8_t { kNI = 0, kOvrL = 1, kOvrR = 2 };

inline uint8_t leastGreaterOdd(uint8_t x)  { return static_cast<uint8_t>((x + 1) | 1u); }
inline uint8_t leastGreaterEven(uint8_t x) { return static_cast<uint8_t>((x + 2) & static_cast<uint8_t>(~1u)); }

inline bool isIsolateInitiator(uint8_t t) { return t == kLRI || t == kRLI || t == kFSI; }
// NI: neutral or isolate-formatting (Table 3).
inline bool isNI(uint8_t t) {
    return t == kB || t == kS || t == kWS || t == kON ||
           t == kFSI || t == kLRI || t == kRLI || t == kPDI;
}

// Paired brackets from BidiBrackets.txt (Unicode 17.0): {opening, closing}.
// BD16 treats U+232A and U+3009 as equivalent (handled in bracketEq).
struct Bracket { uint32_t open; uint32_t close; };
static const Bracket kBrackets[] = {
    {0x0028, 0x0029}, {0x005B, 0x005D}, {0x007B, 0x007D}, {0x0F3A, 0x0F3B},
    {0x0F3C, 0x0F3D}, {0x169B, 0x169C}, {0x2045, 0x2046}, {0x207D, 0x207E},
    {0x208D, 0x208E}, {0x2308, 0x2309}, {0x230A, 0x230B}, {0x2329, 0x232A},
    {0x2768, 0x2769}, {0x276A, 0x276B}, {0x276C, 0x276D}, {0x276E, 0x276F},
    {0x2770, 0x2771}, {0x2772, 0x2773}, {0x2774, 0x2775}, {0x27C5, 0x27C6},
    {0x27E6, 0x27E7}, {0x27E8, 0x27E9}, {0x27EA, 0x27EB}, {0x27EC, 0x27ED},
    {0x27EE, 0x27EF}, {0x2983, 0x2984}, {0x2985, 0x2986}, {0x2987, 0x2988},
    {0x2989, 0x298A}, {0x298B, 0x298C}, {0x298D, 0x2990}, {0x298F, 0x298E},
    {0x2991, 0x2992}, {0x2993, 0x2994}, {0x2995, 0x2996}, {0x2997, 0x2998},
    {0x29D8, 0x29D9}, {0x29DA, 0x29DB}, {0x29FC, 0x29FD}, {0x2E22, 0x2E23},
    {0x2E24, 0x2E25}, {0x2E26, 0x2E27}, {0x2E28, 0x2E29}, {0x2E55, 0x2E56},
    {0x2E57, 0x2E58}, {0x2E59, 0x2E5A}, {0x2E5B, 0x2E5C}, {0x3008, 0x3009},
    {0x300A, 0x300B}, {0x300C, 0x300D}, {0x300E, 0x300F}, {0x3010, 0x3011},
    {0x3014, 0x3015}, {0x3016, 0x3017}, {0x3018, 0x3019}, {0x301A, 0x301B},
    {0xFE59, 0xFE5A}, {0xFE5B, 0xFE5C}, {0xFE5D, 0xFE5E}, {0xFF08, 0xFF09},
    {0xFF3B, 0xFF3D}, {0xFF5B, 0xFF5D}, {0xFF5F, 0xFF60}, {0xFF62, 0xFF63}
};
const size_t kBracketCount = sizeof(kBrackets) / sizeof(kBrackets[0]);

// Returns 0 = not a bracket, 1 = opening, 2 = closing; fills *pair with the
// Bidi_Paired_Bracket partner (closing partner for an opener, opening for a closer).
int bracketKind(uint32_t c, uint32_t* pair) {
    for (size_t i = 0; i < kBracketCount; i++) {
        if (kBrackets[i].open == c)  { *pair = kBrackets[i].close; return 1; }
        if (kBrackets[i].close == c) { *pair = kBrackets[i].open;  return 2; }
    }
    return 0;
}
inline bool bracketEq(uint32_t a, uint32_t b) {
    if (a == b) return true;
    if ((a == 0x232A && b == 0x3009) || (a == 0x3009 && b == 0x232A)) return true;
    return false;
}

// A self-contained UBA resolver. Operates on a UTF-16 buffer of `length` units;
// all per-index arrays (types/levels) are indexed by UTF-16 unit, and surrogate
// pairs share the type of their full code point.
struct BidiResolver {
    const UChar* text;
    int32_t length;
    UBiDiClassCallback* callback;
    const void* callbackCtx;
    uint8_t paraLevel = 0;

    std::vector<uint8_t> types;      // working bidi type per index (modified by X/W/N)
    std::vector<uint8_t> origTypes;  // raw bidi class per index (never modified; for BD9/L1)
    std::vector<uint8_t> levels;     // resolved embedding level per index
    std::vector<uint8_t> removed;    // 1 for X9-removed chars (BN, RLE, LRE, RLO, LRO, PDF, B)
    std::vector<int32_t> isoMatch;   // BD9: isoMatch[i] = matching PDI/initiator index, else -1

    int32_t decode(int32_t i, uint32_t& c) const {
        uint16_t u = static_cast<uint16_t>(text[i]);
        if (u >= 0xD800 && u <= 0xDBFF && i + 1 < length) {
            uint16_t u2 = static_cast<uint16_t>(text[i + 1]);
            if (u2 >= 0xDC00 && u2 <= 0xDFFF) {
                c = 0x10000u + ((static_cast<uint32_t>(u - 0xD800) << 10) |
                                static_cast<uint32_t>(u2 - 0xDC00));
                return i + 2;
            }
        }
        c = u;
        return i + 1;
    }
    uint32_t codePointAt(int32_t i) const { uint32_t c; decode(i, c); return c; }

    uint8_t typeOf(uint32_t c) const {
        if (callback) {
            UCharDirection d = callback(callbackCtx, c);
            if (d >= 0 && d < 23) return static_cast<uint8_t>(d);
        }
        return static_cast<uint8_t>(u_charDirection(c));
    }

    void precomputeTypes() {
        types.assign(length, kL);
        origTypes.assign(length, kL);
        for (int32_t i = 0; i < length; ) {
            uint32_t c; int32_t n = decode(i, c);
            uint8_t t = typeOf(c);
            for (int32_t j = i; j < n; j++) { types[j] = t; origTypes[j] = t; }
            i = n;
        }
    }

    // P2/P3: first strong character (skipping isolate content) decides the level.
    uint8_t resolveParaLevel(uint8_t requested) {
        if (requested != UBIDI_DEFAULT_LTR && requested != UBIDI_DEFAULT_RTL) {
            return requested & 1;
        }
        int isoDepth = 0;
        for (int32_t i = 0; i < length; ) {
            uint32_t c; int32_t n = decode(i, c);
            uint8_t t = typeOf(c);
            if (isoDepth == 0) {
                if (t == kL) return 0;
                if (t == kR || t == kAL) return 1;
            }
            if (isIsolateInitiator(t)) isoDepth++;
            else if (t == kPDI && isoDepth > 0) isoDepth--;
            i = n;
        }
        return (requested == UBIDI_DEFAULT_RTL) ? 1 : 0;
    }

    // X5c: determine whether an FSI behaves as LRI or RLI by scanning its content.
    bool fsiIsRtl(int32_t pos) const {
        int isoDepth = 0;
        for (int32_t i = pos + 1; i < length; ) {
            uint32_t c; int32_t n = decode(i, c);
            uint8_t t = typeOf(c);
            if (isoDepth == 0) {
                if (t == kL) return false;
                if (t == kR || t == kAL) return true;
                if (t == kPDI) break;
            }
            if (isIsolateInitiator(t)) isoDepth++;
            else if (t == kPDI && isoDepth > 0) isoDepth--;
            i = n;
        }
        return false;
    }

    // X1-X8: assign explicit embedding levels and apply overrides.
    void applyExplicit() {
        levels.assign(length, paraLevel);
        removed.assign(length, 0);

        struct Entry { uint8_t level; uint8_t ovr; bool isolate; };
        std::vector<Entry> stack;
        stack.reserve(16);
        stack.push_back({paraLevel, kNI, false});

        int ovIso = 0, ovEmb = 0, validIso = 0;

        for (int32_t i = 0; i < length; i++) {
            const uint8_t t = types[i];
            switch (t) {
            case kRLE: case kLRE: case kRLO: case kLRO: {
                const bool rtl = (t == kRLE || t == kRLO);
                const bool ovr = (t == kRLO || t == kLRO);
                levels[i] = stack.back().level;  // retain level for the initiator
                uint8_t newLevel = rtl ? leastGreaterOdd(stack.back().level)
                                       : leastGreaterEven(stack.back().level);
                if (ovIso == 0 && ovEmb == 0 && newLevel <= UBIDI_MAX_EXPLICIT_LEVEL) {
                    stack.push_back({newLevel, ovr ? (rtl ? kOvrR : kOvrL) : kNI, false});
                } else if (ovIso == 0) {
                    ovEmb++;
                }
                removed[i] = 1;
                break;
            }
            case kRLI: case kLRI: case kFSI: {
                bool rtl;
                if (t == kFSI) rtl = fsiIsRtl(i);
                else rtl = (t == kRLI);
                levels[i] = stack.back().level;  // the initiator keeps the container level (X5a/b)
                const uint8_t ovr = stack.back().ovr;
                if (ovr == kOvrL) types[i] = kL;
                else if (ovr == kOvrR) types[i] = kR;
                uint8_t newLevel = rtl ? leastGreaterOdd(stack.back().level)
                                       : leastGreaterEven(stack.back().level);
                if (newLevel <= UBIDI_MAX_EXPLICIT_LEVEL && ovIso == 0 && ovEmb == 0) {
                    stack.push_back({newLevel, kNI, true});
                    validIso++;
                } else {
                    ovIso++;
                }
                break;
            }
            case kPDI: {
                if (ovIso > 0) {
                    ovIso--;
                } else if (validIso == 0) {
                    // unmatched PDI: ignored
                } else {
                    ovEmb = 0;
                    while (!stack.back().isolate) stack.pop_back();
                    stack.pop_back();
                    validIso--;
                }
                levels[i] = stack.back().level;
                const uint8_t ovr = stack.back().ovr;
                if (ovr == kOvrL) types[i] = kL;
                else if (ovr == kOvrR) types[i] = kR;
                break;
            }
            case kPDF: {
                levels[i] = stack.back().level;  // retain level
                if (ovIso > 0) {
                    // within an overflow isolate: ignore
                } else if (ovEmb > 0) {
                    ovEmb--;
                } else if (stack.size() > 1 && !stack.back().isolate) {
                    stack.pop_back();
                }
                removed[i] = 1;
                break;
            }
            case kB: {
                // X8: paragraph separator terminates all embeddings.
                levels[i] = paraLevel;
                removed[i] = 1;
                stack.clear();
                stack.push_back({paraLevel, kNI, false});
                ovIso = ovEmb = validIso = 0;
                break;
            }
            case kBN: {
                levels[i] = stack.back().level;  // retaining: BN carries the current level
                removed[i] = 1;
                break;
            }
            default: {
                // X6
                levels[i] = stack.back().level;
                const uint8_t ovr = stack.back().ovr;
                if (ovr == kOvrL) types[i] = kL;
                else if (ovr == kOvrR) types[i] = kR;
                break;
            }
            }
        }
    }

    // BD9: pair each isolate initiator with its matching PDI.
    void pairIsolates() {
        isoMatch.assign(length, -1);
        std::vector<int32_t> stk;
        for (int32_t i = 0; i < length; i++) {
            const uint8_t t = origTypes[i];
            if (isIsolateInitiator(t)) stk.push_back(i);
            else if (t == kPDI && !stk.empty()) {
                const int32_t ini = stk.back();
                stk.pop_back();
                isoMatch[ini] = i;
                isoMatch[i] = ini;
            }
        }
    }

    struct LevelRun { int32_t start, limit; uint8_t level; };
    std::vector<LevelRun> runs;

    // BD7: maximal sequences of consecutive (non-X9-removed) chars at the same level.
    void buildLevelRuns() {
        runs.clear();
        int32_t i = 0;
        while (i < length) {
            while (i < length && removed[i]) i++;
            if (i >= length) break;
            const uint8_t lvl = levels[i];
            const int32_t start = i;
            int32_t j = i + 1;
            while (j < length) {
                if (removed[j]) { j++; continue; }
                if (levels[j] != lvl) break;
                j++;
            }
            runs.push_back({start, j, lvl});
            i = j;
        }
    }

    int32_t lastActiveInRun(const LevelRun& r) const {
        for (int32_t k = r.limit - 1; k >= r.start; k--) if (!removed[k]) return k;
        return -1;
    }

    struct Seq {
        std::vector<int32_t> idx;  // original char indices, in sequence order
        uint8_t level;
        uint8_t sos, eos;
    };
    std::vector<Seq> sequences;

    // BD13: chain level runs across isolate boundaries into isolating run sequences.
    void buildSequences() {
        const int n = static_cast<int>(runs.size());
        std::vector<int32_t> runStartAt(length, -1);
        for (int r = 0; r < n; r++) runStartAt[runs[r].start] = r;

        std::vector<int32_t> chainNext(n, -1);
        std::vector<bool> pointedTo(n, false);
        for (int r = 0; r < n; r++) {
            const int32_t last = lastActiveInRun(runs[r]);
            if (last < 0) continue;
            const uint8_t t = origTypes[last];
            if (isIsolateInitiator(t)) {
                const int32_t p = isoMatch[last];
                if (p >= 0) {
                    const int s = runStartAt[p];
                    if (s >= 0) { chainNext[r] = s; pointedTo[s] = true; }
                }
            }
        }

        sequences.clear();
        std::vector<bool> done(n, false);
        for (int r = 0; r < n; r++) {
            if (pointedTo[r] || done[r]) continue;
            Seq seq;
            seq.level = runs[r].level;
            int cur = r;
            while (cur >= 0 && !done[cur]) {
                done[cur] = true;
                for (int32_t k = runs[cur].start; k < runs[cur].limit; k++)
                    if (!removed[k]) seq.idx.push_back(k);
                cur = chainNext[cur];
            }
            sequences.push_back(std::move(seq));
        }
        for (int r = 0; r < n; r++) {
            if (done[r]) continue;
            Seq seq;
            seq.level = runs[r].level;
            for (int32_t k = runs[r].start; k < runs[r].limit; k++)
                if (!removed[k]) seq.idx.push_back(k);
            sequences.push_back(std::move(seq));
        }
    }

    // X10: sos/eos depend on the higher level on either side of the sequence.
    void computeSosEos(Seq& seq) const {
        const int32_t first = seq.idx.front();
        const int32_t last = seq.idx.back();
        const bool firstIsInitiator = isIsolateInitiator(origTypes[first]);
        int32_t p = first - 1;
        while (p >= 0 && removed[p]) p--;
        uint8_t prevLevel;
        if (p < 0) {
            prevLevel = paraLevel;
        } else if (firstIsInitiator && levels[p] > seq.level) {
            // The char before an isolate initiator cannot be inside an embedding
            // that is still open (the initiator would then be deeper), so a deeper
            // predecessor means that embedding has already closed: the active level
            // at the boundary is the container level (= seq.level).
            prevLevel = seq.level;
        } else {
            prevLevel = levels[p];
        }
        seq.sos = ((std::max(seq.level, prevLevel)) & 1) ? kR : kL;

        const uint8_t lastType = origTypes[last];
        const bool danglingInitiator = isIsolateInitiator(lastType) && isoMatch[last] < 0;
        uint8_t nextLevel = paraLevel;
        if (!danglingInitiator) {
            int32_t q = last + 1;
            while (q < length && removed[q]) q++;
            if (q < length) nextLevel = levels[q];
        }
        seq.eos = ((std::max(levels[last], nextLevel)) & 1) ? kR : kL;
    }

    // W1-W7: resolve weak types within an isolating run sequence.
    void resolveW(Seq& seq) {
        const auto& ix = seq.idx;
        const int n = static_cast<int>(ix.size());
        if (n == 0) return;

        // W1: NSM adopts the type of the previous char (sos at the start). Isolate
        // initiators/PDI are already ON (X10), so an NSM following them becomes ON.
        for (int k = 0; k < n; k++) {
            if (types[ix[k]] == kNSM) types[ix[k]] = (k == 0) ? seq.sos : types[ix[k - 1]];
        }
        // W2: EN whose nearest preceding strong type (R/L/AL/sos) is AL -> AN.
        for (int k = 0; k < n; k++) {
            if (types[ix[k]] != kEN) continue;
            uint8_t found = seq.sos;
            for (int j = k - 1; j >= 0; j--) {
                const uint8_t tj = types[ix[j]];
                if (tj == kL || tj == kR || tj == kAL) { found = tj; break; }
            }
            if (found == kAL) types[ix[k]] = kAN;
        }
        // W3: AL -> R.
        for (int k = 0; k < n; k++) if (types[ix[k]] == kAL) types[ix[k]] = kR;
        // W4: a single ES between two EN -> EN; a single CS between two same-type numbers -> that type.
        for (int k = 1; k < n - 1; k++) {
            const uint8_t t = types[ix[k]];
            const uint8_t tp = types[ix[k - 1]];
            const uint8_t tn = types[ix[k + 1]];
            if (t == kES && tp == kEN && tn == kEN) types[ix[k]] = kEN;
            else if (t == kCS && tp == tn && (tp == kEN || tp == kAN)) types[ix[k]] = tp;
        }
        // W5: any run of ETs adjacent to an EN -> EN.
        for (int k = 0; k < n; k++) {
            if (types[ix[k]] != kET) continue;
            int a = k, b = k;
            while (a > 0 && types[ix[a - 1]] == kET) a--;
            while (b < n - 1 && types[ix[b + 1]] == kET) b++;
            const bool adjEN = (a > 0 && types[ix[a - 1]] == kEN) ||
                               (b < n - 1 && types[ix[b + 1]] == kEN);
            if (adjEN) for (int m = a; m <= b; m++) types[ix[m]] = kEN;
            k = b;
        }
        // W6: remaining ES/ET/CS -> ON.
        for (int k = 0; k < n; k++) {
            const uint8_t t = types[ix[k]];
            if (t == kES || t == kET || t == kCS) types[ix[k]] = kON;
        }
        // W7: EN whose nearest preceding strong type (R/L/sos) is L -> L.
        for (int k = 0; k < n; k++) {
            if (types[ix[k]] != kEN) continue;
            uint8_t found = seq.sos;
            for (int j = k - 1; j >= 0; j--) {
                const uint8_t tj = types[ix[j]];
                if (tj == kL || tj == kR) { found = tj; break; }
            }
            if (found == kL) types[ix[k]] = kL;
        }
    }

    // Directional influence for N0/N1, in a 0-free encoding so it can be tested
    // for "found" without colliding with kL (which is 0): 0 = none, 1 = L, 2 = R.
    // EN and AN act as R for neutral resolution.
    static uint8_t n1Dir(uint8_t t) {
        if (t == kL) return 1;
        if (t == kR || t == kEN || t == kAN) return 2;
        return 0;
    }
    static uint8_t dirOfSos(uint8_t sosOrEos) { return (sosOrEos == kR) ? 2 : 1; }  // kL->1, kR->2
    static uint8_t dirToType(uint8_t dir) { return (dir == 2) ? kR : kL; }

    void setBracketType(const Seq& seq, int k, uint8_t d) {
        const auto& ix = seq.idx;
        types[ix[k]] = d;
        // NSMs (original type) immediately following the bracket adopt its type.
        for (int m = k + 1; m < static_cast<int>(ix.size()); m++) {
            if (origTypes[ix[m]] == kNSM) types[ix[m]] = d;
            else break;
        }
    }

    // N0: resolve bracket pairs within an isolating run sequence.
    void resolveN0(Seq& seq) {
        const auto& ix = seq.idx;
        const int n = static_cast<int>(ix.size());
        if (n == 0) return;
        const uint8_t e = (seq.level & 1) ? 2 : 1;  // embedding direction (1=L, 2=R)
        const uint8_t o = (seq.level & 1) ? 1 : 2;  // opposite direction

        // BD16: find bracket pairs.
        struct StkEnt { uint32_t partner; int k; };
        std::vector<StkEnt> stk;
        struct Pair { int openK; int closeK; };
        std::vector<Pair> pairs;
        for (int k = 0; k < n; k++) {
            if (types[ix[k]] != kON) continue;  // BD14/BD15: only current type ON
            const uint32_t c = codePointAt(ix[k]);
            uint32_t partner = 0;
            const int kind = bracketKind(c, &partner);
            if (kind == 1) {
                if (static_cast<int>(stk.size()) < 63) stk.push_back({partner, k});
                else { pairs.clear(); break; }
            } else if (kind == 2) {
                for (int s = static_cast<int>(stk.size()) - 1; s >= 0; s--) {
                    if (bracketEq(stk[s].partner, c)) {
                        pairs.push_back({stk[s].k, k});
                        stk.resize(s);
                        break;
                    }
                }
            }
        }
        std::sort(pairs.begin(), pairs.end(),
                  [](const Pair& a, const Pair& b) { return a.openK < b.openK; });

        for (const auto& pr : pairs) {
            bool foundEmbed = false;
            bool foundOpposite = false;
            for (int m = pr.openK + 1; m < pr.closeK; m++) {
                const uint8_t d = n1Dir(types[ix[m]]);
                if (d == 0) continue;
                if (d == e) { foundEmbed = true; break; }
                foundOpposite = true;
            }
            if (foundEmbed) {
                setBracketType(seq, pr.openK, dirToType(e));
                setBracketType(seq, pr.closeK, dirToType(e));
                continue;
            }
            if (foundOpposite) {
                uint8_t preceding = dirOfSos(seq.sos);
                for (int j = pr.openK - 1; j >= 0; j--) {
                    const uint8_t d = n1Dir(types[ix[j]]);
                    if (d) { preceding = d; break; }
                }
                const uint8_t dir = (preceding == o) ? o : e;
                setBracketType(seq, pr.openK, dirToType(dir));
                setBracketType(seq, pr.closeK, dirToType(dir));
                continue;
            }
            // no strong type inside: leave to N1/N2
        }
    }

    // N0-N2.
    void resolveN(Seq& seq) {
        const auto& ix = seq.idx;
        const int n = static_cast<int>(ix.size());
        if (n == 0) return;
        resolveN0(seq);

        const uint8_t e = (seq.level & 1) ? kR : kL;
        // N1: a run of NIs between two same-direction strong contexts adopts that direction.
        for (int k = 0; k < n; k++) {
            if (!isNI(types[ix[k]])) continue;
            int ke = k;
            while (ke < n && isNI(types[ix[ke]])) ke++;
            uint8_t leftDir = dirOfSos(seq.sos);
            for (int j = k - 1; j >= 0; j--) {
                const uint8_t d = n1Dir(types[ix[j]]);
                if (d) { leftDir = d; break; }
            }
            uint8_t rightDir = dirOfSos(seq.eos);
            for (int j = ke; j < n; j++) {
                const uint8_t d = n1Dir(types[ix[j]]);
                if (d) { rightDir = d; break; }
            }
            if (leftDir == rightDir) {
                const uint8_t res = dirToType(leftDir);
                for (int m = k; m < ke; m++) types[ix[m]] = res;
            }
            k = ke - 1;
        }
        // N2: any remaining NIs take the embedding direction.
        for (int k = 0; k < n; k++) if (isNI(types[ix[k]])) types[ix[k]] = e;
    }

    // I1-I2: adjust levels based on the resolved types.
    void resolveImplicit() {
        for (int32_t i = 0; i < length; i++) {
            if (removed[i]) continue;
            const uint8_t t = types[i];
            if ((levels[i] & 1) == 0) {       // I1: even level
                if (t == kR) levels[i] += 1;
                else if (t == kAN || t == kEN) levels[i] += 2;
            } else {                           // I2: odd level
                if (t == kL || t == kEN || t == kAN) levels[i] += 1;
            }
        }
    }

    // L1: reset segment/paragraph separators and surrounding whitespace/isolate
    // formatting to the paragraph level. Uses original types. X9-removed formatting
    // characters are transparent to these scans (they are virtually removed), so a
    // trailing/preceding whitespace sequence continues through them.
    void applyL1() {
        for (int32_t i = 0; i < length; i++) {
            const uint8_t t = origTypes[i];
            if (t == kS || t == kB) {
                levels[i] = paraLevel;
                for (int32_t j = i - 1; j >= 0; j--) {
                    const uint8_t tj = origTypes[j];
                    if (tj == kWS || isIsolateInitiator(tj) || tj == kPDI) {
                        levels[j] = paraLevel;
                    } else if (removed[j]) {
                        continue;  // skip X9-removed formatting (invisible)
                    } else {
                        break;
                    }
                }
            }
        }
        for (int32_t j = length - 1; j >= 0; j--) {
            const uint8_t tj = origTypes[j];
            if (tj == kWS || isIsolateInitiator(tj) || tj == kPDI) {
                levels[j] = paraLevel;
            } else if (removed[j]) {
                continue;
            } else {
                break;
            }
        }
    }

    void resolve(uint8_t requested) {
        if (length == 0) { paraLevel = 0; levels.clear(); return; }
        paraLevel = resolveParaLevel(requested);
        precomputeTypes();
        applyExplicit();
        pairIsolates();
        buildLevelRuns();
        buildSequences();
        // X10: treat isolate initiators and PDI as ON for the W and N rules.
        for (int32_t i = 0; i < length; i++) {
            const uint8_t t = origTypes[i];
            if (isIsolateInitiator(t) || t == kPDI) types[i] = kON;
        }
        for (auto& s : sequences) {
            computeSosEos(s);
            resolveW(s);
            resolveN(s);
        }
        resolveImplicit();
        applyL1();
    }
};

// L2 reordering. Produces `order` where order[visualPos] = logicalIndex.
void reorderVisual(const uint8_t* levels, int32_t len, std::vector<int32_t>& order) {
    order.resize(len);
    for (int32_t i = 0; i < len; i++) order[i] = i;
    uint8_t maxL = 0, minOdd = 255;
    for (int32_t i = 0; i < len; i++) {
        if (levels[i] > maxL) maxL = levels[i];
        if ((levels[i] & 1) && levels[i] < minOdd) minOdd = levels[i];
    }
    for (uint8_t L = maxL; L >= minOdd && minOdd != 255; L--) {
        int32_t a = 0;
        while (a < len) {
            while (a < len && levels[order[a]] < L) a++;
            if (a >= len) break;
            int32_t b = a;
            while (b < len && levels[order[b]] >= L) b++;
            for (int32_t lo = a, hi = b - 1; lo < hi; ++lo, --hi) std::swap(order[lo], order[hi]);
            a = b;
        }
    }
}

// Group the L2-ordered indices into runs in visual order. Each run covers a
// contiguous logical range at a single level. RTL runs return the low logical
// index as the start (the consumer draws the range in RTL within the run).
void buildVisualRuns(const uint8_t* levels, int32_t len,
                     std::vector<int32_t>& runStart, std::vector<int32_t>& runLen,
                     std::vector<uint8_t>& runLevel) {
    if (len == 0) return;
    std::vector<int32_t> order;
    reorderVisual(levels, len, order);
    int32_t k = 0;
    while (k < len) {
        const uint8_t lvl = levels[order[k]];
        const int32_t start = k;
        while (k < len && levels[order[k]] == lvl) k++;
        const int32_t rlen = k - start;
        const int32_t ls = ((lvl & 1) == 0) ? order[start] : order[k - 1];
        runStart.push_back(ls);
        runLen.push_back(rlen);
        runLevel.push_back(lvl);
    }
}

}  // namespace

struct UBiDi {
    const UChar* text;
    int32_t length;
    UBiDiLevel paraLevel;
    UBiDiDirection direction;
    int32_t runCount;
    UBiDiClassCallback* callback;
    const void* callbackContext;

    UBiDiLevel* levels;     // resolved embedding levels (logical order)
    int32_t* runStarts;     // logical start of each visual run
    int32_t* runLengths;
    UBiDiLevel* runLevels;
    int32_t levelsCapacity;
    int32_t runsCapacity;

    UBool isInverse;
    UBool orderParagraphsLTR;
    UBiDiReorderingMode reorderingMode;
    uint32_t reorderingOptions;
};

static void freeBuffers(UBiDi* pBiDi) {
    if (pBiDi->levels)      { free(pBiDi->levels);      pBiDi->levels = nullptr; }
    if (pBiDi->runStarts)   { free(pBiDi->runStarts);   pBiDi->runStarts = nullptr; }
    if (pBiDi->runLengths)  { free(pBiDi->runLengths);  pBiDi->runLengths = nullptr; }
    if (pBiDi->runLevels)   { free(pBiDi->runLevels);   pBiDi->runLevels = nullptr; }
    pBiDi->levelsCapacity = 0;
    pBiDi->runsCapacity = 0;
}

static void ensureLevelsCapacity(UBiDi* pBiDi, int32_t capacity) {
    if (capacity <= 0) return;
    if (pBiDi->levelsCapacity < capacity) {
        freeBuffers(pBiDi);
        pBiDi->levels = static_cast<UBiDiLevel*>(malloc(capacity * sizeof(UBiDiLevel)));
        pBiDi->levelsCapacity = capacity;
    }
}

static void ensureRunsCapacity(UBiDi* pBiDi, int32_t capacity) {
    if (capacity <= 0) return;
    if (pBiDi->runsCapacity < capacity) {
        if (pBiDi->runStarts)  free(pBiDi->runStarts);
        if (pBiDi->runLengths) free(pBiDi->runLengths);
        if (pBiDi->runLevels)  free(pBiDi->runLevels);
        pBiDi->runStarts  = static_cast<int32_t*>(malloc(capacity * sizeof(int32_t)));
        pBiDi->runLengths = static_cast<int32_t*>(malloc(capacity * sizeof(int32_t)));
        pBiDi->runLevels  = static_cast<UBiDiLevel*>(malloc(capacity * sizeof(UBiDiLevel)));
        pBiDi->runsCapacity = capacity;
    }
}

U_CAPI UBiDi* U_EXPORT2 ubidi_open(void) {
    UBiDi* pBiDi = static_cast<UBiDi*>(calloc(1, sizeof(UBiDi)));
    if (pBiDi) {
        pBiDi->paraLevel = 0;
        pBiDi->direction = UBIDI_LTR;
        pBiDi->runCount = 0;
        pBiDi->orderParagraphsLTR = true;
        pBiDi->reorderingMode = UBIDI_REORDER_DEFAULT;
        pBiDi->reorderingOptions = UBIDI_OPTION_DEFAULT;
    }
    return pBiDi;
}

U_CAPI UBiDi* U_EXPORT2 ubidi_openSized(int32_t maxLength, int32_t maxRunCount, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) return nullptr;
    if (maxLength < 0 || maxRunCount < 0) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return nullptr;
    }
    UBiDi* pBiDi = ubidi_open();
    if (pBiDi == nullptr) {
        *pErrorCode = U_MEMORY_ALLOCATION_ERROR;
        return nullptr;
    }
    if (maxLength > 0) ensureLevelsCapacity(pBiDi, maxLength);
    if (maxRunCount > 0) ensureRunsCapacity(pBiDi, maxRunCount);
    return pBiDi;
}

U_CAPI void U_EXPORT2 ubidi_close(UBiDi* pBiDi) {
    if (pBiDi) {
        freeBuffers(pBiDi);
        free(pBiDi);
    }
}

U_CAPI void U_EXPORT2 ubidi_setInverse(UBiDi* pBiDi, UBool isInverse) {
    if (pBiDi) pBiDi->isInverse = isInverse;
}

U_CAPI UBool U_EXPORT2 ubidi_isInverse(UBiDi* pBiDi) {
    return pBiDi ? pBiDi->isInverse : false;
}

U_CAPI void U_EXPORT2 ubidi_orderParagraphsLTR(UBiDi* pBiDi, UBool orderParagraphsLTR) {
    if (pBiDi) pBiDi->orderParagraphsLTR = orderParagraphsLTR;
}

U_CAPI UBool U_EXPORT2 ubidi_isOrderParagraphsLTR(UBiDi* pBiDi) {
    return pBiDi ? pBiDi->orderParagraphsLTR : true;
}

U_CAPI void U_EXPORT2 ubidi_setReorderingMode(UBiDi* pBiDi, UBiDiReorderingMode mode) {
    if (pBiDi) pBiDi->reorderingMode = mode;
}

U_CAPI UBiDiReorderingMode U_EXPORT2 ubidi_getReorderingMode(UBiDi* pBiDi) {
    return pBiDi ? pBiDi->reorderingMode : UBIDI_REORDER_DEFAULT;
}

U_CAPI void U_EXPORT2 ubidi_setReorderingOptions(UBiDi* pBiDi, uint32_t options) {
    if (pBiDi) pBiDi->reorderingOptions = options;
}

U_CAPI uint32_t U_EXPORT2 ubidi_getReorderingOptions(UBiDi* pBiDi) {
    return pBiDi ? pBiDi->reorderingOptions : UBIDI_OPTION_DEFAULT;
}

U_CAPI void U_EXPORT2 ubidi_setContext(UBiDi*, const UChar*, int32_t, const UChar*, int32_t, UErrorCode*) {
    // Context prologue/epilogue not supported in this minimal implementation.
}

U_CAPI void U_EXPORT2 ubidi_setPara(UBiDi* pBiDi, const UChar* text, int32_t length,
                                    UBiDiLevel paraLevel, UBiDiLevel* embeddingLevels,
                                    UErrorCode* pErrorCode) {
    if (pBiDi == nullptr || pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) return;

    pBiDi->text = text;
    pBiDi->length = length;
    pBiDi->runCount = 0;

    if (length == 0) {
        pBiDi->paraLevel = 0;
        pBiDi->direction = UBIDI_LTR;
        return;
    }

    ensureLevelsCapacity(pBiDi, length);
    ensureRunsCapacity(pBiDi, length);

    BidiResolver r;
    r.text = text;
    r.length = length;
    r.callback = pBiDi->callback;
    r.callbackCtx = pBiDi->callbackContext;
    r.resolve(paraLevel);

    pBiDi->paraLevel = r.paraLevel;
    memcpy(pBiDi->levels, r.levels.data(), length * sizeof(UBiDiLevel));
    if (embeddingLevels != nullptr) memcpy(embeddingLevels, pBiDi->levels, length * sizeof(UBiDiLevel));

    std::vector<int32_t> runStart, runLen;
    std::vector<uint8_t> runLevel;
    buildVisualRuns(pBiDi->levels, length, runStart, runLen, runLevel);

    pBiDi->runCount = static_cast<int32_t>(runStart.size());
    ensureRunsCapacity(pBiDi, std::max<int32_t>(pBiDi->runCount, 1));
    for (int32_t i = 0; i < pBiDi->runCount; i++) {
        pBiDi->runStarts[i] = runStart[i];
        pBiDi->runLengths[i] = runLen[i];
        pBiDi->runLevels[i] = runLevel[i];
    }

    bool hasL = false, hasR = false;
    for (int32_t i = 0; i < pBiDi->runCount; i++) {
        if (pBiDi->runLevels[i] & 1) hasR = true; else hasL = true;
    }
    pBiDi->direction = (hasL && hasR) ? UBIDI_MIXED
                                       : (hasR ? UBIDI_RTL : UBIDI_LTR);
}

U_CAPI void U_EXPORT2 ubidi_setLine(const UBiDi* pParaBiDi, int32_t start, int32_t limit,
                                    UBiDi* pLineBiDi, UErrorCode* pErrorCode) {
    if (pParaBiDi == nullptr || pLineBiDi == nullptr ||
        pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return;
    }
    if (start < 0 || limit > pParaBiDi->length || start >= limit) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }

    pLineBiDi->text = pParaBiDi->text + start;
    pLineBiDi->length = limit - start;
    pLineBiDi->paraLevel = pParaBiDi->paraLevel;
    pLineBiDi->runCount = 0;

    ensureLevelsCapacity(pLineBiDi, pLineBiDi->length);
    memcpy(pLineBiDi->levels, pParaBiDi->levels + start, pLineBiDi->length * sizeof(UBiDiLevel));

    std::vector<int32_t> runStart, runLen;
    std::vector<uint8_t> runLevel;
    buildVisualRuns(pLineBiDi->levels, pLineBiDi->length, runStart, runLen, runLevel);

    pLineBiDi->runCount = static_cast<int32_t>(runStart.size());
    ensureRunsCapacity(pLineBiDi, std::max<int32_t>(pLineBiDi->runCount, 1));
    for (int32_t i = 0; i < pLineBiDi->runCount; i++) {
        pLineBiDi->runStarts[i] = runStart[i];
        pLineBiDi->runLengths[i] = runLen[i];
        pLineBiDi->runLevels[i] = runLevel[i];
    }

    bool hasL = false, hasR = false;
    for (int32_t i = 0; i < pLineBiDi->runCount; i++) {
        if (pLineBiDi->runLevels[i] & 1) hasR = true; else hasL = true;
    }
    pLineBiDi->direction = (hasL && hasR) ? UBIDI_MIXED
                                          : (hasR ? UBIDI_RTL : UBIDI_LTR);
}

U_CAPI UBiDiDirection U_EXPORT2 ubidi_getDirection(const UBiDi* pBiDi) {
    return pBiDi ? pBiDi->direction : UBIDI_LTR;
}

U_CAPI UBiDiDirection U_EXPORT2 ubidi_getBaseDirection(const UChar* text, int32_t length) {
    if (text == nullptr || length <= 0) return UBIDI_NEUTRAL;
    int isoDepth = 0;
    for (int32_t i = 0; i < length; ) {
        UChar32 c = text[i];
        int32_t n = i + 1;
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < length) {
            UChar c2 = text[i + 1];
            if (c2 >= 0xDC00 && c2 <= 0xDFFF) {
                c = 0x10000 + (((c - 0xD800) << 10) | (c2 - 0xDC00));
                n = i + 2;
            }
        }
        const UCharDirection t = u_charDirection(c);
        if (isoDepth == 0) {
            if (t == U_LEFT_TO_RIGHT) return UBIDI_LTR;
            if (t == U_RIGHT_TO_LEFT || t == U_RIGHT_TO_LEFT_ARABIC) return UBIDI_RTL;
        }
        if (t == U_LEFT_TO_RIGHT_ISOLATE || t == U_RIGHT_TO_LEFT_ISOLATE || t == U_FIRST_STRONG_ISOLATE) isoDepth++;
        else if (t == U_POP_DIRECTIONAL_ISOLATE && isoDepth > 0) isoDepth--;
        i = n;
    }
    return UBIDI_NEUTRAL;
}

U_CAPI const UChar* U_EXPORT2 ubidi_getText(const UBiDi* pBiDi) {
    return pBiDi ? pBiDi->text : nullptr;
}

U_CAPI int32_t U_EXPORT2 ubidi_getLength(const UBiDi* pBiDi) {
    return pBiDi ? pBiDi->length : 0;
}

U_CAPI UBiDiLevel U_EXPORT2 ubidi_getParaLevel(const UBiDi* pBiDi) {
    return pBiDi ? pBiDi->paraLevel : 0;
}

U_CAPI int32_t U_EXPORT2 ubidi_countParagraphs(UBiDi* pBiDi) {
    if (pBiDi == nullptr) return 0;
    int32_t count = 0;
    for (int32_t i = 0; i < pBiDi->length; i++) {
        if (pBiDi->text[i] == 0x0A || pBiDi->text[i] == 0x0D) {
            count++;
            if (i + 1 < pBiDi->length && pBiDi->text[i] == 0x0D && pBiDi->text[i + 1] == 0x0A) i++;
        }
    }
    return count + 1;
}

U_CAPI int32_t U_EXPORT2 ubidi_getParagraph(const UBiDi* pBiDi, int32_t charIndex, int32_t* pParaStart,
                                            int32_t* pParaEnd, UBiDiLevel* pParaLevel, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) return -1;
    if (pBiDi == nullptr) { *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR; return -1; }
    if (charIndex < 0 || charIndex > pBiDi->length) {
        *pErrorCode = U_INDEX_OUTOFBOUNDS_ERROR;
        return -1;
    }

    int32_t paraStart = 0;
    int32_t paraIndex = 0;
    for (int32_t i = 0; i < pBiDi->length; i++) {
        if (pBiDi->text[i] == 0x0A || pBiDi->text[i] == 0x0D) {
            if (i >= charIndex) break;
            paraStart = i + 1;
            if (i + 1 < pBiDi->length && pBiDi->text[i] == 0x0D && pBiDi->text[i + 1] == 0x0A) { paraStart++; i++; }
            paraIndex++;
        }
    }
    int32_t paraEnd = pBiDi->length;
    for (int32_t i = paraStart; i < pBiDi->length; i++) {
        if (pBiDi->text[i] == 0x0A || pBiDi->text[i] == 0x0D) { paraEnd = i; break; }
    }
    if (pParaStart) *pParaStart = paraStart;
    if (pParaEnd) *pParaEnd = paraEnd;
    if (pParaLevel) *pParaLevel = pBiDi->paraLevel;
    return paraIndex;
}

U_CAPI void U_EXPORT2 ubidi_getParagraphByIndex(const UBiDi* pBiDi, int32_t paraIndex,
                                                int32_t* pParaStart, int32_t* pParaEnd,
                                                UBiDiLevel* pParaLevel, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) return;
    if (pBiDi == nullptr) { *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR; return; }

    const int32_t paraCount = ubidi_countParagraphs(const_cast<UBiDi*>(pBiDi));
    if (paraIndex < 0 || paraIndex >= paraCount) { *pErrorCode = U_INDEX_OUTOFBOUNDS_ERROR; return; }

    int32_t currentPara = 0;
    int32_t paraStart = 0;
    for (int32_t i = 0; i < pBiDi->length; i++) {
        if (pBiDi->text[i] == 0x0A || pBiDi->text[i] == 0x0D) {
            if (currentPara == paraIndex) {
                if (pParaStart) *pParaStart = paraStart;
                if (pParaEnd) *pParaEnd = i;
                if (pParaLevel) *pParaLevel = pBiDi->paraLevel;
                return;
            }
            currentPara++;
            paraStart = i + 1;
            if (i + 1 < pBiDi->length && pBiDi->text[i] == 0x0D && pBiDi->text[i + 1] == 0x0A) { paraStart++; i++; }
        }
    }
    if (currentPara == paraIndex) {
        if (pParaStart) *pParaStart = paraStart;
        if (pParaEnd) *pParaEnd = pBiDi->length;
        if (pParaLevel) *pParaLevel = pBiDi->paraLevel;
    }
}

U_CAPI UBiDiLevel U_EXPORT2 ubidi_getLevelAt(const UBiDi* pBiDi, int32_t charIndex) {
    if (pBiDi == nullptr || charIndex < 0 || charIndex >= pBiDi->length) return 0;
    return pBiDi->levels[charIndex];
}

U_CAPI const UBiDiLevel* U_EXPORT2 ubidi_getLevels(UBiDi* pBiDi, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) return nullptr;
    if (pBiDi == nullptr) { *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR; return nullptr; }
    return pBiDi->levels;
}

U_CAPI void U_EXPORT2 ubidi_getLogicalRun(const UBiDi* pBiDi, int32_t logicalPosition,
                                          int32_t* pLogicalLimit, UBiDiLevel* pLevel) {
    if (pBiDi == nullptr || logicalPosition < 0 || logicalPosition >= pBiDi->length) return;
    const UBiDiLevel lvl = pBiDi->levels[logicalPosition];
    int32_t limit = logicalPosition + 1;
    while (limit < pBiDi->length && pBiDi->levels[limit] == lvl) limit++;
    if (pLogicalLimit) *pLogicalLimit = limit;
    if (pLevel) *pLevel = lvl;
}

U_CAPI int32_t U_EXPORT2 ubidi_countRuns(UBiDi* pBiDi, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) return -1;
    if (pBiDi == nullptr) { *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR; return -1; }
    return pBiDi->runCount;
}

U_CAPI UBiDiDirection U_EXPORT2 ubidi_getVisualRun(UBiDi* pBiDi, int32_t runIndex,
                                                   int32_t* pLogicalStart, int32_t* pLength) {
    if (pBiDi == nullptr || runIndex < 0 || runIndex >= pBiDi->runCount) {
        if (pLogicalStart) *pLogicalStart = UBIDI_MAP_NOWHERE;
        if (pLength) *pLength = UBIDI_MAP_NOWHERE;
        return UBIDI_LTR;
    }
    if (pLogicalStart) *pLogicalStart = pBiDi->runStarts[runIndex];
    if (pLength) *pLength = pBiDi->runLengths[runIndex];
    return (pBiDi->runLevels[runIndex] & 1) ? UBIDI_RTL : UBIDI_LTR;
}

U_CAPI int32_t U_EXPORT2 ubidi_getVisualIndex(UBiDi* pBiDi, int32_t logicalIndex, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) return -1;
    if (pBiDi == nullptr) { *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR; return -1; }
    if (logicalIndex < 0 || logicalIndex >= pBiDi->length) {
        *pErrorCode = U_INDEX_OUTOFBOUNDS_ERROR;
        return -1;
    }
    std::vector<int32_t> order;
    reorderVisual(pBiDi->levels, pBiDi->length, order);
    return order[logicalIndex];
}

U_CAPI int32_t U_EXPORT2 ubidi_getLogicalIndex(UBiDi* pBiDi, int32_t visualIndex, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) return -1;
    if (pBiDi == nullptr) { *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR; return -1; }
    if (visualIndex < 0 || visualIndex >= pBiDi->length) {
        *pErrorCode = U_INDEX_OUTOFBOUNDS_ERROR;
        return -1;
    }
    std::vector<int32_t> order;
    reorderVisual(pBiDi->levels, pBiDi->length, order);
    return order[visualIndex];
}

U_CAPI void U_EXPORT2 ubidi_getLogicalMap(UBiDi* pBiDi, int32_t* indexMap, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) return;
    if (pBiDi == nullptr) { *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR; return; }
    if (indexMap == nullptr) return;
    std::vector<int32_t> order;
    reorderVisual(pBiDi->levels, pBiDi->length, order);
    // indexMap[visual] = logical
    for (int32_t i = 0; i < pBiDi->length; i++) indexMap[i] = order[i];
}

U_CAPI void U_EXPORT2 ubidi_getVisualMap(UBiDi* pBiDi, int32_t* indexMap, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) return;
    if (pBiDi == nullptr) { *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR; return; }
    if (indexMap == nullptr) return;
    std::vector<int32_t> order;
    reorderVisual(pBiDi->levels, pBiDi->length, order);
    // indexMap[logical] = visual
    for (int32_t i = 0; i < pBiDi->length; i++) indexMap[order[i]] = i;
}

// L2 reorder producing indexMap[visual] = logical.
U_CAPI void U_EXPORT2 ubidi_reorderLogical(const UBiDiLevel* levels, int32_t length, int32_t* indexMap) {
    if (levels == nullptr || length <= 0 || indexMap == nullptr) return;
    std::vector<int32_t> order;
    reorderVisual(levels, length, order);
    for (int32_t i = 0; i < length; i++) indexMap[order[i]] = i;  // logical -> visual
}

U_CAPI void U_EXPORT2 ubidi_reorderVisual(const UBiDiLevel* levels, int32_t length, int32_t* indexMap) {
    if (levels == nullptr || length <= 0 || indexMap == nullptr) return;
    std::vector<int32_t> order;
    reorderVisual(levels, length, order);
    for (int32_t i = 0; i < length; i++) indexMap[i] = order[i];  // visual -> logical
}

U_CAPI void U_EXPORT2 ubidi_invertMap(const int32_t* srcMap, int32_t* destMap, int32_t length) {
    if (srcMap == nullptr || destMap == nullptr || length <= 0) return;
    for (int32_t i = 0; i < length; i++) destMap[srcMap[i]] = i;
}

U_CAPI int32_t U_EXPORT2 ubidi_getProcessedLength(const UBiDi* pBiDi) {
    return pBiDi ? pBiDi->length : 0;
}

U_CAPI UCharDirection U_EXPORT2 ubidi_getCustomizedClass(UBiDi* pBiDi, UChar32 c) {
    if (pBiDi && pBiDi->callback) return pBiDi->callback(pBiDi->callbackContext, c);
    return U_OTHER_NEUTRAL;
}

U_CAPI void U_EXPORT2 ubidi_setClassCallback(UBiDi* pBiDi, UBiDiClassCallback* newFn,
                                             const void* newContext, UBiDiClassCallback** oldFn,
                                             const void** oldContext, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) return;
    if (pBiDi == nullptr) { *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR; return; }
    if (oldFn) *oldFn = pBiDi->callback;
    if (oldContext) *oldContext = pBiDi->callbackContext;
    pBiDi->callback = newFn;
    pBiDi->callbackContext = newContext;
}
