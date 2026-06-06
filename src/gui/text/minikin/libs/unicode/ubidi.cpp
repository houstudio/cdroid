#include "unicode/ubidi.h"
#include "unicode/utf16.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <fribidi.h>
#include <vector>

#ifndef U_INDEX_OUTOFBOUNDS_ERROR
#define U_INDEX_OUTOFBOUNDS_ERROR U_ILLEGAL_ARGUMENT_ERROR
#endif
struct UBiDi {
    const UChar* text;
    int32_t length;
    UBiDiLevel paraLevel;
    UBiDiDirection direction;
    int32_t runCount;
    UBiDiClassCallback* callback;
    const void* callbackContext;

    UBiDiLevel* levels;
    int32_t* runs;
    int32_t* runLengths;
    UBiDiLevel* runLevels;
    int32_t levelsCapacity;
    int32_t runsCapacity;

    UBool isInverse;
    UBool orderParagraphsLTR;
    UBiDiReorderingMode reorderingMode;
    uint32_t reorderingOptions;

    const UChar* prologue;
    int32_t proLength;
    const UChar* epilogue;
    int32_t epiLength;

    UChar* processedText;
    int32_t processedLength;
    int32_t* originalIndices;

    FriBidiCharType* fribidiTypes;
    FriBidiLevel* fribidiLevels;
    FriBidiChar* visualStr;
    FriBidiStrIndex* positionsLtoV;
    FriBidiStrIndex* positionsVtoL;
    FriBidiJoiningType* joiningTypes;
    FriBidiArabicProp* arabicProps;
};

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
    if (pBiDi->fribidiTypes != nullptr) {
        free(pBiDi->fribidiTypes);
        pBiDi->fribidiTypes = nullptr;
    }
    if (pBiDi->fribidiLevels != nullptr) {
        free(pBiDi->fribidiLevels);
        pBiDi->fribidiLevels = nullptr;
    }
    if (pBiDi->visualStr != nullptr) {
        free(pBiDi->visualStr);
        pBiDi->visualStr = nullptr;
    }
    if (pBiDi->positionsLtoV != nullptr) {
        free(pBiDi->positionsLtoV);
        pBiDi->positionsLtoV = nullptr;
    }
    if (pBiDi->positionsVtoL != nullptr) {
        free(pBiDi->positionsVtoL);
        pBiDi->positionsVtoL = nullptr;
    }
    if (pBiDi->joiningTypes != nullptr) {
        free(pBiDi->joiningTypes);
        pBiDi->joiningTypes = nullptr;
    }
    if (pBiDi->arabicProps != nullptr) {
        free(pBiDi->arabicProps);
        pBiDi->arabicProps = nullptr;
    }
    pBiDi->levelsCapacity = 0;
    pBiDi->runsCapacity = 0;
    pBiDi->processedLength = 0;
}

