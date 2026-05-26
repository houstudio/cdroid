#include "unicode/ubidi.h"
#include "unicode/utf16.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>

struct UBiDi {
    const UChar* text;
    int32_t length;
    UBiDiLevel paraLevel;
    UBiDiDirection direction;
    int32_t runCount;
    UBiDiClassCallback* callback;
    const void* callbackContext;
    
    // Additional fields for full bidi implementation
    UBiDiLevel* levels;
    int32_t* runs;
    int32_t* runLengths;
    UBiDiLevel* runLevels;
    int32_t levelsCapacity;
    int32_t runsCapacity;
    
    // Fields for advanced bidi features
    UBool isInverse;
    UBool orderParagraphsLTR;
    UBiDiReorderingMode reorderingMode;
    uint32_t reorderingOptions;
    
    // Context fields
    const UChar* prologue;
    int32_t proLength;
    const UChar* epilogue;
    int32_t epiLength;
    
    // Fields for controls handling (UBIDI_OPTION_INSERT_MARKS/REMOVE_CONTROLS)
    UChar* processedText;
    int32_t processedLength;
    int32_t* originalIndices;  // Maps processed index to original index
};

static UCharDirection getCharDirection(UBiDi* pBiDi, UChar32 c) {
    if (pBiDi != nullptr && pBiDi->callback != nullptr) {
        return pBiDi->callback(pBiDi->callbackContext, c);
    }
    return u_charDirection(c);
}

static void freeLevelsAndRuns(UBiDi* pBiDi) {
    if (pBiDi->levels != nullptr) {
        free(pBiDi->levels);
        pBiDi->levels = nullptr;
    }
    if (pBiDi->runs != nullptr) {
        free(pBiDi->runs);
        pBiDi->runs = nullptr;
    }
    if (pBiDi->runLengths != nullptr) {
        free(pBiDi->runLengths);
        pBiDi->runLengths = nullptr;
    }
    if (pBiDi->runLevels != nullptr) {
        free(pBiDi->runLevels);
        pBiDi->runLevels = nullptr;
    }
    if (pBiDi->processedText != nullptr) {
        free(pBiDi->processedText);
        pBiDi->processedText = nullptr;
    }
    if (pBiDi->originalIndices != nullptr) {
        free(pBiDi->originalIndices);
        pBiDi->originalIndices = nullptr;
    }
    pBiDi->levelsCapacity = 0;
    pBiDi->runsCapacity = 0;
    pBiDi->processedLength = 0;
}

static void ensureLevelsCapacity(UBiDi* pBiDi, int32_t capacity) {
    if (pBiDi->levelsCapacity < capacity) {
        freeLevelsAndRuns(pBiDi);
        pBiDi->levels = (UBiDiLevel*)malloc(capacity * sizeof(UBiDiLevel));
        pBiDi->levelsCapacity = capacity;
    }
}

static void ensureRunsCapacity(UBiDi* pBiDi, int32_t capacity) {
    if (pBiDi->runsCapacity < capacity) {
        if (pBiDi->runs != nullptr) free(pBiDi->runs);
        if (pBiDi->runLengths != nullptr) free(pBiDi->runLengths);
        if (pBiDi->runLevels != nullptr) free(pBiDi->runLevels);
        pBiDi->runs = (int32_t*)malloc(capacity * sizeof(int32_t));
        pBiDi->runLengths = (int32_t*)malloc(capacity * sizeof(int32_t));
        pBiDi->runLevels = (UBiDiLevel*)malloc(capacity * sizeof(UBiDiLevel));
        pBiDi->runsCapacity = capacity;
    }
}

U_CAPI UBiDi* U_EXPORT2 ubidi_open(void) {
    UBiDi* pBiDi = (UBiDi*)malloc(sizeof(UBiDi));
    if (pBiDi != nullptr) {
        pBiDi->text = nullptr;
        pBiDi->length = 0;
        pBiDi->paraLevel = 0;
        pBiDi->direction = UBIDI_LTR;
        pBiDi->runCount = 1;
        pBiDi->callback = nullptr;
        pBiDi->callbackContext = nullptr;
        pBiDi->levels = nullptr;
        pBiDi->runs = nullptr;
        pBiDi->runLengths = nullptr;
        pBiDi->runLevels = nullptr;
        pBiDi->levelsCapacity = 0;
        pBiDi->runsCapacity = 0;
        
        // Initialize advanced bidi fields
        pBiDi->isInverse = false;
        pBiDi->orderParagraphsLTR = true;
        pBiDi->reorderingMode = UBIDI_REORDER_DEFAULT;
        pBiDi->reorderingOptions = UBIDI_OPTION_DEFAULT;
        
        // Initialize context fields
        pBiDi->prologue = nullptr;
        pBiDi->proLength = 0;
        pBiDi->epilogue = nullptr;
        pBiDi->epiLength = 0;
        
        // Initialize controls handling fields
        pBiDi->processedText = nullptr;
        pBiDi->processedLength = 0;
        pBiDi->originalIndices = nullptr;
    }
    return pBiDi;
}

U_CAPI UBiDi* U_EXPORT2 ubidi_openSized(int32_t maxLength, int32_t maxRunCount, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr) {
        return nullptr;
    }
    if (*pErrorCode > U_ZERO_ERROR) {
        return nullptr;
    }
    if (maxLength < 0 || maxRunCount < 0) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return nullptr;
    }
    UBiDi* pBiDi = ubidi_open();
    if (pBiDi == nullptr) {
        *pErrorCode = U_MEMORY_ALLOCATION_ERROR;
        return nullptr;
    }
    if (maxLength > 0) {
        pBiDi->levels = (UBiDiLevel*)malloc(maxLength * sizeof(UBiDiLevel));
        if (pBiDi->levels == nullptr) {
            *pErrorCode = U_MEMORY_ALLOCATION_ERROR;
            ubidi_close(pBiDi);
            return nullptr;
        }
        pBiDi->levelsCapacity = maxLength;
        if (maxRunCount > 0) {
            pBiDi->runs = (int32_t*)malloc(maxRunCount * sizeof(int32_t));
            pBiDi->runLengths = (int32_t*)malloc(maxRunCount * sizeof(int32_t));
            pBiDi->runLevels = (UBiDiLevel*)malloc(maxRunCount * sizeof(UBiDiLevel));
            if (pBiDi->runs == nullptr || pBiDi->runLengths == nullptr || pBiDi->runLevels == nullptr) {
                *pErrorCode = U_MEMORY_ALLOCATION_ERROR;
                ubidi_close(pBiDi);
                return nullptr;
            }
            pBiDi->runsCapacity = maxRunCount;
        }
    }
    return pBiDi;
}

