#ifndef __DISPLAY_METRICS_H__
#define __DISPLAY_METRICS_H__
namespace cdroid{

class DisplayMetrics{
public:
    static constexpr int DENSITY_LOW    = 120;
    static constexpr int DENSITY_MEDIUM = 160;
    static constexpr int DENSITY_TV     = 213;
    static constexpr int DENSITY_HIGH   = 240;
    static constexpr int DENSITY_XXHIGH = 480;
    static constexpr int DENSITY_XXXHIGH= 640;
    static constexpr int DENSITY_DEFAULT= DENSITY_MEDIUM;
    static constexpr float DENSITY_DEFAULT_SCALE = 1.0f / DENSITY_DEFAULT;
    static int DENSITY_DEVICE;
    static int DENSITY_DEVICEE_STABLE;
public:
    int widthPixels;
    int heightPixels;
    float density;
    int densityDpi;
    float scaledDensity;
    float xdpi;
    float ydpi;
    int noncompatWidthPixels;
    int noncompatHeightPixels;
    float noncompatDensity;
    int noncompatDensityDpi;
    float noncompatScaledDensity;
    float noncompatXdpi;
    float noncompatYdpi;
public:
    DisplayMetrics();
    void setTo(const DisplayMetrics&);
    void setToDefaults();
};

}
#endif
