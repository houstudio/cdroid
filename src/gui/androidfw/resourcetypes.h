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
//
// Resource binary-format types: port of AOSP frameworks/base/libs/androidfw's
// ResourceTypes.{h,cpp}. This file currently ports only the ResStringPool (string
// pool reader) — the first of three runtime pieces for the aapt2 binary resource
// mode (resources.arsc + AXML). ResXMLTree / ResTable follow in later stages.
//
// Adaptations from the Android original (this module is a standalone static
// library, registered at the top-level CMake scope and NOT linked into
// libcdroid.so; logging is the one external dependency — <porting/cdlog.h>
// backed by libtvhal — everything else is self-contained):
//   * incfs::map_ptr / verified_map_ptr / .convert() / .offset() / .verify()  -> raw
//     const uint8_t* base + offset pointer arithmetic; copyData mallocs mOwnedData.
//   * base::expected<StringPiece16, NullOrIOError> -> raw pointer + out-len param,
//     nullptr on error.
//   * Mutex mDecodeLock dropped (resource loading is single-threaded on CDROID).
//   * char16_t** mCache -> mutable std::vector<std::u16string> (RAII decode cache).
//   * utf8_to_utf16 / utf8_to_utf16_length (libutils/Unicode.cpp) reimplemented
//     inline (standard UTF-8<->UTF-16 with surrogate pairs).
//   * dtohl/dtohs/htodl/htods are little-endian identity (arsc is LE; host is LE).
//
// When this module is wired into cdroid core, status_t/NO_ERROR/BAD_TYPE below
// should be replaced by utils/errors.h (same values).
#ifndef __CDROID_ANDROIDFW_RESOURCETYPES_H
#define __CDROID_ANDROIDFW_RESOURCETYPES_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <sys/types.h> // ssize_t

namespace cdroid {

// status_t + error codes. Values aligned with AOSP
// system/core/libutils/include/utils/Errors.h (errno-based) so that integrating
// later is a drop-in replacement with utils/errors.h.
typedef int status_t;
enum {
    OK                = 0,
    NO_ERROR          = OK,
    UNKNOWN_ERROR     = (-2147483647 - 1), // INT32_MIN
    NO_MEMORY         = -12,   // -ENOMEM
    INVALID_OPERATION = -38,   // -ENOSYS
    BAD_VALUE         = -22,   // -EINVAL
    BAD_TYPE          = UNKNOWN_ERROR + 1,
    NAME_NOT_FOUND    = -2,    // -ENOENT
    NO_INIT           = -19,   // -ENODEV
    BAD_INDEX         = -75,   // -EOVERFLOW
};

// Byte-order helpers. resources.arsc / AXML are little-endian; CDROID assumes a
// little-endian host, so these are identity — mirrors AOSP
// system/core/libutils/include/utils/ByteOrder.h.
static inline uint16_t dtohs(uint16_t v) { return v; }
static inline uint32_t dtohl(uint32_t v) { return v; }
static inline uint16_t htods(uint16_t v) { return v; }
static inline uint32_t htodl(uint32_t v) { return v; }

// Binary on-disk chunk structures must be tightly packed.
#pragma pack(push, 1)

/**
 * Header that appears at the front of every data chunk in a resource.
 */
struct ResChunk_header {
    // Type identifier for this chunk. The meaning depends on the containing chunk.
    uint16_t type;

    // Size of the chunk header (in bytes). Adding this value to the address of
    // the chunk allows you to find its associated data (if any).
    uint16_t headerSize;

    // Total size of this chunk (in bytes). This is the chunkSize plus the size
    // of any data associated with the chunk. Adding this value to the chunk
    // allows you to completely skip its contents (including any child chunks).
    uint32_t size;
};

enum {
    RES_NULL_TYPE                     = 0x0000,
    RES_STRING_POOL_TYPE              = 0x0001,
    RES_TABLE_TYPE                    = 0x0002,
    RES_XML_TYPE                      = 0x0003,

    // Chunk types in RES_XML_TYPE
    RES_XML_FIRST_CHUNK_TYPE          = 0x0100,
    RES_XML_START_NAMESPACE_TYPE      = 0x0100,
    RES_XML_END_NAMESPACE_TYPE        = 0x0101,
    RES_XML_START_ELEMENT_TYPE        = 0x0102,
    RES_XML_END_ELEMENT_TYPE          = 0x0103,
    RES_XML_CDATA_TYPE                = 0x0104,
    RES_XML_LAST_CHUNK_TYPE           = 0x017f,
    // This contains a uint32_t array mapping strings in the string pool back to
    // resource identifiers. It is optional.
    RES_XML_RESOURCE_MAP_TYPE         = 0x0180,

    // Chunk types in RES_TABLE_TYPE
    RES_TABLE_PACKAGE_TYPE            = 0x0200,
    RES_TABLE_TYPE_TYPE               = 0x0201,
    RES_TABLE_TYPE_SPEC_TYPE          = 0x0202,
    RES_TABLE_LIBRARY_TYPE            = 0x0203,
    RES_TABLE_OVERLAYABLE_TYPE        = 0x0204,
    RES_TABLE_OVERLAYABLE_POLICY_TYPE = 0x0205,
    RES_TABLE_STAGED_ALIAS_TYPE       = 0x0206,
};

/**
 * Reference to a string in a string pool.
 */
struct ResStringPool_ref {
    // Index into the string pool table (uint32_t-offset from the indices
    // immediately after ResStringPool_header) at which to find the location of
    // the string data in the pool.
    uint32_t index;
};

/**
 * Definition for a pool of strings.  The data of this chunk is an array of
 * uint32_t providing indices into the pool, relative to stringsStart.  At
 * stringsStart are all of the UTF-16 strings concatenated together; each starts
 * with a uint16_t of the string's length and each ends with a 0x0000 terminator.
 * If a string is > 32767 characters, the high bit of the length is set meaning
 * to take those 15 bits as a high word and it will be followed by another
 * uint16_t containing the low word.
 *
 * If styleCount is not zero, then immediately following the array of uint32_t
 * indices into the string table is another array of indices into a style table
 * starting at stylesStart.  Each entry in the style table is an array of
 * ResStringPool_span structures.
 */
struct ResStringPool_header {
    struct ResChunk_header header;