U_CAPI void U_EXPORT2 ubidi_close(UBiDi* pBiDi) {
    if (pBiDi != nullptr) {
        freeLevelsAndRuns(pBiDi);
        free(pBiDi);
    }
}

U_CAPI void U_EXPORT2 ubidi_setInverse(UBiDi* pBiDi, UBool isInverse) {
    if (pBiDi != nullptr) {
        pBiDi->isInverse = isInverse;
    }
}

U_CAPI UBool U_EXPORT2 ubidi_isInverse(UBiDi* pBiDi) {
    if (pBiDi == nullptr) {
        return false;
    }
    return pBiDi->isInverse;
}

U_CAPI void U_EXPORT2 ubidi_orderParagraphsLTR(UBiDi* pBiDi, UBool orderParagraphsLTR) {
    if (pBiDi != nullptr) {
        pBiDi->orderParagraphsLTR = orderParagraphsLTR;
    }
}

U_CAPI UBool U_EXPORT2 ubidi_isOrderParagraphsLTR(UBiDi* pBiDi) {
    if (pBiDi == nullptr) {
        return true;
    }
    return pBiDi->orderParagraphsLTR;
}

U_CAPI void U_EXPORT2 ubidi_setReorderingMode(UBiDi* pBiDi, UBiDiReorderingMode reorderingMode) {
    if (pBiDi != nullptr) {
        pBiDi->reorderingMode = reorderingMode;
    }
}

U_CAPI UBiDiReorderingMode U_EXPORT2 ubidi_getReorderingMode(UBiDi* pBiDi) {
    if (pBiDi == nullptr) {
        return UBIDI_REORDER_DEFAULT;
    }
    return pBiDi->reorderingMode;
}

U_CAPI void U_EXPORT2 ubidi_setReorderingOptions(UBiDi* pBiDi, uint32_t reorderingOptions) {
    if (pBiDi != nullptr) {
        pBiDi->reorderingOptions = reorderingOptions;
    }
}

U_CAPI uint32_t U_EXPORT2 ubidi_getReorderingOptions(UBiDi* pBiDi) {
    if (pBiDi == nullptr) {
        return UBIDI_OPTION_DEFAULT;
    }
    return pBiDi->reorderingOptions;
}

U_CAPI void U_EXPORT2 ubidi_setContext(UBiDi* pBiDi,
                                       const UChar* prologue, int32_t proLength,
                                       const UChar* epilogue, int32_t epiLength,
                                       UErrorCode* pErrorCode) {
    if (pBiDi == nullptr || pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return;
    }
    
    pBiDi->prologue = prologue;
    pBiDi->proLength = proLength;
    pBiDi->epilogue = epilogue;
    pBiDi->epiLength = epiLength;
}

static UBiDiLevel resolveParaLevel(UBiDi* pBiDi, UBiDiLevel paraLevel) {
    if (paraLevel == UBIDI_DEFAULT_LTR || paraLevel == UBIDI_DEFAULT_RTL) {
        bool hasRTL = false;
        bool hasLTR = false;
        
        for (int32_t i = 0; i < pBiDi->length; i++) {
            UChar32 c = pBiDi->text[i];
            UCharDirection dir = getCharDirection(pBiDi, c);
            
            if (dir == U_RIGHT_TO_LEFT || dir == U_RIGHT_TO_LEFT_ARABIC) {
                hasRTL = true;
            } else if (dir == U_LEFT_TO_RIGHT) {
                hasLTR = true;
            }
            
            if (hasRTL && hasLTR) break;
        }
        
        if (paraLevel == UBIDI_DEFAULT_RTL) {
            return hasLTR && !hasRTL ? 0 : 1;
        } else {
            return hasRTL && !hasLTR ? 1 : 0;
        }
    }
    return paraLevel;
}

// Check if a character is a Bidi control character
static bool isBidiControl(UChar32 c) {
    // Bidi controls: LRE (0x202A), RLE (0x202B), LRO (0x202D), RLO (0x202E), PDF (0x202C)
    // Isolate controls: LRI (0x2066), RLI (0x2067), FSI (0x2068), PDI (0x2069)
    // Arabic controls: ALM (0x061C)
    switch (c) {
        case 0x202A: // LRE
        case 0x202B: // RLE
        case 0x202C: // PDF
        case 0x202D: // LRO
        case 0x202E: // RLO
        case 0x2066: // LRI
        case 0x2067: // RLI
        case 0x2068: // FSI
        case 0x2069: // PDI
        case 0x061C: // ALM
            return true;
        default:
            return false;
    }
}

// Process text according to reordering options
// Handles UBIDI_OPTION_REMOVE_CONTROLS and UBIDI_OPTION_INSERT_MARKS
static void processTextWithOptions(UBiDi* pBiDi) {
    if (!(pBiDi->reorderingOptions & (UBIDI_OPTION_REMOVE_CONTROLS | UBIDI_OPTION_INSERT_MARKS))) {
        // No processing needed
        pBiDi->processedText = nullptr;
        pBiDi->processedLength = pBiDi->length;
        return;
    }
    
    // Allocate processed text buffer (worst case: insert marks can add up to 2 per character)
    int32_t maxLen = pBiDi->length * 3;
    pBiDi->processedText = (UChar*)malloc(maxLen * sizeof(UChar));
    pBiDi->originalIndices = (int32_t*)malloc(maxLen * sizeof(int32_t));
    
    int32_t outIdx = 0;
    UBiDiLevel currentLevel = pBiDi->paraLevel;
    
    for (int32_t i = 0; i < pBiDi->length; i++) {
        UChar32 c = pBiDi->text[i];
        
        if (pBiDi->reorderingOptions & UBIDI_OPTION_REMOVE_CONTROLS && isBidiControl(c)) {
            // Skip bidi control characters
            continue;
        }
        
        // Insert directional marks if needed (UBIDI_OPTION_INSERT_MARKS)
        if (pBiDi->reorderingOptions & UBIDI_OPTION_INSERT_MARKS) {
            // Check if we need to insert a mark
            // For simplicity, we insert LRM (0x200E) for LTR and RLM (0x200F) for RTL
            UBiDiLevel charLevel = (currentLevel & 1) ? 1 : 0;
            if (charLevel != (pBiDi->paraLevel & 1)) {
                // Insert mark before this character
                pBiDi->processedText[outIdx] = (charLevel == 0) ? 0x200E : 0x200F;
                pBiDi->originalIndices[outIdx] = i;
                outIdx++;
            }
        }
        
        pBiDi->processedText[outIdx] = c;
        pBiDi->originalIndices[outIdx] = i;
        outIdx++;
        
        // Update current level based on explicit controls (even if we're removing them)
        UCharDirection dir = getCharDirection(pBiDi, c);
        switch (dir) {
            case U_LEFT_TO_RIGHT_EMBEDDING:
                currentLevel = (currentLevel + 1) & ~1;
                break;
            case U_RIGHT_TO_LEFT_EMBEDDING:
                currentLevel = (currentLevel + 1) | 1;
                break;
            case U_LEFT_TO_RIGHT_OVERRIDE:
                currentLevel = (currentLevel & ~1) | 0;
                break;
            case U_RIGHT_TO_LEFT_OVERRIDE:
                currentLevel = (currentLevel & ~1) | 1;
                break;
            case U_POP_DIRECTIONAL_FORMAT:
                if (currentLevel > pBiDi->paraLevel) {
                    currentLevel--;
                }
                break;
            default:
                break;
        }
    }
    
    pBiDi->processedLength = outIdx;
}

