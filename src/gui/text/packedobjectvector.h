#ifndef __PACKED_OBJECT_VECTOR_H__
#define __PACKED_OBJECT_VECTOR_H__
namespace cdroid{
template<typename E>
class PackedObjectVector{
private:
    int mColumns;
    int mRows;
    int mRowGapStart;
    int mRowGapLength;
    std::vector<E> mValues;
public:
    PackedObjectVector(int columns){
        mColumns = columns;
        //mValues = EmptyArray.OBJECT;
        mRows = 0;

        mRowGapStart = 0;
        mRowGapLength = mRows;
    }

    E getValue(int row, int column){
        if (row >= mRowGapStart)
            row += mRowGapLength;
        E value = mValues[row * mColumns + column];
        return value;
    }

    void setValue(int row, int column, E value){
        if (row >= mRowGapStart)
            row += mRowGapLength;
        mValues[row * mColumns + column] = value;
    }

    void insertAt(int row, const std::vector<E>& values){
        moveRowGapTo(row);
        if (mRowGapLength == 0)
            growBuffer();
        mRowGapStart++;
        mRowGapLength--;

        if (values.empty())// == null)
            for (int i = 0; i < mColumns; i++)
                setValue(row, i, nullptr);
        else
            for (int i = 0; i < mColumns; i++)
                setValue(row, i, values[i]);
    }

    void deleteAt(int row, int count){
        moveRowGapTo(row + count);
        mRowGapStart -= count;
        mRowGapLength += count;
    }

    int size()const {
        return mRows - mRowGapLength;
    }

    int  width()const {
        return mColumns;
    }
private:
    int growSize(int currentSize) const{
        return currentSize <= 4 ? 8 : currentSize * 2;
    }
    void growBuffer(){
        const int newCapacity=growSize(size())*mColumns;
        const int newsize = newCapacity/ mColumns;
        mValues.resize(newCapacity);
        //Object[] newvalues = ArrayUtils.newUnpaddedObjectArray(GrowingArrayUtils.growSize(size()) * mColumns);
        const int after = mRows - (mRowGapStart + mRowGapLength);

        //System.arraycopy(mValues, 0, newvalues, 0, mColumns * mRowGapStart);
        //System.arraycopy(mValues, (mRows - after) * mColumns, newvalues, (newsize - after) * mColumns, after * mColumns);
        if(after>0){
            std::copy_backward(
                mValues.begin() + (mRows - after) * mColumns,
                mValues.begin() + mRows * mColumns,
                mValues.end());
        }
        mRowGapLength += newsize - mRows;
        mRows = newsize;
    }

    void  moveRowGapTo(int where){
        if (where == mRowGapStart)
            return;
        if (where > mRowGapStart){
            const int moving = where + mRowGapLength - (mRowGapStart + mRowGapLength);
            for (int i = mRowGapStart + mRowGapLength; i < mRowGapStart + mRowGapLength + moving; i++){
                const int destrow = i - (mRowGapStart + mRowGapLength) + mRowGapStart;
                for (int j = 0; j < mColumns; j++){
                    E val = mValues[i * mColumns + j];
                    mValues[destrow * mColumns + j] = val;
                }
            }
        } else {/* where < mRowGapStart */
            const int moving = mRowGapStart - where;
            for (int i = where + moving - 1; i >= where; i--){
                int destrow = i - where + mRowGapStart + mRowGapLength - moving;

                for (int j = 0; j < mColumns; j++){
                    E val = mValues[i * mColumns + j];
                    mValues[destrow * mColumns + j] = val;
                }
            }
        }

        mRowGapStart = where;
    }
};
}/*endof namespace*/
#endif/*__PACKED_OBJECT_VECTOR_H__*/