static void ensureLevelsCapacity(UBiDi* pBiDi, int32_t capacity) {
    if (pBiDi->levelsCapacity < capacity) {
        freeLevelsAndRuns(pBiDi);
        pBiDi->levels = (UBiDiLevel*)malloc(capacity * sizeof(UBiDiLevel));
        pBiDi->fribidiLevels = (FriBidiLevel*)malloc(capacity * sizeof(FriBidiLevel));
        pBiDi->fribidiTypes = (FriBidiCharType*)malloc(capacity * sizeof(FriBidiCharType));
        pBiDi->visualStr = (FriBidiChar*)malloc(capacity * sizeof(FriBidiChar));
        pBiDi->positionsLtoV = (FriBidiStrIndex*)malloc(capacity * sizeof(FriBidiStrIndex));
        pBiDi->positionsVtoL = (FriBidiStrIndex*)malloc(capacity * sizeof(FriBidiStrIndex));
        pBiDi->joiningTypes = (FriBidiJoiningType*)malloc(capacity * sizeof(FriBidiJoiningType));
        pBiDi->arabicProps = (FriBidiArabicProp*)malloc(capacity * sizeof(FriBidiArabicProp));
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

        pBiDi->isInverse = false;
        pBiDi->orderParagraphsLTR = true;
        pBiDi->reorderingMode = UBIDI_REORDER_DEFAULT;
        pBiDi->reorderingOptions = UBIDI_OPTION_DEFAULT;

        pBiDi->prologue = nullptr;
        pBiDi->proLength = 0;
        pBiDi->epilogue = nullptr;
        pBiDi->epiLength = 0;

        pBiDi->processedText = nullptr;
        pBiDi->processedLength = 0;
        pBiDi->originalIndices = nullptr;

        pBiDi->fribidiTypes = nullptr;
        pBiDi->fribidiLevels = nullptr;
        pBiDi->visualStr = nullptr;
        pBiDi->positionsLtoV = nullptr;
        pBiDi->positionsVtoL = nullptr;
        pBiDi->joiningTypes = nullptr;
        pBiDi->arabicProps = nullptr;
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
    ensureLevelsCapacity(pBiDi, maxLength);
    ensureRunsCapacity(pBiDi, maxRunCount);
    return pBiDi;
}

U_CAPI void U_EXPORT2 ubidi_close(UBiDi* pBiDi) {
    if (pBiDi != nullptr) {
        freeLevelsAndRuns(pBiDi);
        free(pBiDi);
        pBiDi= nullptr;
    }
}

U_CAPI void U_EXPORT2 ubidi_setInverse(UBiDi* pBiDi, UBool isInverse) {
    if (pBiDi != nullptr) {
        pBiDi->isInverse = isInverse;
    }
}

U_CAPI UBool U_EXPORT2 ubidi_isInverse(UBiDi* pBiDi) {
    return pBiDi != nullptr ? pBiDi->isInverse : false;
}

U_CAPI void U_EXPORT2 ubidi_orderParagraphsLTR(UBiDi* pBiDi, UBool orderParagraphsLTR) {
    if (pBiDi != nullptr) {
        pBiDi->orderParagraphsLTR = orderParagraphsLTR;
    }
}

U_CAPI UBool U_EXPORT2 ubidi_isOrderParagraphsLTR(UBiDi* pBiDi) {
    return pBiDi != nullptr ? pBiDi->orderParagraphsLTR : true;
}

U_CAPI void U_EXPORT2 ubidi_setReorderingMode(UBiDi* pBiDi, UBiDiReorderingMode reorderingMode) {
    if (pBiDi != nullptr) {
        pBiDi->reorderingMode = reorderingMode;
    }
}

U_CAPI UBiDiReorderingMode U_EXPORT2 ubidi_getReorderingMode(UBiDi* pBiDi) {
    return pBiDi != nullptr ? pBiDi->reorderingMode : UBIDI_REORDER_DEFAULT;
}

U_CAPI void U_EXPORT2 ubidi_setReorderingOptions(UBiDi* pBiDi, uint32_t reorderingOptions) {
    if (pBiDi != nullptr) {
        pBiDi->reorderingOptions = reorderingOptions;
    }
}

U_CAPI uint32_t U_EXPORT2 ubidi_getReorderingOptions(UBiDi* pBiDi) {
    return pBiDi != nullptr ? pBiDi->reorderingOptions : UBIDI_OPTION_DEFAULT;
}

U_CAPI void U_EXPORT2 ubidi_setContext(UBiDi* pBiDi,
                                       const UChar* prologue, int32_t proLength,
                                       const UChar* epilogue, int32_t epiLength,
                                       UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return;
    }
    if (pBiDi == nullptr) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }
    pBiDi->prologue = prologue;
    pBiDi->proLength = proLength;
    pBiDi->epilogue = epilogue;
    pBiDi->epiLength = epiLength;
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

    ensureLevelsCapacity(pBiDi, length);
    ensureRunsCapacity(pBiDi, length);

    if (paraLevel == UBIDI_DEFAULT_LTR) {
        pBiDi->paraLevel = 0;
    } else if (paraLevel == UBIDI_DEFAULT_RTL) {
        pBiDi->paraLevel = 1;
    } else {
        pBiDi->paraLevel = paraLevel;
    }

    FriBidiParType baseDir = (pBiDi->paraLevel & 1) ? FRIBIDI_PAR_RTL : FRIBIDI_PAR_LTR;

    std::vector<FriBidiChar> utf32Text(length);
    for (int32_t i = 0; i < length; i++) {
        utf32Text[i] = text[i];
    }

    fribidi_get_bidi_types(utf32Text.data(), length, pBiDi->fribidiTypes);

    FriBidiLevel maxLevel = fribidi_get_par_embedding_levels_ex(pBiDi->fribidiTypes, nullptr, length,
                                                                 &baseDir, pBiDi->fribidiLevels);

    pBiDi->paraLevel = (baseDir == FRIBIDI_PAR_RTL) ? 1 : 0;
    pBiDi->direction = (baseDir == FRIBIDI_PAR_RTL) ? UBIDI_RTL : UBIDI_LTR;

    for (int32_t i = 0; i < length; i++) {
        pBiDi->levels[i] = pBiDi->fribidiLevels[i];
    }

    if (embeddingLevels != nullptr && length > 0) {
        memcpy(embeddingLevels, pBiDi->levels, length * sizeof(UBiDiLevel));
    }

    FriBidiStrIndex len = length;
    
    for (int32_t i = 0; i < length; i++) {
        pBiDi->visualStr[i] = utf32Text[i];
    }
    
    fribidi_log2vis(pBiDi->visualStr, len, &baseDir,
                    pBiDi->visualStr, pBiDi->positionsLtoV, pBiDi->positionsVtoL,
                    pBiDi->fribidiLevels);

    pBiDi->runCount = 0;
    FriBidiStrIndex pos = 0;
    while (pos < len) {
        FriBidiLevel runLevel = pBiDi->fribidiLevels[pos];
        
        int32_t runLength = 1;
        while (pos + runLength < len && pBiDi->fribidiLevels[pos + runLength] == runLevel) {
            runLength++;
        }
        
        pBiDi->runs[pBiDi->runCount] = pos;
        pBiDi->runLengths[pBiDi->runCount] = runLength;
        pBiDi->runLevels[pBiDi->runCount] = runLevel;
        pBiDi->runCount++;
        pos += runLength;
    }



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
    memcpy(pLineBiDi->fribidiLevels, pParaBiDi->fribidiLevels + start,
           pLineBiDi->length * sizeof(FriBidiLevel));

    if (pLineBiDi->length > 0) {
        ensureRunsCapacity(pLineBiDi, pLineBiDi->length);

        pLineBiDi->runCount = 1;
        pLineBiDi->runs[0] = 0;
        pLineBiDi->runLengths[0] = 1;
        pLineBiDi->runLevels[0] = pLineBiDi->fribidiLevels[0];

        for (int32_t i = 1; i < pLineBiDi->length; i++) {
            if (pLineBiDi->fribidiLevels[i] != pLineBiDi->runLevels[pLineBiDi->runCount - 1]) {
                pLineBiDi->runs[pLineBiDi->runCount] = i;
                pLineBiDi->runLengths[pLineBiDi->runCount] = 1;
                pLineBiDi->runLevels[pLineBiDi->runCount] = pLineBiDi->fribidiLevels[i];
                pLineBiDi->runCount++;
            } else {
                pLineBiDi->runLengths[pLineBiDi->runCount - 1]++;
            }
        }

        FriBidiParType lineBaseDir = (pLineBiDi->paraLevel & 1) ? FRIBIDI_PAR_RTL : FRIBIDI_PAR_LTR;
        fribidi_log2vis((const FriBidiChar*)pLineBiDi->text, pLineBiDi->length, &lineBaseDir,
                        pLineBiDi->visualStr, pLineBiDi->positionsLtoV, pLineBiDi->positionsVtoL,
                        pLineBiDi->fribidiLevels);

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

    FriBidiCharType* types = (FriBidiCharType*)malloc(length * sizeof(FriBidiCharType));
    if (types == nullptr) return UBIDI_NEUTRAL;

    fribidi_get_bidi_types((const FriBidiChar*)text, length, types);

    FriBidiParType dir = fribidi_get_par_direction(types, length);
    free(types);

    switch (dir) {
        case FRIBIDI_PAR_LTR: return UBIDI_LTR;
        case FRIBIDI_PAR_RTL: return UBIDI_RTL;
        default: return UBIDI_NEUTRAL;
    }
}

