#pragma once
#include <widget/textview.h>
#include <spannablestring.h>

namespace cdroid{

class SimpleText:public SpannableString{
private:
    std::list<Range>rangeList;
    std::map<Range,void*>tagsMap;
    Context*context;
    int textColor;
    int pressedTextColor;
    int pressedBackgroundRadius;
public:
    SimpleText(Context*context,const std::string&);
    static SimpleText* from(const std::string&);
    SimpleText& first(const std::string&);
    SimpleText& last(const std::string&);
    SimpleText& all(const std::string&);
    SimpleText& all();
    SimpleText& allStartWith(const std::vector<std::string>&prefixs);
    SimpleText& range(int from, int to);
    SimpleText& ranges(const std::vector<Range>&ranges);//List<Range> ranges);
    SimpleText between(const std::string& startText,const std::string& endText);
};

}
