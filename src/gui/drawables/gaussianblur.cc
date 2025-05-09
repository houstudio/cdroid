#include <cmath>
namespace cdroid{

static void CalGaussianCoeff(float sigma, float *a0, float *a1, float *a2, float *a3, float *b1, float *b2,
                      float *cprev, float *cnext) {
    float alpha, lamma, k;

    if (sigma < 0.5f)
        sigma = 0.5f;
    alpha = (float) std::exp((0.726) * (0.726)) / sigma;
    lamma = (float) std::exp(-alpha);
    *b2 = (float) std::exp(-2.f * alpha);
    k = (1 - lamma) * (1 - lamma) / (1 + 2 * alpha * lamma - (*b2));
    *a0 = k;
    *a1 = k * (alpha - 1) * lamma;
    *a2 = k * (alpha + 1) * lamma;
    *a3 = -k * (*b2);
    *b1 = -2 * lamma;
    *cprev = (*a0 + *a1) / (1 + *b1 + *b2);
    *cnext = (*a2 + *a3) / (1 + *b1 + *b2);
}

static void gaussianVertical(uint8_t *bufferPerLine, uint8_t *lpRowInitial,
                      uint8_t *lpColInitial, int height, int width, int Channels, float a0a1,
                      float a2a3, float b1b2, float cprev, float cnext) {

    int WidthStep = Channels * width;
    int HeightSubOne = height - 1;
    if (Channels == 3) {
        float prevOut[3];
        prevOut[0] = (lpRowInitial[0] * cprev);
        prevOut[1] = (lpRowInitial[1] * cprev);
        prevOut[2] = (lpRowInitial[2] * cprev);

        for (int y = 0; y < height; y++) {
            prevOut[0] = ((lpRowInitial[0] * a0a1) - (prevOut[0] * b1b2));
            prevOut[1] = ((lpRowInitial[1] * a0a1) - (prevOut[1] * b1b2));
            prevOut[2] = ((lpRowInitial[2] * a0a1) - (prevOut[2] * b1b2));
            bufferPerLine[0] = prevOut[0];
            bufferPerLine[1] = prevOut[1];
            bufferPerLine[2] = prevOut[2];
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        bufferPerLine -= Channels;
        lpColInitial += WidthStep * HeightSubOne;
        prevOut[0] = (lpRowInitial[0] * cnext);
        prevOut[1] = (lpRowInitial[1] * cnext);
        prevOut[2] = (lpRowInitial[2] * cnext);
        for (int y = HeightSubOne; y >= 0; y--) {
            prevOut[0] = ((lpRowInitial[0] * a2a3) - (prevOut[0] * b1b2));
            prevOut[1] = ((lpRowInitial[1] * a2a3) - (prevOut[1] * b1b2));
            prevOut[2] = ((lpRowInitial[2] * a2a3) - (prevOut[2] * b1b2));
            bufferPerLine[0] += prevOut[0];
            bufferPerLine[1] += prevOut[1];
            bufferPerLine[2] += prevOut[2];
            lpColInitial[0] = bufferPerLine[0];
            lpColInitial[1] = bufferPerLine[1];
            lpColInitial[2] = bufferPerLine[2];
            lpRowInitial -= Channels;
            lpColInitial -= WidthStep;
            bufferPerLine -= Channels;
        }
    } else if (Channels == 4) {
        float prevOut[4];

        prevOut[0] = (lpRowInitial[0] * cprev);
        prevOut[1] = (lpRowInitial[1] * cprev);
        prevOut[2] = (lpRowInitial[2] * cprev);
        prevOut[3] = (lpRowInitial[3] * cprev);

        for (int y = 0; y < height; y++) {
            prevOut[0] = ((lpRowInitial[0] * a0a1) - (prevOut[0] * b1b2));
            prevOut[1] = ((lpRowInitial[1] * a0a1) - (prevOut[1] * b1b2));
            prevOut[2] = ((lpRowInitial[2] * a0a1) - (prevOut[2] * b1b2));
            prevOut[3] = ((lpRowInitial[3] * a0a1) - (prevOut[3] * b1b2));
            bufferPerLine[0] = prevOut[0];
            bufferPerLine[1] = prevOut[1];
            bufferPerLine[2] = prevOut[2];
            bufferPerLine[3] = prevOut[3];
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        bufferPerLine -= Channels;
        lpColInitial += WidthStep * HeightSubOne;
        prevOut[0] = (lpRowInitial[0] * cnext);
        prevOut[1] = (lpRowInitial[1] * cnext);
        prevOut[2] = (lpRowInitial[2] * cnext);
        prevOut[3] = (lpRowInitial[3] * cnext);
        for (int y = HeightSubOne; y >= 0; y--) {
            prevOut[0] = ((lpRowInitial[0] * a2a3) - (prevOut[0] * b1b2));
            prevOut[1] = ((lpRowInitial[1] * a2a3) - (prevOut[1] * b1b2));
            prevOut[2] = ((lpRowInitial[2] * a2a3) - (prevOut[2] * b1b2));
            prevOut[3] = ((lpRowInitial[3] * a2a3) - (prevOut[3] * b1b2));
            bufferPerLine[0] += prevOut[0];
            bufferPerLine[1] += prevOut[1];
            bufferPerLine[2] += prevOut[2];
            bufferPerLine[3] += prevOut[3];
            lpColInitial[0] = bufferPerLine[0];
            lpColInitial[1] = bufferPerLine[1];
            lpColInitial[2] = bufferPerLine[2];
            lpColInitial[3] = bufferPerLine[3];
            lpRowInitial -= Channels;
            lpColInitial -= WidthStep;
            bufferPerLine -= Channels;
        }
    } else if (Channels == 1) {
        float prevOut = 0;
        prevOut = (lpRowInitial[0] * cprev);
        for (int y = 0; y < height; y++) {
            prevOut = ((lpRowInitial[0] * a0a1) - (prevOut * b1b2));
            bufferPerLine[0] = prevOut;
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        bufferPerLine -= Channels;
        lpColInitial += WidthStep * HeightSubOne;
        prevOut = (lpRowInitial[0] * cnext);
        for (int y = HeightSubOne; y >= 0; y--) {
            prevOut = ((lpRowInitial[0] * a2a3) - (prevOut * b1b2));
            bufferPerLine[0] += prevOut;
            lpColInitial[0] = bufferPerLine[0];
            lpRowInitial -= Channels;
            lpColInitial -= WidthStep;
            bufferPerLine -= Channels;
        }
    }
}

static void gaussianHorizontal(uint8_t *bufferPerLine, uint8_t *lpRowInitial,
                        uint8_t *lpColumn, int width, int height, int Channels, int Nwidth,
                        float a0a1, float a2a3, float b1b2, float cprev, float cnext) {
    int HeightStep = Channels * height;
    int WidthSubOne = width - 1;
    if (Channels == 3) {
        float prevOut[3];
        prevOut[0] = (lpRowInitial[0] * cprev);
        prevOut[1] = (lpRowInitial[1] * cprev);
        prevOut[2] = (lpRowInitial[2] * cprev);
        for (int x = 0; x < width; ++x) {
            prevOut[0] = ((lpRowInitial[0] * (a0a1)) - (prevOut[0] * (b1b2)));
            prevOut[1] = ((lpRowInitial[1] * (a0a1)) - (prevOut[1] * (b1b2)));
            prevOut[2] = ((lpRowInitial[2] * (a0a1)) - (prevOut[2] * (b1b2)));
            bufferPerLine[0] = prevOut[0];
            bufferPerLine[1] = prevOut[1];
            bufferPerLine[2] = prevOut[2];
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        lpColumn += HeightStep * WidthSubOne;
        bufferPerLine -= Channels;
        prevOut[0] = (lpRowInitial[0] * cnext);
        prevOut[1] = (lpRowInitial[1] * cnext);
        prevOut[2] = (lpRowInitial[2] * cnext);

        for (int x = WidthSubOne; x >= 0; --x) {
            prevOut[0] = ((lpRowInitial[0] * (a2a3)) - (prevOut[0] * (b1b2)));
            prevOut[1] = ((lpRowInitial[1] * (a2a3)) - (prevOut[1] * (b1b2)));
            prevOut[2] = ((lpRowInitial[2] * (a2a3)) - (prevOut[2] * (b1b2)));
            bufferPerLine[0] += prevOut[0];
            bufferPerLine[1] += prevOut[1];
            bufferPerLine[2] += prevOut[2];
            lpColumn[0] = bufferPerLine[0];
            lpColumn[1] = bufferPerLine[1];
            lpColumn[2] = bufferPerLine[2];
            lpRowInitial -= Channels;
            lpColumn -= HeightStep;
            bufferPerLine -= Channels;
        }
    } else if (Channels == 4) {
        float prevOut[4];

        prevOut[0] = (lpRowInitial[0] * cprev);
        prevOut[1] = (lpRowInitial[1] * cprev);
        prevOut[2] = (lpRowInitial[2] * cprev);
        prevOut[3] = (lpRowInitial[3] * cprev);
        for (int x = 0; x < width; ++x) {
            prevOut[0] = ((lpRowInitial[0] * (a0a1)) - (prevOut[0] * (b1b2)));
            prevOut[1] = ((lpRowInitial[1] * (a0a1)) - (prevOut[1] * (b1b2)));
            prevOut[2] = ((lpRowInitial[2] * (a0a1)) - (prevOut[2] * (b1b2)));
            prevOut[3] = ((lpRowInitial[3] * (a0a1)) - (prevOut[3] * (b1b2)));

            bufferPerLine[0] = prevOut[0];
            bufferPerLine[1] = prevOut[1];
            bufferPerLine[2] = prevOut[2];
            bufferPerLine[3] = prevOut[3];
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        lpColumn += HeightStep * WidthSubOne;
        bufferPerLine -= Channels;

        prevOut[0] = (lpRowInitial[0] * cnext);
        prevOut[1] = (lpRowInitial[1] * cnext);
        prevOut[2] = (lpRowInitial[2] * cnext);
        prevOut[3] = (lpRowInitial[3] * cnext);

        for (int x = WidthSubOne; x >= 0; --x) {
            prevOut[0] = ((lpRowInitial[0] * a2a3) - (prevOut[0] * b1b2));
            prevOut[1] = ((lpRowInitial[1] * a2a3) - (prevOut[1] * b1b2));
            prevOut[2] = ((lpRowInitial[2] * a2a3) - (prevOut[2] * b1b2));
            prevOut[3] = ((lpRowInitial[3] * a2a3) - (prevOut[3] * b1b2));
            bufferPerLine[0] += prevOut[0];
            bufferPerLine[1] += prevOut[1];
            bufferPerLine[2] += prevOut[2];
            bufferPerLine[3] += prevOut[3];
            lpColumn[0] = bufferPerLine[0];
            lpColumn[1] = bufferPerLine[1];
            lpColumn[2] = bufferPerLine[2];
            lpColumn[3] = bufferPerLine[3];
            lpRowInitial -= Channels;
            lpColumn -= HeightStep;
            bufferPerLine -= Channels;
        }
    } else if (Channels == 1) {
        float prevOut = (lpRowInitial[0] * cprev);

        for (int x = 0; x < width; ++x) {
            prevOut = ((lpRowInitial[0] * (a0a1)) - (prevOut * (b1b2)));
            bufferPerLine[0] = prevOut;
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        lpColumn += HeightStep * WidthSubOne;
        bufferPerLine -= Channels;

        prevOut = (lpRowInitial[0] * cnext);

        for (int x = WidthSubOne; x >= 0; --x) {
            prevOut = ((lpRowInitial[0] * a2a3) - (prevOut * b1b2));
            bufferPerLine[0] += prevOut;
            lpColumn[0] = bufferPerLine[0];
            lpRowInitial -= Channels;
            lpColumn -= HeightStep;
            bufferPerLine -= Channels;
        }
    }
}

void GaussianBlurFilter(uint8_t *input, int Width, int Height, float GaussianSigma) {
    int Channels = 4;
    int Stride = Channels * Width;
    float a0, a1, a2, a3, b1, b2, cprev, cnext;

    CalGaussianCoeff(GaussianSigma, &a0, &a1, &a2, &a3, &b1, &b2, &cprev, &cnext);

    const float a0a1 = (a0 + a1);
    const float a2a3 = (a2 + a3);
    const float b1b2 = (b1 + b2);

    int bufferSizePerThread = (Width > Height ? Width : Height) * Channels;
    uint8_t *bufferPerLine = (uint8_t *) malloc(bufferSizePerThread);
    uint8_t *tempData = (uint8_t *) malloc(Height * Stride);
    if (bufferPerLine == nullptr || tempData == nullptr) {
        if (tempData) {
            free(tempData);
        }
        if (bufferPerLine) {
            free(bufferPerLine);
        }
        return;
    }
    for (int y = 0; y < Height; ++y) {
        uint8_t *lpRowInitial = input + Stride * y;
        uint8_t *lpColInitial = tempData + y * Channels;
        gaussianHorizontal(bufferPerLine, lpRowInitial, lpColInitial, Width, Height, Channels,
                           Width, a0a1, a2a3, b1b2, cprev, cnext);
    }
    int HeightStep = Height * Channels;
    for (int x = 0; x < Width; ++x) {
        uint8_t *lpColInitial = input + x * Channels;
        uint8_t *lpRowInitial = tempData + HeightStep * x;
        gaussianVertical(bufferPerLine, lpRowInitial, lpColInitial, Height, Width, Channels, a0a1,
                         a2a3, b1b2, cprev, cnext);
    }

    free(bufferPerLine);
    free(tempData);
}
}/*endof namespace*/

