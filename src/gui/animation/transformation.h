#ifndef __TRANSFORMATION_H__
#define __TRANSFORMATION_H__
#include <core/rect.h>
#include <cairomm/matrix.h>
#include <animation/interpolators.h>
#include <core/attributeset.h>

typedef Cairo::Matrix Matrix;
namespace cdroid{

class Transformation{
public:
    static constexpr int TYPE_IDENTITY = 0x0;
    static constexpr int TYPE_ALPHA = 0x1;;
    static constexpr int TYPE_MATRIX = 0x2;;
    static constexpr int TYPE_BOTH = TYPE_ALPHA | TYPE_MATRIX;
protected:
    Matrix mMatrix;
    float mAlpha;
    int mTransformationType;
    bool mHasClipRect;
    Rect mClipRect ;
public:
    Transformation();
    void operator=(const Transformation&);
    void clear();
    int getTransformationType()const;
    void setTransformationType(int transformationType);
    void set(const Transformation& t);
    void compose(Transformation t);
    void postCompose(Transformation t);
    const Matrix& getMatrix()const;
    Matrix& getMatrix();
    void setAlpha(/*from=0.0, to=1.0)*/float alpha);
    void setClipRect(Rect r);
    void setClipRect(int l, int t, int w, int h);
    Rect getClipRect()const;
    bool hasClipRect()const;
    float getAlpha()const;
};

}//endof namespace
#endif
