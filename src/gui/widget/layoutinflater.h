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
public:
    static LayoutInflater*from(Context*context);
    static bool registInflater(const std::string&name,ViewInflater fun);
    static ViewInflater getInflater(const std::string&);
    View* inflate(std::istream&stream,ViewGroup*root);
    View* inflate(const std::string&resource,ViewGroup* root);
    View* inflate(const std::string&resource,ViewGroup* root, bool attachToRoot);
};

template<typename T>
class InflaterRegister{
public:
    InflaterRegister(const std::string&name){
        LayoutInflater::registInflater(name,[](Context*ctx,const AttributeSet&attr)->View*{return new T(ctx,attr);});
    }
};

#define DECLARE_WIDGET(T) static InflaterRegister<T> widget_inflater_##T(#T);

}//endof namespace
#endif