// Character type classification for Bidi algorithm
static int8_t getBidiClass(UBiDi* pBiDi, UChar32 c) {
    UCharDirection dir = getCharDirection(pBiDi, c);
    switch (dir) {
        case U_LEFT_TO_RIGHT: return 0;   // L
        case U_RIGHT_TO_LEFT: return 1;   // R
        case U_RIGHT_TO_LEFT_ARABIC: return 2;   // AL
        case U_EUROPEAN_NUMBER: return 3;   // EN
        case U_EUROPEAN_NUMBER_SEPARATOR: return 4;   // ES
        case U_EUROPEAN_NUMBER_TERMINATOR: return 5;   // ET
        case U_ARABIC_NUMBER: return 6;   // AN
        case U_COMMON_NUMBER_SEPARATOR: return 7;   // CS
        case U_SEGMENT_SEPARATOR: return 8;   // B
        case U_WHITE_SPACE_NEUTRAL: return 9;   // WS
        case U_OTHER_NEUTRAL: return 10;   // ON
        case U_LEFT_TO_RIGHT_EMBEDDING: return 11;   // LRE
        case U_LEFT_TO_RIGHT_OVERRIDE: return 12;   // LRO
        case U_RIGHT_TO_LEFT_EMBEDDING: return 13;   // RLE
        case U_RIGHT_TO_LEFT_OVERRIDE: return 14;   // RLO
        case U_POP_DIRECTIONAL_FORMAT: return 15;   // PDF
        case U_LEFT_TO_RIGHT_ISOLATE: return 16;   // LRI
        case U_RIGHT_TO_LEFT_ISOLATE: return 17;   // RLI
        case U_FIRST_STRONG_ISOLATE: return 18;   // FSI
        case U_POP_DIRECTIONAL_ISOLATE: return 19;   // PDI
        default: return 10;  // ON - Other Neutral
    }
}

// Apply special number reordering mode (UBIDI_REORDER_NUMBERS_SPECIAL)
// This mode groups numbers with surrounding text differently for Windows XP compatibility
static void applySpecialNumberReordering(UBiDi* pBiDi, int8_t* types, int32_t len) {
    // In special mode, EN (European numbers) are treated as weak L characters
    // when surrounded by LTR text, but still form their own runs
    for (int32_t i = 0; i < len; i++) {
        if (types[i] == 3) { // EN
            // Check if surrounded by L characters
            bool leftIsL = (i > 0 && types[i-1] == 0);
            bool rightIsL = (i < len - 1 && types[i+1] == 0);
            
            if (leftIsL && rightIsL) {
                // EN between L characters - keep as EN but will be treated as LTR run
            } else if (leftIsL || rightIsL) {
                // EN adjacent to L but not surrounded - treat as L
                types[i] = 0; // L
            }
        }
    }
}

// Apply group numbers with R mode (UBIDI_REORDER_GROUP_NUMBERS_WITH_R)
static void applyGroupNumbersWithR(UBiDi* pBiDi, int8_t* types, int32_t len) {
    // Group EN and AN numbers with adjacent R characters
    for (int32_t i = 0; i < len; i++) {
        if (types[i] == 3 || types[i] == 6) { // EN or AN
            // Check left
            if (i > 0 && types[i-1] == 1) { // R
                types[i] = 1; // R
            }
            // Check right
            if (i < len - 1 && types[i+1] == 1) { // R
                types[i] = 1; // R
            }
        }
    }
}

