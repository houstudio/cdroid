#include <widget/rendernode.h>
namespace cdroid{

RenderNode::RenderNode(){
    mX = mY = mZ =.0f;
    mAlpha  = 1.f;
    mScaleX = mScaleY=1.f;
    mTranslationX = mTranslationY =.0f;
    mRotation  = .0f;
    mRotationX = mRotationY = .0f;
    mMatrix = identity_matrix();
}

bool RenderNode::hasIdentityMatrix()const{
    const bool rc= (mX==.0f) && (mY==.0f) && (mZ==.0f) &&
       (mTranslationX==.0f) && (mTranslationY==.0f) &&
       (mScaleX ==1.f) && (mScaleY==1.f) && (mRotation==.0f);
    return rc;
}

static inline float sdot(float a,float b,float c,float d){
    return a * b + c * d;
}

Matrix RenderNode::getMatrix()const{
    Matrix matrix= identity_matrix();
    matrix.translate(mTranslationX,mTranslationY);
    matrix.scale(mScaleX,mScaleY);

    const float radians=mRotation*M_PI/180.f;
    const float fsin=sin(radians);
    const float fcos=cos(radians);
    Matrix rt(fcos,-fsin, fsin,fcos, sdot(-fsin,mPivotY,1.f-fcos,mPivotX),sdot(fsin,mPivotX,1.f-fcos,mPivotY));
    matrix.multiply(matrix,rt);
    return matrix;
}

Matrix RenderNode::getInverseMatrix()const{
    Matrix matrix=getMatrix();
    matrix.invert();
    return matrix;
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
    mScaleX = scale;
}

float RenderNode::getScaleX()const{
    return mScaleX;
}

void RenderNode::setScaleY(float scale){
    mScaleY = scale;
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
    return true;
}

void RenderNode::setLeft(float){
}

void RenderNode::setTop(float){
}

void RenderNode::setRight(float){
}

void RenderNode::setBottom(float){
}

}
