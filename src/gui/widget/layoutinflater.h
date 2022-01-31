#ifndef __LAYOUT_INFLATE_H__
#define __LAYOUT_INFLATE_H__
#include <core/attributeset.h>
#include <core/attributeset.h>
#include <core/context.h>
namespace cdroid{
class View;
class ViewGroup;
class LayoutInflater{
public:
    typedef std::function<View*(Context*ctx, const AttributeSet&attrs)>ViewInflater;
private:
    Context*mContext;
    LayoutInflater(Context*ctx);
    static std::map<const std::string,ViewInflater>&getMap();
protected:
    View* inflate(std::istream&stream,ViewGroup*root,bool attachToRoot);
public:
    static LayoutInflater*from(Context*context);
    static ViewInflater getInflater(const std::string&);
    static bool registInflater(const std::string&name,ViewInflater fun);
    View* inflate(const std::string&resource,ViewGroup* root, bool attachToRoot=true);
};

template<typename T>
class InflaterRegister{
public:
    InflaterRegister(const std::string&name){
        LayoutInflater::registInflater(name,[](Context*ctx,const AttributeSet&attr)->View*{return new T(ctx,attr);});
    }
};

#define DECLARE_WIDGET(T) static InflaterRegister<T> widget_inflater_##T(#T);
#define DECLARE_WIDGET2(T,name) static InflaterRegister<T> widget_inflater_##T_##name(#name);
}//endof namespace
#endif