static void computeEmbeddingLevels(UBiDi* pBiDi) {
    // Use processed text if available (for UBIDI_OPTION_REMOVE_CONTROLS/INSERT_MARKS)
    const UChar* srcText = (pBiDi->processedText != nullptr) ? pBiDi->processedText : pBiDi->text;
    int32_t len = (pBiDi->processedText != nullptr) ? pBiDi->processedLength : pBiDi->length;
    
    ensureLevelsCapacity(pBiDi, len);
    
    UBiDiLevel paraLevel = pBiDi->paraLevel;
    
    // Step 1: Prepare an array of character types
    int8_t* types = (int8_t*)malloc(len * sizeof(int8_t));
    for (int32_t i = 0; i < len; i++) {
        types[i] = getBidiClass(pBiDi, srcText[i]);
    }
    
    // Step 2: Initialize embedding levels array
    UBiDiLevel* levels = (UBiDiLevel*)malloc(len * sizeof(UBiDiLevel));
    for (int32_t i = 0; i < len; i++) {
        levels[i] = paraLevel;
    }
    
    // Step X1-X10: Process explicit directional marks (LRE, RLE, LRO, RLO, PDF)
    // Also handle LRI, RLI, FSI, PDI for isolation
    UBiDiLevel currentLevel = paraLevel;
    int32_t isolateCount = 0;
    
    for (int32_t i = 0; i < len; i++) {
        switch (types[i]) {
            case 11: // LRE - Left-to-Right Embedding
                currentLevel = (currentLevel + 1) & ~1;
                levels[i] = currentLevel;
                break;
            case 12: // LRO - Left-to-Right Override
                currentLevel = (currentLevel & ~1) | 0;
                levels[i] = currentLevel;
                break;
            case 13: // RLE - Right-to-Left Embedding
                currentLevel = (currentLevel + 1) | 1;
                levels[i] = currentLevel;
                break;
            case 14: // RLO - Right-to-Left Override
                currentLevel = (currentLevel & ~1) | 1;
                levels[i] = currentLevel;
                break;
            case 15: // PDF - Pop Directional Format
                if (currentLevel > paraLevel) {
                    currentLevel--;
                }
                levels[i] = currentLevel;
                break;
            case 16: // LRI - Left-to-Right Isolate
                isolateCount++;
                levels[i] = currentLevel;
                currentLevel = (currentLevel + 1) & ~1;
                break;
            case 17: // RLI - Right-to-Left Isolate
                isolateCount++;
                levels[i] = currentLevel;
                currentLevel = (currentLevel + 1) | 1;
                break;
            case 18: // FSI - First Strong Isolate
                isolateCount++;
                levels[i] = currentLevel;
                // Will determine direction later based on first strong character
                break;
            case 19: // PDI - Pop Directional Isolate
                if (isolateCount > 0 && currentLevel > paraLevel) {
                    isolateCount--;
                    currentLevel--;
                }
                levels[i] = currentLevel;
                break;
            default:
                levels[i] = currentLevel;
                break;
        }
    }
    
    // Apply reordering mode specific transformations before W rules
    switch (pBiDi->reorderingMode) {
        case UBIDI_REORDER_NUMBERS_SPECIAL:
            applySpecialNumberReordering(pBiDi, types, len);
            break;
        case UBIDI_REORDER_GROUP_NUMBERS_WITH_R:
            applyGroupNumbersWithR(pBiDi, types, len);
            break;
        case UBIDI_REORDER_INVERSE_NUMBERS_AS_L:
            // Treat all numbers as L for inverse mode
            for (int32_t i = 0; i < len; i++) {
                if (types[i] == 3 || types[i] == 6) {
                    types[i] = 0; // L
                }
            }
            break;
        default:
            break;
    }
    
    // Step W1: Examine each non-spacing mark (NSM). For each NSM, change its type
    // to the type of the base character to which it applies.
    
    // Step W2: Search backward from each instance of a European number (EN)
    // until a strong type (L, R, AL) is found. If an AL is found, change the
    // type of the EN to Arabic number (AN).
    for (int32_t i = 0; i < len; i++) {
        if (types[i] == 3) { // EN
            for (int32_t j = i - 1; j >= 0; j--) {
                if (types[j] == 0) { // L
                    break;
                } else if (types[j] == 1 || types[j] == 2) { // R or AL
                    types[i] = 6; // AN
                    break;
                }
            }
        }
    }
    
    // Step W3: Change all ALs to R
    for (int32_t i = 0; i < len; i++) {
        if (types[i] == 2) { // AL
            types[i] = 1; // R
        }
    }
    
    // Step W4: A single European separator between two European numbers
    // changes to European number. A single common separator between two
    // numbers of the same type changes to that type.
    for (int32_t i = 1; i < len - 1; i++) {
        if (types[i] == 4) { // ES
            if (types[i-1] == 3 && types[i+1] == 3) { // EN on both sides
                types[i] = 3; // EN
            }
        } else if (types[i] == 7) { // CS
            if ((types[i-1] == 3 && types[i+1] == 3) || 
                (types[i-1] == 6 && types[i+1] == 6)) { // Same number type on both sides
                types[i] = types[i-1];
            }
        }
    }
    
    // Step W5: A sequence of European terminators and/or separators
    // adjacent to European numbers changes to all EN.
    for (int32_t i = 0; i < len; i++) {
        if (types[i] == 3) { // EN
            // Look backward
            int32_t j = i - 1;
            while (j >= 0 && (types[j] == 4 || types[j] == 5)) { // ES or ET
                types[j] = 3; // EN
                j--;
            }
            // Look forward
            j = i + 1;
            while (j < len && (types[j] == 4 || types[j] == 5)) { // ES or ET
                types[j] = 3; // EN
                j++;
            }
        }
    }
    
    // Step W6: If a European number is adjacent to an Arabic number,
    // then the European number changes to Arabic number.
    for (int32_t i = 0; i < len; i++) {
        if (types[i] == 3) { // EN
            if ((i > 0 && types[i-1] == 6) || (i < len - 1 && types[i+1] == 6)) {
                types[i] = 6; // AN
            }
        }
    }
    
    // Step W7: European numbers become L, Arabic numbers become R
    UBiDiLevel* resolvedLevels = (UBiDiLevel*)malloc(len * sizeof(UBiDiLevel));
    memcpy(resolvedLevels, levels, len * sizeof(UBiDiLevel));
    
    // Step N0: Handle neutrals at start/end
    for (int32_t i = 0; i < len; i++) {
        int8_t t = types[i];
        if (t == 0 || t == 1) { // L or R
            resolvedLevels[i] = (t == 0) ? (levels[i] & ~1) : (levels[i] | 1);
        } else if (t == 3) { // EN -> L
            resolvedLevels[i] = levels[i] & ~1;
        } else if (t == 6) { // AN -> R
            resolvedLevels[i] = levels[i] | 1;
        }
    }
    
    // Step N1-N3: Resolve neutral characters
    for (int32_t i = 0; i < len; i++) {
        int8_t t = types[i];
        if (t == 8 || t == 9 || t == 10) { // B, WS, ON - neutral types
            // Find nearest strong character to the left
            int32_t left = -1;
            for (int32_t j = i - 1; j >= 0; j--) {
                if (types[j] == 0 || types[j] == 1 || types[j] == 3 || types[j] == 6) {
                    left = j;
                    break;
                }
            }
            // Find nearest strong character to the right
            int32_t right = -1;
            for (int32_t j = i + 1; j < len; j++) {
                if (types[j] == 0 || types[j] == 1 || types[j] == 3 || types[j] == 6) {
                    right = j;
                    break;
                }
            }
            
            if (left >= 0 && right >= 0) {
                // Both sides have strong characters
                if (types[left] == types[right]) {
                    resolvedLevels[i] = resolvedLevels[left];
                } else {
                    // Different directions - use paragraph level
                    resolvedLevels[i] = paraLevel;
                }
            } else if (left >= 0) {
                resolvedLevels[i] = resolvedLevels[left];
            } else if (right >= 0) {
                resolvedLevels[i] = resolvedLevels[right];
            } else {
                // No strong characters anywhere - use paragraph level
                resolvedLevels[i] = paraLevel;
            }
        }
    }
    
    // Step I1-I2: Handle bidirectional mirrors and resolve opposite directional runs
    // This is handled by the reordering logic, not level assignment
    
    // Apply context (prologue/epilogue) if enabled
    if ((pBiDi->reorderingOptions & UBIDI_OPTION_STREAMING) && 
        (pBiDi->proLength > 0 || pBiDi->epiLength > 0)) {
        // In streaming mode, consider context for boundary resolution
        // This is a simplified implementation
        if (pBiDi->proLength > 0) {
            // Use prologue to influence first character's level
            UChar32 lastChar = pBiDi->prologue[pBiDi->proLength - 1];
            int8_t lastType = getBidiClass(pBiDi, lastChar);
            if (lastType == 0 || lastType == 1) {
                resolvedLevels[0] = (lastType == 0) ? (resolvedLevels[0] & ~1) : (resolvedLevels[0] | 1);
            }
        }
    }
    
    // Copy results back
    memcpy(pBiDi->levels, resolvedLevels, len * sizeof(UBiDiLevel));
    
    free(types);
    free(levels);
    free(resolvedLevels);
}

