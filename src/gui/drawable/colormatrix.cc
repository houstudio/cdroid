/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <drawable/colormatrix.h>
#include <core/color.h>
#include <math.h>
#include <algorithm>
using namespace Cairo;
namespace cdroid{

ColorVector::ColorVector(){
}

ColorVector::ColorVector(const float(&v)[5]){
    for(int i=0;i<5;i++)
        values[i]=v[i];
}
#if 0
ColorVector operator*(const ColorMatrix &m, const ColorVector &v){
    return ColorVector({
        m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2] + m[0][3] * v[3] + m[0][4] * v[4],
        m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2] + m[1][3] * v[3] + m[1][4] * v[4],
        m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2] + m[2][3] * v[3] + m[2][4] * v[4],
        m[3][0] * v[0] + m[3][1] * v[1] + m[3][2] * v[2] + m[3][3] * v[3] + m[3][4] * v[4],
        m[4][0] * v[0] + m[4][1] * v[1] + m[4][2] * v[2] + m[4][3] * v[3] + m[4][4] * v[4]
    });
}
#endif
ColorMatrix::ColorMatrix(){
    reset();
}

ColorMatrix::ColorMatrix(const ColorMatrix& src){
    for(int i=0;i<20;i++)mArray[i]=src.mArray[i];
}

ColorMatrix::ColorMatrix(const float (&v)[20]){
    std::copy(std::begin(v), std::end(v), (float*)this->mArray);
}

void ColorMatrix::reset(){
    for(int i=0;i<20;i++)mArray[i]=.0;
    mArray[0] = mArray[6] = mArray[12] = mArray[18] = 1;
}

void ColorMatrix::set(const ColorMatrix& src){
    for(int i=0;i<20;i++)mArray[i]=src.mArray[i];
}

void ColorMatrix::set(const float(&v)[20]){
    for(int i=0;i<20;i++)mArray[i]=v[i];
}

void ColorMatrix::setScale(float rScale, float gScale, float bScale,float aScale){
    for (int i = 0; i <20; i++){
        mArray[i] = 0;
    }
    mArray[0] = rScale;
    mArray[6] = gScale;
    mArray[12]= bScale;
    mArray[18]= aScale;
}

void ColorMatrix::setRotate(int axis, float degrees){
    reset();
    double radians = degrees * M_PI / 180.f;
    float cosine = (float) cos(radians);
    float sine = (float) sin(radians);
    switch (axis) {
    // Rotation around the red color
    case 0:
        mArray[6] = mArray[12] = cosine;
        mArray[7] = sine;
        mArray[11] = -sine;
        break;
    // Rotation around the green color
    case 1:
        mArray[0] = mArray[12] = cosine;
        mArray[2] = -sine;
        mArray[10] = sine;
        break;
    // Rotation around the blue color
    case 2:
        mArray[0] = mArray[6] = cosine;
        mArray[1] = sine;
        mArray[5] = -sine;
        break;
    default:  break;//throw new RuntimeException();
    }
}

void ColorMatrix::setConcat(ColorMatrix& matA,ColorMatrix& matB){
     float* tmp;
     if (&matA == this || &matB == this) {
         tmp = new float[20];
     } else {
         tmp = mArray;
     }

     float* a = matA.mArray;
     float* b = matB.mArray;
     int index = 0;
     for (int j = 0; j < 20; j += 5) {
         for (int i = 0; i < 4; i++) {
             tmp[index++] = a[j + 0] * b[i + 0] +  a[j + 1] * b[i + 5] +
                      a[j + 2] * b[i + 10] + a[j + 3] * b[i + 15];
         }
         tmp[index++] = a[j + 0] * b[4] +  a[j + 1] * b[9] +
                   a[j + 2] * b[14] + a[j + 3] * b[19] +  a[j + 4];
     }

     if (tmp != mArray) {
	 for(int i=0;i<20;i++)mArray[i]=tmp[i];
	 delete []tmp;
     }
}

void ColorMatrix::preConcat(ColorMatrix& prematrix){
    setConcat(*this, prematrix);
}
void ColorMatrix::postConcat(ColorMatrix& postmatrix){
    setConcat(postmatrix, *this);
}

