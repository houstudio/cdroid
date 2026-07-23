#ifndef __PACKED_INT_VECTOR_H__
#define __PACKED_INT_VECTOR_H__
#include <vector>
namespace cdroid{
class PackedIntVector {
private:
    int mColumns;
    int mRows;
    int mRowGapStart;
    int mRowGapLength;
    std::vector<int> mValues;
    std::vector<int> mValueGap; // starts, followed by lengths
private:
    void setValueInternal(int row, int column, int value);
    void growBuffer();
    void moveValueGapTo(int column, int where);
    void moveRowGapTo(int where);
public:
    PackedIntVector(int columns);

    int getValue(int row, int column);

    void setValue(int row, int column, int value);

    void adjustValuesBelow(int startRow, int column, int delta);

    void insertAt(int row, const std::vector<int>& values);

    void deleteAt(int row, int count);

    int size() const;

    int width() const;
};
}/*endof namespace*/
#endif/*__PACKED_INT_VECTOR_H__*/