    // Number of strings in this pool (number of uint32_t indices that follow).
    uint32_t stringCount;

    // Number of style span arrays in the pool (number of uint32_t indices that
    // follow the string indices).
    uint32_t styleCount;

    // Flags.
    enum {
        // If set, the string index is sorted by the string values.
        SORTED_FLAG = 1 << 0,
        // String pool is encoded in UTF-8
        UTF8_FLAG   = 1 << 8,
    };
    uint32_t flags;

    // Index from header of the string data.
    uint32_t stringsStart;

    // Index from header of the style data.
    uint32_t stylesStart;
};

/**
 * This structure defines a span of style information associated with a string
 * in the pool.
 */
struct ResStringPool_span {
    enum {
        END = 0xFFFFFFFF
    };

    // Name of the span (XML tag that defined it). END (0xFFFFFFFF) ends an array.
    ResStringPool_ref name;

    // Range of characters in the string that this span applies to.
    uint32_t firstChar, lastChar;
};

#pragma pack(pop)

/**
 * Convenience class for accessing data in a ResStringPool resource.
 */
class ResStringPool {
public:
    ResStringPool();
    ResStringPool(const void* data, size_t size, bool copyData = false);
    virtual ~ResStringPool();

    void setToEmpty();
    status_t setTo(const void* data, size_t size, bool copyData = false);

    status_t getError() const;

    void uninit();

    // Return string entry as UTF-16. For a UTF-8 pool the bytes are converted
    // (and cached) before returning. Returns nullptr on error/bad index; on
    // success *outLen is set to the char16_t count.
    const char16_t* stringAt(size_t idx, size_t* outLen) const;

    // Return raw UTF-8 bytes of a string. Returns nullptr if the pool is not
    // UTF-8 or on error. On success *outLen is set to the byte count.
    const char* string8At(size_t idx, size_t* outLen) const;

    // Return the style span array for the given string index, or nullptr on
    // error. The returned array is terminated by a span with
    // name.index == ResStringPool_span::END.
    const ResStringPool_span* styleAt(size_t idx) const;

    inline const ResStringPool_span* styleAt(const ResStringPool_ref& ref) const {
        return styleAt(ref.index);
    }

    size_t size() const;       // number of strings
    size_t styleCount() const; // number of style entries
    size_t bytes() const;      // size of the backing data
    bool isSorted() const;
    bool isUTF8() const;

private:
    // Decode raw UTF-8 bytes for string idx, accounting for AAPT truncation of
    // encoded lengths > 0x7FFF. Returns pointer into the pool (nullptr on error);
    // *outLen receives the decoded byte length (which may exceed encLen when AAPT
    // truncated the stored length).
    const char* stringDecodeAt(size_t idx, const uint8_t* str, size_t encLen,
                               size_t* outLen) const;

    status_t                    mError;
    void*                       mOwnedData;       // malloc'd copy when copyData / LE swap
    const ResStringPool_header* mHeader;
    size_t                      mSize;            // == mHeader->header.size
    const uint32_t*             mEntries;         // stringCount offsets into mStrings
    const uint32_t*             mEntryStyles;     // styleCount offsets into mStyles
    const void*                 mStrings;         // string data region
    uint32_t                    mStringPoolSize;  // element count (uint16_t, or uint8_t for UTF-8)
    const uint32_t*             mStyles;          // style data region
    uint32_t                    mStylePoolSize;   // uint32_t count
    // Lazy UTF-8 -> UTF-16 decode cache. For a UTF-8 pool, stringAt converts and
    // caches here; empty string marks not-yet-decoded (re-decode is harmless).
    mutable std::vector<std::u16string> mCache;
};

// ---------------------------------------------------------------------------
// Res_value — a single typed resource value (port of ResourceTypes.h).
// Used for attribute typed values in ResXMLTree and entry values in ResTable.
// ---------------------------------------------------------------------------

struct Res_value {
    // Number of bytes in this structure.
    uint16_t size;

    // Always set to 0.
    uint8_t res0;

    // Type of the data value.
    enum : uint8_t {
        TYPE_NULL             = 0x00,
        TYPE_REFERENCE        = 0x01,
        TYPE_ATTRIBUTE        = 0x02,
        TYPE_STRING           = 0x03,
        TYPE_FLOAT            = 0x04,
        TYPE_DIMENSION        = 0x05,
        TYPE_FRACTION         = 0x06,
        TYPE_DYNAMIC_REFERENCE = 0x07,
        TYPE_DYNAMIC_ATTRIBUTE = 0x08,

        TYPE_FIRST_INT        = 0x10,
        TYPE_INT_DEC          = 0x10,
        TYPE_INT_HEX          = 0x11,
        TYPE_INT_BOOLEAN      = 0x12,

        TYPE_FIRST_COLOR_INT  = 0x1c,
        TYPE_INT_COLOR_ARGB8  = 0x1c,
        TYPE_INT_COLOR_RGB8   = 0x1d,
        TYPE_INT_COLOR_ARGB4  = 0x1e,
        TYPE_INT_COLOR_RGB4   = 0x1f,
        TYPE_LAST_COLOR_INT   = 0x1f,
        TYPE_LAST_INT         = 0x1f
    };
    uint8_t dataType;

