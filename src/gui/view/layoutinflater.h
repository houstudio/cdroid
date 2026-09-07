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
#ifndef __LAYOUT_INFLATE_H__
#define __LAYOUT_INFLATE_H__
#include <core/attributeset.h>
#include <core/context.h>
#include <content/contextthemewrapper.h>
#include <core/xmlpullparser.h>

namespace cdroid{
class View;
class ViewGroup;
class MenuInflater;
class LayoutInflater{
public:
    // AOSP Factory/Factory2.onCreateView declare AttributeSet @NonNull — the
    // inflater chain keeps references; only the widget ctor boundary (nullable
    // AttributeSet*) takes a pointer.
    typedef std::function<View*(Context*ctx, const AttributeSet&attrs)>ViewInflater;
    typedef std::function<bool(const std::string&)>Filter;
    typedef std::function<View*(const std::string&,Context*,const AttributeSet&)>Factory;
    typedef std::function<View*(View*,const std::string&,Context*,const AttributeSet&)>Factory2;
private:

    class FactoryMerger {
    private:
        Factory mF1, mF2;
        Factory2 mF12,mF22;
    public:
        FactoryMerger(const Factory& f1, const Factory2& f12, const Factory& f2,const Factory2& f22);
        View* onCreateView(View* parent, const std::string& name, Context* context, const AttributeSet& attrs);
    };
    Context*mContext;
    Factory mFactory;
    Factory2 mFactory2;
    Factory2 mPrivateFactory;
    Filter mFilter;
    std::unordered_map<std::string,bool>mFilterMap;
    // ContextThemeWrappers created for android:theme tag overrides. AOSP relies
    // on GC; CDROID views hold a raw Context*, so the inflater (cached per
    // Context by from()) owns the wrappers for the process lifetime.
    std::vector<std::unique_ptr<ContextThemeWrapper>> mThemeContexts;
    std::shared_ptr<FactoryMerger> mFactoryMerger;
    bool mFactorySet;
private:
    static void consumeChildElements(XmlPullParser& parser);
    void advanceToRootNode(XmlPullParser&);
    void failNotAllowed(const std::string& name, const std::string& prefix, Context* context,const AttributeSet& attrs);
    void parseViewTag(XmlPullParser&parser, View*parent,const AttributeSet& attrs);
    void parseInclude(XmlPullParser&parser, Context*,View*prent,const AttributeSet& attrs);
protected:
    friend MenuInflater;
    LayoutInflater(Context*ctx);
    View* createViewFromTag(View* parent,const std::string& name, Context* context,const AttributeSet& attrs,bool ignoreThemeAttr);
    void rInflateChildren(XmlPullParser& parser, View* parent,const AttributeSet& attrs,bool finishInflate);
    void rInflate(XmlPullParser& parser, View* parent, Context* context,const AttributeSet& attrs, bool finishInflate);
public:
    static LayoutInflater*from(Context*context);
    static ViewInflater getInflater(const std::string&);
    // The factory lambda already carries any defStyleAttr (the DECLARE_WIDGET
    // macros bake it into the closure that calls the widget's AOSP ctor), so
    // the registration entry only pairs a tag key with a factory.
    static bool registerInflater(const std::string&name,ViewInflater fun);
    Context*getContext()const;
    Factory getFactory()const;
    Factory2 getFactory2()const;
    void setFactory(const Factory& factory);
    void setFactory2(const Factory2& factory);
    void setPrivateFactory(const Factory2& factory);
    Filter getFilter()const;
    void setFilter(const Filter& f);

    [[deprecated("This function is deprecated")]]
    //View* inflate(const std::string&package,std::istream&stream,ViewGroup*root,bool attachToRoot,AttributeSet*);
    View* inflate(XmlPullParser& parser,ViewGroup* root);

    View* inflate(int resource, ViewGroup* root);
    View* inflate(int resource, ViewGroup* root, bool attachToRoot);
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

    View* createView(const std::string& name, const std::string& prefix,const AttributeSet& attrs);
    View* createView(Context* viewContext, const std::string& name, const std::string& prefix,const AttributeSet& attrs);
    View* tryCreateView(View* parent,const std::string& name, Context* context,const AttributeSet& attrs);
    virtual View* onCreateView(const std::string& name,const AttributeSet& attrs);
    virtual View* onCreateView(View* parent, const std::string& name,const AttributeSet& attrs);
    virtual View* onCreateView(Context* viewContext, View* parent, const std::string& name,const AttributeSet& attrs);
};

// AOSP LayoutInflater instantiates views through ctor(Context, AttributeSet)
// alone — defStyleAttr/defStyleRes are per-class constants each widget injects
// by delegating its 2-arg ctor to the styled one (see TextView), never inputs
// of the inflation path. The registration factory follows that contract.
template<typename T>
class InflaterRegister{
public:
    explicit InflaterRegister(const std::string&name){
        LayoutInflater::registerInflater(name,[](Context*ctx,const AttributeSet&attr)->View*{
            return new T(ctx,&attr);
        });
    }
};

/* Registration macros. DECLARE_WIDGET(T) keys the bare class name.
   DECLARE_WIDGET2(T, key) takes the registry key as a string — either the
   bare name ("TimerItem") or the upstream fully-qualified tag
   ("android.widget.TextView", "androidx.recyclerview.widget.RecyclerView");
   library FQCNs also answer to their simple XML shorthand — see
   registerInflater's alias rule. T must be a simple identifier — bring
   namespace-qualified names into scope with a using-declaration first.
   Default styles are per-class ctor constants (each widget's 2-arg ctor
   delegates them, AOSP shape), never registration inputs. */
#define DECLARE_WIDGET(T) static InflaterRegister<T> widget_inflater_##T(#T);
#define DECLARE_WIDGET2(T,key) static InflaterRegister<T> widget_inflater_##T(key);
}//endof namespace
#endif
