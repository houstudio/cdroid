#ifndef __PARCELABLE_H__
#define __PARCELABLE_H__
class Parcel;
namespace cdroid{
class Parcelable {
public:

    /**
     * Flag for use with {@link #writeToParcel}: the object being written
     * is a return value, that is the result of a function such as
     * "<code>Parcelable someFunction()</code>",
     * "<code>void someFunction(out Parcelable)</code>", or
     * "<code>void someFunction(inout Parcelable)</code>".  Some implementations
     * may want to release resources at this point.
     */
    static constexpr int PARCELABLE_WRITE_RETURN_VALUE = 0x0001;

    /**
     * Flag for use with {@link #writeToParcel}: a parent object will take
     * care of managing duplicate state/data that is nominally replicated
     * across its inner data members.  This flag instructs the inner data
     * types to omit that data during marshaling.  Exact behavior may vary
     * on a case by case basis.
     * @hide
     */
    static constexpr int PARCELABLE_ELIDE_DUPLICATES = 0x0002;

    /*
     * Bit masks for use with {@link #describeContents}: each bit represents a
     * kind of object considered to have potential special significance when
     * marshalled.
     */


    /**
     * Descriptor bit used with {@link #describeContents()}: indicates that
     * the Parcelable object's flattened representation includes a file descriptor.
     *
     * @see #describeContents()
     */
    static constexpr int CONTENTS_FILE_DESCRIPTOR = 0x0001;

    /**
     * Describe the kinds of special objects contained in this Parcelable
     * instance's marshaled representation. For example, if the object will
     * include a file descriptor in the output of {@link #writeToParcel(Parcel, int)},
     * the return value of this method must include the
     * {@link #CONTENTS_FILE_DESCRIPTOR} bit.
     *
     * @return a bitmask indicating the set of special object types marshaled
     * by this Parcelable object instance.
     */
public:
    int describeContents();

    /**
     * Flatten this object in to a Parcel.
     *
     * @param dest The Parcel in which the object should be written.
     * @param flags Additional flags about how the object should be written.
     * May be 0 or {@link #PARCELABLE_WRITE_RETURN_VALUE}.
     */
    virtual void writeToParcel(Parcel& dest,int flags){};
};
}/*endof namespace*/
#endif