    // Structure of complex data values (TYPE_DIMENSION / TYPE_FRACTION).
    enum {
        COMPLEX_UNIT_SHIFT = 0,
        COMPLEX_UNIT_MASK  = 0xf,
        COMPLEX_UNIT_PX    = 0,
        COMPLEX_UNIT_DIP   = 1,
        COMPLEX_UNIT_SP    = 2,
        COMPLEX_UNIT_PT    = 3,
        COMPLEX_UNIT_IN    = 4,
        COMPLEX_UNIT_MM    = 5,
        COMPLEX_UNIT_FRACTION      = 0,
        COMPLEX_UNIT_FRACTION_PARENT = 1,
        COMPLEX_RADIX_SHIFT = 4,
        COMPLEX_RADIX_MASK  = 0x3,
        COMPLEX_RADIX_23p0  = 0,
        COMPLEX_RADIX_16p7  = 1,
        COMPLEX_RADIX_8p15  = 2,
        COMPLEX_RADIX_0p23  = 3,
        COMPLEX_MANTISSA_SHIFT = 8,
        COMPLEX_MANTISSA_MASK  = 0xffffff
    };

    // Possible data values for TYPE_NULL.
    enum {
        DATA_NULL_UNDEFINED = 0,
        DATA_NULL_EMPTY     = 1
    };

    // The data for this item, as interpreted according to dataType.
    typedef uint32_t data_type;
    data_type data;

    // Copies a device-endian Res_value to host-endian (identity on LE host).
    void copyFrom_dtoh(const Res_value& src);
};

// ---------------------------------------------------------------------------
// ResTable_config — a resource configuration descriptor (port of
// ResourceTypes.h). The multi-axis matching engine that lets the same resource
// id resolve to different values across locale / density / screen-size / etc.
// variants. The comparison/scoring methods (match / isBetterThan /
// isMoreSpecificThan / compare / diff) are faithful ports of AOSP; the locale
// axis delegates to the vendored android::localeData* (LocaleData.{h,cpp}).
// ACONFIGURATION_* values are inlined as the integer constants from
// frameworks/native/include/android/configuration.h (no NDK dep).
// ---------------------------------------------------------------------------

struct ResTable_config {
    // Number of bytes in this structure.
    uint32_t size;

    union {
        struct {
            uint16_t mcc;   // Mobile country code (SIM); 0 = any
            uint16_t mnc;   // Mobile network code (SIM); 0 = any
        };
        uint32_t imsi;
    };

    union {
        struct {
            // Two 7-bit ASCII ISO-639-1 language, or a packed 3-letter code
            // (high bit set). Bigendian layout irrespective of host.
            char language[2];
            // Two 7-bit ASCII region, or packed 3-digit UN M.49 code.
            char country[2];
        };
        uint32_t locale;
    };

    enum {
        ORIENTATION_ANY    = 0x0000,
        ORIENTATION_PORT   = 0x0001,
        ORIENTATION_LAND   = 0x0002,
        ORIENTATION_SQUARE = 0x0003,
    };
    enum {
        TOUCHSCREEN_ANY     = 0x0000,
        TOUCHSCREEN_NOTOUCH = 0x0001,
        TOUCHSCREEN_STYLUS  = 0x0002,
        TOUCHSCREEN_FINGER  = 0x0003,
    };
    enum {
        DENSITY_DEFAULT = 0,
        DENSITY_LOW     = 120,
        DENSITY_MEDIUM  = 160,
        DENSITY_TV      = 213,
        DENSITY_HIGH    = 240,
        DENSITY_XHIGH   = 320,
        DENSITY_XXHIGH  = 480,
        DENSITY_XXXHIGH = 640,
        DENSITY_ANY     = 0xfffe,
        DENSITY_NONE    = 0xffff
    };

    union {
        struct {
            uint8_t  orientation;
            uint8_t  touchscreen;
            uint16_t density;
        };
        uint32_t screenType;
    };

    enum {
        KEYBOARD_ANY    = 0x0000,
        KEYBOARD_NOKEYS = 0x0001,
        KEYBOARD_QWERTY = 0x0002,
        KEYBOARD_12KEY  = 0x0003,
    };
    enum {
        NAVIGATION_ANY      = 0x0000,
        NAVIGATION_NONAV    = 0x0001,
        NAVIGATION_DPAD     = 0x0002,
        NAVIGATION_TRACKBALL = 0x0003,
        NAVIGATION_WHEEL    = 0x0004,
    };
    enum {
        MASK_KEYSHIDDEN  = 0x0003,
        KEYSHIDDEN_ANY   = 0x0000,
        KEYSHIDDEN_NO    = 0x0001,
        KEYSHIDDEN_YES   = 0x0002,
        KEYSHIDDEN_SOFT  = 0x0003,
    };
    enum {
        MASK_NAVHIDDEN  = 0x000c,
        SHIFT_NAVHIDDEN = 2,
        NAVHIDDEN_ANY   = 0x0000 << 2,
        NAVHIDDEN_NO    = 0x0001 << 2,
        NAVHIDDEN_YES   = 0x0002 << 2,
    };

    union {
        struct {
            uint8_t keyboard;
            uint8_t navigation;
            uint8_t inputFlags;
            uint8_t inputPad0;
        };
        uint32_t input;
    };

    union {
        struct {
            uint16_t screenWidth;
            uint16_t screenHeight;
        };
        uint32_t screenSize;
    };

    union {
        struct {
            uint16_t sdkVersion;
            uint16_t minorVersion;   // always 0
        };
        uint32_t version;
    };

    enum {
        MASK_SCREENSIZE    = 0x0f,
        SCREENSIZE_ANY     = 0x00,
        SCREENSIZE_SMALL   = 0x01,
        SCREENSIZE_NORMAL  = 0x02,
        SCREENSIZE_LARGE   = 0x03,
        SCREENSIZE_XLARGE  = 0x04,

        MASK_SCREENLONG    = 0x30,
        SHIFT_SCREENLONG   = 4,
        SCREENLONG_ANY     = 0x00 << 4,
        SCREENLONG_NO      = 0x01 << 4,
        SCREENLONG_YES     = 0x02 << 4,

