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
const ColorMatrix ColorMatrix::Identity({
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f, 0.0f
    //0.0f, 0.0f, 0.0f, 0.0f, 1.0f
});
}