U_CAPI const UChar* U_EXPORT2 ubidi_getText(const UBiDi* pBiDi) {
    return pBiDi != nullptr ? pBiDi->text : nullptr;
}

U_CAPI int32_t U_EXPORT2 ubidi_getLength(const UBiDi* pBiDi) {
    return pBiDi != nullptr ? pBiDi->length : 0;
}

U_CAPI UBiDiLevel U_EXPORT2 ubidi_getParaLevel(const UBiDi* pBiDi) {
    return pBiDi != nullptr ? pBiDi->paraLevel : 0;
}

U_CAPI int32_t U_EXPORT2 ubidi_countParagraphs(UBiDi* pBiDi) {
    if (pBiDi == nullptr) return 0;

    int32_t count = 0;
    for (int32_t i = 0; i < pBiDi->length; i++) {
        if (pBiDi->text[i] == 0x0A || pBiDi->text[i] == 0x0D) {
            count++;
            if (i + 1 < pBiDi->length && pBiDi->text[i] == 0x0D && pBiDi->text[i + 1] == 0x0A) {
                i++;
            }
        }
    }
    return count + 1;
}

U_CAPI int32_t U_EXPORT2 ubidi_getParagraph(const UBiDi* pBiDi, int32_t charIndex, int32_t* pParaStart,
                                             int32_t* pParaEnd, UBiDiLevel* pParaLevel, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return -1;
    }
    if (pBiDi == nullptr) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return -1;
    }
    if (charIndex < 0 || charIndex > pBiDi->length) {
        *pErrorCode = U_INDEX_OUTOFBOUNDS_ERROR;
        return -1;
    }

    int32_t paraStart = 0;
    int32_t paraIndex = 0;
    for (int32_t i = 0; i < pBiDi->length; i++) {
        if (pBiDi->text[i] == 0x0A || pBiDi->text[i] == 0x0D) {
            if (i >= charIndex) {
                break;
            }
            paraStart = i + 1;
            if (i + 1 < pBiDi->length && pBiDi->text[i] == 0x0D && pBiDi->text[i + 1] == 0x0A) {
                paraStart++;
                i++;
            }
            paraIndex++;
        }
    }

    int32_t paraEnd = pBiDi->length;
    for (int32_t i = paraStart; i < pBiDi->length; i++) {
        if (pBiDi->text[i] == 0x0A || pBiDi->text[i] == 0x0D) {
            paraEnd = i;
            break;
        }
    }

    if (pParaStart != nullptr) *pParaStart = paraStart;
    if (pParaEnd != nullptr) *pParaEnd = paraEnd;
    if (pParaLevel != nullptr) *pParaLevel = pBiDi->paraLevel;

    return paraIndex;
}

