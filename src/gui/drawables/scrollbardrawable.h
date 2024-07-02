#ifndef __SCROLLBAR_DRAWABLE_H__
#define __SCROLLBAR_DRAWABLE_H__
#include <drawables/drawable.h>
namespace cdroid{

class ScrollBarDrawable:public Drawable,Drawable::Callback{
private:
    Drawable* mVerticalTrack;
    Drawable* mHorizontalTrack;
    Drawable* mVerticalThumb;
    Drawable* mHorizontalThumb;
    int mRange;
    int mOffset;
    int mExtent;

    bool mVertical;
    bool mBoundsChanged;
    bool mRangeChanged;
    bool mAlwaysDrawHorizontalTrack;
    bool mAlwaysDrawVerticalTrack;
    bool mMutated;

    int mAlpha;
    bool mHasSetAlpha;
    //ColorFilter mColorFilter;
    bool mHasSetColorFilter;

    void drawTrack(Canvas&canvas,const Rect& bounds, bool vertical);
    void drawThumb(Canvas& canvas,const Rect& bounds, int offset, int length, bool vertical);
    void propagateCurrentState(Drawable* d);
protected:
    void onBoundsChange(const Rect& bounds)override;
    bool onStateChange(const std::vector<int>&state)override;
public:
    ScrollBarDrawable();
    virtual ~ScrollBarDrawable();
    void setAlwaysDrawHorizontalTrack(bool alwaysDrawTrack);
    void setAlwaysDrawVerticalTrack(bool alwaysDrawTrack);
    bool getAlwaysDrawVerticalTrack()const;
    bool getAlwaysDrawHorizontalTrack()const;
    void setParameters(int range, int offset, int extent, bool vertical);
    void draw(Canvas&canvas)override;
    bool isStateful()const override;
    void setVerticalThumbDrawable(Drawable* thumb);
    void setVerticalTrackDrawable(Drawable* track);
    void setHorizontalThumbDrawable(Drawable* thumb);
    void setHorizontalTrackDrawable(Drawable* track);
    Drawable*getVerticalThumbDrawable()const;
    Drawable*getVerticalTrackDrawable()const;
    Drawable*getHorizontalThumbDrawable()const;
    Drawable*getHorizontalTrackDrawable()const;
    int getSize(bool vertical);
    ScrollBarDrawable* mutate();
    void setAlpha(int alpha)override;
    int getAlpha() const override;
    int getOpacity()override;
    void invalidateDrawable(Drawable& who)override;
    void scheduleDrawable(Drawable& who, Runnable& what, long when)override;
    void unscheduleDrawable(Drawable& who,Runnable& what)override;
};

}
#endif
