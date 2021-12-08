#include <widget/pagetransformers.h>
namespace cdroid{

void ABaseTransformer::transformPage(View& page, float position) {
    onPreTransform(page, position);
    onTransform(page, position);
    onPostTransform(page, position);
}

bool ABaseTransformer::hideOffscreenPages(){
    return true;
}

bool ABaseTransformer::isPagingEnabled(){
    return false;
}

void ABaseTransformer::onPreTransform(View& page, float position) {
    const float width = page.getWidth();

    page.setRotationX(0);
    page.setRotationY(0);
    page.setRotation(0);
    page.setScaleX(1);
    page.setScaleY(1);
    page.setPivotX(0);
    page.setPivotY(0);
    page.setTranslationY(0);
    page.setTranslationX(isPagingEnabled() ? .0f : -width * position);

    if (hideOffscreenPages()) {
        page.setAlpha(position <= -1.f || position >= 1.f ? .0f : 1.f);
        //page.setEnabled(false);
    } else {
        //page.setEnabled(true);
        page.setAlpha(1.f);
    }
}

void ABaseTransformer::onPostTransform(View& page, float position){
}

void AccordionTransformer::onTransform(View& view, float position) {
    view.setPivotX(position < 0 ? 0 : view.getWidth());
    view.setScaleX(position < 0 ? 1.f + position : 1.f - position);
}


void CubeInTransformer::onTransform(View& view, float position) {
    LOGD("cubein %d,%d",view.getWidth(),view.getHeight());
    // Rotate the fragment on the left or right edge
    view.setPivotX(position > 0 ? 0 : view.getWidth());
    view.setPivotY(0);
    view.setRotationY(-90.f * position);
}

bool CubeInTransformer::isPagingEnabled() {
    return true;
}

void CubeOutTransformer::onTransform(View& view, float position) {
    view.setPivotX(position < .0f ? view.getWidth() : .0f);
    view.setPivotY(view.getHeight() * 0.5f);
    view.setRotationY(90.f * position);
}

bool CubeOutTransformer::isPagingEnabled() {
    return true;
}

void DefaultTransformer::onTransform(View& view, float position){}
bool DefaultTransformer::isPagingEnabled(){return true;}


void FlipHorizontalTransformer::onTransform(View& view, float position){
    const float rotation = 180.f * position;

    view.setAlpha(rotation > 90.f || rotation < -90.f ? 0 : 1);
    view.setPivotX(view.getWidth() * 0.5f);
    view.setPivotY(view.getHeight() * 0.5f);
    view.setRotationY(rotation);
}

void FlipHorizontalTransformer::onPostTransform(View& page, float position){
    ABaseTransformer::onPostTransform(page, position);

    //resolve problem: new page can't handle click event!
    page.setVisibility((position > -0.5f && position < 0.5f)?View::VISIBLE:View::INVISIBLE);
}

void FlipVerticalTransformer::onTransform(View& view, float position) {
    const float rotation = -180.f * position;

    view.setAlpha(rotation > 90.f || rotation < -90.f ? .0f : 1.f);
    view.setPivotX(view.getWidth() * 0.5f);
    view.setPivotY(view.getHeight() * 0.5f);
    view.setRotationX(rotation);
}

void FlipVerticalTransformer::onPostTransform(View& page, float position) {
    ABaseTransformer::onPostTransform(page, position);

    page.setVisibility((position > -0.5f && position < 0.5f)?View::VISIBLE:View::INVISIBLE);
}

void ParallaxTransformer::transformPage(View& page, float position){
    const int width = page.getWidth();
    if (position < -1) {
        page.setScrollX((int) (width * 0.75 * -1));
    } else if (position <= 1) {
        if (position < 0) {
            page.setScrollX((int) (width * 0.75 * position));
        } else {
            page.setScrollX((int) (width * 0.75 * position));
        }
    } else {
        page.setScrollX((int) (width * 0.75));
    }
}

void RotateDownTransformer::onTransform(View& view, float position) {
    const float width = view.getWidth();
    const float height = view.getHeight();
    const float rotation = ROT_MOD * position * -1.25f;

    view.setPivotX(width * 0.5f);
    view.setPivotY(height);
    view.setRotation(rotation);
}
	
bool RotateDownTransformer::isPagingEnabled() {
   return true;
}

void RotateUpTransformer::onTransform(View& view, float position) {
    const float width = view.getWidth();
    const float rotation = ROT_MOD * position;

    view.setPivotX(width * 0.5f);
    view.setPivotY(.0f);
    view.setTranslationX(.0f);
    view.setRotation(rotation);
}
	
bool RotateUpTransformer::isPagingEnabled() {
    return true;
}

void ScaleInOutTransformer::onTransform(View& view, float position){
    view.setPivotX(position < 0 ? 0 : view.getWidth());
    view.setPivotY(view.getHeight() / 2.f);
    float scale = position < 0 ? 1.f + position : 1.f - position;
    view.setScaleX(scale);
    view.setScaleY(scale);
}

void StackTransformer::onTransform(View& view, float position) {
    view.setTranslationX(position < 0 ? .0f : -view.getWidth() * position);
}

void ZoomInTransformer::onTransform(View& view, float position) {
    const float scale = position < 0 ? position + 1.f : std::abs(1.f - position);
    view.setScaleX(scale);
    view.setScaleY(scale);
    view.setPivotX(view.getWidth() * 0.5f);
    view.setPivotY(view.getHeight()* 0.5f);
    view.setAlpha(position < -1.f || position > 1.f ? .0f : 1.f - (scale - 1.f));
}

void ZoomOutTransformer::onTransform(View& view, float position) {
    const float scale = position < 0 ? position + 1.f : std::abs(1.f - position);
    view.setScaleX(scale);
    view.setScaleY(scale);
    view.setPivotX(view.getWidth() * 0.5f);
    view.setPivotY(view.getHeight() * 0.5f);
    view.setAlpha(position < -1.f || position > 1.f ? .0f : 1.f - (scale - 1.f));
}

ZoomOutSlideTransformer::ZoomOutSlideTransformer():ZoomOutSlideTransformer(.85f,.5f){
}

ZoomOutSlideTransformer::ZoomOutSlideTransformer(float minscale,float minalpha){
    mMinScale=minscale;
    mMinAlpha=minalpha;
}

void ZoomOutSlideTransformer::onTransform(View& view, float position) {
    if (position >= -1 || position <= 1) {
	// Modify the default slide transition to shrink the page as well
	const float height = view.getHeight();
	const float width = view.getWidth();
	const float scaleFactor = std::max(mMinScale, 1.f - std::abs(position));
	const float vertMargin = height * (1.f - scaleFactor) / 2.f;
	const float horzMargin = width * (1.f - scaleFactor) / 2.f;

	// Center vertically
	view.setPivotY(0.5f * height);
	view.setPivotX(0.5f * width);

	if (position < 0) {
	    view.setTranslationX(horzMargin - vertMargin / 2.f);
	} else {
	    view.setTranslationX(-horzMargin+ vertMargin / 2.f);
	}

	// Scale the page down (between MIN_SCALE and 1)
	view.setScaleX(scaleFactor);
	view.setScaleY(scaleFactor);

	// Fade the page relative to its size.
	view.setAlpha(mMinAlpha + (scaleFactor - mMinScale) / (1.f - mMinScale) * (1.f - mMinAlpha));
    }
}
}//endof namespace
