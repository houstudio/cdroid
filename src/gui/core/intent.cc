#include <core/intent.h>
#include <widget/cdwindow.h>

namespace cdroid{

std::map<std::string,Intent::WindowFactory>Intent::mIntentMaps;

Intent::Intent(const std::string&action){
    mAction=action;
}

Intent::Intent(const Intent&){
}

bool Intent::operator==(const Intent&other)const{
    return mAction == other.mAction;
}

int Intent::registIntent(const std::string&intentName,WindowFactory factory){
    return 0;
}

}