        MASK_LAYOUTDIR     = 0xC0,
        SHIFT_LAYOUTDIR    = 6,
        LAYOUTDIR_ANY      = 0x00 << 6,
        LAYOUTDIR_LTR      = 0x01 << 6,
        LAYOUTDIR_RTL      = 0x02 << 6,
    };
    enum {
        MASK_UI_MODE_TYPE  = 0x0f,
        UI_MODE_TYPE_ANY       = 0x00,
        UI_MODE_TYPE_NORMAL    = 0x01,
        UI_MODE_TYPE_DESK      = 0x02,
        UI_MODE_TYPE_CAR       = 0x03,
        UI_MODE_TYPE_TELEVISION = 0x04,
        UI_MODE_TYPE_APPLIANCE = 0x05,
        UI_MODE_TYPE_WATCH     = 0x06,
        UI_MODE_TYPE_VR_HEADSET = 0x07,

        MASK_UI_MODE_NIGHT = 0x30,
        SHIFT_UI_MODE_NIGHT = 4,
        UI_MODE_NIGHT_ANY  = 0x00 << 4,
        UI_MODE_NIGHT_NO   = 0x01 << 4,
        UI_MODE_NIGHT_YES  = 0x02 << 4,
    };

    union {
        struct {
            uint8_t  screenLayout;
            uint8_t  uiMode;
            uint16_t smallestScreenWidthDp;
        };
        uint32_t screenConfig;
    };

    union {
        struct {
            uint16_t screenWidthDp;
            uint16_t screenHeightDp;
        };
        uint32_t screenSizeDp;
    };

    char localeScript[4];       // ISO-15924 script (Hant, Latn, ...)
    char localeVariant[8];      // BCP-47 variant subtag

    enum {
        MASK_SCREENROUND   = 0x03,
        SCREENROUND_ANY    = 0x00,
        SCREENROUND_NO     = 0x01,
        SCREENROUND_YES    = 0x02,
    };
    enum {
        MASK_WIDE_COLOR_GAMUT = 0x03,
        WIDE_COLOR_GAMUT_ANY  = 0x00,
        WIDE_COLOR_GAMUT_NO   = 0x01,
        WIDE_COLOR_GAMUT_YES  = 0x02,

        MASK_HDR         = 0x0c,
        SHIFT_COLOR_MODE_HDR = 2,
        HDR_ANY          = 0x00 << 2,
        HDR_NO           = 0x01 << 2,
        HDR_YES          = 0x02 << 2,
    };

    union {
        struct {
            uint8_t  screenLayout2;     // round/notround qualifier
            uint8_t  colorMode;         // wide gamut, HDR
            uint16_t screenConfigPad2;  // reserved
        };
        uint32_t screenConfig2;
    };

    bool localeScriptWasComputed;
    char localeNumberingSystem[8];   // BCP-47 'nu' extension

    // Configuration-change flags (values from AConfiguration CONFIG_*).
    enum {
        CONFIG_MCC                 = 0x0001,
        CONFIG_MNC                 = 0x0002,
        CONFIG_LOCALE              = 0x0004,
        CONFIG_TOUCHSCREEN         = 0x0008,
        CONFIG_KEYBOARD            = 0x0010,
        CONFIG_KEYBOARD_HIDDEN     = 0x0020,
        CONFIG_NAVIGATION          = 0x0040,
        CONFIG_ORIENTATION         = 0x0080,
        CONFIG_DENSITY             = 0x0100,
        CONFIG_SCREEN_SIZE         = 0x0200,
        CONFIG_VERSION             = 0x0400,
        CONFIG_SCREEN_LAYOUT       = 0x0800,
        CONFIG_UI_MODE             = 0x1000,
        CONFIG_SMALLEST_SCREEN_SIZE = 0x2000,
        CONFIG_LAYOUTDIR           = 0x4000,
        CONFIG_SCREEN_ROUND        = 0x8000,
        CONFIG_COLOR_MODE          = 0x10000,
    };

    void copyFromDeviceNoSwap(const ResTable_config& o);
    void copyFromDtoH(const ResTable_config& o);

    int compare(const ResTable_config& o) const;
    int compareLogical(const ResTable_config& o) const;
    inline bool operator<(const ResTable_config& o) const { return compare(o) < 0; }

    int  diff(const ResTable_config& o) const;
    bool isMoreSpecificThan(const ResTable_config& o) const;
    int  isLocaleMoreSpecificThan(const ResTable_config& o) const;
    bool isLocaleBetterThan(const ResTable_config& o, const ResTable_config* requested) const;
    bool isBetterThan(const ResTable_config& o, const ResTable_config* requested) const;
    bool match(const ResTable_config& settings) const;

