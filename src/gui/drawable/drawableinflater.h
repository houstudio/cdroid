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
#ifndef __DRAWABLE_INFLATER_H__
#define __DRAWABLE_INFLATER_H__
#include <core/context.h>
#include <core/resources.h>   // Resources::Theme (themed inflation)
namespace cdroid{
class XmlPullParser;
class DrawableInflater {
private:
    static Drawable* inflateFromTag(const std::string& name);
public:
    /**
     * Loads the drawable resource with the specified identifier.
     *
     * <p>AOSP DrawableInflater.loadDrawable(Resources res, int id, Theme theme):
     * resolves the value by id and dispatches to the xml inflater. Color ints
     * and bitmap files stay with the caller (ResourcesImpl::
     * getDrawableForDensity — CDROID's ImageDecoder takes a Context*, AOSP's
     * goes through AssetManager streams).
     *
     * @param res the resources from which the drawable is loaded
     * @param id the identifier of the drawable resource
     * @param theme the theme against which the drawable should be inflated, or
     *              {@code null} to not inflate against a theme
     * @return a drawable, or {@code null} if the drawable failed to load
     */
    static Drawable* loadDrawable(Resources& res, int id, const Resources::Theme* theme = nullptr);

    /**
     * Version of {@link #loadDrawable(Resources, int, Theme)} that accepts a
     * resolved TypedValue plus an override density (AOSP
     * DrawableInflater.loadDrawableForDensity(Resources, TypedValue, int, int, Theme)).
     */
    static Drawable* loadDrawableForDensity(Resources& res, const TypedValue& value,
            int id, int density, const Resources::Theme* theme = nullptr);

    /**
     * Inflates a drawable from inside an XML document using an optional
     * {@link Theme}.
     * <p>
     * This method should be called on a parser positioned at a tag in an XML
     * document defining a drawable resource. It will attempt to create a
     * Drawable from the tag at the current position.
     *
     * @param name the name of the tag at the current position
     * @param parser an XML parser positioned at the drawable tag
     * @param attrs an attribute set that wraps the parser
     * @param theme the theme against which the drawable should be inflated, or
     *              {@code null} to not inflate against a theme
     * @return a drawable
     *
     * @throws XmlPullParserException
     * @throws IOException
     */
    static Drawable* inflateFromXml(Resources& r,const std::string& name,XmlPullParser& parser,const AttributeSet& attrs);

    /**
     * Version of {@link #inflateFromXml(String, XmlPullParser, AttributeSet)} that accepts
     * a theme against which to inflate the drawable (AOSP @Nullable Theme).
     */
    static Drawable* inflateFromXml(Resources& r,const std::string& name,XmlPullParser& parser,const AttributeSet& attrs,const Resources::Theme* theme);

    /**
     * Version of {@link #inflateFromXml(String, XmlPullParser, AttributeSet, Theme)} that accepts
     * an override density.
     */
    static Drawable* inflateFromXmlForDensity(Resources& r,const std::string& name,XmlPullParser& parser,const AttributeSet& attrs, int density);

    /**
     * Version of {@link #inflateFromXml(String, XmlPullParser, AttributeSet, Theme)} that accepts
     * an override density.
     */
    static Drawable* inflateFromXmlForDensity(Resources& r,const std::string& name,XmlPullParser& parser,const AttributeSet& attrs, int density,const Resources::Theme* theme);
};
}/*endof namspace*/
#endif/*__DRAWABLE_INFLATER_H__*/
