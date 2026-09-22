// Port of SupportWearDemos RoundedDrawableDemo. Upstream is layout-only
// (rd_demo.xml's ImageView sources @drawable/rd_example, a RoundedDrawable
// drawable-XML wrapping the walk-glyph vector); CDROID's DrawableInflater
// registry has no RoundedDrawable entry yet, so the drawable is built here
// in code with the same three parameters the upstream XML carries
// (src=@drawable/rd_inner_drawable, backgroundColor=@color/rd_background_color,
// radius=@dimen/rd_image_view_half_edge).
#include <core/app.h>
#include <cdroid.h>
#include <core/activityfactory.h>
#include <widget/imageview.h>
#include <widgetEx/wear/roundeddrawable.h>
#include "R.h"

using namespace cdroid;

class RoundedDrawableDemo : public Window {
public:
    RoundedDrawableDemo() : Window(&App::getInstance(), 0, 0, -1, -1) {
        ViewGroup* root = (ViewGroup*)LayoutInflater::from(getContext())
                ->inflate(weardemos::R::layout::rd_demo, this, false);
        addView(root);

        ImageView* image = (ImageView*)root->findViewById(weardemos::R::id::rd_image);
        RoundedDrawable* rounded = new RoundedDrawable();
        rounded->setDrawable(getContext()->getDrawable(weardemos::R::drawable::rd_inner_drawable));
        rounded->setBackgroundColor(getContext()->getColor(weardemos::R::color::rd_background_color));
        rounded->setRadius(getContext()->getDimensionPixelSize(
                weardemos::R::dimen::rd_image_view_half_edge));
        image->setImageDrawable(rounded);
    }
};
REGISTER_ACTIVITY(RoundedDrawableDemo);
