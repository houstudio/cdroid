#include <cstdlib>
#include <core/displaymetrics.h>
#include <cdgraph.h>

namespace cdroid{

int DisplayMetrics::DENSITY_DEVICE = getDeviceDensity();//DENSITY_DEFAULT;
int DisplayMetrics::DENSITY_DEVICEE_STABLE = getDeviceDensity();//DENSITY_DEFAULT;

DisplayMetrics::DisplayMetrics(){
}

void DisplayMetrics::setTo(const DisplayMetrics& o) {
    if (this == &o)return;

    widthPixels = o.widthPixels;
    heightPixels= o.heightPixels;
    density = o.density;
    densityDpi = o.densityDpi;
    scaledDensity = o.scaledDensity;
    xdpi = o.xdpi;
    ydpi = o.ydpi;
    noncompatWidthPixels = o.noncompatWidthPixels;
    noncompatHeightPixels= o.noncompatHeightPixels;
    noncompatDensity   = o.noncompatDensity;
    noncompatDensityDpi= o.noncompatDensityDpi;
    noncompatScaledDensity = o.noncompatScaledDensity;
    noncompatXdpi = o.noncompatXdpi;
    noncompatYdpi = o.noncompatYdpi;
}

void DisplayMetrics::setToDefaults() {
    GFXGetDisplaySize(0,(uint32_t*)&widthPixels,(uint32_t*)&heightPixels);
    density =  DENSITY_DEVICE / (float) DENSITY_DEFAULT;
    densityDpi =  DENSITY_DEVICE;
    scaledDensity = density;
    xdpi = DENSITY_DEVICE;
    ydpi = DENSITY_DEVICE;
    noncompatWidthPixels = widthPixels;
    noncompatHeightPixels= heightPixels;
    noncompatDensity = density;
    noncompatDensityDpi = densityDpi;
    noncompatScaledDensity = scaledDensity;
    noncompatXdpi = xdpi;
    noncompatYdpi = ydpi;
}

bool DisplayMetrics::equals(const DisplayMetrics& other)const{
    return equalsPhysical(other)
                && scaledDensity == other.scaledDensity
                && noncompatScaledDensity == other.noncompatScaledDensity;
}

bool DisplayMetrics::equalsPhysical(const DisplayMetrics& other)const{
    return widthPixels == other.widthPixels  && heightPixels == other.heightPixels
                && density == other.density  && densityDpi == other.densityDpi
                && xdpi == other.xdpi  && ydpi == other.ydpi
                && noncompatWidthPixels == other.noncompatWidthPixels
                && noncompatHeightPixels == other.noncompatHeightPixels
                && noncompatDensity == other.noncompatDensity
                && noncompatDensityDpi == other.noncompatDensityDpi
                && noncompatXdpi == other.noncompatXdpi
                && noncompatYdpi == other.noncompatYdpi;
}

int DisplayMetrics::getDeviceDensity() {
    // qemu.sf.lcd_density can be used to override ro.sf.lcd_density
    // when running in the emulator, allowing for dynamic configurations.
    // The reason for this is that ro.sf.lcd_density is write-once and is
    // set by the init process when it parses build.prop before anything else.
    const char* sdensity=getenv("LCD_DENSITY");
    return sdensity ? std::atoi(sdensity) : DENSITY_DEFAULT;
}

}
