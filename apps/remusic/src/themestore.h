// ThemeHelper's palette, lean port: the 8 MagicaSakura themes collapse to one
// accent color persisted in prefs and read by the chrome views.
#ifndef __REMUSIC_THEMESTORE_H__
#define __REMUSIC_THEMESTORE_H__

#include <cstdint>
#include <string>
#include <vector>

#include <core/context.h>

namespace remusic {

class ThemeStore {
public:
    static ThemeStore& get();
    void init(cdroid::Context* context) { mContext = context; }

    // The 8 originals: sakura/storm/wood/light/thunder/sand/firey/hope.
    static const std::vector<std::pair<const char*, uint32_t>>& palette();

    uint32_t accent() const;
    void setAccent(uint32_t color);

    /** The original's day/night toggle (Settings). Day is the default and
     *  matches the reference screenshots; night is the port's long-standing
     *  dark look. */
    bool night() const;
    void setNight(bool night);

private:
    ThemeStore() = default;
    cdroid::Context* mContext = nullptr;
};

} // namespace remusic
#endif