static void detectRuns(UBiDi* pBiDi) {
    if (pBiDi->length == 0) {
        pBiDi->runCount = 0;
        return;
    }
    
    ensureRunsCapacity(pBiDi, pBiDi->length);
    
    pBiDi->runCount = 1;
    pBiDi->runs[0] = 0;
    pBiDi->runLengths[0] = 1;
    pBiDi->runLevels[0] = pBiDi->levels[0];
    
    for (int32_t i = 1; i < pBiDi->length; i++) {
        if (pBiDi->levels[i] != pBiDi->runLevels[pBiDi->runCount - 1]) {
            pBiDi->runs[pBiDi->runCount] = i;
            pBiDi->runLengths[pBiDi->runCount] = 1;
            pBiDi->runLevels[pBiDi->runCount] = pBiDi->levels[i];
            pBiDi->runCount++;
        } else {
            pBiDi->runLengths[pBiDi->runCount - 1]++;
        }
    }
}

U_CAPI void U_EXPORT2 ubidi_setPara(UBiDi* pBiDi, const UChar* text, int32_t length,
                                    UBiDiLevel paraLevel, UBiDiLevel* embeddingLevels,
                                    UErrorCode* pErrorCode) {
    if (pBiDi == nullptr || pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return;
    }
    
    pBiDi->text = text;
    pBiDi->length = length;
    
    if (length == 0) {
        pBiDi->paraLevel = 0;
        pBiDi->direction = UBIDI_LTR;
        pBiDi->runCount = 0;
        return;
    }
    
    // Process text with options (remove controls, insert marks)
    processTextWithOptions(pBiDi);
    
    pBiDi->paraLevel = resolveParaLevel(pBiDi, paraLevel);
    pBiDi->direction = (pBiDi->paraLevel & 1) ? UBIDI_RTL : UBIDI_LTR;
    
    // Compute embedding levels
    computeEmbeddingLevels(pBiDi);
    
    // Detect runs
    detectRuns(pBiDi);
    
    // Copy levels to output if requested
    if (embeddingLevels != nullptr && length > 0) {
        memcpy(embeddingLevels, pBiDi->levels, length * sizeof(UBiDiLevel));
    }
    
    // Update direction to MIXED if there are multiple runs with different directions
    if (pBiDi->runCount > 1) {
        pBiDi->direction = UBIDI_MIXED;
    }
}

U_CAPI void U_EXPORT2 ubidi_setLine(const UBiDi* pParaBiDi,
                                    int32_t start, int32_t limit,
                                    UBiDi* pLineBiDi,
                                    UErrorCode* pErrorCode) {
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
    
    ensureLevelsCapacity(pLineBiDi, pLineBiDi->length);
    memcpy(pLineBiDi->levels, pParaBiDi->levels + start, 
           pLineBiDi->length * sizeof(UBiDiLevel));
    
    // Detect runs for this line
    if (pLineBiDi->length > 0) {
        ensureRunsCapacity(pLineBiDi, pLineBiDi->length);
        
        pLineBiDi->runCount = 1;
        pLineBiDi->runs[0] = 0;
        pLineBiDi->runLengths[0] = 1;
        pLineBiDi->runLevels[0] = pLineBiDi->levels[0];
        
        for (int32_t i = 1; i < pLineBiDi->length; i++) {
            if (pLineBiDi->levels[i] != pLineBiDi->runLevels[pLineBiDi->runCount - 1]) {
                pLineBiDi->runs[pLineBiDi->runCount] = i;
                pLineBiDi->runLengths[pLineBiDi->runCount] = 1;
                pLineBiDi->runLevels[pLineBiDi->runCount] = pLineBiDi->levels[i];
                pLineBiDi->runCount++;
            } else {
                pLineBiDi->runLengths[pLineBiDi->runCount - 1]++;
            }
        }
        
        pLineBiDi->direction = (pLineBiDi->runCount > 1) ? UBIDI_MIXED : 
                               ((pLineBiDi->paraLevel & 1) ? UBIDI_RTL : UBIDI_LTR);
    } else {
        pLineBiDi->runCount = 0;
        pLineBiDi->direction = UBIDI_LTR;
    }
}

U_CAPI UBiDiDirection U_EXPORT2 ubidi_getDirection(const UBiDi* pBiDi) {
    if (pBiDi == nullptr) return UBIDI_LTR;
    return pBiDi->direction;
}

U_CAPI UBiDiDirection U_EXPORT2 ubidi_getBaseDirection(const UChar* text, int32_t length) {
    if (text == nullptr || length <= 0) return UBIDI_NEUTRAL;
    
    bool hasRTL = false;
    bool hasLTR = false;
    
    for (int32_t i = 0; i < length; i++) {
        UCharDirection dir = u_charDirection(text[i]);
        if (dir == U_RIGHT_TO_LEFT || dir == U_RIGHT_TO_LEFT_ARABIC) {
            hasRTL = true;
        } else if (dir == U_LEFT_TO_RIGHT) {
            hasLTR = true;
        }
        if (hasRTL && hasLTR) return UBIDI_MIXED;
    }
    
    if (hasRTL) return UBIDI_RTL;
    if (hasLTR) return UBIDI_LTR;
    return UBIDI_NEUTRAL;
}

U_CAPI const UChar* U_EXPORT2 ubidi_getText(const UBiDi* pBiDi) {
    if (pBiDi == nullptr) return nullptr;
    return pBiDi->text;
}

U_CAPI int32_t U_EXPORT2 ubidi_getLength(const UBiDi* pBiDi) {
    if (pBiDi == nullptr) return 0;
    return pBiDi->length;
}

U_CAPI UBiDiLevel U_EXPORT2 ubidi_getParaLevel(const UBiDi* pBiDi) {
    if (pBiDi == nullptr) return 0;
    return pBiDi->paraLevel;
}

