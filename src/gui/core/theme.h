#ifndef __THEME_STYLE_H__
#define __THEME_STYLE_H__
#include <cdtypes.h>
#include <cairomm/context.h>
#include <string>
#include <list>
#include <map>
#include <core/attributeset.h>
using namespace Cairo;
namespace cdroid{

class Theme{
protected:
    std::map<std::string,AttributeSet>mStyles;
    static Theme*mInst;
public:
    static Theme&getInstance();
    int parseStyles(std::istream&s);
    const AttributeSet getStyle(const std::string&name)const;
};

}//namespace
#endif