    void packLanguage(const char* language);
    void packRegion(const char* region);
    size_t unpackLanguage(char language[4]) const;
    size_t unpackRegion(char region[4]) const;
    // BCP-47 locale string (e.g. "en-US"), up to RESTABLE_MAX_LOCALE_LEN.
    void getBcp47Locale(char out[40], bool canonicalize = false) const;
};

// ---------------------------------------------------------------------------
// Binary XML tree structures (port of ResourceTypes.h).
// ---------------------------------------------------------------------------

// Header that appears at the front of every XML tree.
struct ResXMLTree_header {
    struct ResChunk_header header;
};

// Basic XML tree node. Extended info is at header.headerSize bytes past the node.
struct ResXMLTree_node {
    struct ResChunk_header header;
    uint32_t lineNumber;                  // line in original source
    struct ResStringPool_ref comment;     // optional comment; -1 if none
};

// Extended node for CDATA (after a ResXMLTree_node).
struct ResXMLTree_cdataExt {
    struct ResStringPool_ref data;        // raw CDATA character data
    struct Res_value typedData;           // typed value of the CDATA
};

// Extended node for namespace start/end.
struct ResXMLTree_namespaceExt {
    struct ResStringPool_ref prefix;
    struct ResStringPool_ref uri;
};

// Extended node for element start/end.
struct ResXMLTree_endElementExt {
    struct ResStringPool_ref ns;
    struct ResStringPool_ref name;
};

// Extended node for start tags (includes attributes).
struct ResXMLTree_attrExt {
    struct ResStringPool_ref ns;
    struct ResStringPool_ref name;
    uint16_t attributeStart;   // byte offset from this struct to the attributes
    uint16_t attributeSize;    // size of each ResXMLTree_attribute
    uint16_t attributeCount;   // number of attributes
    uint16_t idIndex;          // 1-based index of "id" attribute; 0 if none
    uint16_t classIndex;       // 1-based index of "class" attribute; 0 if none
    uint16_t styleIndex;       // 1-based index of "style" attribute; 0 if none
};

struct ResXMLTree_attribute {
    struct ResStringPool_ref ns;
    struct ResStringPool_ref name;
    struct ResStringPool_ref rawValue;   // original raw string value (-1 if none)
    struct Res_value typedValue;         // processed typed value
};

class ResXMLTree;

// Pull-style parser over a binary XML tree. Mirrors AOSP ResXMLParser: it holds a
// reference to the owning ResXMLTree (string pool / resource id map / data bounds)
// and walks the node chunk stream via next().
class ResXMLParser {
public:
    explicit ResXMLParser(const ResXMLTree& tree);

    enum event_code_t {
        BAD_DOCUMENT      = -1,
        START_DOCUMENT    = 0,
        END_DOCUMENT      = 1,

        FIRST_CHUNK_CODE  = RES_XML_FIRST_CHUNK_TYPE,

        START_NAMESPACE   = RES_XML_START_NAMESPACE_TYPE,
        END_NAMESPACE     = RES_XML_END_NAMESPACE_TYPE,
        START_TAG         = RES_XML_START_ELEMENT_TYPE,
        END_TAG           = RES_XML_END_ELEMENT_TYPE,
        TEXT              = RES_XML_CDATA_TYPE
    };

    struct ResXMLPosition {
        event_code_t           eventCode;
        const ResXMLTree_node* curNode;
        const void*            curExt;
    };

    void restart();
    const ResStringPool& getStrings() const;

    event_code_t getEventType() const;
    // Note, unlike XmlPullParser, the first call to next() returns the START_TAG
    // of the first element.
    event_code_t next();

    // Available for all nodes:
    int32_t getCommentID() const;
    const char16_t* getComment(size_t* outLen) const;
    uint32_t getLineNumber() const;

    // Available for TEXT:
    int32_t getTextID() const;
    const char16_t* getText(size_t* outLen) const;
    ssize_t getTextValue(Res_value* outValue) const;

    // Available for START_NAMESPACE / END_NAMESPACE:
    int32_t getNamespacePrefixID() const;
    const char16_t* getNamespacePrefix(size_t* outLen) const;
    int32_t getNamespaceUriID() const;
    const char16_t* getNamespaceUri(size_t* outLen) const;

    // Available for START_TAG / END_TAG:
    int32_t getElementNamespaceID() const;
    const char16_t* getElementNamespace(size_t* outLen) const;
    int32_t getElementNameID() const;
    const char16_t* getElementName(size_t* outLen) const;

    // Attribute accessors (START_TAG only):
    size_t getAttributeCount() const;
    int32_t getAttributeNamespaceID(size_t idx) const;          // -1 none, -2 oob
    const char16_t* getAttributeNamespace(size_t idx, size_t* outLen) const;

    int32_t getAttributeNameID(size_t idx) const;
    const char16_t* getAttributeName(size_t idx, size_t* outLen) const;
    uint32_t getAttributeNameResID(size_t idx) const;

    // Only if the underlying string pool is UTF-8:
    const char* getAttributeNamespace8(size_t idx, size_t* outLen) const;
    const char* getAttributeName8(size_t idx, size_t* outLen) const;

    int32_t getAttributeValueStringID(size_t idx) const;
    const char16_t* getAttributeStringValue(size_t idx, size_t* outLen) const;

    int32_t getAttributeDataType(size_t idx) const;
    int32_t getAttributeData(size_t idx) const;
    ssize_t getAttributeValue(size_t idx, Res_value* outValue) const;

    ssize_t indexOfAttribute(const char* ns, const char* attr) const;
    ssize_t indexOfAttribute(const char16_t* ns, size_t nsLen,
                             const char16_t* attr, size_t attrLen) const;

    ssize_t indexOfID() const;
    ssize_t indexOfClass() const;
    ssize_t indexOfStyle() const;

    void getPosition(ResXMLPosition* pos) const;
    void setPosition(const ResXMLPosition& pos);

    void setSourceResourceId(const uint32_t resId);
    uint32_t getSourceResourceId() const;

private:
    friend class ResXMLTree;

    event_code_t nextNode();

    const ResXMLTree&  mTree;
    event_code_t       mEventCode;
    const ResXMLTree_node* mCurNode;
    const void*        mCurExt;
    uint32_t           mSourceResourceId;
};

/**
 * Convenience class for accessing data in a ResXMLTree resource. Owns the backing
 * data (optionally a copy), the string pool and the resource-id map; IS-A
 * ResXMLParser so it can walk itself.
 */
class ResXMLTree : public ResXMLParser {
public:
    ResXMLTree();
    ~ResXMLTree();

    status_t setTo(const void* data, size_t size, bool copyData = false);
    status_t getError() const;
    void uninit();

private:
    friend class ResXMLParser;

    status_t validateNode(const ResXMLTree_node* node) const;

