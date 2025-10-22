/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <stdlib.h>
#include <private/inputeventlabels.h>
#include <view/keyevent.h>
#include <tokenizer.h>
#include <keylayoutmap.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <tokenizer.h>
#include <fstream>
// Enables debug output for the parser.
#define DEBUG_PARSER 0

// Enables debug output for mapping.
#define DEBUG_MAPPING 0 


namespace cdroid {

static const char* WHITESPACE = " \t\r";

// --- KeyLayoutMap ---

KeyLayoutMap::KeyLayoutMap() {
}

KeyLayoutMap::~KeyLayoutMap() {
}

int KeyLayoutMap::load(const std::string& filename, KeyLayoutMap*& outMap) {
    outMap=nullptr;
    Tokenizer* tokenizer;
    std::ifstream fs(filename,std::ios_base::in);
    int status = Tokenizer::fromStream(filename,fs, &tokenizer);
    if (status) {
        LOGE("Error %d opening key layout map file %s.", status, filename.c_str());
    } else {
        KeyLayoutMap* map = new KeyLayoutMap();
        if (map==nullptr) {
            LOGE("Error allocating key layout map.");
            status = -1;//NO_MEMORY
        } else {
            Parser parser(map, tokenizer);
            status = parser.parse();
            if (!status)
                outMap = map;
            else delete map; 
        }
        delete tokenizer;
    }
    return status;
}

int KeyLayoutMap::mapKey(int32_t scanCode,int32_t usageCode,int32_t* outKeyCode,uint32_t* outFlags) const {
    const Key* key = getKey(scanCode, usageCode);
    if (!key) {
#if DEBUG_MAPPING
        LOGD("mapKey: scanCode=%d, usageCode=0x%08x ~ Failed.", scanCode, usageCode);
#endif
        *outKeyCode =KeyEvent::KEYCODE_UNKNOWN;
        *outFlags = 0;
        return -1;//NAME_NOT_FOUND;
    }

    *outKeyCode = key->keyCode;
    *outFlags = key->flags;

#if DEBUG_MAPPING
    LOGD("mapKey: scanCode=%d, usageCode=0x%08x ~ Result keyCode=%d, outFlags=0x%08x.",
            scanCode, usageCode, *outKeyCode, *outFlags);
#endif
    return 0;//NO_ERROR;
}

const KeyLayoutMap::Key* KeyLayoutMap::getKey(int32_t scanCode, int32_t usageCode) const {
    if (usageCode) {
        std::map<int32_t, Key>::const_iterator itr=mKeysByUsageCode.find(usageCode);
        if (itr !=mKeysByUsageCode.end())
            return &itr->second;
    }
    if (scanCode) {
        std::map<int32_t, Key>::const_iterator itr= mKeysByScanCode.find(scanCode);
        if (itr!=mKeysByScanCode.end()) 
            return &itr->second;
    }
    return NULL;
}

int KeyLayoutMap::findScanCodesForKey(int32_t keyCode, std::vector<int32_t>& outScanCodes) const {
    for (auto k: mKeysByScanCode) {
        if (k.second.keyCode == keyCode) {
            outScanCodes.push_back(k.first);
        }
    }
    return 0;
}

int KeyLayoutMap::mapAxis(int32_t scanCode, AxisInfo* outAxisInfo) const {
    std::map<int32_t, AxisInfo>::const_iterator itr=mAxes.find(scanCode);
    if (itr==mAxes.end()) {
#if DEBUG_MAPPING
        LOGD("mapAxis: scanCode=%d ~ Failed.", scanCode);
#endif
        return -1;//NAME_NOT_FOUND;
    }

    *outAxisInfo = itr->second;

#if DEBUG_MAPPING
    LOGD("mapAxis: scanCode=%d ~ Result mode=%d, axis=%d, highAxis=%d, "
            "splitValue=%d, flatOverride=%d.",  scanCode,
            outAxisInfo->mode, outAxisInfo->axis, outAxisInfo->highAxis,
            outAxisInfo->splitValue, outAxisInfo->flatOverride);
#endif
    return 0;//NO_ERROR;
}

int KeyLayoutMap::findScanCodeForLed(int32_t ledCode, int32_t* outScanCode) const {
    for (auto led:mLedsByScanCode) {
        if (led.second.ledCode == ledCode) {
            *outScanCode =led.first;
#if DEBUG_MAPPING
            LOGD("findScanCodeForLed: ledCode=%d, scanCode=%d.", ledCode, *outScanCode);
#endif
            return 0;//NO_ERROR;
        }
    }
#if DEBUG_MAPPING
            LOGD("findScanCodeForLed: ledCode=%d ~ Not found.", ledCode);
#endif
    return -1;//NAME_NOT_FOUND;
}

int KeyLayoutMap::findUsageCodeForLed(int32_t ledCode, int32_t* outUsageCode) const {
    for (auto led:mLedsByUsageCode) {
        if (led.second.ledCode == ledCode) {
            *outUsageCode = led.first;
#if DEBUG_MAPPING
            LOGD("findUsageForLed: ledCode=%d, usage=%x.", ledCode, *outUsageCode);
#endif
            return 0;//NO_ERROR;
        }
    }
#if DEBUG_MAPPING
            LOGD("findUsageForLed: ledCode=%d ~ Not found.", ledCode);
#endif
    return -1;//NAME_NOT_FOUND;
}


// --- KeyLayoutMap::Parser ---

KeyLayoutMap::Parser::Parser(KeyLayoutMap* map, Tokenizer* tokenizer) :
        mMap(map), mTokenizer(tokenizer) {
}

KeyLayoutMap::Parser::~Parser() {
}

int KeyLayoutMap::Parser::parse() {
    while (!mTokenizer->isEof()) {
#if DEBUG_PARSER
        LOGV("Parsing %s: '%s'.", mTokenizer->getLocation().c_str(),
                mTokenizer->peekRemainderOfLine().c_str());
#endif

        mTokenizer->skipDelimiters(WHITESPACE);

        if (!mTokenizer->isEol() && mTokenizer->peekChar() != '#') {
            std::string keywordToken = mTokenizer->nextToken(WHITESPACE);
            if (keywordToken == "key") {
                mTokenizer->skipDelimiters(WHITESPACE);
                parseKey();
            } else if (keywordToken == "axis") {
                mTokenizer->skipDelimiters(WHITESPACE);
                parseAxis();
            } else if (keywordToken == "led") {
                mTokenizer->skipDelimiters(WHITESPACE);
                parseLed();
            } else {
                LOGE("%s: Expected keyword, got '%s'.", mTokenizer->getLocation().c_str(),
                        keywordToken.c_str());
                return -1;//BAD_VALUE;
            }

            mTokenizer->skipDelimiters(WHITESPACE);
            if (!mTokenizer->isEol() && mTokenizer->peekChar() != '#') {
                LOGE("%s: Expected end of line or trailing comment, got '%s'.",
                        mTokenizer->getLocation().c_str(),
                        mTokenizer->peekRemainderOfLine().c_str());
                return -1;//BAD_VALUE;
            }
        }

        mTokenizer->nextLine();
    }
    LOGD("Sizes mKeysByScanCode=%d, mKeysByUsageCode=%d mAxes=%d mLedsByScanCode=%d mLedsByUsageCode=%d",
        mMap->mKeysByScanCode.size(),mMap->mKeysByUsageCode.size(),mMap->mAxes.size(),
        mMap->mLedsByScanCode.size(),mMap->mLedsByUsageCode.size());
    return 0;//NO_ERROR;
}

int KeyLayoutMap::Parser::parseKey() {
    std::string codeToken = mTokenizer->nextToken(WHITESPACE);
    bool mapUsage = false;
    if (codeToken == "usage") {
        mapUsage = true;
        mTokenizer->skipDelimiters(WHITESPACE);
        codeToken = mTokenizer->nextToken(WHITESPACE);
    }

    char* end;
    int32_t code = int32_t(strtol(codeToken.c_str(), &end, 0));
    if (*end) {
        LOGE("%s: Expected key %s number, got '%s'.", mTokenizer->getLocation().c_str(),
                mapUsage ? "usage" : "scan code", codeToken.c_str());
        return -1;//BAD_VALUE;
    }
    std::map<int32_t, Key>& map = mapUsage ? mMap->mKeysByUsageCode : mMap->mKeysByScanCode;
    if (map.find(code) !=map.end()) {
        LOGE("%s: Duplicate entry for key %s '%s'.", mTokenizer->getLocation().c_str(),
                mapUsage ? "usage" : "scan code", codeToken.c_str());
        return -1;//BAD_VALUE;
    }

    mTokenizer->skipDelimiters(WHITESPACE);
    std::string keyCodeToken = mTokenizer->nextToken(WHITESPACE);
    int32_t keyCode = getKeyCodeByLabel(keyCodeToken.c_str());
    if (!keyCode) {
        LOGE("%s: Expected key code label, got '%s'. keycode=%d", mTokenizer->getLocation().c_str(),
                keyCodeToken.c_str(),keyCode);
        return -1;//BAD_VALUE;
    }

    uint32_t flags = 0;
    for (;;) {
        mTokenizer->skipDelimiters(WHITESPACE);
        if (mTokenizer->isEol() || mTokenizer->peekChar() == '#') break;

        std::string flagToken = mTokenizer->nextToken(WHITESPACE);
        uint32_t flag = getKeyFlagByLabel(flagToken.c_str());
        if (!flag) {
            LOGE("%s: Expected key flag label, got '%s'.", mTokenizer->getLocation().c_str(),
                    flagToken.c_str());
            return -1;//BAD_VALUE;
        }
        if (flags & flag) {
            LOGE("%s: Duplicate key flag '%s'.", mTokenizer->getLocation().c_str(),
                    flagToken.c_str());
            return -1;//BAD_VALUE;
        }
        flags |= flag;
    }

#if DEBUG_PARSER
    LOGD("Parsed key %s: code=%d, keyCode=%d, flags=0x%08x.",
            mapUsage ? "usage" : "scan code", code, keyCode, flags);
#endif
    Key key;
    key.keyCode = keyCode;
    key.flags = flags;
    map[code]= key;
    return 0;//NO_ERROR;
}

int KeyLayoutMap::Parser::parseAxis() {
    std::string scanCodeToken = mTokenizer->nextToken(WHITESPACE);
    char* end;
    int32_t scanCode = int32_t(strtol(scanCodeToken.c_str(), &end, 0));
    if (*end) {
        LOGE("%s: Expected axis scan code number, got '%s'.", mTokenizer->getLocation().c_str(),
                scanCodeToken.c_str());
        return -1;//BAD_VALUE;
    }
    if (mMap->mAxes.find(scanCode)!=mMap->mAxes.end()){//indexOfKey(scanCode) >= 0) {
        LOGE("%s: Duplicate entry for axis scan code '%s'.", mTokenizer->getLocation().c_str(),
                scanCodeToken.c_str());
        return -1;//BAD_VALUE;
    }

    AxisInfo axisInfo;

    mTokenizer->skipDelimiters(WHITESPACE);
    std::string token = mTokenizer->nextToken(WHITESPACE);
    if (token == "invert") {
        axisInfo.mode = AxisInfo::MODE_INVERT;

        mTokenizer->skipDelimiters(WHITESPACE);
        std::string axisToken = mTokenizer->nextToken(WHITESPACE);
        axisInfo.axis = getAxisByLabel(axisToken.c_str());
        if (axisInfo.axis < 0) {
            LOGE("%s: Expected inverted axis label, got '%s'.",
                    mTokenizer->getLocation().c_str(), axisToken.c_str());
            return -1;//BAD_VALUE;
        }
    } else if (token == "split") {
        axisInfo.mode = AxisInfo::MODE_SPLIT;

        mTokenizer->skipDelimiters(WHITESPACE);
        std::string splitToken = mTokenizer->nextToken(WHITESPACE);
        axisInfo.splitValue = int32_t(strtol(splitToken.c_str(), &end, 0));
        if (*end) {
            LOGE("%s: Expected split value, got '%s'.",
                    mTokenizer->getLocation().c_str(), splitToken.c_str());
            return -1;//BAD_VALUE;
        }

        mTokenizer->skipDelimiters(WHITESPACE);
        std::string lowAxisToken = mTokenizer->nextToken(WHITESPACE);
        axisInfo.axis = getAxisByLabel(lowAxisToken.c_str());
        if (axisInfo.axis < 0) {
            LOGE("%s: Expected low axis label, got '%s'.",
                    mTokenizer->getLocation().c_str(), lowAxisToken.c_str());
            return -1;//BAD_VALUE;
        }

        mTokenizer->skipDelimiters(WHITESPACE);
        std::string highAxisToken = mTokenizer->nextToken(WHITESPACE);
        axisInfo.highAxis = getAxisByLabel(highAxisToken.c_str());
        if (axisInfo.highAxis < 0) {
            LOGE("%s: Expected high axis label, got '%s'.",
                    mTokenizer->getLocation().c_str(), highAxisToken.c_str());
            return -1;//BAD_VALUE;
        }
    } else {
        axisInfo.axis = getAxisByLabel(token.c_str());
        if (axisInfo.axis < 0) {
            LOGE("%s: Expected axis label, 'split' or 'invert', got '%s'.",
                    mTokenizer->getLocation().c_str(), token.c_str());
            return -1;//BAD_VALUE;
        }
    }

    for (;;) {
        mTokenizer->skipDelimiters(WHITESPACE);
        if (mTokenizer->isEol() || mTokenizer->peekChar() == '#') {
            break;
        }
        std::string keywordToken = mTokenizer->nextToken(WHITESPACE);
        if (keywordToken == "flat") {
            mTokenizer->skipDelimiters(WHITESPACE);
            std::string flatToken = mTokenizer->nextToken(WHITESPACE);
            axisInfo.flatOverride = int32_t(strtol(flatToken.c_str(), &end, 0));
            if (*end) {
                LOGE("%s: Expected flat value, got '%s'.",
                        mTokenizer->getLocation().c_str(), flatToken.c_str());
                return -1;//BAD_VALUE;
            }
        } else {
            LOGE("%s: Expected keyword 'flat', got '%s'.",
                    mTokenizer->getLocation().c_str(), keywordToken.c_str());
            return -1;//BAD_VALUE;
        }
    }

#if DEBUG_PARSER
    LOGD("Parsed axis: scanCode=%d, mode=%d, axis=%d, highAxis=%d, "
            "splitValue=%d, flatOverride=%d.",
            scanCode,
            axisInfo.mode, axisInfo.axis, axisInfo.highAxis,
            axisInfo.splitValue, axisInfo.flatOverride);
#endif
    mMap->mAxes[scanCode]= axisInfo;
    return 0;//NO_ERROR;
}

int KeyLayoutMap::Parser::parseLed() {
    std::string codeToken = mTokenizer->nextToken(WHITESPACE);
    bool mapUsage = false;
    if (codeToken == "usage") {
        mapUsage = true;
        mTokenizer->skipDelimiters(WHITESPACE);
        codeToken = mTokenizer->nextToken(WHITESPACE);
    }
    char* end;
    int32_t code = int32_t(strtol(codeToken.c_str(), &end, 0));
    if (*end) {
        LOGE("%s: Expected led %s number, got '%s'.", mTokenizer->getLocation().c_str(),
                mapUsage ? "usage" : "scan code", codeToken.c_str());
        return -1;//BAD_VALUE;
    }

    std::map<int32_t, Led>& map = mapUsage ? mMap->mLedsByUsageCode : mMap->mLedsByScanCode;
    if (map.find(code)!=map.end()) {
        LOGE("%s: Duplicate entry for led %s '%s'.", mTokenizer->getLocation().c_str(),
                mapUsage ? "usage" : "scan code", codeToken.c_str());
        return -1;//BAD_VALUE;
    }

    mTokenizer->skipDelimiters(WHITESPACE);
    std::string ledCodeToken = mTokenizer->nextToken(WHITESPACE);
    int32_t ledCode = getLedByLabel(ledCodeToken.c_str());
    if (ledCode < 0) {
        LOGE("%s: Expected LED code label, got '%s'.", mTokenizer->getLocation().c_str(),
                ledCodeToken.c_str());
        return -1;//BAD_VALUE;
    }

#if DEBUG_PARSER
    LOGD("Parsed led %s: code=%d, ledCode=%d.",
            mapUsage ? "usage" : "scan code", code, ledCode);
#endif

    Led led;
    led.ledCode = ledCode;
    map[code]= led;
    return 1;//NO_ERROR;
}
};
