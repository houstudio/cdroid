#ifndef __BLUR_H__
#define __BLUR_H__
namespace cdroid{
class Blur {
public:
    // If radius > 0, return the corresponding sigma, else return 0
    static float convertRadiusToSigma(float radius);
    // If sigma > 0.5, return the corresponding radius, else return 0
    static float convertSigmaToRadius(float sigma);
    // If the original radius was on an integer boundary then after the sigma to
    // radius conversion a small rounding error may be introduced. This function
    // accounts for that error and snaps to the appropriate integer boundary.
    static uint32_t convertRadiusToInt(float radius);

    static void generateGaussianWeights(float* weights, float radius);
    static void horizontal(float* weights, int32_t radius, const uint8_t* source, uint8_t* dest,
                           int32_t width, int32_t height);
    static void vertical(float* weights, int32_t radius, const uint8_t* source, uint8_t* dest,
                         int32_t width, int32_t height);
};
}/*endof namespace*/
#endif