    status_t                 mError;
    void*                    mOwnedData;
    const ResXMLTree_header* mHeader;
    size_t                   mSize;
    const uint8_t*           mDataEnd;
    ResStringPool            mStrings;
    const uint32_t*          mResIds;
    size_t                   mNumResIds;
    const ResXMLTree_node*   mRootNode;
    const void*              mRootExt;
    event_code_t             mRootCode;
};

// ---------------------------------------------------------------------------
// Resource table (resources.arsc) structures + ResTable (port of
// ResourceTypes.{h,cpp}). ResTable loads an arsc and resolves a resource id to
// a value, selecting the best ResTable_config variant (locale / density / ...)
// for the current parameters. Stage 3b: a focused core (load + getResource +
// reference resolution + bag read); overlays / idmaps / dynamic packages /
// themes are deferred. The resolution/selection semantics are faithful to AOSP
// (getEntry / parsePackage).
// ---------------------------------------------------------------------------

// Build a resource identifier from package/type/entry.
static inline uint32_t Res_MAKEID(uint32_t packageId, uint32_t typeId, uint32_t entryId) {
    return (((packageId + 1) << 24) | (((typeId + 1) & 0xFF) << 16) | (entryId & 0xFFFF));
}
// Resource-id field accessors (port of ResourceTypes.h macros).
static inline bool     Res_VALIDID(uint32_t id) { return id != 0; }
static inline uint32_t Res_GETPACKAGE(uint32_t id) { return ((id >> 24) - 1); }   // 0-based
static inline uint32_t Res_GETTYPE(uint32_t id)    { return (((id >> 16) & 0xFF) - 1); }
static inline uint32_t Res_GETENTRY(uint32_t id)   { return (id & 0xFFFF); }
#define APP_PACKAGE_ID 0x7f
#define SYS_PACKAGE_ID 0x01

// Shared-library / overlay package-id translation. A package compiled with a
// build-time id (e.g. an app-as-library built as 0x7f) gets a runtime-assigned
// id when loaded alongside other packages; this table rewrites the package byte
// of resource references. Port of AOSP DynamicRefTable. load() (from a
// RES_TABLE_LIBRARY_TYPE chunk) and staged aliases are deferred.
class DynamicRefTable {
public:
    DynamicRefTable();
    DynamicRefTable(uint8_t packageId, bool appAsLib);
    virtual ~DynamicRefTable() = default;

    // Direct build-time -> run-time package id mapping (the path addInternal uses
    // when assigning runtime ids to loaded packages).
    void addMapping(uint8_t buildPackageId, uint8_t runtimePackageId);
    // Name-based mapping (resolves via the entries loaded from a lib chunk).
    status_t addMapping(const std::string& packageName, uint8_t packageId);
    // Merge another table's mappings (same assigned package id); conflicts fail.
    status_t addMappings(const DynamicRefTable& other);

    bool requiresLookup(const Res_value* value) const;
    virtual status_t lookupResourceId(uint32_t* resId) const;
    status_t lookupResourceValue(Res_value* value) const;

    const std::map<std::string, uint8_t>& entries() const { return mEntries; }
private:
    uint8_t mAssignedPackageId;
    uint8_t mLookupTable[256];
    std::map<std::string, uint8_t> mEntries;
    bool mAppAsLib;
};

// Reference to a unique entry (0xpptteeee) in a resource table.
struct ResTable_ref {
    uint32_t ident;
};

// arsc file header.
struct ResTable_header {
    struct ResChunk_header header;
    uint32_t packageCount;   // number of ResTable_package chunks
};

// A package: type/key string pools + type/typeSpec chunks follow.
struct ResTable_package {
    struct ResChunk_header header;
    uint32_t id;             // package id (starts at 1); 0 = not a base package
    uint16_t name[128];      // package name, NUL-terminated
    uint32_t typeStrings;    // offset to type symbol string pool
    uint32_t lastPublicType;
    uint32_t keyStrings;     // offset to key symbol string pool
    uint32_t lastPublicKey;
    uint32_t typeIdOffset;
};

// TypeSpec: per-entry config-change flags for a type id.
struct ResTable_typeSpec {
    struct ResChunk_header header;
    uint8_t  id;             // type id (starts at 1)
    uint8_t  res0;
    uint16_t res1;
    uint32_t entryCount;     // number of uint32_t spec masks that follow
    enum : uint32_t {
        SPEC_PUBLIC      = 0x40000000u,
        SPEC_STAGED_API  = 0x20000000u,
    };
};

// A set of entries for one type under one config. Multiple per type id (one
// per config variant). Followed by entryCount uint32_t offsets (or sparse
// entries), then the entry data at entriesStart.
struct ResTable_type {
    struct ResChunk_header header;
    enum { NO_ENTRY = 0xFFFFFFFF };
    uint8_t  id;             // type id (starts at 1)
    enum { FLAG_SPARSE = 0x01 };
    uint8_t  flags;
    uint16_t reserved;
    uint32_t entryCount;     // number of entry offsets that follow
    uint32_t entriesStart;   // offset from header where entry data starts
    ResTable_config config;  // MUST be last (variable-size across releases)
};

// Sparse entry (when FLAG_SPARSE set): packs entry idx + offset/4.
union ResTable_sparseTypeEntry {
    uint32_t entry;
    struct { uint16_t idx; uint16_t offset; };
};

// Start of an entry: followed by a Res_value (simple) or ResTable_map[] (bag).
struct ResTable_entry {
    uint16_t size;
    enum {
        FLAG_COMPLEX = 0x0001,  // followed by ResTable_map[]
        FLAG_PUBLIC  = 0x0002,
        FLAG_WEAK    = 0x0004,
    };
    uint16_t flags;
    struct ResStringPool_ref key;   // into the package keyStrings
};

// A bag (complex) entry: inherits from a parent, then count name/value maps.
struct ResTable_map_entry {
    uint16_t size;
    uint16_t flags;
    struct ResStringPool_ref key;
    ResTable_ref parent;     // parent map resource id, or 0
    uint32_t count;          // number of ResTable_map that follow
};

// A single name/value pair in a bag.
struct ResTable_map {
    ResTable_ref name;
    Res_value value;
};

class ResTable {
public:
    ResTable();
    ~ResTable();

