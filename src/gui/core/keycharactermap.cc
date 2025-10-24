#include <stdlib.h>
#include <string.h>
#include <core/inputdevice.h>
#include <private/inputeventlabels.h>
#include <private/keycharactermap.h>
#include <core/tokenizer.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <fstream>
#include <chrono>
// Enables debug output for the parser.
#define DEBUG_PARSER 0

// Enables debug output for mapping.
#define DEBUG_MAPPING 0

#define NO_ERROR 0
#define OK 0
#define BAD_VALUE -1
namespace cdroid {

static const char* WHITESPACE = " \t\r";
static const char* WHITESPACE_OR_PROPERTY_DELIMITER = " \t\r,:";

struct Modifier {
    const char* label;
    int32_t metaState;
};
enum{
  AMETA_NONE = 0,
  AMETA_ALT_ON = 0x02,
  AMETA_ALT_LEFT_ON = 0x10,
  AMETA_ALT_RIGHT_ON = 0x20,
  AMETA_SHIFT_ON = 0x01,
  AMETA_SHIFT_LEFT_ON = 0x40,
  AMETA_SHIFT_RIGHT_ON = 0x80,
  AMETA_SYM_ON = 0x04,
  AMETA_FUNCTION_ON = 0x08,
  AMETA_CTRL_ON = 0x1000,
  AMETA_CTRL_LEFT_ON = 0x2000,
  AMETA_CTRL_RIGHT_ON = 0x4000,
  AMETA_META_ON = 0x10000,
  AMETA_META_LEFT_ON = 0x20000,
  AMETA_META_RIGHT_ON = 0x40000,
  AMETA_CAPS_LOCK_ON = 0x100000,
  AMETA_NUM_LOCK_ON = 0x200000,
  AMETA_SCROLL_LOCK_ON = 0x400000
};
static const Modifier modifiers[] = {
        { "shift", AMETA_SHIFT_ON },
        { "lshift", AMETA_SHIFT_LEFT_ON },
        { "rshift", AMETA_SHIFT_RIGHT_ON },
        { "alt", AMETA_ALT_ON },
        { "lalt", AMETA_ALT_LEFT_ON },
        { "ralt", AMETA_ALT_RIGHT_ON },
        { "ctrl", AMETA_CTRL_ON },
        { "lctrl", AMETA_CTRL_LEFT_ON },
        { "rctrl", AMETA_CTRL_RIGHT_ON },
        { "meta", AMETA_META_ON },
        { "lmeta", AMETA_META_LEFT_ON },
        { "rmeta", AMETA_META_RIGHT_ON },
        { "sym", AMETA_SYM_ON },
        { "fn", AMETA_FUNCTION_ON },
        { "capslock", AMETA_CAPS_LOCK_ON },
        { "numlock", AMETA_NUM_LOCK_ON },
        { "scrolllock", AMETA_SCROLL_LOCK_ON },
};

#if DEBUG_MAPPING
static std::string toString(const char16_t* chars, size_t numChars) {
    std::string result;
    for (size_t i = 0; i < numChars; i++) {
        result.append(i == 0 ? "%d" : ", %d", chars[i]);
    }
    return result;
}
#endif


// --- KeyCharacterMap ---

KeyCharacterMap* KeyCharacterMap::sEmpty = new KeyCharacterMap();

KeyCharacterMap::KeyCharacterMap() :
    mType(KEYBOARD_TYPE_UNKNOWN) {
}

KeyCharacterMap::KeyCharacterMap(const KeyCharacterMap& other)
    :mType(other.mType), mKeysByScanCode(other.mKeysByScanCode),
    mKeysByUsageCode(other.mKeysByUsageCode) {
    for (auto k:other.mKeys) {
        mKeys[k.first]=new Key(*k.second);
    }
}

KeyCharacterMap::~KeyCharacterMap() {
    for (auto k:mKeys) {
        Key* key = k.second;
        delete key;
    }
}

int KeyCharacterMap::load(const std::string& filename, std::istream&in,Format format,KeyCharacterMap*& outMap){
    Tokenizer* tokenizer;
    int status=Tokenizer::fromStream(filename,in, &tokenizer);
    outMap=nullptr;
    if (status) {
        LOGE("Error %d opening key character map file %s.", status, filename.c_str());
    } else {
        status = load(tokenizer, format, outMap);
        delete tokenizer;
    }
    return status;
}

int KeyCharacterMap::load(const std::string& filename,Format format, KeyCharacterMap*& outMap) {
    std::ifstream fs(filename);
    return load(filename,fs,format,outMap);
}

int KeyCharacterMap::loadContents(const std::string& filename, const char* contents,Format format,KeyCharacterMap*& outMap){
    outMap=nullptr;

    Tokenizer* tokenizer;
    int status = Tokenizer::fromContents(filename, contents, &tokenizer);
    if (status) {
        LOGE("Error %d opening key character map.", status);
    } else {
        status = load(tokenizer, format, outMap);
        delete tokenizer;
    }
    return status;
}

int KeyCharacterMap::load(Tokenizer* tokenizer,Format format,KeyCharacterMap*& outMap) {
    int status = OK;
    KeyCharacterMap* map = new KeyCharacterMap();
    if (map==nullptr) {
        LOGE("Error allocating key character map.");
        status = -1;//NO_MEMORY;
    } else {
        Parser parser(map, tokenizer, format);
        status = parser.parse();
        if (!status) 
            outMap = map;
        else delete map;
    }
    return status;
}

KeyCharacterMap* KeyCharacterMap::combine(const KeyCharacterMap* base,const KeyCharacterMap* overlay) {
    if (overlay == NULL) {
        return (KeyCharacterMap*)base;
    }
    if (base == NULL) {
        return (KeyCharacterMap*)overlay;
    }

    KeyCharacterMap* map = new KeyCharacterMap(*base);
    for (auto k:overlay->mKeys) {
        int32_t keyCode = k.first;//overlay->mKeys.keyAt(i);
        Key* key = k.second;//overlay->mKeys.valueAt(i);
        auto itr = map->mKeys.find(keyCode);
        if (itr!=map->mKeys.end()) {
            delete itr->second;
            itr->second = new Key(*key);
        } else {
            map->mKeys[keyCode]= new Key(*key);
        }
    }

    for (auto k:overlay->mKeysByScanCode){//size_t i = 0; i < overlay->mKeysByScanCode.size(); i++) {
        //map->mKeysByScanCode.replaceValueFor(overlay->mKeysByScanCode.keyAt(i),overlay->mKeysByScanCode.valueAt(i));
        map->mKeysByScanCode[k.first]=k.second;
    }

    for (auto k:overlay->mKeysByUsageCode){//size_t i = 0; i < overlay->mKeysByUsageCode.size(); i++) {
        //map->mKeysByUsageCode.replaceValueFor(overlay->mKeysByUsageCode.keyAt(i),overlay->mKeysByUsageCode.valueAt(i));
        map->mKeysByUsageCode[k.first]=k.second;
    }
    return map;
}

KeyCharacterMap* KeyCharacterMap::empty() {
    return sEmpty;
}

int32_t KeyCharacterMap::getKeyboardType() const {
    return mType;
}

char16_t KeyCharacterMap::getDisplayLabel(int32_t keyCode) const {
    char16_t result = 0;
    const Key* key;
    if (getKey(keyCode, &key)) {
        result = key->label;
    }
#if DEBUG_MAPPING
    LOGD("getDisplayLabel: keyCode=%d ~ Result %d.", keyCode, result);
#endif
    return result;
}

char16_t KeyCharacterMap::getNumber(int32_t keyCode) const {
    char16_t result = 0;
    const Key* key;
    if (getKey(keyCode, &key)) {
        result = key->number;
    }
#if DEBUG_MAPPING
    LOGD("getNumber: keyCode=%d ~ Result %d.", keyCode, result);
#endif
    return result;
}

char16_t KeyCharacterMap::getCharacter(int32_t keyCode, int32_t metaState) const {
    char16_t result = 0;
    const Key* key;
    const Behavior* behavior;
    if (getKeyBehavior(keyCode, metaState, &key, &behavior)) {
        result = behavior->character;
    }
#if DEBUG_MAPPING
    LOGD("getCharacter: keyCode=%d, metaState=0x%08x ~ Result %d.", keyCode, metaState, result);
#endif
    return result;
}

bool KeyCharacterMap::getFallbackAction(int32_t keyCode, int32_t metaState,
        FallbackAction* outFallbackAction) const {
    outFallbackAction->keyCode = 0;
    outFallbackAction->metaState = 0;

    bool result = false;
    const Key* key;
    const Behavior* behavior;
    if (getKeyBehavior(keyCode, metaState, &key, &behavior)) {
        if (behavior->fallbackKeyCode) {
            outFallbackAction->keyCode = behavior->fallbackKeyCode;
            outFallbackAction->metaState = metaState & ~behavior->metaState;
            result = true;
        }
    }
#if DEBUG_MAPPING
    LOGD("getFallbackKeyCode: keyCode=%d, metaState=0x%08x ~ Result %s, "
            "fallback keyCode=%d, fallback metaState=0x%08x.",
            keyCode, metaState, result ? "true" : "false",
            outFallbackAction->keyCode, outFallbackAction->metaState);
#endif
    return result;
}

char16_t KeyCharacterMap::getMatch(int32_t keyCode, const char16_t* chars, size_t numChars,
        int32_t metaState) const {
    char16_t result = 0;
    const Key* key;
    if (getKey(keyCode, &key)) {
        // Try to find the most general behavior that maps to this character.
        // For example, the base key behavior will usually be last in the list.
        // However, if we find a perfect meta state match for one behavior then use that one.
        for (const Behavior* behavior = key->firstBehavior; behavior; behavior = behavior->next) {
            if (behavior->character) {
                for (size_t i = 0; i < numChars; i++) {
                    if (behavior->character == chars[i]) {
                        result = behavior->character;
                        if ((behavior->metaState & metaState) == behavior->metaState) {
                            goto ExactMatch;
                        }
                        break;
                    }
                }
            }
        }
    ExactMatch: ;
    }
#if DEBUG_MAPPING
    LOGD("getMatch: keyCode=%d, chars=[%s], metaState=0x%08x ~ Result %d.",
            keyCode, toString(chars, numChars).c_str(), metaState, result);
#endif
    return result;
}

bool KeyCharacterMap::getEvents(int32_t deviceId, const char16_t* chars, size_t numChars,
        std::vector<KeyEvent>& outEvents) const {
    nsecs_t now = std::chrono::steady_clock::now().time_since_epoch().count();// systemTime(SYSTEM_TIME_MONOTONIC);
     
    for (size_t i = 0; i < numChars; i++) {
        int32_t keyCode, metaState;
        char16_t ch = chars[i];
        if (!findKey(ch, &keyCode, &metaState)) {
#if DEBUG_MAPPING
            LOGD("getEvents: deviceId=%d, chars=[%s] ~ Failed to find mapping for character %d.",
                    deviceId, toString(chars, numChars).c_str(), ch);
#endif
            return false;
        }

        int32_t currentMetaState = 0;
        addMetaKeys(outEvents, deviceId, metaState, true, now, &currentMetaState);
        addKey(outEvents, deviceId, keyCode, currentMetaState, true, now);
        addKey(outEvents, deviceId, keyCode, currentMetaState, false, now);
        addMetaKeys(outEvents, deviceId, metaState, false, now, &currentMetaState);
    }
#if DEBUG_MAPPING
    LOGD("getEvents: deviceId=%d, chars=[%s] ~ Generated %d events.",
            deviceId, toString(chars, numChars).c_str(), int32_t(outEvents.size()));
    for (size_t i = 0; i < outEvents.size(); i++) {
        LOGD("  Key: keyCode=%d, metaState=0x%08x, %s.",
                outEvents[i].getKeyCode(), outEvents[i].getMetaState(),
                outEvents[i].getAction() == KeyEvent::ACTION_DOWN ? "down" : "up");
    }
#endif
    return true;
}

int KeyCharacterMap::mapKey(int32_t scanCode, int32_t usageCode, int32_t* outKeyCode) const {
    if (usageCode) {
        auto itr= mKeysByUsageCode.find(usageCode);
        if (itr!=mKeysByUsageCode.end()) {
            *outKeyCode = itr->second;//mKeysByUsageCode.valueAt(index);
#if DEBUG_MAPPING
            LOGD("mapKey: scanCode=%d, usageCode=0x%08x ~ Result keyCode=%d.",
                    scanCode, usageCode, *outKeyCode);
#endif
            return OK;
        }
    }
    if (scanCode) {
        auto itr = mKeysByScanCode.find(scanCode);
        if (itr!=mKeysByScanCode.end()) {
            *outKeyCode = itr->second;
#if DEBUG_MAPPING
            LOGD("mapKey: scanCode=%d, usageCode=0x%08x ~ Result keyCode=%d.",
                    scanCode, usageCode, *outKeyCode);
#endif
            return OK;
        }
    }

#if DEBUG_MAPPING
    LOGD("mapKey: scanCode=%d, usageCode=0x%08x ~ Failed.", scanCode, usageCode);
#endif
    *outKeyCode = KeyEvent::KEYCODE_UNKNOWN;//AKEYCODE_UNKNOWN;
    return -1;//NAME_NOT_FOUND;
}

void KeyCharacterMap::tryRemapKey(int32_t keyCode, int32_t metaState,
                                  int32_t *outKeyCode, int32_t *outMetaState) const {
    *outKeyCode = keyCode;
    *outMetaState = metaState;

    const Key* key;
    const Behavior* behavior;
    if (getKeyBehavior(keyCode, metaState, &key, &behavior)) {
        if (behavior->replacementKeyCode) {
            *outKeyCode = behavior->replacementKeyCode;
            int32_t newMetaState = metaState & ~behavior->metaState;
            // Reset dependent meta states.
            if (behavior->metaState & KeyEvent::META_ALT_ON) {
                newMetaState &= ~(KeyEvent::META_ALT_LEFT_ON | KeyEvent::META_ALT_RIGHT_ON);
            }
            if (behavior->metaState & (KeyEvent::META_ALT_LEFT_ON | KeyEvent::META_ALT_RIGHT_ON)) {
                newMetaState &= ~KeyEvent::META_ALT_ON;
            }
            if (behavior->metaState & KeyEvent::META_CTRL_ON) {
                newMetaState &= ~(KeyEvent::META_CTRL_LEFT_ON | KeyEvent::META_CTRL_RIGHT_ON);
            }
            if (behavior->metaState & (KeyEvent::META_CTRL_LEFT_ON | KeyEvent::META_CTRL_RIGHT_ON)) {
                newMetaState &= ~KeyEvent::META_CTRL_ON;
            }
            if (behavior->metaState & KeyEvent::META_SHIFT_ON) {
                newMetaState &= ~(KeyEvent::META_SHIFT_LEFT_ON | KeyEvent::META_SHIFT_RIGHT_ON);
            }
            if (behavior->metaState & (KeyEvent::META_SHIFT_LEFT_ON | KeyEvent::META_SHIFT_RIGHT_ON)) {
                newMetaState &= ~KeyEvent::META_SHIFT_ON;
            }
            // ... and put universal bits back if needed
            *outMetaState = KeyEvent::normalizeMetaState(newMetaState);
        }
    }

#if DEBUG_MAPPING
    LOGD("tryRemapKey: keyCode=%d, metaState=0x%08x ~ "
            "replacement keyCode=%d, replacement metaState=0x%08x.",
            keyCode, metaState, *outKeyCode, *outMetaState);
#endif
}

bool KeyCharacterMap::getKey(int32_t keyCode, const Key** outKey) const {
    auto itr = mKeys.find(keyCode);
    if (itr!=mKeys.end()) {
        *outKey = itr->second;//mKeys.valueAt(index);
        return true;
    }
    return false;
}

bool KeyCharacterMap::getKeyBehavior(int32_t keyCode, int32_t metaState,
        const Key** outKey, const Behavior** outBehavior) const {
    const Key* key;
    if (getKey(keyCode, &key)) {
        const Behavior* behavior = key->firstBehavior;
        while (behavior) {
            LOGV("metaState=%x behavior=%p behavior.metastate=%x macted=%d",metaState,behavior,behavior->metaState,matchesMetaState(metaState, behavior->metaState));
            if (matchesMetaState(metaState, behavior->metaState)) {///zhhou
                *outKey = key;
                *outBehavior = behavior;
                return true;
            }
            behavior = behavior->next;
        }
    }
    return false;
}

bool KeyCharacterMap::matchesMetaState(int32_t eventMetaState, int32_t behaviorMetaState) {
    // Behavior must have at least the set of meta states specified.
    // And if the key event has CTRL, ALT or META then the behavior must exactly
    // match those, taking into account that a behavior can specify that it handles
    // one, both or either of a left/right modifier pair.
    if ((eventMetaState & behaviorMetaState) == behaviorMetaState) {
        const int32_t EXACT_META_STATES =
                AMETA_CTRL_ON | AMETA_CTRL_LEFT_ON | AMETA_CTRL_RIGHT_ON
                | AMETA_ALT_ON | AMETA_ALT_LEFT_ON | AMETA_ALT_RIGHT_ON
                | AMETA_META_ON | AMETA_META_LEFT_ON | AMETA_META_RIGHT_ON;
        int32_t unmatchedMetaState = eventMetaState & ~behaviorMetaState & EXACT_META_STATES;
        if (behaviorMetaState & AMETA_CTRL_ON) {
            unmatchedMetaState &= ~(AMETA_CTRL_LEFT_ON | AMETA_CTRL_RIGHT_ON);
        } else if (behaviorMetaState & (AMETA_CTRL_LEFT_ON | AMETA_CTRL_RIGHT_ON)) {
            unmatchedMetaState &= ~AMETA_CTRL_ON;
        }
        if (behaviorMetaState & AMETA_ALT_ON) {
            unmatchedMetaState &= ~(AMETA_ALT_LEFT_ON | AMETA_ALT_RIGHT_ON);
        } else if (behaviorMetaState & (AMETA_ALT_LEFT_ON | AMETA_ALT_RIGHT_ON)) {
            unmatchedMetaState &= ~AMETA_ALT_ON;
        }
        if (behaviorMetaState & AMETA_META_ON) {
            unmatchedMetaState &= ~(AMETA_META_LEFT_ON | AMETA_META_RIGHT_ON);
        } else if (behaviorMetaState & (AMETA_META_LEFT_ON | AMETA_META_RIGHT_ON)) {
            unmatchedMetaState &= ~AMETA_META_ON;
        }
        return !unmatchedMetaState;
    }
    return false;
}

bool KeyCharacterMap::findKey(char16_t ch, int32_t* outKeyCode, int32_t* outMetaState) const {
    if (!ch) {
        return false;
    }

    for (auto k:mKeys){//size_t i = 0; i < mKeys.size(); i++) {
        const Key* key = k.second;//mKeys.valueAt(i);

        // Try to find the most general behavior that maps to this character.
        // For example, the base key behavior will usually be last in the list.
        const Behavior* found = NULL;
        for (const Behavior* behavior = key->firstBehavior; behavior; behavior = behavior->next) {
            if (behavior->character == ch) {
                found = behavior;
            }
        }
        if (found) {
            *outKeyCode = k.first;//mKeys.keyAt(i);
            *outMetaState = found->metaState;
            return true;
        }
    }
    return false;
}

void KeyCharacterMap::addKey(std::vector<KeyEvent>& outEvents,
        int32_t deviceId, int32_t keyCode, int32_t metaState, bool down, nsecs_t time) {
    KeyEvent event;
    event.initialize(deviceId,InputDevice::SOURCE_KEYBOARD,0,
            down ? KeyEvent::ACTION_DOWN : KeyEvent::ACTION_UP,
            0, keyCode, 0, metaState, 0, time, time);
    outEvents.push_back(event);
}

void KeyCharacterMap::addMetaKeys(std::vector<KeyEvent>& outEvents,
        int32_t deviceId, int32_t metaState, bool down, nsecs_t time,
        int32_t* currentMetaState) {
    // Add and remove meta keys symmetrically.
#if 0
    if (down) {
        addLockedMetaKey(outEvents, deviceId, metaState, time,
                AKEYCODE_CAPS_LOCK, AMETA_CAPS_LOCK_ON, currentMetaState);
        addLockedMetaKey(outEvents, deviceId, metaState, time,
                AKEYCODE_NUM_LOCK, AMETA_NUM_LOCK_ON, currentMetaState);
        addLockedMetaKey(outEvents, deviceId, metaState, time,
                AKEYCODE_SCROLL_LOCK, AMETA_SCROLL_LOCK_ON, currentMetaState);

        addDoubleEphemeralMetaKey(outEvents, deviceId, metaState, true, time,
                AKEYCODE_SHIFT_LEFT, AMETA_SHIFT_LEFT_ON,
                AKEYCODE_SHIFT_RIGHT, AMETA_SHIFT_RIGHT_ON,
                AMETA_SHIFT_ON, currentMetaState);
        addDoubleEphemeralMetaKey(outEvents, deviceId, metaState, true, time,
                AKEYCODE_ALT_LEFT, AMETA_ALT_LEFT_ON,
                AKEYCODE_ALT_RIGHT, AMETA_ALT_RIGHT_ON,
                AMETA_ALT_ON, currentMetaState);
        addDoubleEphemeralMetaKey(outEvents, deviceId, metaState, true, time,
                AKEYCODE_CTRL_LEFT, AMETA_CTRL_LEFT_ON,
                AKEYCODE_CTRL_RIGHT, AMETA_CTRL_RIGHT_ON,
                AMETA_CTRL_ON, currentMetaState);
        addDoubleEphemeralMetaKey(outEvents, deviceId, metaState, true, time,
                AKEYCODE_META_LEFT, AMETA_META_LEFT_ON,
                AKEYCODE_META_RIGHT, AMETA_META_RIGHT_ON,
                AMETA_META_ON, currentMetaState);

        addSingleEphemeralMetaKey(outEvents, deviceId, metaState, true, time,
                AKEYCODE_SYM, AMETA_SYM_ON, currentMetaState);
        addSingleEphemeralMetaKey(outEvents, deviceId, metaState, true, time,
                AKEYCODE_FUNCTION, AMETA_FUNCTION_ON, currentMetaState);
    } else {
        addSingleEphemeralMetaKey(outEvents, deviceId, metaState, false, time,
                AKEYCODE_FUNCTION, AMETA_FUNCTION_ON, currentMetaState);
        addSingleEphemeralMetaKey(outEvents, deviceId, metaState, false, time,
                AKEYCODE_SYM, AMETA_SYM_ON, currentMetaState);

        addDoubleEphemeralMetaKey(outEvents, deviceId, metaState, false, time,
                AKEYCODE_META_LEFT, AMETA_META_LEFT_ON,
                AKEYCODE_META_RIGHT, AMETA_META_RIGHT_ON,
                AMETA_META_ON, currentMetaState);
        addDoubleEphemeralMetaKey(outEvents, deviceId, metaState, false, time,
                AKEYCODE_CTRL_LEFT, AMETA_CTRL_LEFT_ON,
                AKEYCODE_CTRL_RIGHT, AMETA_CTRL_RIGHT_ON,
                AMETA_CTRL_ON, currentMetaState);
        addDoubleEphemeralMetaKey(outEvents, deviceId, metaState, false, time,
                AKEYCODE_ALT_LEFT, AMETA_ALT_LEFT_ON,
                AKEYCODE_ALT_RIGHT, AMETA_ALT_RIGHT_ON,
                AMETA_ALT_ON, currentMetaState);
        addDoubleEphemeralMetaKey(outEvents, deviceId, metaState, false, time,
                AKEYCODE_SHIFT_LEFT, AMETA_SHIFT_LEFT_ON,
                AKEYCODE_SHIFT_RIGHT, AMETA_SHIFT_RIGHT_ON,
                AMETA_SHIFT_ON, currentMetaState);

        addLockedMetaKey(outEvents, deviceId, metaState, time,
                AKEYCODE_SCROLL_LOCK, AMETA_SCROLL_LOCK_ON, currentMetaState);
        addLockedMetaKey(outEvents, deviceId, metaState, time,
                AKEYCODE_NUM_LOCK, AMETA_NUM_LOCK_ON, currentMetaState);
        addLockedMetaKey(outEvents, deviceId, metaState, time,
                AKEYCODE_CAPS_LOCK, AMETA_CAPS_LOCK_ON, currentMetaState);
    }
#endif
}
static uint32_t updateMetaState(int32_t keycode,bool down,int state){
  return 0;//add by zhhou for dummy
}
bool KeyCharacterMap::addSingleEphemeralMetaKey(std::vector<KeyEvent>& outEvents,
        int32_t deviceId, int32_t metaState, bool down, nsecs_t time,
        int32_t keyCode, int32_t keyMetaState,
        int32_t* currentMetaState) {
    if ((metaState & keyMetaState) == keyMetaState) {
        *currentMetaState = updateMetaState(keyCode, down, *currentMetaState);
        addKey(outEvents, deviceId, keyCode, *currentMetaState, down, time);
        return true;
    }
    return false;
}

void KeyCharacterMap::addDoubleEphemeralMetaKey(std::vector<KeyEvent>& outEvents,
        int32_t deviceId, int32_t metaState, bool down, nsecs_t time,
        int32_t leftKeyCode, int32_t leftKeyMetaState,
        int32_t rightKeyCode, int32_t rightKeyMetaState,
        int32_t eitherKeyMetaState,
        int32_t* currentMetaState) {
    bool specific = false;
    specific |= addSingleEphemeralMetaKey(outEvents, deviceId, metaState, down, time,
            leftKeyCode, leftKeyMetaState, currentMetaState);
    specific |= addSingleEphemeralMetaKey(outEvents, deviceId, metaState, down, time,
            rightKeyCode, rightKeyMetaState, currentMetaState);

    if (!specific) {
        addSingleEphemeralMetaKey(outEvents, deviceId, metaState, down, time,
                leftKeyCode, eitherKeyMetaState, currentMetaState);
    }
}

void KeyCharacterMap::addLockedMetaKey(std::vector<KeyEvent>& outEvents,
        int32_t deviceId, int32_t metaState, nsecs_t time,
        int32_t keyCode, int32_t keyMetaState,
        int32_t* currentMetaState) {
    if ((metaState & keyMetaState) == keyMetaState) {
        *currentMetaState = updateMetaState(keyCode, true, *currentMetaState);
        addKey(outEvents, deviceId, keyCode, *currentMetaState, true, time);
        *currentMetaState = updateMetaState(keyCode, false, *currentMetaState);
        addKey(outEvents, deviceId, keyCode, *currentMetaState, false, time);
    }
}


// --- KeyCharacterMap::Key ---

KeyCharacterMap::Key::Key() :
        label(0), number(0), firstBehavior(NULL) {
}

KeyCharacterMap::Key::Key(const Key& other) :
        label(other.label), number(other.number),
        firstBehavior(other.firstBehavior ? new Behavior(*other.firstBehavior) : NULL) {
}

KeyCharacterMap::Key::~Key() {
    Behavior* behavior = firstBehavior;
    while (behavior) {
        Behavior* next = behavior->next;
        delete behavior;
        behavior = next;
    }
}


// --- KeyCharacterMap::Behavior ---

KeyCharacterMap::Behavior::Behavior() :
        next(NULL), metaState(0), character(0), fallbackKeyCode(0), replacementKeyCode(0) {
}

KeyCharacterMap::Behavior::Behavior(const Behavior& other) :
        next(other.next ? new Behavior(*other.next) : NULL),
        metaState(other.metaState), character(other.character),
        fallbackKeyCode(other.fallbackKeyCode),
        replacementKeyCode(other.replacementKeyCode) {
}


// --- KeyCharacterMap::Parser ---

KeyCharacterMap::Parser::Parser(KeyCharacterMap* map, Tokenizer* tokenizer, Format format) :
        mMap(map), mTokenizer(tokenizer), mFormat(format), mState(STATE_TOP) {
}

KeyCharacterMap::Parser::~Parser() {
}

int KeyCharacterMap::Parser::parse() {
    while (!mTokenizer->isEof()) {
#if DEBUG_PARSER
        LOGD("Parsing %s: '%s'.", mTokenizer->getLocation().c_str(),
                mTokenizer->peekRemainderOfLine().c_str());
#endif

        mTokenizer->skipDelimiters(WHITESPACE);

        if (!mTokenizer->isEol() && mTokenizer->peekChar() != '#') {
            switch (mState) {
            case STATE_TOP: {
                std::string keywordToken = mTokenizer->nextToken(WHITESPACE);
                if (keywordToken == "type") {
                    mTokenizer->skipDelimiters(WHITESPACE);
                    int status = parseType();
                    if (status) return status;
                } else if (keywordToken == "map") {
                    mTokenizer->skipDelimiters(WHITESPACE);
                    int status = parseMap();
                    if (status) return status;
                } else if (keywordToken == "key") {
                    mTokenizer->skipDelimiters(WHITESPACE);
                    int status = parseKey();
                    //if (status) return status;
                } else {
                    LOGE("%s: Expected keyword, got '%s'.", mTokenizer->getLocation().c_str(),
                            keywordToken.c_str());
                    return BAD_VALUE;
                }
                break;
            }

            case STATE_KEY: {
                int status = parseKeyProperty();
                //if (status) return status;
                break;
            }
            }

            mTokenizer->skipDelimiters(WHITESPACE);
            if (!mTokenizer->isEol() && mTokenizer->peekChar() != '#') {
                LOGE("%s: Expected end of line or trailing comment, got '%s'.",
                        mTokenizer->getLocation().c_str(),
                        mTokenizer->peekRemainderOfLine().c_str());
                return BAD_VALUE;
            }
        }

        mTokenizer->nextLine();
    }

    if (mState != STATE_TOP) {
        LOGE("%s: Unterminated key description at end of file.",
                mTokenizer->getLocation().c_str());
        return BAD_VALUE;
    }

    if (mMap->mType == KEYBOARD_TYPE_UNKNOWN) {
        LOGE("%s: Keyboard layout missing required keyboard 'type' declaration.",
                mTokenizer->getLocation().c_str());
        return BAD_VALUE;
    }

    if (mFormat == FORMAT_BASE) {
        if (mMap->mType == KEYBOARD_TYPE_OVERLAY) {
            LOGE("%s: Base keyboard layout must specify a keyboard 'type' other than 'OVERLAY'.",
                    mTokenizer->getLocation().c_str());
            return BAD_VALUE;
        }
    } else if (mFormat == FORMAT_OVERLAY) {
        if (mMap->mType != KEYBOARD_TYPE_OVERLAY) {
            LOGE("%s: Overlay keyboard layout missing required keyboard "
                    "'type OVERLAY' declaration.",
                    mTokenizer->getLocation().c_str());
            return BAD_VALUE;
        }
    }

    return NO_ERROR;
}

int KeyCharacterMap::Parser::parseType() {
    if (mMap->mType != KEYBOARD_TYPE_UNKNOWN) {
        LOGE("%s: Duplicate keyboard 'type' declaration.",
                mTokenizer->getLocation().c_str());
        return BAD_VALUE;
    }

    KeyboardType type;
    std::string typeToken = mTokenizer->nextToken(WHITESPACE);
    if (typeToken == "NUMERIC") {
        type = KEYBOARD_TYPE_NUMERIC;
    } else if (typeToken == "PREDICTIVE") {
        type = KEYBOARD_TYPE_PREDICTIVE;
    } else if (typeToken == "ALPHA") {
        type = KEYBOARD_TYPE_ALPHA;
    } else if (typeToken == "FULL") {
        type = KEYBOARD_TYPE_FULL;
    } else if (typeToken == "SPECIAL_FUNCTION") {
        LOGW("The SPECIAL_FUNCTION type is now declared in the device's IDC file, please set "
                "the property 'keyboard.specialFunction' to '1' there instead.");
        // TODO: return BAD_VALUE here in Q
        type = KEYBOARD_TYPE_SPECIAL_FUNCTION;
    } else if (typeToken == "OVERLAY") {
        type = KEYBOARD_TYPE_OVERLAY;
    } else {
        LOGE("%s: Expected keyboard type label, got '%s'.", mTokenizer->getLocation().c_str(),
                typeToken.c_str());
        return BAD_VALUE;
    }

#if DEBUG_PARSER
    LOGD("Parsed type: type=%d.", type);
#endif
    mMap->mType = type;
    return NO_ERROR;
}

int KeyCharacterMap::Parser::parseMap() {
    std::string keywordToken = mTokenizer->nextToken(WHITESPACE);
    if (keywordToken == "key") {
        mTokenizer->skipDelimiters(WHITESPACE);
        return parseMapKey();
    }
    LOGE("%s: Expected keyword after 'map', got '%s'.", mTokenizer->getLocation().c_str(),
            keywordToken.c_str());
    return BAD_VALUE;
}

int KeyCharacterMap::Parser::parseMapKey() {
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
        return BAD_VALUE;
    }
    std::map<int32_t, int32_t>& map =mapUsage ? mMap->mKeysByUsageCode : mMap->mKeysByScanCode;
    if (map.find(code) !=map.end()) {
        LOGE("%s: Duplicate entry for key %s '%s'.", mTokenizer->getLocation().c_str(),
                mapUsage ? "usage" : "scan code", codeToken.c_str());
        return BAD_VALUE;
    }

