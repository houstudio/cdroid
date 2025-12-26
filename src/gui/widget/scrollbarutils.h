#ifndef __SCROLLBAR_UTILS_H__
#define __SCROLLBAR_UTILS_H__
namespace cdroid{
class ScrollBarUtils {
public:
    static int getThumbLength(int size, int thickness, int extent, int range) {
        // Avoid the tiny thumb.
        const int minLength = thickness * 2;
        int length = std::round((float) size * extent / range);
        if (length < minLength) {
            length = minLength;
        }
        return length;
    }

    static int getThumbOffset(int size, int thumbLength, int extent, int range, int offset) {
        // Avoid the too-big thumb.
        int thumbOffset = std::round((float) (size - thumbLength) * offset / (range - extent));
        if (thumbOffset > size - thumbLength) {
            thumbOffset = size - thumbLength;
        }
        return thumbOffset;
    }
};
}/*endof namespace*/
#endif
