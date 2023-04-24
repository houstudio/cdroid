#ifndef __RENDERNODE_H__
#define __RENDERNODE_H__
#include <core/canvas.h>
namespace cdroid{

class RenderNode{
private:
    float mX,mY,mZ;
    float mScaleX,mScaleY;
    float mElevation;
    float mAlpha;
    float mRotationX,mRotationY,mRotation;
    float mPivotX,mPivotY;
    float mTranslationX,mTranslationY,mTranslationZ;
    Cairo::Matrix mMatrix;
public:
    RenderNode();
    bool  hasIdentityMatrix()const;
    void  getMatrix(Cairo::Matrix&)const;
    void  getInverseMatrix(Cairo::Matrix&)const;
    void  setAlpha(float);
    float getAlpha()const;
    void  setElevation(float);
    float getElevation()const;

    void  setTranslationX(float);
    float getTranslationX()const;
    void  setTranslationY(float);
    float getTranslationY()const;
    void  setTranslationZ(float);
    float getTranslationZ()const;

    void  setRotation(float);
    float getRotation()const;
    void  setRotationX(float);
    float getRotationX()const;
    void  setRotationY(float);
    float getRotationY()const;
    
    void  setScaleX(float);
    float getScaleX()const;
    void  setScaleY(float);
    float getScaleY()const;
 
    void  setPivotX(float);
    float getPivotX()const;
    void  setPivotY(float);
    float getPivotY()const;
    bool  isPivotExplicitlySet()const;

    void  setLeft(float);
    void  setTop(float);
    void  setRight(float);
    void  setBottom(float);
};

}
#endif
