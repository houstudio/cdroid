#ifndef __GET_CHARS_H__
#define __GET_CHARS_H__

namespace cdroid {

class GetChars {
public:
    virtual ~GetChars() = default;
    virtual void getChars(int start, int end, char16_t* dest, int destPos) const = 0;
};

}/* namespace cdroid */
#endif/*__GET_CHARS_H__*/
