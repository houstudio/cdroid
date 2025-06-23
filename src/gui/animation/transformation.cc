#include <animation/transformation.h>

namespace cdroid{

Transformation::Transformation() {
    clear();
}

void Transformation::operator=(const Transformation&o){
    mMatrix  = o.mMatrix;
    mClipRect= o.mClipRect;
    mAlpha   = o.mAlpha;
    mHasClipRect=o.mHasClipRect;
    mTransformationType=o.mTransformationType;
}

void Transformation::clear(){
    mMatrix=Cairo::identity_matrix();
    mClipRect.setEmpty();
    mHasClipRect = false;
    mAlpha = 1.0f;
    mTransformationType = TYPE_BOTH;
}

int Transformation::getTransformationType()const{
    return mTransformationType;
}

void Transformation::setTransformationType(int transformationType) {
    mTransformationType = transformationType;
}

void Transformation::set(const Transformation& t) {
    mAlpha = t.getAlpha();
    mMatrix= t.getMatrix();
    if (t.mHasClipRect) {
        setClipRect(t.getClipRect());
    } else {
        mHasClipRect = false;
        mClipRect.setEmpty();
    }
    mTransformationType = t.getTransformationType();
}

void Transformation::compose(Transformation t) {
    mAlpha *= t.getAlpha();
    mMatrix.multiply(mMatrix,t.getMatrix());//preConcat(t.getMatrix());
    if (t.mHasClipRect) {
        Rect bounds = t.getClipRect();
        if (mHasClipRect) {
            setClipRect(mClipRect.left + bounds.left, mClipRect.top + bounds.top,
                    mClipRect.width + bounds.width, mClipRect.height + bounds.height);
        } else {
            setClipRect(bounds);
        }
    }
}

void Transformation::postCompose(Transformation t) {
    mAlpha *= t.getAlpha();
    mMatrix.multiply(mMatrix,t.getMatrix());//postConcat(t.getMatrix());
    if (t.mHasClipRect) {
        Rect bounds = t.getClipRect();
        if (mHasClipRect) {
            setClipRect(mClipRect.left + bounds.left, mClipRect.top + bounds.top,
                    mClipRect.width + bounds.width, mClipRect.height + bounds.height);
        } else {
            setClipRect(bounds);
        }
    }
}

Matrix& Transformation::getMatrix(){
    return mMatrix;
}
const Matrix& Transformation::getMatrix()const{
    return mMatrix;
}

void Transformation::setAlpha(float alpha) {
    mAlpha = alpha;
}

float Transformation::getAlpha()const {
    return mAlpha;
}

void Transformation::setClipRect(const Rect& r) {
    setClipRect(r.left, r.top, r.width, r.height);
}

void Transformation::setClipRect(int l, int t, int w, int h) {
    mClipRect.set(l, t, w, h);
    mHasClipRect = true;
}

Rect Transformation::getClipRect()const{
    return mClipRect;
}

bool Transformation::hasClipRect()const {
    return mHasClipRect;
}

void Transformation::setInsets(int left, int top, int right, int bottom) {
    mInsets = Insets::of(left, top, right, bottom);
}

const Insets& Transformation::getInsets()const{
    return mInsets;
}
}

