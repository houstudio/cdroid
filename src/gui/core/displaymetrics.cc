#include <core/displaymetrics.h>
#include <porting/cdgraph.h>

namespace cdroid{

int DisplayMetrics::DENSITY_DEVICE=DENSITY_DEFAULT;
int DisplayMetrics::DENSITY_DEVICEE_STABLE=DENSITY_DEFAULT;

DisplayMetrics::DisplayMetrics(){
    setToDefaults();
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
    widthPixels = 1280;
    heightPixels = 720;
    GFXGetDisplaySize(0,(UINT*)&widthPixels,(UINT*)&heightPixels);
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
}
