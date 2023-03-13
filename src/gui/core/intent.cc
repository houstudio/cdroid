#include <core/intent.h>
#include <widget/cdwindow.h>

namespace cdroid{

Intent::Intent(const std::string&action){
    mAction=action;
}

bool Intent::operator==(const Intent&other)const{
    return mAction == other.mAction;
}

}