U_CAPI void U_EXPORT2 ubidi_getParagraphByIndex(const UBiDi* pBiDi, int32_t paraIndex,
                                                 int32_t* pParaStart, int32_t* pParaEnd,
                                                 UBiDiLevel* pParaLevel, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return;
    }
    if (pBiDi == nullptr) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }

    int32_t paraCount = ubidi_countParagraphs((UBiDi*)pBiDi);
    if (paraIndex < 0 || paraIndex >= paraCount) {
        *pErrorCode = U_INDEX_OUTOFBOUNDS_ERROR;
        return;
    }

    int32_t currentPara = 0;
    int32_t paraStart = 0;
    for (int32_t i = 0; i < pBiDi->length; i++) {
        if (pBiDi->text[i] == 0x0A || pBiDi->text[i] == 0x0D) {
            if (currentPara == paraIndex) {
                if (pParaStart != nullptr) *pParaStart = paraStart;
                if (pParaEnd != nullptr) *pParaEnd = i;
                if (pParaLevel != nullptr) *pParaLevel = pBiDi->paraLevel;
                return;
            }
            currentPara++;
            paraStart = i + 1;
            if (i + 1 < pBiDi->length && pBiDi->text[i] == 0x0D && pBiDi->text[i + 1] == 0x0A) {
                paraStart++;
                i++;
            }
        }
    }

    if (currentPara == paraIndex) {
        if (pParaStart != nullptr) *pParaStart = paraStart;
        if (pParaEnd != nullptr) *pParaEnd = pBiDi->length;
        if (pParaLevel != nullptr) *pParaLevel = pBiDi->paraLevel;
    }
}

