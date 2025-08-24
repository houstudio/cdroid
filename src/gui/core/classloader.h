/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __CLASS_LOADER_H__
#define __CLASS_LOADER_H__
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#ifdef _MSC_VER
#include <utility>
#endif
namespace cdroid{
template<typename BaseClass, typename... Args>
class ClassLoader {
private:
    std::unordered_map<std::string, std::function<BaseClass*(Args...)>> factory_map;
    ClassLoader() = default;
public:
    static ClassLoader& instance() {
        static ClassLoader instance;
        return instance;
    }

    template<typename DerivedClass>
    bool registerClass(const std::string& name, std::function<BaseClass*(Args...)> factory) {
        return factory_map.emplace(name, factory).second;
    }

    std::unique_ptr<BaseClass> create(const std::string& name, Args... args) {
        auto it = factory_map.find(name);
        if (it != factory_map.end()) {
            return std::unique_ptr<BaseClass>((it->second)(std::forward<Args>(args)...));
        }
        return nullptr;
    }

    bool isRegistered(const std::string& name) const {
        return factory_map.find(name) != factory_map.end();
    }
};

#define REGISTER_CLASS_NOARG(ClassName, BaseClass) \
    static BaseClass* create##ClassName##Factory() { \
        return new ClassName(); \
    } \
    static const bool registered##ClassName = cdroid::ClassLoader<BaseClass>::instance().template registerClass<ClassName>(#ClassName, create##ClassName##Factory);

#define REGISTER_CLASS_1ARG(ClassName, BaseClass, ArgType) \
    static BaseClass* create##ClassName##Factory(ArgType arg) { \
        return new ClassName(arg); \
    } \
    static const bool registered##ClassName = cdroid::ClassLoader<BaseClass, ArgType>::instance().template registerClass<ClassName>(#ClassName, create##ClassName##Factory);

#define REGISTER_CLASS_2ARG(ClassName, BaseClass, ArgType1, ArgType2) \
    static BaseClass* create##ClassName##Factory(ArgType1 arg1, ArgType2 arg2) { \
        return new ClassName(arg1, arg2); \
    } \
    static const bool registered##ClassName = cdroid::ClassLoader<BaseClass, ArgType1, ArgType2>::instance().template registerClass<ClassName>(#ClassName, create##ClassName##Factory);

#define REGISTER_CLASS_3ARG(ClassName, BaseClass, ArgType1, ArgType2, ArgType3) \
    static BaseClass* create##ClassName##Factory(ArgType1 arg1, ArgType2 arg2, ArgType3 arg3) { \
        return new ClassName(arg1, arg2, arg3); \
    } \
    static const bool registered##ClassName = cdroid::ClassLoader<BaseClass, ArgType1, ArgType2, ArgType3>::instance().template registerClass<ClassName>(#ClassName, create##ClassName##Factory);

#define REGISTER_CLASS_FULL(ClassName, BaseClass, FactoryName, ...) \
    static BaseClass* create##FactoryName(__VA_ARGS__) { \
        return new ClassName(__VA_ARGS__); \
    } \
    static const bool registered##FactoryName = cdroid::ClassLoader<BaseClass, ##__VA_ARGS__>::instance().template registerClass<ClassName>(#ClassName, create##FactoryName);

#define DECLARE_MYWIDGET(ClassName)\
    REGISTER_CLASS_2ARG(ClassName,View,Context*,const AttributeSet&)

} /*namespace cdroid*/
#endif/*__CLASS_LOADER_H__*/