U_CAPI int32_t U_EXPORT2 ubidi_countParagraphs(UBiDi* pBiDi) {
    if (pBiDi == nullptr || pBiDi->length == 0) {
        return 0;
    }
    
    int32_t count = 1;
    for (int32_t i = 0; i < pBiDi->length; i++) {
        UChar c = pBiDi->text[i];
        if (c == 0x000A || c == 0x000D) {  // LF or CR
            count++;
            // Skip CRLF
            if (c == 0x000D && i + 1 < pBiDi->length && pBiDi->text[i + 1] == 0x000A) {
                i++;
            }
        }
    }
    return count;
}

U_CAPI int32_t U_EXPORT2 ubidi_getParagraph(const UBiDi* pBiDi, int32_t charIndex, int32_t* pParaStart,
                                            int32_t* pParaLimit, UBiDiLevel* pParaLevel,
                                            UErrorCode* pErrorCode) {
    if (pBiDi == nullptr || pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        if (pParaStart != nullptr) *pParaStart = UBIDI_MAP_NOWHERE;
        if (pParaLimit != nullptr) *pParaLimit = UBIDI_MAP_NOWHERE;
        if (pParaLevel != nullptr) *pParaLevel = 0;
        return -1;
    }
    
    if (charIndex < 0 || charIndex >= pBiDi->length) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        if (pParaStart != nullptr) *pParaStart = UBIDI_MAP_NOWHERE;
        if (pParaLimit != nullptr) *pParaLimit = UBIDI_MAP_NOWHERE;
        if (pParaLevel != nullptr) *pParaLevel = 0;
        return -1;
    }
    
    int32_t paraIndex = 0;
    int32_t start = 0;
    
    for (int32_t i = 0; i <= charIndex; i++) {
        UChar c = pBiDi->text[i];
        if (c == 0x000A || c == 0x000D) {
            paraIndex++;
            start = i + 1;
            if (c == 0x000D && i + 1 < pBiDi->length && pBiDi->text[i + 1] == 0x000A) {
                start++;
                i++;
            }
        }
    }
    
    int32_t limit = pBiDi->length;
    for (int32_t i = charIndex + 1; i < pBiDi->length; i++) {
        UChar c = pBiDi->text[i];
        if (c == 0x000A || c == 0x000D) {
            limit = i;
            break;
        }
    }
    
    if (pParaStart != nullptr) *pParaStart = start;
    if (pParaLimit != nullptr) *pParaLimit = limit;
    if (pParaLevel != nullptr) *pParaLevel = pBiDi->paraLevel;
    
    return paraIndex;
}

U_CAPI void U_EXPORT2 ubidi_getParagraphByIndex(const UBiDi* pBiDi, int32_t paraIndex,
                                                int32_t* pParaStart, int32_t* pParaLimit,
                                                UBiDiLevel* pParaLevel, UErrorCode* pErrorCode) {
    if (pBiDi == nullptr || pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        if (pParaStart != nullptr) *pParaStart = UBIDI_MAP_NOWHERE;
        if (pParaLimit != nullptr) *pParaLimit = UBIDI_MAP_NOWHERE;
        if (pParaLevel != nullptr) *pParaLevel = 0;
        return;
    }
    
    if (paraIndex < 0) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        if (pParaStart != nullptr) *pParaStart = UBIDI_MAP_NOWHERE;
        if (pParaLimit != nullptr) *pParaLimit = UBIDI_MAP_NOWHERE;
        if (pParaLevel != nullptr) *pParaLevel = 0;
        return;
    }
    
    int32_t currentPara = 0;
    int32_t start = 0;
    
    for (int32_t i = 0; i < pBiDi->length; i++) {
        UChar c = pBiDi->text[i];
        if (c == 0x000A || c == 0x000D) {
            if (currentPara == paraIndex) {
                if (pParaStart != nullptr) *pParaStart = start;
                if (pParaLimit != nullptr) *pParaLimit = i;
                if (pParaLevel != nullptr) *pParaLevel = pBiDi->paraLevel;
                return;
            }
            currentPara++;
            start = i + 1;
            if (c == 0x000D && i + 1 < pBiDi->length && pBiDi->text[i + 1] == 0x000A) {
                start++;
                i++;
            }
        }
    }
    
    if (currentPara == paraIndex) {
        if (pParaStart != nullptr) *pParaStart = start;
        if (pParaLimit != nullptr) *pParaLimit = pBiDi->length;
        if (pParaLevel != nullptr) *pParaLevel = pBiDi->paraLevel;
    } else {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        if (pParaStart != nullptr) *pParaStart = UBIDI_MAP_NOWHERE;
        if (pParaLimit != nullptr) *pParaLimit = UBIDI_MAP_NOWHERE;
        if (pParaLevel != nullptr) *pParaLevel = 0;
    }
}

U_CAPI UBiDiLevel U_EXPORT2 ubidi_getLevelAt(const UBiDi* pBiDi, int32_t charIndex) {
    if (pBiDi == nullptr || charIndex < 0 || charIndex >= pBiDi->length) {
        return 0;
    }
    if (pBiDi->levels == nullptr) {
        return pBiDi->paraLevel;
    }
    return pBiDi->levels[charIndex];
}

U_CAPI const UBiDiLevel* U_EXPORT2 ubidi_getLevels(UBiDi* pBiDi, UErrorCode* pErrorCode) {
    if (pBiDi == nullptr || pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return nullptr;
    }
    return pBiDi->levels;
}

U_CAPI void U_EXPORT2 ubidi_getLogicalRun(const UBiDi* pBiDi, int32_t logicalPosition,
                                          int32_t* pLogicalLimit, UBiDiLevel* pLevel) {
    if (pBiDi == nullptr || logicalPosition < 0 || logicalPosition >= pBiDi->length) {
        if (pLogicalLimit != nullptr) *pLogicalLimit = UBIDI_MAP_NOWHERE;
        if (pLevel != nullptr) *pLevel = 0;
        return;
    }
    
    UBiDiLevel level = pBiDi->levels != nullptr ? pBiDi->levels[logicalPosition] : pBiDi->paraLevel;
    
    // Find the start of this run
    int32_t start = logicalPosition;
    while (start > 0) {
        UBiDiLevel prevLevel = pBiDi->levels != nullptr ? pBiDi->levels[start - 1] : pBiDi->paraLevel;
        if (prevLevel != level) break;
        start--;
    }
    
    // Find the end of this run
    int32_t limit = logicalPosition + 1;
    while (limit < pBiDi->length) {
        UBiDiLevel nextLevel = pBiDi->levels != nullptr ? pBiDi->levels[limit] : pBiDi->paraLevel;
        if (nextLevel != level) break;
        limit++;
    }
    
    if (pLogicalLimit != nullptr) *pLogicalLimit = limit;
    if (pLevel != nullptr) *pLevel = level;
}

