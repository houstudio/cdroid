#ifndef __LAYOUT_INFLATE_H__
#define __LAYOUT_INFLATE_H__
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
    typedef std::map<const std::string,ViewInflater>INFLATERMAPPER;
    typedef std::map<const std::string,const std::string>STYLEMAPPER;
    static INFLATERMAPPER& getInflaterMap();
    static STYLEMAPPER& getStyleMap();
protected:
    View* inflate(const std::string&package,std::istream&stream,ViewGroup*root,bool attachToRoot);
public:
    static LayoutInflater*from(Context*context);
    static ViewInflater getInflater(const std::string&);
    static bool registInflater(const std::string&name,const std::string&,ViewInflater fun);
    const std::string getDefaultStyle(const std::string&name)const;
    View* inflate(const std::string&resource,ViewGroup* root, bool attachToRoot=true);
};

template<typename T>
class InflaterRegister{
public:
    InflaterRegister(const std::string&name,const std::string&defstyle){
        LayoutInflater::registInflater(name,defstyle,[](Context*ctx,const AttributeSet&attr)->View*{return new T(ctx,attr);});
    }
};

#define DECLARE_WIDGET(T) static InflaterRegister<T> widget_inflater_##T(#T,"");
#define DECLARE_WIDGET2(T,style) static InflaterRegister<T> widget_inflater_##T(#T,style);
#define DECLARE_WIDGET3(T,name,style) static InflaterRegister<T> widget_inflater_##name(#name,style);
}//endof namespace
#endif
