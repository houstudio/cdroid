#ifndef __BIT_ARRAY_H__
#define __BIT_ARRAY_H__
#include <bitset>
#include <array>
namespace cdroid{
template <std::size_t BITS>
class BitArray {
    using Element = std::uint32_t;
    static constexpr size_t WIDTH = sizeof(Element) * CHAR_BIT;
    static constexpr size_t COUNT = (BITS + WIDTH - 1) / WIDTH;
public:
    /* BUFFER type declaration for BitArray */
    using Buffer = std::array<Element, COUNT>;
    inline bool test(size_t bit) const {
        return (bit < BITS) && mData[bit / WIDTH].test(bit % WIDTH);
    }
    inline bool set(size_t bit, bool value) {
        if (bit >= BITS) {
            return false;
        }
        mData[bit / WIDTH].set(bit % WIDTH, value);
        return true;
    }
    inline size_t bytes() { return (BITS + CHAR_BIT - 1) / CHAR_BIT; }

    bool any(size_t startIndex, size_t endIndex) {
        if (startIndex >= endIndex || startIndex > BITS || endIndex > BITS + 1) {
            LOGE("Invalid start/end index. start = %zu, end = %zu, total bits = %zu", startIndex, endIndex, BITS);
            return false;
        }
        size_t se = startIndex / WIDTH; // Start of element
        size_t ee = endIndex / WIDTH;   // End of element
        size_t si = startIndex % WIDTH; // Start index in start element
        size_t ei = endIndex % WIDTH;   // End index in end element
        // Need to check first unaligned bitset for any non zero bit
        if (si > 0) {
            size_t nBits = se == ee ? ei - si : WIDTH - si;
            // Generate the mask of interested bit range
            Element mask = ((1 << nBits) - 1) << si;
            if (mData[se++].to_ulong() & mask) {
                return true;
            }
        }
        // Check whole bitset for any bit set
        for (; se < ee; se++) {
            if (mData[se].any()) {
                return true;
            }
        }
        // Need to check last unaligned bitset for any non zero bit
        if (ei > 0 && se <= ee) { // Generate the mask of interested bit range
            Element mask = (1 << ei) - 1;
            if (mData[se].to_ulong() & mask) {
                return true;
            }
        }
        return false;
    }
   void loadFromBuffer(const Buffer& buffer) {
        for (size_t i = 0; i < COUNT; i++) {
            mData[i] = std::bitset<WIDTH>(buffer[i]);
        }
    }
    inline std::string dumpSetIndices(std::string separator,
            std::function<std::string(size_t /*index*/)> format) {
        std::string dmp;
        for (size_t i = 0; i < BITS; i++) {
            if (test(i)) {
                if (!dmp.empty()) {
                    dmp += separator;
                }
                dmp += format(i);
            }
        }
        return dmp.empty() ? "<none>" : dmp;
    }
private:
    std::array<std::bitset<WIDTH>, COUNT> mData;
};
}/*endof namespace*/
#endif/*__BIT_ARRAY_H__*/