void ColorMatrix::setSaturation(float sat) {
    reset();
    float* m = mArray;

    float invSat = 1 - sat;
    float R = 0.213f * invSat;
    float G = 0.715f * invSat;
    float B = 0.072f * invSat;

    m[0] = R + sat; m[1] = G;       m[2] = B;
    m[5] = R;       m[6] = G + sat; m[7] = B;
    m[10] = R;      m[11] = G;      m[12] = B + sat;
}
/**Set the matrix to convert RGB to YUV*/
void ColorMatrix::setRGB2YUV() {
    reset();
    float* m = mArray;
    // these coefficients match those in libjpeg
    m[0]  = 0.299f;    m[1]  = 0.587f;    m[2]  = 0.114f;
    m[5]  = -0.16874f; m[6]  = -0.33126f; m[7]  = 0.5f;
    m[10] = 0.5f;      m[11] = -0.41869f; m[12] = -0.08131f;
}


void ColorMatrix::setYUV2RGB() {
    reset();
    float*m = mArray;
    // these coefficients match those in libjpeg
    m[2] = 1.402f;
    m[5] = 1;   m[6] = -0.34414f;   m[7] = -0.71414f;
    m[10] = 1;  m[11] = 1.772f;     m[12] = 0;
}
const ColorMatrix ColorMatrix::Identity({
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f, 0.0f
    //0.0f, 0.0f, 0.0f, 0.0f, 1.0f
});
#if 0
ColorMatrix operator * (const ColorMatrix &m1, const ColorMatrix &m2){
    return ColorMatrix({
        m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0] + m1[0][2] * m2[2][0] + m1[0][3] * m2[3][0] + m1[0][4] * m2[4][0],
        m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1] + m1[0][2] * m2[2][1] + m1[0][3] * m2[3][1] + m1[0][4] * m2[4][1],
        m1[0][0] * m2[0][2] + m1[0][1] * m2[1][2] + m1[0][2] * m2[2][2] + m1[0][3] * m2[3][2] + m1[0][4] * m2[4][2],
        m1[0][0] * m2[0][3] + m1[0][1] * m2[1][3] + m1[0][2] * m2[2][3] + m1[0][3] * m2[3][3] + m1[0][4] * m2[4][3],
        m1[0][0] * m2[0][4] + m1[0][1] * m2[1][4] + m1[0][2] * m2[2][4] + m1[0][3] * m2[3][4] + m1[0][4] * m2[4][4],
        m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0] + m1[1][2] * m2[2][0] + m1[1][3] * m2[3][0] + m1[1][4] * m2[4][0],
        m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1] + m1[1][2] * m2[2][1] + m1[1][3] * m2[3][1] + m1[1][4] * m2[4][1],
        m1[1][0] * m2[0][2] + m1[1][1] * m2[1][2] + m1[1][2] * m2[2][2] + m1[1][3] * m2[3][2] + m1[1][4] * m2[4][2],
        m1[1][0] * m2[0][3] + m1[1][1] * m2[1][3] + m1[1][2] * m2[2][3] + m1[1][3] * m2[3][3] + m1[1][4] * m2[4][3],
        m1[1][0] * m2[0][4] + m1[1][1] * m2[1][4] + m1[1][2] * m2[2][4] + m1[1][3] * m2[3][4] + m1[1][4] * m2[4][4],
        m1[2][0] * m2[0][0] + m1[2][1] * m2[1][0] + m1[2][2] * m2[2][0] + m1[2][3] * m2[3][0] + m1[2][4] * m2[4][0],
        m1[2][0] * m2[0][1] + m1[2][1] * m2[1][1] + m1[2][2] * m2[2][1] + m1[2][3] * m2[3][1] + m1[2][4] * m2[4][1],
        m1[2][0] * m2[0][2] + m1[2][1] * m2[1][2] + m1[2][2] * m2[2][2] + m1[2][3] * m2[3][2] + m1[2][4] * m2[4][2],
        m1[2][0] * m2[0][3] + m1[2][1] * m2[1][3] + m1[2][2] * m2[2][3] + m1[2][3] * m2[3][3] + m1[2][4] * m2[4][3],
        m1[2][0] * m2[0][4] + m1[2][1] * m2[1][4] + m1[2][2] * m2[2][4] + m1[2][3] * m2[3][4] + m1[2][4] * m2[4][4],
        m1[3][0] * m2[0][0] + m1[3][1] * m2[1][0] + m1[3][2] * m2[2][0] + m1[3][3] * m2[3][0] + m1[3][4] * m2[4][0],
        m1[3][0] * m2[0][1] + m1[3][1] * m2[1][1] + m1[3][2] * m2[2][1] + m1[3][3] * m2[3][1] + m1[3][4] * m2[4][1],
        m1[3][0] * m2[0][2] + m1[3][1] * m2[1][2] + m1[3][2] * m2[2][2] + m1[3][3] * m2[3][2] + m1[3][4] * m2[4][2],
        m1[3][0] * m2[0][3] + m1[3][1] * m2[1][3] + m1[3][2] * m2[2][3] + m1[3][3] * m2[3][3] + m1[3][4] * m2[4][3],
        m1[3][0] * m2[0][4] + m1[3][1] * m2[1][4] + m1[3][2] * m2[2][4] + m1[3][3] * m2[3][4] + m1[3][4] * m2[4][4],
        m1[4][0] * m2[0][0] + m1[4][1] * m2[1][0] + m1[4][2] * m2[2][0] + m1[4][3] * m2[3][0] + m1[4][4] * m2[4][0],
        m1[4][0] * m2[0][1] + m1[4][1] * m2[1][1] + m1[4][2] * m2[2][1] + m1[4][3] * m2[3][1] + m1[4][4] * m2[4][1],
        m1[4][0] * m2[0][2] + m1[4][1] * m2[1][2] + m1[4][2] * m2[2][2] + m1[4][3] * m2[3][2] + m1[4][4] * m2[4][2],
        m1[4][0] * m2[0][3] + m1[4][1] * m2[1][3] + m1[4][2] * m2[2][3] + m1[4][3] * m2[3][3] + m1[4][4] * m2[4][3],
        m1[4][0] * m2[0][4] + m1[4][1] * m2[1][4] + m1[4][2] * m2[2][4] + m1[4][3] * m2[3][4] + m1[4][4] * m2[4][4]
  });
}

