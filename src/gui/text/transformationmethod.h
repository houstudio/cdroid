#ifndef __TRANSFORMATION_METHOD_H__
#define __TRANSFORMATION_METHOD_H__
namespace cdroid{
class TransformationMethod{
public:
    virtual ~TransformationMethod()=default;
    /**
     * Returns a CharSequence that is a transformation of the source text --
     * for example, replacing each character with a dot in a password field.
     * Beware that the returned text must be exactly the same length as
     * the source text, and that if the source text is Editable, the returned
     * text must mirror it dynamically instead of doing a one-time copy.
     * The method should not return {@code null} unless {@code source}
     * is {@code null}.
     */
    virtual CharSequence* getTransformation(CharSequence& source, View& view)=0;

    /**
     * This method is called when the TextView that uses this
     * TransformationMethod gains or loses focus.
     */
    virtual void onFocusChanged(View& view, CharSequence& sourceText,bool focused,
            int direction, const Rect& previouslyFocusedRect)=0;
    //@UnsupportedAppUsage
    virtual void setLengthChangesAllowed(bool allowLengthChanges)=0;
};
}/*endof namespace*/
#endif/*__TRANSFORMATION_METHOD_H__*/

