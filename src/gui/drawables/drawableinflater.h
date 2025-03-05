#ifndef __DRAWABLE_INFLATER_H__
#define __DRAWABLE_INFLATER_H__
#include <core/context.h>
namespace cdroid{
class XmlPullParser;
class DrawableInflater {
private:
    static Drawable* inflateFromTag(const std::string& name);
public:
    /**
     * Loads the drawable resource with the specified identifier.
     *
     * @param context the context in which the drawable should be loaded
     * @param id the identifier of the drawable resource
     * @return a drawable, or {@code null} if the drawable failed to load
     */
    static Drawable* loadDrawable(Context* context, const std::string&id);

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
    static Drawable* inflateFromXml(const std::string& name,XmlPullParser& parser,const AttributeSet& attrs);

    /**
     * Version of {@link #inflateFromXml(String, XmlPullParser, AttributeSet, Theme)} that accepts
     * an override density.
     */
    static Drawable* inflateFromXmlForDensity(const std::string& name,XmlPullParser& parser,const AttributeSet& attrs, int density);
};
}/*endof namspace*/
#endif/*__DRAWABLE_INFLATER_H__*/
