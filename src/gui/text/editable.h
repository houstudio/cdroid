#ifndef __EDITABLE_H__
#define __EDITABLE_H__
namespace cdroid{
class CharSequence;
class Editable:virtual public CharSequence, virtual public Spannable{
public:
    virtual Editable& replace(int st, int en,const CharSequence& source, int start, int end)=0;

    /**
     * Convenience for replace(st, en, text, 0, text.length())
     * @see #replace(int, int, CharSequence, int, int)
     */
    virtual Editable& replace(int st, int en,const CharSequence& text)=0;

    /**
     * Convenience for replace(where, where, text, start, end)
     * @see #replace(int, int, CharSequence, int, int)
     */
    virtual Editable& insert(int where, const CharSequence& text, int start, int end)=0;

    /**
     * Convenience for replace(where, where, text, 0, text.length());
     * @see #replace(int, int, CharSequence, int, int)
     */
    virtual Editable& insert(int where, const CharSequence& text)=0;

    /**
     * Convenience for replace(st, en, "", 0, 0)
     * @see #replace(int, int, CharSequence, int, int)
     */
    virtual Editable& Delete(int st, int en)=0;

    /**
     * Convenience for replace(length(), length(), text, 0, text.length())
     * @see #replace(int, int, CharSequence, int, int)
     */
    virtual Editable& append(const CharSequence& text)=0;

    /**
     * Convenience for replace(length(), length(), text, start, end)
     * @see #replace(int, int, CharSequence, int, int)
     */
    virtual Editable& append(const CharSequence& text, int start, int end)=0;

    /**
     * Convenience for append(String.valueOf(text)).
     * @see #replace(int, int, CharSequence, int, int)
     */
    virtual Editable& append(char16_t text)=0;

    /**
     * Convenience for replace(0, length(), "", 0, 0).
     * Note that this clears the text, not the spans;
     * use {@link #clearSpans} if you need that.
     * @see #replace(int, int, CharSequence, int, int)
     */
    virtual void clear()=0;

    /**
     * Removes all spans from the Editable, as if by calling
     * {@link #removeSpan} on each of them.
     */
    virtual void clearSpans()=0;
#if 0
    /**
     * Sets the series of filters that will be called in succession
     * whenever the text of this Editable is changed, each of which has
     * the opportunity to limit or transform the text that is being inserted.
     */
    void setFilters(InputFilter[] filters);

    /**
     * Returns the array of input filters that are currently applied
     * to changes to this Editable.
     */
    InputFilter[] getFilters();

    /**
     * Factory used by TextView to create new {@link Editable Editables}. You can subclass
     * it to provide something other than {@link SpannableStringBuilder}.
     *
     * @see android.widget.TextView#setEditableFactory(Factory)
     */
    static class Factory {
        private static Editable.Factory sInstance = new Editable.Factory();

        /**
         * Returns the standard Editable Factory.
         */
        public static Editable.Factory getInstance() {
            return sInstance;
        }

        /**
         * Returns a new SpannableStringBuilder from the specified
         * CharSequence.  You can override this to provide
         * a different kind of Spanned.
         */
        public Editable newEditable(CharSequence source) {
            return new SpannableStringBuilder(source);
        }
    };
#endif
};
}/*endof namespace*/
#endif/*__EDITABLE_H__*/
