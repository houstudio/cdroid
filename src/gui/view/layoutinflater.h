#ifndef __LAYOUT_INFLATE_H__
#define __LAYOUT_INFLATE_H__
#include <core/attributeset.h>
#include <core/context.h>
#include <core/xmlpullparser.h>

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

    static void consumeChildElements(XmlPullParser& parser);
    void parseViewTag(XmlPullParser&parser, View*parent,const AttributeSet& attrs);
    void parseInclude(XmlPullParser&parser, Context*,View*prent,const AttributeSet& attrs);
protected:
    View* createView(const std::string& name, const std::string& prefix,const AttributeSet& attrs);
    View* createViewFromTag(View* parent,const std::string& name, Context* context,AttributeSet& attrs,bool ignoreThemeAttr);
    void rInflateChildren(XmlPullParser& parser, View* parent,AttributeSet& attrs,bool finishInflate);
    void rInflate(XmlPullParser& parser, View* parent, Context* context,AttributeSet& attrs, bool finishInflate);
public:
    static LayoutInflater*from(Context*context);
    static ViewInflater getInflater(const std::string&);
    static bool registerInflater(const std::string&name,const std::string&,ViewInflater fun);
    const std::string getDefaultStyle(const std::string&name)const;
    [[deprecated("This function is deprecated")]]
    View* inflate(const std::string&package,std::istream&stream,ViewGroup*root,bool attachToRoot,AttributeSet*);
    View* inflate(XmlPullParser& parser,ViewGroup* root);
    /**
      * Inflate a new view hierarchy from the specified xml resource. Throws
      * {@link InflateException} if there is an error.
      *
      * @param resource ID for an XML layout resource to load (e.g.,
      *        <code>R.layout.main_page</code>)
      * @param root Optional view to be the parent of the generated hierarchy (if
      *        <em>attachToRoot</em> is true), or else simply an object that
      *        provides a set of LayoutParams values for root of the returned
      *        hierarchy (if <em>attachToRoot</em> is false.)
      * @param attachToRoot Whether the inflated hierarchy should be attached to
      *        the root parameter? If false, root is only used to create the
      *        correct subclass of LayoutParams for the root view in the XML.
      * @return In Android The root View of the inflated hierarchy. If root was supplied and
      *         attachToRoot is true, this is root; otherwise it is the root of the inflated XML file.
      *         In cdroid ,we allways return  the root of the inflated XML file.
      */
    View* inflate(XmlPullParser& parser,ViewGroup* root, bool attachToRoot);
    View* inflate(const std::string&resource,ViewGroup* root);
    View* inflate(const std::string&resource,ViewGroup* root, bool attachToRoot);
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