    mTokenizer->skipDelimiters(WHITESPACE);
    std::string keyCodeToken = mTokenizer->nextToken(WHITESPACE);
    int32_t keyCode = InputEventLookup::getKeyCodeByLabel(keyCodeToken.c_str());
    if (!keyCode) {
        LOGE("%s: Expected key code label, got '%s'.", mTokenizer->getLocation().c_str(),
                keyCodeToken.c_str());
        return BAD_VALUE;
    }

#if DEBUG_PARSER
    LOGD("Parsed map key %s: code=%d, keyCode=%d.",
            mapUsage ? "usage" : "scan code", code, keyCode);
#endif
    map[code]= keyCode;
    return NO_ERROR;
}

int KeyCharacterMap::Parser::parseKey() {
    std::string keyCodeToken = mTokenizer->nextToken(WHITESPACE);
    int32_t keyCode = InputEventLookup::getKeyCodeByLabel(keyCodeToken.c_str());
    if (!keyCode) {
        LOGE("%s: Expected key code label, got '%s'.", mTokenizer->getLocation().c_str(),
                keyCodeToken.c_str());
        return BAD_VALUE;
    }
    if (mMap->mKeys.find(keyCode) !=mMap->mKeys.end()) {
        LOGE("%s: Duplicate entry for key code '%s'.", mTokenizer->getLocation().c_str(),
                keyCodeToken.c_str());
        return BAD_VALUE;
    }

    mTokenizer->skipDelimiters(WHITESPACE);
    std::string openBraceToken = mTokenizer->nextToken(WHITESPACE);
    if (openBraceToken != "{") {
        LOGE("%s: Expected '{' after key code label, got '%s'.",
                mTokenizer->getLocation().c_str(), openBraceToken.c_str());
        return BAD_VALUE;
    }

#if DEBUG_PARSER
    LOGD("Parsed beginning of key: keyCode=%d.", keyCode);
#endif
    mKeyCode = keyCode;
    mMap->mKeys[keyCode]= new Key();
    mState = STATE_KEY;
    return NO_ERROR;
}

