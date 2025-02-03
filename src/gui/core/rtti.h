#ifndef __CDROID_RTTI_H__
#define __CDROID_RTTI_H__

#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>
#include <memory>

// 检测是否启用了RTTI
#ifdef __GXX_RTTI
    #define RTTI_ENABLED 1
#elif defined(_CPPRTTI)//MSVC
    #define RTTI_ENABLED 1
#else
    #define RTTI_ENABLED 0
#endif
namespace cdroid{

class TypeInfo {
public:
    TypeInfo(const std::string& name) : name_(name) {}
    ~TypeInfo() {
        for (auto base : bases_) {delete base;}
    }
    const std::string& name() const { return name_; }

    void addBase(TypeInfo* base) {
        bases_.push_back(base);
    }

    bool isDerivedFrom(const TypeInfo* base) const {
        if (this == base) return true;
        for (const auto& b : bases_) {
            if (b->isDerivedFrom(base)) return true;
        }
        return false;
    }

private:
    std::string name_;
    std::vector<TypeInfo*> bases_;
};

class RTTIObject {
public:
    virtual ~RTTIObject() = default;
    virtual TypeInfo* getTypeInfo() const = 0;
};

#define RTTI_DECLARE() \
public: \
    static TypeInfo typeInfo; \
    virtual TypeInfo* getTypeInfo() const override { return &typeInfo; } \
    static void initializeTypeInfo();

#define RTTI_IMPLEMENT(type, ...) \
TypeInfo type::typeInfo(#type); \
void type::initializeTypeInfo() { \
    (typeInfo.addBase(&__VA_ARGS__::typeInfo), ...); \
}

template <typename Target, typename Source>
Target* rtti_cast(Source* source) {
    if (source && source->getTypeInfo()->isDerivedFrom(&Target::typeInfo)) {
        return static_cast<Target>(source);
    }
    return nullptr;
}

template <typename Target, typename Source>
Target& rtti_cast(Source& source) {
    if (source.getTypeInfo()->isDerivedFrom(&Target::typeInfo)) {
        return static_cast<Target&>(source);
    }
    throw std::bad_cast();
}
#if 0
class Base1 : public RTTIObject {
    RTTI_DECLARE()
};

class Base2 : public RTTIObject {
    RTTI_DECLARE()
};

class Derived : public Base1, public Base2 {
    RTTI_DECLARE()
};

RTTI_IMPLEMENT(Base1)
RTTI_IMPLEMENT(Base2)
RTTI_IMPLEMENT(Derived, Base1, Base2)

int main() {
    Derived::initializeTypeInfo();
    Base1* b1 = new Derived();
    Base2* b2 = rtti_cast<Base2*>(b1);

    if (b2) {
        std::cout << "Cast successful: " << b2->getTypeInfo()->name() << std::endl;
    } else {
        std::cout << "Cast failed" << std::endl;
    }

    delete b1;
    return 0;
}
#endif
}/*endof namespace*/
#endif/*__CDROID_RTTI_H__*/
