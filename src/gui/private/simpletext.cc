#include <simpletext.h>

namespace cdroid{

SimpleText::SimpleText(Context*context,const std::string&text)
   :SpannableString(text){
    this->context = context;   
}

SimpleText* SimpleText::from(const std::string& text) {
    return new SimpleText(/*ContextProvider.context*/nullptr, text);
}

static std::string toString(){
    return std::string("");
}

SimpleText& SimpleText::first(const std::string& target) {
    rangeList.clear();
    int index = toString().find_first_of(target);//indexOf(target);
    Range range = {index, index + target.length()};
    rangeList.push_back(range);
    return *this;
}

SimpleText& SimpleText::last(const std::string& target) {
    rangeList.clear();
    int index = toString().find_last_of(target);//lastIndexOf(target);
    Range range = {index, index + target.length()};
    rangeList.push_back(range);
    return *this;
}

SimpleText& SimpleText::all(const std::string& target) {
    rangeList.clear();
    size_t pos=0;
    std::list<int> indexes;
    while(pos!=std::string::npos){
        pos = toString().find(target,pos);//Utils.indexesOf(toString(), target);
        if(pos!=std::string::npos)
           indexes.push_back(pos);
    }
    for (int index : indexes) {
	Range range = {index, index + target.length()};
        rangeList.push_back(range);
    }
    return *this;
}

SimpleText& SimpleText::all() {
    rangeList.clear();
    Range range = {0, toString().length()};
    rangeList.push_back(range);
    return *this;
}

SimpleText& SimpleText::allStartWith(const std::vector<std::string>& prefixs) {
    rangeList.clear();
    for (auto prefix : prefixs) {
	std::list<Range> ranges;// = Utils.ranges(toString(), Pattern.quote(prefix) + "\\w+");
        //rangeList.addAll(ranges);
	rangeList.insert(rangeList.end(),ranges.begin(),ranges.end());
    }
    return *this;
}

SimpleText& SimpleText::range(int from, int to) {
    rangeList.clear();
    Range range = {from,to+1};
    rangeList.push_back(range);
    return *this;
}

SimpleText& SimpleText::ranges(const std::vector<Range>& ranges) {
    rangeList.clear();
    //rangeList.addAll(ranges);
    rangeList.insert(rangeList.end(), ranges.begin(), ranges.end());
    return *this;
}

struct hash_function{
    bool operator ()(const Range &c1, const Range &c2) const{
        if (c1.from != c2.from){
            return c1.from < c2.from;
        }
        if (c1.to != c2.to){
            return c1.to < c2.to;
        }
        return false;
    }
};

}/*endof namespace*/
