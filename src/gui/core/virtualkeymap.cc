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
#include <core/tokenizer.h>
#include <core/virtualkeymap.h>
#include <utils/errors.h>
namespace cdroid {

static const char* WHITESPACE = " \t\r";
static const char* WHITESPACE_OR_FIELD_DELIMITER = " \t\r:";


// --- VirtualKeyMap ---

VirtualKeyMap::VirtualKeyMap() {
}

VirtualKeyMap::~VirtualKeyMap() {
}

std::unique_ptr<VirtualKeyMap> VirtualKeyMap::fromStream(std::istream& istream) {
    Tokenizer* t;
    int32_t status = Tokenizer::fromStream("",istream, &t);
    if (status != OK) {
        //LOGE("Error %d opening virtual key map file %s.", status, filename.c_str());
        return nullptr;
    }
    std::unique_ptr<Tokenizer> tokenizer(t);
    // Using 'new' to access a non-public constructor
    std::unique_ptr<VirtualKeyMap> map(new VirtualKeyMap());
    if (!map) {
        LOGE("Error allocating virtual key map.");
        return nullptr;
    }

    Parser parser(map.get(), tokenizer.get());
    status = parser.parse();
    if (status != OK) {
        return nullptr;
    }

    return map;
}


// --- VirtualKeyMap::Parser ---

VirtualKeyMap::Parser::Parser(VirtualKeyMap* map, Tokenizer* tokenizer) :
        mMap(map), mTokenizer(tokenizer) {
}

VirtualKeyMap::Parser::~Parser() {
}

int32_t VirtualKeyMap::Parser::parse() {
    while (!mTokenizer->isEof()) {
#if DEBUG_PARSER
        LOGD("Parsing %s: '%s'.", mTokenizer->getLocation().c_str(),
              mTokenizer->peekRemainderOfLine().c_str());
#endif

        mTokenizer->skipDelimiters(WHITESPACE);

        if (!mTokenizer->isEol() && mTokenizer->peekChar() != '#') {
            // Multiple keys can appear on one line or they can be broken up across multiple lines.
            do {
                std::string token = mTokenizer->nextToken(WHITESPACE_OR_FIELD_DELIMITER);
                if (token != "0x01") {
                    LOGE("%s: Unknown virtual key type, expected 0x01.",
                          mTokenizer->getLocation().c_str());
                    return BAD_VALUE;
                }

                VirtualKeyDefinition defn;
                bool success = parseNextIntField(&defn.scanCode)
                        && parseNextIntField(&defn.centerX)
                        && parseNextIntField(&defn.centerY)
                        && parseNextIntField(&defn.width)
                        && parseNextIntField(&defn.height);
                if (!success) {
                    LOGE("%s: Expected 5 colon-delimited integers in virtual key definition.",
                          mTokenizer->getLocation().c_str());
                    return BAD_VALUE;
                }

#if DEBUG_PARSER
                LOGD("Parsed virtual key: scanCode=%d, centerX=%d, centerY=%d, "
                        "width=%d, height=%d",
                        defn.scanCode, defn.centerX, defn.centerY, defn.width, defn.height);
#endif
                mMap->mVirtualKeys.push_back(defn);
            } while (consumeFieldDelimiterAndSkipWhitespace());

            if (!mTokenizer->isEol()) {
                LOGE("%s: Expected end of line, got '%s'.", mTokenizer->getLocation().c_str(),
                      mTokenizer->peekRemainderOfLine().c_str());
                return BAD_VALUE;
            }
        }

        mTokenizer->nextLine();
    }

    return NO_ERROR;
}

bool VirtualKeyMap::Parser::consumeFieldDelimiterAndSkipWhitespace() {
    mTokenizer->skipDelimiters(WHITESPACE);
    if (mTokenizer->peekChar() == ':') {
        mTokenizer->nextChar();
        mTokenizer->skipDelimiters(WHITESPACE);
        return true;
    }
    return false;
}

bool VirtualKeyMap::Parser::parseNextIntField(int32_t* outValue) {
    if (!consumeFieldDelimiterAndSkipWhitespace()) {
        return false;
    }

    std::string token = mTokenizer->nextToken(WHITESPACE_OR_FIELD_DELIMITER);
    char* end;
    *outValue = strtol(token.c_str(), &end, 0);
    if (token.empty() || *end != '\0') {
        LOGE("Expected an integer, got '%s'.", token.c_str());
        return false;
    }
    return true;
}

} // namespace cdroid
