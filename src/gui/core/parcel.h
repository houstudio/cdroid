#ifndef __PARCEL_H__
#define __PARCEL_H__
namespace cdroid{
class Parcelable;
class Parcel{
private:
    static constexpr int POOL_SIZE = 6;
    //static constexpr Parcel *sOwnedPool;//[POOL_SIZE];
    //static constexpr Parcel *sHolderPool;//[POOL_SIZE];

    // Keep in sync with frameworks/native/include/private/binder/ParcelValTypes.h.
    static constexpr int VAL_NULL = -1;
    static constexpr int VAL_STRING = 0;
    static constexpr int VAL_INTEGER = 1;
    static constexpr int VAL_MAP = 2;
    static constexpr int VAL_BUNDLE = 3;
    static constexpr int VAL_PARCELABLE = 4;
    static constexpr int VAL_SHORT = 5;
    static constexpr int VAL_LONG = 6;
    static constexpr int VAL_FLOAT = 7;
    static constexpr int VAL_DOUBLE = 8;
    static constexpr int VAL_BOOLEAN = 9;
    static constexpr int VAL_CHARSEQUENCE = 10;
    static constexpr int VAL_LIST  = 11;
    static constexpr int VAL_SPARSEARRAY = 12;
    static constexpr int VAL_BYTEARRAY = 13;
    static constexpr int VAL_STRINGARRAY = 14;
    static constexpr int VAL_IBINDER = 15;
    static constexpr int VAL_PARCELABLEARRAY = 16;
    static constexpr int VAL_OBJECTARRAY = 17;
    static constexpr int VAL_INTARRAY = 18;
    static constexpr int VAL_LONGARRAY = 19;
    static constexpr int VAL_BYTE = 20;
    static constexpr int VAL_SERIALIZABLE = 21;
    static constexpr int VAL_SPARSEBOOLEANARRAY = 22;
    static constexpr int VAL_BOOLEANARRAY = 23;
    static constexpr int VAL_CHARSEQUENCEARRAY = 24;
    static constexpr int VAL_PERSISTABLEBUNDLE = 25;
    static constexpr int VAL_SIZE = 26;
    static constexpr int VAL_SIZEF = 27;
    static constexpr int VAL_DOUBLEARRAY = 28;

    // The initial int32 in a Binder call's reply Parcel header:
    // Keep these in sync with libbinder's binder/Status.h.
    static constexpr int EX_SECURITY = -1;
    static constexpr int EX_BAD_PARCELABLE = -2;
    static constexpr int EX_ILLEGAL_ARGUMENT = -3;
    static constexpr int EX_NULL_POINTER = -4;
    static constexpr int EX_ILLEGAL_STATE = -5;
    static constexpr int EX_NETWORK_MAIN_THREAD = -6;
    static constexpr int EX_UNSUPPORTED_OPERATION = -7;
    static constexpr int EX_SERVICE_SPECIFIC = -8;
    static constexpr int EX_PARCELABLE = -9;
    static constexpr int EX_HAS_REPLY_HEADER = -128;  // special; see below
    // EX_TRANSACTION_FAILED is used exclusively in native code.
    // see libbinder's binder/Status.h
    static constexpr int EX_TRANSACTION_FAILED = -129;
private:

public:
    static Parcel* obtain();
    void recycle();
    uint8_t readByte();
    int readInt();
    bool readBoolean();
    float readFloat();
    double readDouble();
    std::string readString();

    void writeByte(uint8_t val);
    void writeInt(int32_t val);
    void writeBoolean(bool);
    void writeFloat(float val);
    void writeDouble(double val);
    void writeString(const std::string& val);
    void writeParcelable(Parcelable*p,int parcelableFlags);
};
}/*endof namespace*/
#endif/*__PARCEL_H__*/