U_CAPI UBiDiLevel U_EXPORT2 ubidi_getLevelAt(const UBiDi* pBiDi, int32_t charIndex) {
    if (pBiDi == nullptr || charIndex < 0 || charIndex >= pBiDi->length) {
        return 0;
    }
    return pBiDi->levels[charIndex];
}

U_CAPI const UBiDiLevel* U_EXPORT2 ubidi_getLevels(UBiDi* pBiDi, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return nullptr;
    }
    if (pBiDi == nullptr) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return nullptr;
    }
    return pBiDi->levels;
}

U_CAPI void U_EXPORT2 ubidi_getLogicalRun(const UBiDi* pBiDi, int32_t logicalPosition,
                                           int32_t* pLogicalStart, int32_t* pLogicalLimit,
                                           UBiDiLevel* pLevel) {
    if (pBiDi == nullptr) {
        return;
    }

    for (int32_t i = 0; i < pBiDi->runCount; i++) {
        int32_t runStart = pBiDi->runs[i];
        int32_t runEnd = runStart + pBiDi->runLengths[i];

        if (logicalPosition >= runStart && logicalPosition < runEnd) {
            if (pLogicalStart != nullptr) *pLogicalStart = runStart;
            if (pLogicalLimit != nullptr) *pLogicalLimit = runEnd;
            if (pLevel != nullptr) *pLevel = pBiDi->runLevels[i];
            return;
        }
    }
}

U_CAPI int32_t U_EXPORT2 ubidi_countRuns(UBiDi* pBiDi, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return -1;
    }
    if (pBiDi == nullptr) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
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

    int32_t logicalStart = pBiDi->runs[runIndex];
    int32_t length = pBiDi->runLengths[runIndex];
    UBiDiLevel level = pBiDi->runLevels[runIndex];
    
    if (pLogicalStart != nullptr) *pLogicalStart = logicalStart;
    if (pLength != nullptr) *pLength = length;
    return (level & 1) ? UBIDI_RTL : UBIDI_LTR;
}

U_CAPI int32_t U_EXPORT2 ubidi_getVisualIndex(UBiDi* pBiDi, int32_t logicalIndex, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
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
        *pErrorCode = U_INDEX_OUTOFBOUNDS_ERROR;
        return -1;
    }
    if (pBiDi->positionsLtoV == nullptr) {
        return logicalIndex;
    }
    return pBiDi->positionsLtoV[logicalIndex];
}

U_CAPI int32_t U_EXPORT2 ubidi_getLogicalIndex(UBiDi* pBiDi, int32_t visualIndex, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return -1;
    }
    if (pBiDi == nullptr) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return -1;
    }
    if (visualIndex < 0 || visualIndex >= pBiDi->length) {
        *pErrorCode = U_INDEX_OUTOFBOUNDS_ERROR;
        return -1;
    }
    if (pBiDi->positionsVtoL == nullptr) {
        return visualIndex;
    }
    return pBiDi->positionsVtoL[visualIndex];
}

