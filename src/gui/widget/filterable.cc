#include <widget/filterable.h>
namespace cdroid{

Filter::Filter(){
}

void Filter::filter(const std::string& constraint){
    filter(constraint, nullptr);
}

void Filter::filter(const std::string& constraint,const Filter::FilterListener& listener){
}

}//endof namespace