ColorMatrix& ColorMatrix::operator*=(const ColorMatrix &mat){
    return *this = *this * mat;
}

template <typename T>
T clamp(T value, T min, T max){
    return value < min ? min  : value > max ? max  : value;
}

unsigned int ColorMatrix::transform(unsigned int color){
    Color c(color);
    ColorVector vec({c.red(), c.green(), c.blue(), c.alpha(), 1.0f });
    vec = (*this) * vec;
    if (vec[4] != 0.0f)
       vec[0] /= vec[4]; vec[1] /= vec[4]; vec[2] /= vec[4]; vec[3] /= vec[4]; // vec[4] = 1.0f;

    return Color::toArgb(clamp<int>(255 * vec[0], 0, 255),  clamp<int>(255 * vec[1], 0, 255),
            clamp<int>(255 * vec[2], 0, 255),    clamp<int>(255 * vec[3], 0, 255));
}

RefPtr<ImageSurface>ColorMatrix::transform(const RefPtr<ImageSurface>&simg){
    const int w = simg->get_width();
    const int h = simg->get_height();
    RefPtr<ImageSurface>dimg=ImageSurface::create(simg->get_format(),w,h);
    const unsigned char*ps=simg->get_data();
    const unsigned char*pd=dimg->get_data();
    for (int y = 0; y < h; ++y) {
         unsigned int*pic=(unsigned int*)ps;
         unsigned int*pid=(unsigned int*)pd;
         for (int x = 0; x < w; ++x) {
             pid[x]=transform(pic[x]);
	 }
         ps+=simg->get_stride();
	 pd+=simg->get_stride();
    }
    return dimg;
}
#endif
}
