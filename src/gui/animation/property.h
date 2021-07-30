#pragma once
#include <string>
#include <vector>

namespace cdroid{

class Property{
private:
    std::string mName;
public:
    Property(const std::string&name){
        mName=name;
    }
    virtual float get(void* t){return .0;};
    virtual void set(void* object, float value){};
    const std::string getName(){return mName;}
};

}
