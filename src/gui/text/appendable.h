#ifndef __APPENDABLE_H__
#define __APPENDABLE_H__

namespace cdroid {

class Appendable {
public:
    virtual ~Appendable() = default;
    virtual Appendable& append(char16_t c) = 0;
    virtual Appendable& append(const char16_t* s, int start, int len) = 0;
protected:
    Appendable() = default;
};

}/* namespace cdroid */
#endif/*__APPENDABLE_H__*/
