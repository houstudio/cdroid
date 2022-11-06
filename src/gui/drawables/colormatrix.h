#ifndef __COLOR_MATRIX_H__
#define __COLOR_MATRIX_H__
#include <vector>
#include <cairomm/refptr.h>
#include <cairomm/surface.h>
namespace cdroid{

class ColorVector{
public:
    float values[5];
    ColorVector();
    ColorVector(const float(&v)[5]);
    float& operator[](size_t i) {return values[i];}
    const float& operator[](size_t i)const{return values[i];}
};

class ColorMatrix{
public:
    float mArray[20];
    static const ColorMatrix Identity;
    ColorMatrix();
    ColorMatrix(const ColorMatrix& src);
    ColorMatrix(const float(&v)[20]);
    ColorMatrix& operator*=(const ColorMatrix&);
    void reset();
    void set(const ColorMatrix& src);
    void set(const float(&v)[20]);
    void setScale(float rScale, float gScale, float bScale,float aScale);
    void setRotate(int axis, float degrees);
    void setConcat(ColorMatrix& matA,ColorMatrix& matB);
    void preConcat(ColorMatrix& prematrix);
    void postConcat(ColorMatrix& postmatrix);
    void setSaturation(float sat);
    void setRGB2YUV();
    void setYUV2RGB();
    unsigned int transform(unsigned int color);
    Cairo::RefPtr<Cairo::ImageSurface>transform(const Cairo::RefPtr<Cairo::ImageSurface>&img);
};

ColorVector operator * (const ColorMatrix &m , const ColorVector &v);
ColorMatrix operator * (const ColorMatrix &m1, const ColorMatrix &m2);

}//namespace 
#endif
