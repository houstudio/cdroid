#ifndef __KEY_LAYOUT_MAP_H__
#define __KEY_LAYOUT_MAP_H__

#include <stdint.h>
#include <map>
#include <string>
#include <vector>
namespace cdroid {
 
struct AxisInfo {
    enum Mode {
        // Axis value is reported directly.
        MODE_NORMAL = 0,
        // Axis value should be inverted before reporting.
        MODE_INVERT = 1,
        // Axis value should be split into two axes
        MODE_SPLIT = 2,
    };

    // Axis mode.
    Mode mode;

    // Axis id.
    // When split, this is the axis used for values smaller than the split position.
    int32_t axis;

    // When split, this is the axis used for values after higher than the split position.
    int32_t highAxis;

    // The split value, or 0 if not split.
    int32_t splitValue;

    // The flat value, or -1 if none.
    int32_t flatOverride;

    AxisInfo() : mode(MODE_NORMAL), axis(-1), highAxis(-1), splitValue(0), flatOverride(-1) {
    }
};

/**
 * Describes a mapping from keyboard scan codes and joystick axes to Android key codes and axes.
 *
 * This object is immutable after it has been loaded.
 */
class KeyLayoutMap{
public:
    static int load(const std::string& filename,KeyLayoutMap*& outMap);

    int mapKey(int32_t scanCode,int32_t usageCode,int32_t* outKeyCode,uint32_t* outFlags) const;
    int findScanCodesForKey(int32_t keyCode, std::vector<int32_t>& outScanCodes) const;
    int findScanCodeForLed(int32_t ledCode, int32_t* outScanCode) const;
    int findUsageCodeForLed(int32_t ledCode, int32_t* outUsageCode) const;

    int mapAxis(int32_t scanCode, AxisInfo* outAxisInfo) const;

protected:
    virtual ~KeyLayoutMap();

private:
    struct Key {
        int32_t keyCode;
        uint32_t flags;
    };

    struct Led {
        int32_t ledCode;
    };


    std::map<int32_t, Key> mKeysByScanCode;
    std::map<int32_t, Key> mKeysByUsageCode;
    std::map<int32_t, AxisInfo> mAxes;
    std::map<int32_t, Led> mLedsByScanCode;
    std::map<int32_t, Led> mLedsByUsageCode;

    KeyLayoutMap();

    const Key* getKey(int32_t scanCode, int32_t usageCode) const;

    class Parser {
        KeyLayoutMap* mMap;
        class Tokenizer* mTokenizer;

    public:
        Parser(KeyLayoutMap* map, Tokenizer* tokenizer);
        ~Parser();
        int parse();

    private:
        int parseKey();
        int parseAxis();
        int parseLed();
    };
};

} // namespace cdroid

#endif // __KEY_LAYOUT_MAP_H_
