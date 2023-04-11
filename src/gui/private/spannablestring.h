#pragma once
namespace cdroid{
//https://github.com/jaychang0917/SimpleText
class SpannableString{
public:
    SpannableString(const std::string&){};
    void setSpan(Object what, int start, int end, int flags);
    SpannableString*subSequence(int start, int end);
    static SpannableString valueOf(const std::string&);
};

struct Range{
    int from;
    int to;
};

}