int KeyCharacterMap::Parser::parseKeyProperty() {
    Key* key = mMap->mKeys[mKeyCode];
    std::string token = mTokenizer->nextToken(WHITESPACE_OR_PROPERTY_DELIMITER);
    if (token == "}") {
        mState = STATE_TOP;
        return finishKey(key);
    }

    std::vector<Property> properties;

    // Parse all comma-delimited property names up to the first colon.
    for (;;) {
        if (token == "label") {
            properties.push_back(Property(PROPERTY_LABEL));
        } else if (token == "number") {
            properties.push_back(Property(PROPERTY_NUMBER));
        } else {
            int32_t metaState;
            int status = parseModifier(token, &metaState);
            if (status) {
                LOGE("%s: Expected a property name or modifier, got '%s'.",
                        mTokenizer->getLocation().c_str(), token.c_str());
                return status;
            }
            properties.push_back(Property(PROPERTY_META, metaState));
        }

        mTokenizer->skipDelimiters(WHITESPACE);
        if (!mTokenizer->isEol()) {
            char ch = mTokenizer->nextChar();
            if (ch == ':') {
                break;
            } else if (ch == ',') {
                mTokenizer->skipDelimiters(WHITESPACE);
                token = mTokenizer->nextToken(WHITESPACE_OR_PROPERTY_DELIMITER);
                continue;
            }
        }

        LOGE("%s: Expected ',' or ':' after property name.",
                mTokenizer->getLocation().c_str());
        return BAD_VALUE;
    }

    // Parse behavior after the colon.
    mTokenizer->skipDelimiters(WHITESPACE);

    Behavior behavior;
    bool haveCharacter = false;
    bool haveFallback = false;
    bool haveReplacement = false;

    do {
        char ch = mTokenizer->peekChar();
        if (ch == '\'') {
            char16_t character;
            int status = parseCharacterLiteral(&character);
            if (status || !character) {
                LOGE("%s: Invalid character literal for key.",
                        mTokenizer->getLocation().c_str());
                return BAD_VALUE;
            }
            if (haveCharacter) {
                LOGE("%s: Cannot combine multiple character literals or 'none'.",
                        mTokenizer->getLocation().c_str());
                return BAD_VALUE;
            }
            if (haveReplacement) {
                LOGE("%s: Cannot combine character literal with replace action.",
                        mTokenizer->getLocation().c_str());
                return BAD_VALUE;
            }
            behavior.character = character;
            haveCharacter = true;
        } else {
            token = mTokenizer->nextToken(WHITESPACE);
            if (token == "none") {
                if (haveCharacter) {
                    LOGE("%s: Cannot combine multiple character literals or 'none'.",
                            mTokenizer->getLocation().c_str());
                    return BAD_VALUE;
                }
                if (haveReplacement) {
                    LOGE("%s: Cannot combine 'none' with replace action.",
                            mTokenizer->getLocation().c_str());
                    return BAD_VALUE;
                }
                haveCharacter = true;
            } else if (token == "fallback") {
                mTokenizer->skipDelimiters(WHITESPACE);
                token = mTokenizer->nextToken(WHITESPACE);
                int32_t keyCode = InputEventLookup::getKeyCodeByLabel(token.c_str());
                if (!keyCode) {
                    LOGE("%s: Invalid key code label for fallback behavior, got '%s'.",
                            mTokenizer->getLocation().c_str(), token.c_str());
                    return 0;//BAD_VALUE;
                }
                if (haveFallback || haveReplacement) {
                    LOGE("%s: Cannot combine multiple fallback/replacement key codes.",
                            mTokenizer->getLocation().c_str());
                    return BAD_VALUE;
                }
                behavior.fallbackKeyCode = keyCode;
                haveFallback = true;
            } else if (token == "replace") {
                mTokenizer->skipDelimiters(WHITESPACE);
                token = mTokenizer->nextToken(WHITESPACE);
                int32_t keyCode = InputEventLookup::getKeyCodeByLabel(token.c_str());
                if (!keyCode) {
                    LOGE("%s: Invalid key code label for replace, got '%s'.",
                            mTokenizer->getLocation().c_str(), token.c_str());
                    return BAD_VALUE;
                }
                if (haveCharacter) {
                    LOGE("%s: Cannot combine character literal with replace action.",
                            mTokenizer->getLocation().c_str());
                    return BAD_VALUE;
                }
                if (haveFallback || haveReplacement) {
                    LOGE("%s: Cannot combine multiple fallback/replacement key codes.",
                            mTokenizer->getLocation().c_str());
                    return BAD_VALUE;
                }
                behavior.replacementKeyCode = keyCode;
                haveReplacement = true;

            } else {
                LOGE("%s: Expected a key behavior after ':'.",
                        mTokenizer->getLocation().c_str());
                return BAD_VALUE;
            }
        }

        mTokenizer->skipDelimiters(WHITESPACE);
    } while (!mTokenizer->isEol() && mTokenizer->peekChar() != '#');

    // Add the behavior.
    for (size_t i = 0; i < properties.size(); i++) {
        const Property& property = properties.at(i);
        switch (property.property) {
        case PROPERTY_LABEL:
            if (key->label) {
                LOGE("%s: Duplicate label for key.",
                        mTokenizer->getLocation().c_str());
                return BAD_VALUE;
            }
            key->label = behavior.character;
#if DEBUG_PARSER
            LOGD("Parsed key label: keyCode=%d, label=%d.", mKeyCode, key->label);
#endif
            break;
        case PROPERTY_NUMBER:
            if (key->number) {
                LOGE("%s: Duplicate number for key.",
                        mTokenizer->getLocation().c_str());
                return BAD_VALUE;
            }
            key->number = behavior.character;
#if DEBUG_PARSER
            LOGD("Parsed key number: keyCode=%d, number=%d.", mKeyCode, key->number);
#endif
            break;
        case PROPERTY_META: {
            for (Behavior* b = key->firstBehavior; b; b = b->next) {
                if (b->metaState == property.metaState) {
                    LOGE("%s: Duplicate key behavior for modifier.",
                            mTokenizer->getLocation().c_str());
                    return BAD_VALUE;
                }
            }
            Behavior* newBehavior = new Behavior(behavior);
            newBehavior->metaState = property.metaState;
            newBehavior->next = key->firstBehavior;
            key->firstBehavior = newBehavior;
#if DEBUG_PARSER
            LOGD("Parsed key meta: keyCode=%d, meta=0x%x, char=%d, fallback=%d replace=%d.",
                    mKeyCode,
                    newBehavior->metaState, newBehavior->character,
                    newBehavior->fallbackKeyCode, newBehavior->replacementKeyCode);
#endif
            break;
        }
        }
    }
    return NO_ERROR;
}

