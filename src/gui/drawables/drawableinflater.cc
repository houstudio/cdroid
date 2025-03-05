#include <drawables/drawableinflater.h>
#include <drawables/vectordrawable.h>
#include <drawables.h>
namespace cdroid{
/**
 * Loads the drawable resource with the specified identifier.
 *
 * @param context the context in which the drawable should be loaded
 * @param id the identifier of the drawable resource
 * @return a drawable, or {@code null} if the drawable failed to load
 */
Drawable* DrawableInflater::loadDrawable(Context* context, const std::string&id) {
    XmlPullParser parser(context,id);
    XmlPullParser::XmlEvent event;
    if(!parser)return nullptr;
    parser.next(event);
    return inflateFromXml(parser.getName(),parser,event.attributes);//loadDrawable(context, id);
}

Drawable* DrawableInflater::inflateFromXml(const std::string& name,XmlPullParser& parser,const AttributeSet& attrs){
    return inflateFromXmlForDensity(name, parser, attrs, 0);
}

/**
 * Version of {@link #inflateFromXml(String, XmlPullParser, AttributeSet, Theme)} that accepts
 * an override density.
 */
Drawable* DrawableInflater::inflateFromXmlForDensity(const std::string& name,XmlPullParser& parser,const AttributeSet& attrs, int density){
    // Inner classes must be referenced as Outer$Inner, but XML tag names
    // can't contain $, so the <drawable> tag allows developers to specify
    // the class in an attribute. We'll still run it through inflateFromTag
    // to stay consistent with how LayoutInflater works.
    /*if (name.compare("drawable")==0) {
        name = attrs.getAttributeValue(null, "class");
        if (name == null) {
            throw ("<drawable> tag must specify class attribute");
        }
    }*/

    Drawable* drawable = inflateFromTag(name);
    /*if (drawable == nullptr) {
        drawable = inflateFromClass(name);
    }*/
    //drawable->setSrcDensityOverride(density);
    drawable->inflate(parser, attrs);
    return drawable;
}

static const std::unordered_map<std::string,std::function<Drawable*()>>drawableParsers={
    {"animated-rotate", [](){return new AnimatedRotateDrawable();}},
    {"animation-list" , [](){return new AnimationDrawable();}},
    {"animated-image" , [](){return new AnimatedImageDrawable();}},
    //{"animated-vector", [](){return new AnimatedVectorDrawable();}},
    {"animated-selector", [](){return new AnimatedStateListDrawable();}},
    {"selector" , [](){return new StateListDrawable();}},
    {"level-list",[](){return new LevelListDrawable();}},
    {"layer-list",[](){return new LayerDrawable();}},
    {"transition",[](){return new TransitionDrawable();}},
    {"ripple",    [](){return new RippleDrawable();}},
    {"color" ,    [](){return new ColorDrawable(0);}},
    {"shape",     [](){return new GradientDrawable();}},
    {"vector",    [](){return new VectorDrawable();}},
    {"scale",     [](){return new ScaleDrawable();}},
    {"clip",      [](){return new ClipDrawable();}},
    {"rotate",    [](){return new RotateDrawable();}},
    {"inset",     [](){return new InsetDrawable();}},
    {"bitmap",    [](){return new BitmapDrawable();}},
    {"nine-patch",[](){return new NinePatchDrawable();}}
};

Drawable* DrawableInflater::inflateFromTag(const std::string& name) {
    auto it = drawableParsers.find(name);
    if(it!=drawableParsers.end()){
        auto func = it->second;
        return func();
    }
    return nullptr;
}
}/*endof namespace*/