    // Load a resources.arsc blob (appAsLib=false). Multiple add() calls
    // accumulate into one table (multi-package); the blockIndex getResource
    // returns identifies the owning Header. Returns NO_ERROR / BAD_TYPE.
    status_t add(const void* data, size_t size, int32_t cookie = -1, bool copyData = false);
    // Load a blob as a shared library (appAsLib=true): a package built with the
    // app id (0x7f) is reassigned a runtime id and its self-references are
    // translated via its DynamicRefTable (the IME / add-on package case).
    status_t add(const void* data, size_t size, bool appAsLib, int32_t cookie, bool copyData);
    status_t getError() const;
    void uninit();

    // Set the request configuration used by getResource() (host-endian; copies).
    // Returns NO_ERROR (matches AOSP signature).
    status_t setParameters(const ResTable_config* params);
    void getParameters(ResTable_config* params) const;

    // The global value string pool of the FIRST loaded arsc (TYPE_STRING values
    // index into their owning arsc's pool; for the common single-arsc case this
    // is it). For multi-arsc, resolve via getResource.
    const ResStringPool& getStringPool() const;

    // Resolve resID to a value for the current parameters. Faithful to AOSP:
    // returns the owning asset's blockIndex (>= 0) on success, or a negative
    // error (BAD_INDEX / BAD_VALUE / NAME_NOT_FOUND). `density` (>0) overrides
    // mParams.density for this call; `outSpecFlags` receives the aggregated
    // TypeSpec flags; `outConfig` receives the matched variant. A complex (bag)
    // entry yields BAD_VALUE unless mayBeBag is set (it only silences the log).
    ssize_t getResource(uint32_t resID, Res_value* outValue, bool mayBeBag = false,
                        uint16_t density = 0, uint32_t* outSpecFlags = nullptr,
                        ResTable_config* outConfig = nullptr) const;

    // Follow TYPE_REFERENCE/ATTRIBUTE chains (bounded, count < 20 — matches
    // AOSP). Returns the final blockIndex (>= 0) or a negative error; on a
    // reference into a non-resolvable (e.g. style) target, returns the last good
    // blockIndex and leaves *value as the reference.
    ssize_t resolveReference(Res_value* value, ssize_t blockIndex,
                             uint32_t* outLastRef = nullptr,
                             uint32_t* inoutTypeSpecFlags = nullptr,
                             ResTable_config* outConfig = nullptr) const;

    // Convenience: resolve a string resource to its UTF-16 text (follows refs).
    // Returns nullptr if the resource is not a (string-resolvable) string.
    const char16_t* getResourceString(uint32_t resId, size_t* outLen) const;

    // Read a bag (complex) entry: fills outCount and returns a pointer to the
    // first ResTable_map (count entries), or nullptr if not a bag / not found.
    const ResTable_map* getBag(uint32_t resId, size_t* outCount,
                               ResTable_config* outConfig = nullptr,
                               ssize_t* outBlock = nullptr,
                               uint32_t* outSpecFlags = nullptr) const;

    // Look up a package's type/key name pools for diagnostics.
    bool getResourceName(uint32_t resId, std::string* outPackage,
                         std::string* outType, std::string* outKey) const;

    // --- AssetManager2 management surface ---
    // Resolve a resource name to its id (0 if not found). package=="" searches
    // all packages. e.g. getIdentifier("hello","string","com.example.verify").
    uint32_t getIdentifier(const std::string& name, const std::string& type,
                           const std::string& package) const;
    // Every distinct configuration present in the table (for introspection).
    void getConfigurations(std::vector<ResTable_config>* out) const;
    // Names of all loaded packages.
    std::vector<std::string> listPackageNames() const;

    // Enumerate EVERY existing resource in the table — the runtime equivalent of
    // dumping the R class from an arsc. For each present entry yields its id
    // (0xpptteeee), type name and key name. Returns the count.
    struct ResourceRef {
        uint32_t    resId;
        uint8_t     packageId;
        std::string type;
        std::string key;
    };
    size_t listAllResources(std::vector<ResourceRef>* out) const;

    // Style parent resource id of a bag (style) entry, or 0 if none / not a bag.
    uint32_t getBagParent(uint32_t resId, ResTable_config* outConfig = nullptr) const;

    // A Theme holds a set of applied styles; attributes resolve against it (with
    // parent inheritance + force/TYPE_NULL override semantics). Port of AOSP
    // ResTable::Theme, minus the per-type bag_set locking cache (a plain map is
    // semantically equivalent). applyStyle follows the style's parent chain so a
    // child style overrides its parent.
    class Theme {
    public:
        explicit Theme(const ResTable& table);
        ~Theme();

        const ResTable& getResTable() const { return mTable; }

        status_t applyStyle(uint32_t resID, bool force = false);
        status_t setTo(const Theme& other);
        status_t clear();

        // Retrieve a themed attribute. Returns the owning-asset blockIndex (>= 0)
        // and fills outValue, or a negative error if not set. Does NOT follow
        // references — call resolveAttributeReference() for that.
        ssize_t getAttribute(uint32_t resID, Res_value* outValue,
                             uint32_t* outTypeSpecFlags = nullptr) const;

        // Like ResTable::resolveReference, but TYPE_ATTRIBUTE is resolved via
        // this theme (getAttribute) rather than the table.
        ssize_t resolveAttributeReference(Res_value* inOutValue, ssize_t blockIndex,
                                         uint32_t* outLastRef = nullptr,
                                         uint32_t* inoutTypeSpecFlags = nullptr,
                                         ResTable_config* inoutConfig = nullptr) const;

        // Bit mask of CONFIG_* changes that would impact this theme (so it must
        // be rebuilt when the configuration changes).
        uint32_t getChangingConfigurations() const;