int KeyCharacterMap::Parser::finishKey(Key* key) {
    // Fill in default number property.
    if (!key->number) {
        char16_t digit = 0;
        char16_t symbol = 0;
        for (Behavior* b = key->firstBehavior; b; b = b->next) {
            char16_t ch = b->character;
            if (ch) {
                if (ch >= '0' && ch <= '9') {
                    digit = ch;
                } else if (ch == '(' || ch == ')' || ch == '#' || ch == '*'
                        || ch == '-' || ch == '+' || ch == ',' || ch == '.'
                        || ch == '\'' || ch == ':' || ch == ';' || ch == '/') {
                    symbol = ch;
                }
            }
        }
        key->number = digit ? digit : symbol;
    }
    return NO_ERROR;
}

int KeyCharacterMap::Parser::parseModifier(const std::string& token, int32_t* outMetaState) {
    if (token == "base") {
        *outMetaState = 0;
        return NO_ERROR;
    }

    int32_t combinedMeta = 0;

    const char* str = token.c_str();
    const char* start = str;
    for (const char* cur = str; ; cur++) {
        char ch = *cur;
        if (ch == '+' || ch == '\0') {
            size_t len = cur - start;
            int32_t metaState = 0;
            for (size_t i = 0; i < sizeof(modifiers) / sizeof(Modifier); i++) {
                if (strlen(modifiers[i].label) == len
                        && strncmp(modifiers[i].label, start, len) == 0) {
                    metaState = modifiers[i].metaState;
                    break;
                }
            }
            if (!metaState) {
                return BAD_VALUE;
            }
            if (combinedMeta & metaState) {
                LOGE("%s: Duplicate modifier combination '%s'.",
                        mTokenizer->getLocation().c_str(), token.c_str());
                return BAD_VALUE;
            }

            combinedMeta |= metaState;
            start = cur + 1;

            if (ch == '\0') {
                break;
            }
        }
    }
    *outMetaState = combinedMeta;
    return NO_ERROR;
}

