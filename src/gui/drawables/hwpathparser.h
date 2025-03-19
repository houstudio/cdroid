#ifndef __HWUI_PATH_PARSER_H__
#define __HWUI_PATH_PARSER_H__
#include <drawables/hwvectordrawable.h>
#include <drawables/hwvectordrawableutils.h>
#include <core/path.h>
namespace cdroid{
namespace hwui{
class PathParser {
public:
    struct ParseResult {
        bool failureOccurred = false;
        std::string failureMessage;
    };
    /**
     * Parse the string literal and create a Skia Path. Return true on success.
     */
    static void parseAsciiStringForPath(Cairo::RefPtr<cdroid::Path>& outPath, ParseResult* result,const char* pathStr, size_t strLength);
    static void getPathDataFromAsciiString(PathData* outData, ParseResult* result,const char* pathStr, size_t strLength);
    static void dump(const PathData& data);
    static void validateVerbAndPoints(char verb, size_t points, ParseResult* result);
};

}
}
#endif/*__HWUI_PATH_PARSER_H__*/
