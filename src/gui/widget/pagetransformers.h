#pragma once
#include <widget/viewpager.h>
namespace cdroid{

//refto: https://github.com/OCNYang/PageTransformerHelp

class ABaseTransformer:public ViewPager::PageTransformer{
protected:
    virtual void onTransform(View&page,float position)=0;
    virtual bool hideOffscreenPages();
    virtual bool isPagingEnabled();
    virtual void onPreTransform(View&page,float position);
    virtual void onPostTransform(View&page,float position);
public:
    void transformPage(View&page,float position)override;
};


class AccordionTransformer:public ABaseTransformer {
protected:
    void onTransform(View& view, float position)override;
};

class CubeInTransformer:public ABaseTransformer{
protected:
    void onTransform(View& view, float position)override;
    bool isPagingEnabled()override;
};

class CubeOutTransformer:public ABaseTransformer{
protected:
    void onTransform(View& view, float position)override;
    bool isPagingEnabled()override;
};

class DefaultTransformer:ABaseTransformer{
protected:
    void onTransform(View& view, float position)override;
    bool isPagingEnabled()override;
};

class FlipHorizontalTransformer:public ABaseTransformer{
protected:
    void onTransform(View& view, float position)override;
    void onPostTransform(View&page,float position)override;   
};

class FlipVerticalTransformer:public ABaseTransformer{
protected:
    void onTransform(View& view, float position)override;
    void onPostTransform(View&page,float position)override;
};

class ParallaxTransformer:public ViewPager::PageTransformer{
public:
   void transformPage(View& page, float position)override;
};

class RotateDownTransformer:public ABaseTransformer{
private:static constexpr  float ROT_MOD = -15.f;
protected:
    void onTransform(View& view, float position)override;
    bool isPagingEnabled()override;
};

class RotateUpTransformer:public ABaseTransformer{
private:static constexpr  float ROT_MOD = -15.f;
protected:
    void onTransform(View& view, float position)override;
    bool isPagingEnabled()override;
};

class ScaleInOutTransformer:public ABaseTransformer{
protected:
    void onTransform(View& view, float position)override;
};

class StackTransformer:public ABaseTransformer{
protected:
    void onTransform(View& view, float position)override;
};

class ZoomInTransformer:public ABaseTransformer{
protected:
    void onTransform(View& view, float position)override;
};

class ZoomOutTransformer:public ABaseTransformer{
protected:
    void onTransform(View& view, float position)override;
};

class ZoomOutSlideTransformer:public ABaseTransformer{
private:
    float mMinScale;
    float mMinAlpha;
protected:
    void onTransform(View& view, float position);
public:
    ZoomOutSlideTransformer();
    ZoomOutSlideTransformer(float minscale,float minalpha);
};

}//endof namespace