U_CAPI int32_t U_EXPORT2 ubidi_countRuns(UBiDi* pBiDi, UErrorCode* pErrorCode) {
    if (pBiDi == nullptr || pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return -1;
    }
    return pBiDi->runCount;
}

U_CAPI UBiDiDirection U_EXPORT2 ubidi_getVisualRun(UBiDi* pBiDi, int32_t runIndex,
                                                   int32_t* pLogicalStart, int32_t* pLength) {
    if (pBiDi == nullptr || runIndex < 0 || runIndex >= pBiDi->runCount) {
        if (pLogicalStart != nullptr) *pLogicalStart = UBIDI_MAP_NOWHERE;
        if (pLength != nullptr) *pLength = UBIDI_MAP_NOWHERE;
        return UBIDI_LTR;
    }
    
    // Find the run that is at position runIndex in visual order
    // Visual order: sorted by (level, RTL before LTR at same level)
    // Create an array of indices sorted by visual order
    typedef struct {
        int32_t index;
        UBiDiLevel level;
        bool isRtl;
    } RunInfo;
    
    RunInfo sortedRuns[256];  // Assuming max 256 runs
    int32_t count = pBiDi->runCount < 256 ? pBiDi->runCount : 256;
    
    for (int32_t i = 0; i < count; i++) {
        sortedRuns[i].index = i;
        sortedRuns[i].level = pBiDi->runLevels[i];
        sortedRuns[i].isRtl = (pBiDi->runLevels[i] & 1) != 0;
    }
    
    // Bubble sort by visual order (level ASC, isRtl DESC for same level)
    for (int32_t i = 0; i < count - 1; i++) {
        for (int32_t j = i + 1; j < count; j++) {
            bool swap = false;
            if (sortedRuns[j].level < sortedRuns[i].level) {
                swap = true;
            } else if (sortedRuns[j].level == sortedRuns[i].level) {
                // Same level: RTL (isRtl=true) comes before LTR (isRtl=false)
                if (sortedRuns[j].isRtl && !sortedRuns[i].isRtl) {
                    swap = true;
                }
            }
            if (swap) {
                RunInfo temp = sortedRuns[i];
                sortedRuns[i] = sortedRuns[j];
                sortedRuns[j] = temp;
            }
        }
    }
    
    // Return the run at visual position runIndex
    int32_t visualRunIdx = sortedRuns[runIndex].index;
    if (pLogicalStart != nullptr) *pLogicalStart = pBiDi->runs[visualRunIdx];
    if (pLength != nullptr) *pLength = pBiDi->runLengths[visualRunIdx];
    return sortedRuns[runIndex].isRtl ? UBIDI_RTL : UBIDI_LTR;
}

U_CAPI int32_t U_EXPORT2 ubidi_getVisualIndex(UBiDi* pBiDi, int32_t logicalIndex, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr) {
        return -1;
    }
    if (*pErrorCode > U_ZERO_ERROR) {
        return -1;
    }
    if (pBiDi == nullptr) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return -1;
    }
    if (logicalIndex < 0 || logicalIndex >= pBiDi->length) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return -1;
    }
    
    if (pBiDi->runCount <= 1) {
        // Single run - just mirror if RTL
        if (pBiDi->direction == UBIDI_RTL) {
            return pBiDi->length - 1 - logicalIndex;
        }
        return logicalIndex;
    }
    
    // Find which run contains this logical index
    int32_t runIdx = 0;
    int32_t posInRun = 0;
    int32_t logicalPos = 0;
    
    for (int32_t i = 0; i < pBiDi->runCount; i++) {
        if (logicalIndex < logicalPos + pBiDi->runLengths[i]) {
            runIdx = i;
            posInRun = logicalIndex - logicalPos;
            break;
        }
        logicalPos += pBiDi->runLengths[i];
    }
    
    // Calculate visual position based on run order
    // Runs are stored in logical order (by level), we need to process in visual order
    // For visual order: sort runs by (level, direction) - lower levels first, RTL before LTR at same level
    int32_t visualPos = 0;
    
    // Find the target run's level
    UBiDiLevel targetLevel = pBiDi->runLevels[runIdx];
    bool targetIsRtl = (targetLevel & 1) != 0;
    
    // First pass: count visual positions of all runs that come BEFORE our run in visual order
    for (int32_t i = 0; i < pBiDi->runCount; i++) {
        if (i == runIdx) continue;
        
        UBiDiLevel level = pBiDi->runLevels[i];
        bool isRtl = (level & 1) != 0;
        
        // Determine if this run comes before our run in visual order
        bool comesBefore = false;
        if (level < targetLevel) {
            // Lower level always comes before
            comesBefore = true;
        } else if (level == targetLevel) {
            // Same level: RTL comes before LTR
            if (targetIsRtl) {
                // Our run is RTL, so LTR runs at same level come after us
                comesBefore = !isRtl;
            } else {
                // Our run is LTR, so RTL runs at same level come before us
                comesBefore = isRtl;
            }
        }
        // Higher level always comes after
        
        if (comesBefore) {
            visualPos += pBiDi->runLengths[i];
        }
    }
    
    // Now add position within our run
    if (targetIsRtl) {
        // RTL run - position from end of run
        visualPos += pBiDi->runLengths[runIdx] - 1 - posInRun;
    } else {
        // LTR run - position from start of run
        visualPos += posInRun;
    }
    
    return visualPos;
}