int KeyCharacterMap::Parser::parseCharacterLiteral(char16_t* outCharacter) {
    char ch = mTokenizer->nextChar();
    if (ch != '\'') {
        goto Error;
    }

    ch = mTokenizer->nextChar();
    if (ch == '\\') {
        // Escape sequence.
        ch = mTokenizer->nextChar();
        if (ch == 'n') {
            *outCharacter = '\n';
        } else if (ch == 't') {
            *outCharacter = '\t';
        } else if (ch == '\\') {
            *outCharacter = '\\';
        } else if (ch == '\'') {
            *outCharacter = '\'';
        } else if (ch == '"') {
            *outCharacter = '"';
        } else if (ch == 'u') {
            *outCharacter = 0;
            for (int i = 0; i < 4; i++) {
                ch = mTokenizer->nextChar();
                int digit;
                if (ch >= '0' && ch <= '9') {
                    digit = ch - '0';
                } else if (ch >= 'A' && ch <= 'F') {
                    digit = ch - 'A' + 10;
                } else if (ch >= 'a' && ch <= 'f') {
                    digit = ch - 'a' + 10;
                } else {
                    goto Error;
                }
                *outCharacter = (*outCharacter << 4) | digit;
            }
        } else {
            goto Error;
        }
    } else if (ch >= 32 && ch <= 126 && ch != '\'') {
        // ASCII literal character.
        *outCharacter = ch;
    } else {
        goto Error;
    }

    ch = mTokenizer->nextChar();
    if (ch != '\'') {
        goto Error;
    }

    // Ensure that we consumed the entire token.
    if (mTokenizer->nextToken(WHITESPACE).empty()) {
        return NO_ERROR;
    }

Error:
    LOGE("%s: Malformed character literal.", mTokenizer->getLocation().c_str());
    return BAD_VALUE;
}

} // namespace cdroid
