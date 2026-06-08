#include "text/packedintvector.h"
#include <porting/cdlog.h>
namespace cdroid{

PackedIntVector::PackedIntVector(int columns) {
    mColumns = columns;
    mRows = 0;

    mRowGapStart = 0;
    mRowGapLength = mRows;

    mValueGap.resize(2 * columns);
}

int PackedIntVector::getValue(int row, int column) {
    const int columns = mColumns;

    if (((row | column) < 0) || (row >= size()) || (column >= columns)) {
        //throw new IndexOutOfBoundsException(row + ", " + column);
    }

    if (row >= mRowGapStart) {
        row += mRowGapLength;
    }

    int value = mValues[row * columns + column];

    auto& valuegap = mValueGap;
    if (row >= valuegap[column]) {
        value += valuegap[column + columns];
    }

    return value;
}

void PackedIntVector::setValue(int row, int column, int value) {
    if (((row | column) < 0) || (row >= size()) || (column >= mColumns)) {
        //throw new IndexOutOfBoundsException(row + ", " + column);
    }

    if (row >= mRowGapStart) {
        row += mRowGapLength;
    }

    auto& valuegap = mValueGap;
    if (row >= valuegap[column]) {
        value -= valuegap[column + mColumns];
    }

    mValues[row * mColumns + column] = value;
}

void PackedIntVector::setValueInternal(int row, int column, int value) {
    if (row >= mRowGapStart) {
        row += mRowGapLength;
    }

    auto& valuegap = mValueGap;
    if (row >= valuegap[column]) {
        value -= valuegap[column + mColumns];
    }

    mValues[row * mColumns + column] = value;
}


void PackedIntVector::adjustValuesBelow(int startRow, int column, int delta) {
    if (((startRow | column) < 0) || (startRow > size()) ||
            (column >= width())) {
        //throw new IndexOutOfBoundsException(startRow + ", " + column);
    }

    if (startRow >= mRowGapStart) {
        startRow += mRowGapLength;
    }

    moveValueGapTo(column, startRow);
    mValueGap[column + mColumns] += delta;
}

void PackedIntVector::insertAt(int row, const std::vector<int>& values) {
    if ((row < 0) || (row > size())) {
        //throw new IndexOutOfBoundsException("row " + row);
    }

    if (/*(values != null) &&*/ (values.size() < width())) {
        //throw new IndexOutOfBoundsException("value count " + values.length);
    }

    moveRowGapTo(row);

    if (mRowGapLength == 0) {
        growBuffer();
    }

    mRowGapStart++;
    mRowGapLength--;

    if (values.empty()){//values == null) {
        for (int i = mColumns - 1; i >= 0; i--) {
            setValueInternal(row, i, 0);
        }
    } else {
        for (int i = mColumns - 1; i >= 0; i--) {
            setValueInternal(row, i, values[i]);
        }
    }
}

void PackedIntVector::deleteAt(int row, int count) {
    if (((row | count) < 0) || (row + count > size())) {
        ///throw new IndexOutOfBoundsException(row + ", " + count);
    }

    moveRowGapTo(row + count);

    mRowGapStart -= count;
    mRowGapLength += count;

    // TODO: Reclaim memory when the new height is much smaller
    // than the allocated size.
}

int PackedIntVector::size() const{
    return mRows - mRowGapLength;
}

int PackedIntVector::width() const{
    return mColumns;
}

static int growSize(int currentSize) {
    return currentSize <= 4 ? 8 : currentSize * 2;
}

void PackedIntVector::growBuffer() {
    const int newCapacity = growSize(size()) * mColumns;
    auto& valuegap = mValueGap;
    const int rowgapstart = mRowGapStart;
    const int after = mRows - (rowgapstart + mRowGapLength);
    const int newsize = newCapacity / mColumns;
    mValues.resize(newCapacity);
    if (after > 0&& mColumns > 0) {
        std::copy_backward(
            mValues.begin() + (mRows - after) * mColumns,
            mValues.begin() + mRows * mColumns,
            mValues.begin() + newsize * mColumns
        );
    }
    for (int i = 0; i < mColumns; i++) {
        if (valuegap[i] >= rowgapstart) {
            valuegap[i] += newsize - mRows;
            if (valuegap[i] < rowgapstart) {
                valuegap[i] = rowgapstart;
            }
        }
    }
    mRowGapLength += newsize - mRows;
    mRows = newsize;
}

void PackedIntVector::moveValueGapTo(int column, int where) {
    auto&  valuegap = mValueGap;
    auto& values = mValues;
    const int columns = mColumns;

    if (where == valuegap[column]) {
        return;
    } else if (where > valuegap[column]) {
        for (int i = valuegap[column]; i < where; i++) {
            values[i * columns + column] += valuegap[column + columns];
        }
    } else /* where < valuegap[column] */ {
        for (int i = where; i < valuegap[column]; i++) {
            values[i * columns + column] -= valuegap[column + columns];
        }
    }

    valuegap[column] = where;
}

void PackedIntVector::moveRowGapTo(int where) {
    if (where == mRowGapStart) {
        return;
    } else if (where > mRowGapStart) {
        int moving = where + mRowGapLength - (mRowGapStart + mRowGapLength);
        const int columns = mColumns;
        auto& valuegap = mValueGap;
        auto& values = mValues;
        const int gapend = mRowGapStart + mRowGapLength;

        for (int i = gapend; i < gapend + moving; i++) {
            int destrow = i - gapend + mRowGapStart;

            for (int j = 0; j < columns; j++) {
                int val = values[i * columns+ j];

                if (i >= valuegap[j]) {
                    val += valuegap[j + columns];
                }

                if (destrow >= valuegap[j]) {
                    val -= valuegap[j + columns];
                }

                values[destrow * columns + j] = val;
            }
        }
    } else /* where < mRowGapStart */ {
        int moving = mRowGapStart - where;
        const int columns = mColumns;
        auto& valuegap = mValueGap;
        auto& values = mValues;
        const int gapend = mRowGapStart + mRowGapLength;

        for (int i = where + moving - 1; i >= where; i--) {
            int destrow = i - where + gapend - moving;

            for (int j = 0; j < columns; j++) {
                int val = values[i * columns+ j];

                if (i >= valuegap[j]) {
                    val += valuegap[j + columns];
                }

                if (destrow >= valuegap[j]) {
                    val -= valuegap[j + columns];
                }

                values[destrow * columns + j] = val;
            }
        }
    }

    mRowGapStart = where;
}
}/*endof namespace*/
