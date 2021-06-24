#ifndef __THEME_STYLE_H__
#define __THEME_STYLE_H__
#include <cdtypes.h>
#include <cairomm/context.h>
#include <string>
#include <list>
#include <map>

using namespace Cairo;
namespace cdroid{

class StylePattern{
public:
    std::string name;
    int states[8];
    RefPtr<Pattern>pat;
public:
	StylePattern();
    void parseStates(const char**atts);
	bool match(int *st)const;
};
	
class Style{
friend class Theme;
protected:
    std::string name;
    Style*mParent;
    std::list<const StylePattern*>mPatterns;
    std::map<std::string,int>mProps;/*fontsize,alignment...*/
public:
	enum{
		ENABLED =0x1,
		FOCUSED =0x2,
		SELECTED=0x4,
		PRESSED =0x8,
		HOVERED =0x10,
	};
    Style(const std::string&name);
	~Style();
	void addPattern(const StylePattern*p){mPatterns.push_back(p);}
    RefPtr<const Pattern>getPattern(const std::string&,int*)const;
    int getProp(const std::string&name,int)const;
};

class Theme{
protected:
    std::map<std::string,RefPtr<Style>>mStyles;
    static Theme*mInst;
public:
    static Theme&getInstance();
    int parseStyles(std::istream&s);
    void addStyle(const std::string&name,Style*);
	Style*getStyle(const std::string&name)const;
};

}//namespace
#endif