U_CAPI void U_EXPORT2 ubidi_getLogicalMap(UBiDi* pBiDi, int32_t* indexMap, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return;
    }
    if (pBiDi == nullptr) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }
    if (indexMap == nullptr) {
        return;
    }
    if (pBiDi->positionsLtoV != nullptr) {
        memcpy(indexMap, pBiDi->positionsLtoV, pBiDi->length * sizeof(int32_t));
    } else {
        for (int32_t i = 0; i < pBiDi->length; i++) {
            indexMap[i] = i;
        }
    }
}

U_CAPI void U_EXPORT2 ubidi_getVisualMap(UBiDi* pBiDi, int32_t* indexMap, UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return;
    }
    if (pBiDi == nullptr) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }
    if (indexMap == nullptr) {
        return;
    }
    if (pBiDi->positionsVtoL != nullptr) {
        memcpy(indexMap, pBiDi->positionsVtoL, pBiDi->length * sizeof(int32_t));
    } else {
        for (int32_t i = 0; i < pBiDi->length; i++) {
            indexMap[i] = i;
        }
    }
}

U_CAPI void U_EXPORT2 ubidi_reorderLogical(const UBiDiLevel* levels, int32_t length, int32_t* indexMap) {
    if (levels == nullptr || length <= 0) {
        return;
    }

    int32_t* tempMap = (int32_t*)malloc(length * sizeof(int32_t));
    for (int32_t i = 0; i < length; i++) {
        tempMap[i] = i;
    }

    for (int32_t i = 0; i < length - 1; i++) {
        for (int32_t j = i + 1; j < length; j++) {
            if (levels[tempMap[i]] > levels[tempMap[j]]) {
                int32_t temp = tempMap[i];
                tempMap[i] = tempMap[j];
                tempMap[j] = temp;
            }
        }
    }

    if (indexMap != nullptr) {
        memcpy(indexMap, tempMap, length * sizeof(int32_t));
    }
    free(tempMap);
}

U_CAPI void U_EXPORT2 ubidi_reorderVisual(const UBiDiLevel* levels, int32_t length, int32_t* indexMap) {
    if (levels == nullptr || length <= 0) {
        return;
    }

    int32_t* tempMap = (int32_t*)malloc(length * sizeof(int32_t));
    for (int32_t i = 0; i < length; i++) {
        tempMap[i] = i;
    }

    for (int32_t i = 0; i < length - 1; i++) {
        for (int32_t j = i + 1; j < length; j++) {
            if (levels[tempMap[i]] > levels[tempMap[j]]) {
                int32_t temp = tempMap[i];
                tempMap[i] = tempMap[j];
                tempMap[j] = temp;
            }
        }
    }

    int32_t* reverseMap = (int32_t*)malloc(length * sizeof(int32_t));
    for (int32_t i = 0; i < length; i++) {
        reverseMap[tempMap[i]] = i;
    }

    if (indexMap != nullptr) {
        memcpy(indexMap, reverseMap, length * sizeof(int32_t));
    }
    free(reverseMap);
    free(tempMap);
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
    return pBiDi != nullptr ? pBiDi->processedLength : 0;
}

U_CAPI UCharDirection U_EXPORT2 ubidi_getCustomizedClass(UBiDi* pBiDi, UChar32 c) {
    if (pBiDi != nullptr && pBiDi->callback != nullptr) {
        return pBiDi->callback(pBiDi->callbackContext, c);
    }
    return U_OTHER_NEUTRAL;
}

U_CAPI void U_EXPORT2 ubidi_setClassCallback(UBiDi* pBiDi, UBiDiClassCallback* newFn,
                                             const void* newContext,
                                             UBiDiClassCallback** oldFn, const void** oldContext,
                                             UErrorCode* pErrorCode) {
    if (pErrorCode == nullptr || *pErrorCode > U_ZERO_ERROR) {
        return;
    }
    if (pBiDi == nullptr) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }
    if (oldFn != nullptr) *oldFn = pBiDi->callback;
    if (oldContext != nullptr) *oldContext = pBiDi->callbackContext;
    pBiDi->callback = newFn;
    pBiDi->callbackContext = newContext;
}
