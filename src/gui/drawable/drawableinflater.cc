/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <drawable/drawables.h>
#include <drawable/animationscalelistdrawable.h>
#include <core/typedvalue.h>
#include <image-decoders/imagedecoder.h>  // ImageDecoder::createAsDrawable(Resources, id)
#include <utils/textutils.h>
namespace cdroid{
/**
 * Loads the drawable resource with the specified identifier (AOSP
 * DrawableInflater.loadDrawable(Resources res, int id, Theme theme)).
 *
 * @param res the resources from which the drawable is loaded
 * @param id the identifier of the drawable resource
 * @param theme the theme against which the drawable should be inflated, or
 *              {@code null} to not inflate against a theme
 * @return a drawable, or {@code null} if the drawable failed to load
 */
Drawable* DrawableInflater::loadDrawable(Resources& res, int id, const Resources::Theme* theme) {
    // AOSP: res.getValue(id, value, true) then loadDrawableForDensity(...).
    TypedValue value;
    if(!res.getValue(id, &value, true)) return nullptr;
    return loadDrawableForDensity(res, value, id, 0, theme);
}

/**
 * AOSP DrawableInflater.loadDrawableForDensity(Resources, TypedValue, id,
 * density, Theme) — the xml branch (createFromXmlForDensity role): open the
 * xml by id, walk to the root tag, inflate themed. Color ints and bitmap
 * files are routed by the caller (ResourcesImpl::getDrawableForDensity).
 */
Drawable* DrawableInflater::loadDrawableForDensity(Resources& res, const TypedValue& value,
        int id, int density, const Resources::Theme* theme) {
    if(value.type != TypedValue::TYPE_STRING) return nullptr;
    // AOSP split: file drawables decode through ImageDecoder
    // (createSource(Resources, id)); xml files inflate below.
    const std::string path = TextUtils::utf16_utf8(
            reinterpret_cast<const uint16_t*>(value.string), value.stringLen);
    if(path.find(".xml") == std::string::npos)
        return ImageDecoder::createAsDrawable(res, id);
    int type;
    auto parser = res.getXml(id);
    const AttributeSet& attrs = *parser;
    if(!*parser)return nullptr;
    while( ((type=parser->next())!=XmlPullParser::START_TAG) && (type!=XmlPullParser::END_DOCUMENT)){
        //NOTHING
    }
    // AOSP Drawable.createFromXml: "No start tag found" — an empty/truncated
    // document must not fall through with a garbage root name.
    if (type != XmlPullParser::START_TAG) {
        LOGE("%08x: no start tag found", id);
        return nullptr;
    }
    return inflateFromXmlForDensity(res,parser->getName(),*parser,attrs,density,theme);
}

Drawable* DrawableInflater::inflateFromXml(Resources& r,const std::string& name,XmlPullParser& parser,const AttributeSet& attrs){
    return inflateFromXmlForDensity(r, name, parser, attrs, 0);
}

Drawable* DrawableInflater::inflateFromXml(Resources& r,const std::string& name,XmlPullParser& parser,const AttributeSet& attrs,const Resources::Theme* theme){
    return inflateFromXmlForDensity(r, name, parser, attrs, 0, theme);
}

/**
 * Version of {@link #inflateFromXml(String, XmlPullParser, AttributeSet, Theme)} that accepts
 * an override density.
 */
Drawable* DrawableInflater::inflateFromXmlForDensity(Resources& r,const std::string& name,XmlPullParser& parser,const AttributeSet& attrs, int density){
    return inflateFromXmlForDensity(r, name, parser, attrs, density, nullptr);
}

Drawable* DrawableInflater::inflateFromXmlForDensity(Resources& r,const std::string& name,XmlPullParser& parser,const AttributeSet& attrs, int density,const Resources::Theme* theme){
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
    // AOSP inflateFromClass throws on an unknown tag; dereferencing a null
    // result crashed instead.
    if (drawable == nullptr) {
        LOGE("unknown drawable root tag <%s>",name.c_str());
        return nullptr;
    }
    /*if (drawable == nullptr) {
        drawable = inflateFromClass(name);
    }*/
    drawable->setSrcDensityOverride(density);
    try {
        drawable->inflate(r, parser, attrs, theme);
    } catch (...) {
        // AOSP lets GC reclaim the half-inflated drawable when inflate throws
        // ("no path defined", "<item> tag requires a drawable", ...); CDROID
        // owns it — free before rethrowing (valgrind: one VectorDrawable lost
        // on a caught inflate exception).
        delete drawable;
        throw;
    }
    return drawable;
}

static const std::unordered_map<std::string,std::function<Drawable*()>>drawableParsers={
    {"AnimationScaleListDrawable",[](){return new AnimationScaleListDrawable();}},
    {"com.android.internal.graphics.drawable.AnimationScaleListDrawable",
        [](){return new AnimationScaleListDrawable();}},
    {"animated-rotate", [](){return new AnimatedRotateDrawable();}},
    {"animation-list" , [](){return new AnimationDrawable();}},
    {"animated-image" , [](){return new AnimatedImageDrawable();}},
    {"animated-vector", [](){return new AnimatedVectorDrawable();}},
    {"animated-selector", [](){return new AnimatedStateListDrawable();}},
    {"adaptive-icon",[]{return new AdaptiveIconDrawable();}},
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
        Drawable*d = it->second();
        return d;
    }
    return nullptr;
}
}/*endof namespace*/
