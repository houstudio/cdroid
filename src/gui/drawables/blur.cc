#include <stdint.h>
#include <cmath>
#include <drawables/blur.h>
namespace cdroid{

#define NON_ZERO_EPSILON (0.001f)
// This constant approximates the scaling done in the software path's
// "high quality" mode, in SkBlurMask::Blur() (1 / sqrt(3)).
static constexpr float BLUR_SIGMA_SCALE = 0.57735f;

float Blur::convertRadiusToSigma(float radius) {
    return radius > 0 ? BLUR_SIGMA_SCALE * radius + 0.5f : 0.0f;
}

float Blur::convertSigmaToRadius(float sigma) {
    return sigma > 0.5f ? (sigma - 0.5f) / BLUR_SIGMA_SCALE : 0.0f;
}

inline static bool isZero(float value) {
    return (value >= -NON_ZERO_EPSILON) && (value <= NON_ZERO_EPSILON);
}
// if the original radius was on an integer boundary and the resulting radius
// is within the conversion error tolerance then we attempt to snap to the
// original integer boundary.
uint32_t Blur::convertRadiusToInt(float radius) {
    const float radiusCeil = std::ceil(radius);
    if (isZero(radiusCeil-radius)) {
        return radiusCeil;
    }
    return radius;
}

/**
 * HWUI has used a slightly different equation than Skia to generate the value
 * for sigma and to preserve compatibility we have kept that logic.
 *
 * Based on some experimental radius and sigma values we approximate the
 * equation sigma = f(radius) as sigma = radius * 0.3  + 0.6.  The larger the
 * radius gets, the more our gaussian blur will resemble a box blur since with
 * large sigma the gaussian curve begins to lose its shape.
 */
static float legacyConvertRadiusToSigma(float radius) {
    return radius > 0 ? 0.3f * radius + 0.6f : 0.0f;
}

void Blur::generateGaussianWeights(float* weights, float radius) {
    int32_t intRadius = convertRadiusToInt(radius);

    // Compute gaussian weights for the blur
    // e is the euler's number
    static float e = 2.718281828459045f;
    static float pi = 3.1415926535897932f;
    // g(x) = ( 1 / sqrt( 2 * pi ) * sigma) * e ^ ( -x^2 / 2 * sigma^2 )
    // x is of the form [-radius .. 0 .. radius]
    // and sigma varies with radius.
    float sigma = legacyConvertRadiusToSigma(radius);

    // Now compute the coefficints
    // We will store some redundant values to save some math during
    // the blur calculations
    // precompute some values
    float coeff1 = 1.0f / (sqrt(2.0f * pi) * sigma);
    float coeff2 = -1.0f / (2.0f * sigma * sigma);

    float normalizeFactor = 0.0f;
    for (int32_t r = -intRadius; r <= intRadius; r++) {
        float floatR = (float)r;
        weights[r + intRadius] = coeff1 * std::pow(e, floatR * floatR * coeff2);
        normalizeFactor += weights[r + intRadius];
    }

    // Now we need to normalize the weights because all our coefficients need to add up to one
    normalizeFactor = 1.0f / normalizeFactor;
    for (int32_t r = -intRadius; r <= intRadius; r++) {
        weights[r + intRadius] *= normalizeFactor;
    }
}

void Blur::horizontal(float* weights, int32_t radius, const uint8_t* source, uint8_t* dest,
                      int32_t width, int32_t height) {
    float blurredPixel = 0.0f;
    float currentPixel = 0.0f;

    for (int32_t y = 0; y < height; y++) {
        const uint8_t* input = source + y * width;
        uint8_t* output = dest + y * width;

        for (int32_t x = 0; x < width; x++) {
            blurredPixel = 0.0f;
            const float* gPtr = weights;
            // Optimization for non-border pixels
            if (x > radius && x < (width - radius)) {
                const uint8_t* i = input + (x - radius);
                for (int r = -radius; r <= radius; r++) {
                    currentPixel = (float)(*i);
                    blurredPixel += currentPixel * gPtr[0];
                    gPtr++;
                    i++;
                }
            } else {
                for (int32_t r = -radius; r <= radius; r++) {
                    // Stepping left and right away from the pixel
                    int validW = x + r;
                    if (validW < 0) {
                        validW = 0;
                    }
                    if (validW > width - 1) {
                        validW = width - 1;
                    }

                    currentPixel = (float)input[validW];
                    blurredPixel += currentPixel * gPtr[0];
                    gPtr++;
                }
            }
            *output = (uint8_t)blurredPixel;
            output++;
        }
    }
}

void Blur::vertical(float* weights, int32_t radius, const uint8_t* source, uint8_t* dest,
                    int32_t width, int32_t height) {
    float blurredPixel = 0.0f;
    float currentPixel = 0.0f;

    for (int32_t y = 0; y < height; y++) {
        uint8_t* output = dest + y * width;

        for (int32_t x = 0; x < width; x++) {
            blurredPixel = 0.0f;
            const float* gPtr = weights;
            const uint8_t* input = source + x;
            // Optimization for non-border pixels
            if (y > radius && y < (height - radius)) {
                const uint8_t* i = input + ((y - radius) * width);
                for (int32_t r = -radius; r <= radius; r++) {
                    currentPixel = (float)(*i);
                    blurredPixel += currentPixel * gPtr[0];
                    gPtr++;
                    i += width;
                }
            } else {
                for (int32_t r = -radius; r <= radius; r++) {
                    int validH = y + r;
                    // Clamp to zero and width
                    if (validH < 0) {
                        validH = 0;
                    }
                    if (validH > height - 1) {
                        validH = height - 1;
                    }

                    const uint8_t* i = input + validH * width;
                    currentPixel = (float)(*i);
                    blurredPixel += currentPixel * gPtr[0];
                    gPtr++;
                }
            }
            *output = (uint8_t)blurredPixel;
            output++;
        }
    }
}
}/*endof namespace*/
