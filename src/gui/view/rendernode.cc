#include <view/rendernode.h>
#include <float.h>
using namespace Cairo;
namespace cdroid{

RenderNode::RenderNode(){
    mX = mY = mZ =.0f;
    mAlpha  = 1.f;
    mScaleX = 1.f;
    mScaleY = 1.f;
    mElevation =.0f;
    mTranslationX = .0f;
    mTranslationY = .0f;
    mTranslationZ = .0f;
    mPivotX = mPivotY = FLT_MIN;
    mRotation  = .0f;
    mRotationX = .0f;
    mRotationY = .0f;
    mMatrix = identity_matrix();
}

bool RenderNode::hasIdentityMatrix()const{
    const bool rc= (mX==.0f) && (mY==.0f) && (mZ==.0f) &&
       (mTranslationX==.0f) && (mTranslationY==.0f) &&
       (mScaleX ==1.f) && (mScaleY==1.f) && (mRotation==.0f);
    return rc;
}


void RenderNode::getMatrix(Matrix&outMatrix)const{
    const float px = (mPivotX==FLT_MIN)?(mRight - mLeft)/2:mPivotX;
    const float py = (mPivotY==FLT_MIN)?(mBottom- mTop)/2:mPivotY;
    outMatrix = identity_matrix();
#if 1
    outMatrix.translate(px,py);
    outMatrix.scale(mScaleX,mScaleY);
    outMatrix.rotate(mRotation*M_PI/180.f);
    outMatrix.translate(-px,-py);
    Matrix rt = identity_matrix();
    rt.translate(mTranslationX,mTranslationY);
    outMatrix.multiply(outMatrix,rt);
#else
    auto sdot=[](float a,float b,float c,float d)->float{
       return a * b + c * d;
    };
    outMatrix.translate(mTranslationX,mTranslationY);
    outMatrix.scale(mScaleX,mScaleY);

    const float radians = mRotation*M_PI/180.f;
    const float fsin = sin(radians);
    const float fcos = cos(radians);
    Matrix rt(fcos,-fsin, fsin,fcos, sdot(-fsin,px,1.f-fcos,py),sdot(fsin,px,1.f-fcos,py));
    outMatrix.multiply(outMatrix,rt);
#endif
}

void RenderNode::getInverseMatrix(Matrix&outMatrix)const{
    getMatrix(outMatrix);
    outMatrix.invert();
}

void RenderNode::setAlpha(float alpha){
    mAlpha = alpha;
}

float RenderNode::getAlpha()const{
    return mAlpha;
}

void RenderNode::setElevation(float elevation){
    mElevation = elevation;
}

float RenderNode::getElevation()const{
    return mElevation;
}

void RenderNode::setTranslationX(float x){
    mTranslationX = x;
}

float RenderNode::getTranslationX()const{
    return mTranslationX;
}

void RenderNode::setTranslationY(float y){
    mTranslationY = y;
}

float RenderNode::getTranslationY()const{
    return mTranslationY;
}

void RenderNode::setTranslationZ(float z){
    mTranslationZ = z;
}

float RenderNode::getTranslationZ()const{
    return mTranslationZ;
}

void RenderNode::setRotation(float angle){
    mRotation = angle;
}

float RenderNode::getRotation()const{
    return mRotation;
}

void RenderNode::setRotationX(float angle){
    mRotationX = angle;
}

float RenderNode::getRotationX()const{
    return mRotationX;
}

void RenderNode::setRotationY(float angle){
    mRotationY = angle;
}

float RenderNode::getRotationY()const{
    return mRotationY;
}

void RenderNode::setScaleX(float scale){
    const float EPSILON = 0.0000001f;
    const float sign = (scale >= 0) ? 1.0f : -1.0f;
    mScaleX = (std::abs(scale)<EPSILON)?sign*EPSILON:scale;
}

float RenderNode::getScaleX()const{
    return mScaleX;
}

void RenderNode::setScaleY(float scale){
    const float EPSILON = 0.0000001f;
    const float sign = (scale >= 0) ? 1.0f : -1.0f;
    mScaleY = (std::abs(scale)<EPSILON)?sign*EPSILON:scale;
}

float RenderNode::getScaleY()const{
    return mScaleY;
}

void RenderNode::setPivotX(float px){
    mPivotX = px;
}

float RenderNode::getPivotX()const{
    return mPivotX;
}

void RenderNode::setPivotY(float py){
    mPivotY = py;
}

float RenderNode::getPivotY()const{
    return mPivotY;
}

bool RenderNode::isPivotExplicitlySet()const{
    return (mPivotX!=FLT_MIN)||(mPivotY!=FLT_MIN);
}

void RenderNode::setLeft(float left){
    mLeft = left;
}

void RenderNode::setTop(float top){
    mTop = top;
}

void RenderNode::setRight(float right){
    mRight = right;
}

void RenderNode::setBottom(float bottom){
    mBottom = bottom;
}

void RenderNode::setLeftTopRightBottom(float left,float top,float right,float bottom){
    mLeft = left;
    mTop = top;
    mRight = right;
    mBottom= bottom;
}

}
