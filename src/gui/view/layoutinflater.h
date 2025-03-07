#ifndef __LAYOUT_INFLATE_H__
#define __LAYOUT_INFLATE_H__
#include <core/attributeset.h>
#include <core/context.h>
#include <core/xmlpullparser.h>
//#define NEW_LAYOUT_INFLATER 1
namespace cdroid{
class View;
class ViewGroup;
class LayoutInflater{
public:
    typedef std::function<View*(Context*ctx, const AttributeSet&attrs)>ViewInflater;
private:
    static constexpr const char* TAG_MERGE = "merge";
    static constexpr const char* TAG_INCLUDE = "include";
    static constexpr const char* TAG_1995 = "blink";
    static constexpr const char* TAG_REQUEST_FOCUS = "requestFocus";
    static constexpr const char* TAG_TAG = "tag";
    static constexpr const char* ATTR_LAYOUT = "layout";

    Context*mContext;
    LayoutInflater(Context*ctx);
    static LayoutInflater*mInst;
    typedef std::unordered_map<std::string,ViewInflater>INFLATERMAPPER;
    typedef std::unordered_map<std::string,std::string>STYLEMAPPER;
private:
    static INFLATERMAPPER& getInflaterMap();
    static STYLEMAPPER& getStyleMap();
    View* inflate(const std::string&package,std::istream&stream,ViewGroup*root,bool attachToRoot,AttributeSet*);
/////
    static void consumeChildElements(XmlPullParser& parser);
    void parseViewTag(XmlPullParser&parser, View*parent,const AttributeSet& attrs);
    void parseInclude(XmlPullParser&parser, Context*,View*prent,const AttributeSet& attrs);
public:
    static LayoutInflater*from(Context*context);
    static ViewInflater getInflater(const std::string&);
    static bool registerInflater(const std::string&name,const std::string&,ViewInflater fun);
    const std::string getDefaultStyle(const std::string&name)const;
#ifndef NEW_LAYOUT_INFLATER
    View* inflate(const std::string&resource,ViewGroup* root, bool attachToRoot=true,AttributeSet*atts=nullptr);
#endif
////
    View*createView(const std::string& name, const std::string& prefix,const AttributeSet& attrs);
    View*createViewFromTag(View* parent,const std::string& name, Context* context,const AttributeSet& attrs,bool ignoreThemeAttr);
    void rInflateChildren(XmlPullParser& parser, View* parent,const AttributeSet& attrs,bool finishInflate);
    void rInflate(XmlPullParser& parser, View* parent, Context* context,const AttributeSet& attrs, bool finishInflate);
#ifdef NEW_LAYOUT_INFLATER
    View*inflate(const std::string&resource,ViewGroup* root, bool attachToRoot=true,AttributeSet*atts=nullptr);
#endif
};

template<typename T>
class InflaterRegister{
public:
    InflaterRegister(const std::string&name,const std::string&defstyle){
        LayoutInflater::registerInflater(name,defstyle,[](Context*ctx,const AttributeSet&attr)->View*{return new T(ctx,attr);});
    }
};

#define DECLARE_WIDGET(T) static InflaterRegister<T> widget_inflater_##T(#T,"");
#define DECLARE_WIDGET2(T,style) static InflaterRegister<T> widget_inflater_##T(#T,style);
#define DECLARE_WIDGET3(T,name,style) static InflaterRegister<T> widget_inflater_##name(#name,style);
}//endof namespace
#endif