U_CAPI int32_t U_EXPORT2 ubidi_getLogicalIndex(UBiDi* pBiDi, int32_t visualIndex, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr) {
        return -1;
    }
    if (*pErrorCode > U_ZERO_ERROR) {
        return -1;
    }
    if (pBiDi == nullptr) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return -1;
    }
    if (visualIndex < 0 || visualIndex >= pBiDi->length) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return -1;
    }
    
    if (pBiDi->runCount <= 1) {
        // Single run - just mirror if RTL
        if (pBiDi->direction == UBIDI_RTL) {
            return pBiDi->length - 1 - visualIndex;
        }
        return visualIndex;
    }
    
    // Find which run contains this visual index by iterating in visual order
    // Visual order: lower levels first, RTL before LTR at same level
    int32_t runIdx = -1;
    int32_t posInRun = 0;
    int32_t visualPos = 0;
    
    for (int32_t i = 0; i < pBiDi->runCount; i++) {
        // Find the run with the lowest level that hasn't been processed yet
        // This is a simplified approach: find the run with smallest (level, isRtl) that contains this visual index
        // Actually, we need to iterate in visual order
        
        // Since runs are stored in logical order, we need to find which run "wins" at each visual position
        // For visual position X, we need to find which run contains it when runs are ordered by (level, RTL before LTR)
        
        // For each position in visual order, determine which logical run it belongs to
        for (int32_t j = 0; j < pBiDi->runCount; j++) {
            UBiDiLevel level = pBiDi->runLevels[j];
            bool isRtl = (level & 1) != 0;
            
            // Check if this run starts at the current visual position
            // We need to track cumulative visual positions in visual order
            
            // First, find all runs that would appear before this one in visual order
            int32_t runsBeforeThis = 0;
            for (int32_t k = 0; k < pBiDi->runCount; k++) {
                if (k == j) continue;
                UBiDiLevel kLevel = pBiDi->runLevels[k];
                bool kIsRtl = (kLevel & 1) != 0;
                
                if (kLevel < level) {
                    runsBeforeThis++;
                } else if (kLevel == level && kIsRtl && !isRtl) {
                    // Same level, but k is RTL and j is LTR, so k comes before j
                    runsBeforeThis++;
                }
            }
            
            int32_t visualStart = 0;
            for (int32_t k = 0; k < pBiDi->runCount; k++) {
                UBiDiLevel kLevel = pBiDi->runLevels[k];
                bool kIsRtl = (kLevel & 1) != 0;
                
                // Check if k comes before j in visual order
                bool kBeforeJ = false;
                if (kLevel < level) {
                    kBeforeJ = true;
                } else if (kLevel == level && kIsRtl && !isRtl) {
                    kBeforeJ = true;
                }
                
                if (kBeforeJ) {
                    visualStart += pBiDi->runLengths[k];
                }
            }
            
            if (visualIndex >= visualStart && visualIndex < visualStart + pBiDi->runLengths[j]) {
                runIdx = j;
                posInRun = visualIndex - visualStart;
                break;
            }
        }
        
        if (runIdx >= 0) break;
    }
    
    if (runIdx < 0) {
        return -1;
    }
    
    // Convert to logical position (runs are stored in logical order)
    int32_t logicalPos = 0;
    for (int32_t i = 0; i < runIdx; i++) {
        logicalPos += pBiDi->runLengths[i];
    }
    
    if (pBiDi->runLevels[runIdx] & 1) {
        // RTL run - posInRun is from visual end
        logicalPos += pBiDi->runLengths[runIdx] - 1 - posInRun;
    } else {
        // LTR run - posInRun is from visual start
        logicalPos += posInRun;
    }
    
    return logicalPos;
}

U_CAPI void U_EXPORT2 ubidi_getLogicalMap(UBiDi* pBiDi, int32_t* indexMap, UErrorCode* pErrorCode) {
    if (pBiDi == nullptr || indexMap == nullptr || pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return;
    }
    
    for (int32_t i = 0; i < pBiDi->length; i++) {
        indexMap[i] = ubidi_getLogicalIndex(pBiDi, i, pErrorCode);
    }
}

U_CAPI void U_EXPORT2 ubidi_getVisualMap(UBiDi* pBiDi, int32_t* indexMap, UErrorCode* pErrorCode) {
    if (pBiDi == nullptr || indexMap == nullptr || pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return;
    }
    
    for (int32_t i = 0; i < pBiDi->length; i++) {
        indexMap[i] = ubidi_getVisualIndex(pBiDi, i, pErrorCode);
    }
}

U_CAPI void U_EXPORT2 ubidi_reorderLogical(const UBiDiLevel* levels, int32_t length, int32_t* indexMap) {
    if (levels == nullptr || length <= 0 || indexMap == nullptr) {
        return;
    }
    
    // Create array of indices
    int32_t* indices = (int32_t*)malloc(length * sizeof(int32_t));
    for (int32_t i = 0; i < length; i++) {
        indices[i] = i;
    }
    
    // Sort by level, then by original position for even levels, or reverse for odd
    std::sort(indices, indices + length, [levels](int32_t a, int32_t b) {
        if (levels[a] != levels[b]) {
            return levels[a] < levels[b];
        }
        // Same level: even levels are LTR, odd are RTL
        if (levels[a] & 1) {
            return a > b;  // RTL: reverse order
        }
        return a < b;  // LTR: normal order
    });
    
    // Create inverse map
    for (int32_t i = 0; i < length; i++) {
        indexMap[indices[i]] = i;
    }
    
    free(indices);
}

U_CAPI void U_EXPORT2 ubidi_reorderVisual(const UBiDiLevel* levels, int32_t length, int32_t* indexMap) {
    if (levels == nullptr || length <= 0 || indexMap == nullptr) {
        return;
    }
    
    // Create array of indices
    int32_t* indices = (int32_t*)malloc(length * sizeof(int32_t));
    for (int32_t i = 0; i < length; i++) {
        indices[i] = i;
    }
    
    // Sort by level, then by original position for even levels, or reverse for odd
    std::sort(indices, indices + length, [levels](int32_t a, int32_t b) {
        if (levels[a] != levels[b]) {
            return levels[a] < levels[b];
        }
        if (levels[a] & 1) {
            return a > b;
        }
        return a < b;
    });
    
    // Direct map: visual index -> logical index
    for (int32_t i = 0; i < length; i++) {
        indexMap[i] = indices[i];
    }
    
    free(indices);
}

U_CAPI void U_EXPORT2 ubidi_invertMap(const int32_t* srcMap, int32_t* destMap, int32_t length) {
    if (srcMap == nullptr || destMap == nullptr || length <= 0) {
        return;
    }
    
    for (int32_t i = 0; i < length; i++) {
        destMap[srcMap[i]] = i;
    }
}

U_CAPI int32_t U_EXPORT2 ubidi_getProcessedLength(const UBiDi* pBiDi) {
    if (pBiDi == nullptr) return 0;
    return pBiDi->length;
}

U_CAPI UCharDirection U_EXPORT2 ubidi_getCustomizedClass(UBiDi* pBiDi, UChar32 c) {
    if (pBiDi == nullptr) return u_charDirection(c);
    if (pBiDi->callback != nullptr) {
        return pBiDi->callback(pBiDi->callbackContext, c);
    }
    return u_charDirection(c);
}

U_CAPI void U_EXPORT2 ubidi_setClassCallback(UBiDi* pBiDi, UBiDiClassCallback* newFn,
                                             const void* newContext, UBiDiClassCallback** oldFn,
                                             const void** oldContext, UErrorCode* pErrorCode) {
    if (pBiDi == nullptr || pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return;
    }
    
    if (oldFn != nullptr) *oldFn = pBiDi->callback;
    if (oldContext != nullptr) *oldContext = pBiDi->callbackContext;
    
    pBiDi->callback = newFn;
    pBiDi->callbackContext = newContext;
}