    private:
        const ResTable& mTable;
        struct ThemedItem {
            Res_value value;
            ssize_t   stringBlock;   // owning header index (for string values)
            uint32_t  typeSpecFlags;
            bool      set;
        };
        std::map<uint32_t, ThemedItem> mEntries;  // attr resID -> item
        uint32_t mTypeSpecFlags = 0;
        status_t applyStyleChain(uint32_t resID, bool force, int depth);
    };

private:
    // One loaded resources.arsc blob (AOSP Header). Owns its global value string
    // pool — TYPE_STRING values index into the pool of the arsc they came from.
    struct Header {
        int32_t              index = 0;
        int32_t              cookie = -1;
        const uint8_t*       data = nullptr;
        size_t               size = 0;
        const uint8_t*       dataEnd = nullptr;
        std::vector<uint8_t> owned;       // copy when copyData
        ResStringPool        values;      // this arsc's global value string pool
    };
    // A config variant group for one type id (from a TypeSpec + its type chunks).
    struct TypeGroup {
        uint32_t             entryCount = 0;
        const uint32_t*      specFlags = nullptr;
        std::vector<const ResTable_type*> configs;
    };
    // One ResTable_package chunk (AOSP Package). Belongs to a Header + a
    // (runtime) id; carries its DynamicRefTable for build->runtime id translation.
    struct Package {
        size_t               headerIndex = 0;  // index into mHeaders (NOT a pointer:
                                               // mHeaders reallocs on each add())
        uint8_t              id = 0;          // RUNTIME package id
        uint8_t              buildId = 0;     // id as declared in the arsc
        bool                 isDynamic = false;
        std::string          name;            // package name (UTF-8)
        DynamicRefTable      dynamicRefTable;
        ResStringPool        typeStrings;
        ResStringPool        keyStrings;
        std::vector<TypeGroup> types;         // indexed [typeId - 1]
    };

    // addInternal mirrors AOSP: appAsLib makes an app-id (0x7f) package be
    // reassigned a runtime id (shared-library / IME / add-on package case).
    status_t addInternal(const void* data, size_t size, bool appAsLib,
                         int32_t cookie, bool copyData);
    status_t parsePackage(const ResTable_package* pkg, Header* header,
                          bool appAsLib, uint8_t* outRuntimeId);
    Package* packageForId(uint32_t pkgId);
    const Package* packageForId(uint32_t pkgId) const;
    // Core selection (mirrors AOSP getEntry): over configs of (typeId) pick the
    // one matching `desired` that isBetterThan the rest. Fills outEntry.
    const ResTable_entry* getBestEntry(const Package& pkg, uint8_t typeId, uint32_t entryId,
                                       const ResTable_config& desired,
                                       ResTable_config* outConfig,
                                       uint32_t* outSpecFlags) const;
    // First present entry for (typeId, entryId) across any config (for enumeration).
    const ResTable_entry* anyEntry(const Package& pkg, uint8_t typeId, uint32_t entryId) const;
    // Decode a string-pool entry to UTF-8 regardless of pool encoding.
    static std::string poolString(const ResStringPool& pool, uint32_t idx);
    // Find a string index in a pool by content (linear).
    static int32_t poolIndexOf(const ResStringPool& pool, const std::string& needle);

    status_t              mError;
    int32_t               mCookie;
    std::vector<Header>   mHeaders;
    std::vector<Package>  mPackages;
    std::vector<int>      mPackageMap;     // runtime package id -> index+1 into mPackages
    uint8_t               mNextPackageId;  // next runtime id for dynamic packages (starts at 2)
    ResTable_config       mParams;         // current request config (host-endian)
};

// ---------------------------------------------------------------------------
// Full attribute resolution (AssetManager.applyStyle merge) + TypedArray.
// obtainStyledAttributes resolves each attr in a styleable set through the
// Android priority chain: the XML element's own value, then the element's
// style=, then defStyleAttr (via the theme), then defStyleRes, then the theme's
// direct value. TypedArray is the typed getter view over the result (indexed by
// position in the attrs[] array), matching the Android TypedArray shape.
// ---------------------------------------------------------------------------

// One resolved attribute. stringBlock is the owning header index for TYPE_STRING
// values sourced from a style/theme (element-sourced strings use the AXML pool).
struct StyledAttr {
    Res_value value;
    ssize_t   stringBlock;
    bool      set;
};

void obtainStyledAttributes(const ResXMLTree& xml, const ResTable& table,
                            const ResTable::Theme* theme,
                            const uint32_t* attrs, size_t attrCount,
                            uint32_t defStyleAttr, uint32_t defStyleRes,
                            StyledAttr* out);

class TypedArray {
public:
    TypedArray(const ResTable& table, const StyledAttr* vals, size_t count,
               const ResXMLTree* xmlSrc = nullptr, float density = 1.0f)
        : mTable(table), mVals(vals), mCount(count), mXml(xmlSrc), mDensity(density) {}
    size_t size() const { return mCount; }
    bool hasValue(size_t idx) const { return idx < mCount && mVals[idx].set; }

    int32_t  getInt(size_t idx, int32_t def) const;
    bool     getBoolean(size_t idx, bool def) const;
    uint32_t getColor(size_t idx, uint32_t def) const;
    float    getDimension(size_t idx, float def) const;
    int32_t  getDimensionPixelSize(size_t idx, int32_t def) const;
    uint32_t getResourceId(size_t idx, uint32_t def) const;
    std::string getString(size_t idx) const;
private:
    bool get(size_t idx, Res_value* v) const {
        if (!hasValue(idx)) return false;
        *v = mVals[idx].value;
        return true;
    }
    const ResTable&    mTable;
    const StyledAttr*  mVals;
    size_t             mCount;
    const ResXMLTree*  mXml;     // for element-sourced TYPE_STRING values
    float              mDensity;
};

} // namespace cdroid

#endif // __CDROID_ANDROIDFW_RESOURCETYPES_H
