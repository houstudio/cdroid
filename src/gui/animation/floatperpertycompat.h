#ifndef __DYNAMIC_ANIMATION_FLOAT_PROPERTYCOMPAT_H__
#define __DYNAMIC_ANIMATION_FLOAT_PROPERTYCOMPAT_H__
namespace cdroid{
class FloatPropertyCompat {
protected:
    std::string mPropertyName;
public:
    /**
     * A constructor that takes an identifying name.
     */
    FloatPropertyCompat(const std::string& name) {
        mPropertyName = name;
    }

    /**
     * Create a {@link FloatPropertyCompat} wrapper for a {@link FloatProperty} object. The new
     * {@link FloatPropertyCompat} instance will access and modify the property value of
     * {@link FloatProperty} through the {@link FloatProperty} instance's setter and getter.
     *
     * @param property FloatProperty instance to be wrapped
     * @param <T> the class on which the Property is declared
     * @return a new {@link FloatPropertyCompat} wrapper for the given {@link FloatProperty} object
     */
    static FloatProperty* createFloatPropertyCompat(FloatProperty* property) {
        return nullptr;/*new FloatPropertyCompat<T>(property->getName()) {
            public float getValue(void* object) {
                return property->get(object);
            }

            public void setValue(void* object, float value) {
                property->setValue(object, value);
            }
        }*/
    }

    /**
     * Returns the current value that this property represents on the given <code>object</code>.
     *
     * @param object object which this property represents
     * @return the current property value of the given object
     */
    virtual float getValue(void* object)=0;

    /**
     * Sets the value on <code>object</code> which this property represents.
     *
     * @param object object which this property represents
     * @param value new value of the property
     */
    virtual void setValue(void* object, float value)=0;
};
}/*endof namespace*/
#endif/*__DYNAMIC_ANIMATION_FLOAT_PROPERTY_H__*/
