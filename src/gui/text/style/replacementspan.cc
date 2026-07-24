#include <core/canvas.h>
#include <drawable/drawable.h>
#include <text/style/replacementspan.h>
namespace cdroid{
int DynamicDrawableSpan::getSize(const Paint& paint, const CharSequence* text, int start, int end, Paint::FontMetricsInt* fm)const{
    Drawable* d = getDrawable();
    Rect rect = d->getBounds();

    if (fm != nullptr) {
        fm->ascent = -rect.height;
        fm->descent = 0;
        fm->top = fm->ascent;
        fm->bottom = 0;
    }

    return rect.width;
}

void DynamicDrawableSpan::draw(Canvas& canvas, const CharSequence* text, int start, int end, float x, int top, int y, int bottom, const Paint& paint)const {
    Drawable* b = getDrawable();
    canvas.save();

    int transY = bottom - b->getBounds().height;
    if (mVerticalAlignment == ALIGN_BASELINE) {
        transY -= paint.getFontMetricsInt().descent;
    } else if (mVerticalAlignment == ALIGN_CENTER) {
        transY = top + (bottom - top) / 2 - b->getBounds().height / 2;
    }

    canvas.translate(x, transY);
    b->draw(canvas);
    canvas.restore();
}

Drawable* ImageSpan::getDrawable() const{
    Drawable* drawable = nullptr;
    if (mDrawable != nullptr) {
        drawable = mDrawable;
    } else if (!mContentUri.empty()) {
        drawable = mContext->getDrawable(mContentUri);
        drawable->setBounds(0, 0, drawable->getIntrinsicWidth(),
                drawable->getIntrinsicHeight());
    }
    return drawable;
}

ImageSpan::ImageSpan(Context* context, const std::string& resourceId)
    :ImageSpan(context, resourceId, ALIGN_BOTTOM){
}

ImageSpan::ImageSpan(Context* context, const std::string& resourceId, int verticalAlignment)
   :DynamicDrawableSpan(verticalAlignment){
    mContext = context;
    mContentUri = resourceId;
}
}/*endof namespace*/