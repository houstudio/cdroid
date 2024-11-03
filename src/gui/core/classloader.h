#ifndef __CLASS_LOADER_H__
#define __CLASS_LOADER_H__
#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>
#include <cstdarg>

namespace cdroid{

template <typename BaseClass, typename... Args>
class ClassLoader{
public:
    using CreateFunc = BaseClass*(*)(Args...);

    static ClassLoader& instance() {
        static ClassLoader theFactory;
        return theFactory;
    }

    bool registerClass(const std::string& className, CreateFunc func) {
        return classMap.emplace(className, func).second;
    }

    std::unique_ptr<BaseClass> create(const std::string& className, Args... args) {
        auto it = classMap.find(className);
        if (it != classMap.end()) {
            return (it->second)(std::forward<Args>(args)...);
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, CreateFunc> classMap;
};

#define REGISTER_CLASS(ClassName, BaseClass, ...) \
  static BaseClass* create##ClassName(__VA_ARGS__) { \
      return dynamic_cast<BaseClass*>(new ClassName(__VA_ARGS__)); } \
    static bool register##ClassName = ClassLoader<BaseClass, ##__VA_ARGS__>::instance().registerClass(#ClassName, create##ClassName);
}
#endif/*__CLASS_LOADER__*/
