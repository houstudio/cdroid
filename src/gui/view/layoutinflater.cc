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
#include <utils/atexit.h>
#include <view/viewgroup.h>
#include <view/layoutinflater.h>
#include <porting/cdlog.h>
#include <string.h>
#include <fstream>
#include <iomanip>

namespace cdroid {

LayoutInflater::LayoutInflater(Context*context) {
    mContext = context;
}

LayoutInflater*LayoutInflater::mInst = nullptr;
LayoutInflater*LayoutInflater::from(Context*context) {
    if(mInst==nullptr){
        mInst = new LayoutInflater(context);
        AtExit::registerCallback([](){
            LOGD("delete LayoutInflater %p",mInst);
            delete mInst;
        });
    }
    return mInst;
}

LayoutInflater::INFLATERMAPPER& LayoutInflater::getInflaterMap() {
    static LayoutInflater::INFLATERMAPPER mFlateMapper;
    return mFlateMapper;
}

LayoutInflater::STYLEMAPPER& LayoutInflater::getStyleMap() {
    static LayoutInflater::STYLEMAPPER mDefaultStyle;
    return mDefaultStyle;
}

const std::string LayoutInflater::getDefaultStyle(const std::string&name)const {
    LayoutInflater::STYLEMAPPER& maps = getStyleMap();
    auto it = maps.find(name);
    return it==maps.end()?std::string():it->second;
}

LayoutInflater::ViewInflater LayoutInflater::getInflater(const std::string&name) {
    const size_t  pt = name.rfind('.');
    LayoutInflater::INFLATERMAPPER &maps =getInflaterMap();
    const std::string sname = (pt!=std::string::npos)?name.substr(pt+1):name;
    auto it = maps.find(sname);
    return (it!=maps.end())?it->second:nullptr;
}

bool LayoutInflater::registerInflater(const std::string&name,const std::string&defstyle,LayoutInflater::ViewInflater inflater) {
    LayoutInflater::INFLATERMAPPER& maps = getInflaterMap();
    LayoutInflater::STYLEMAPPER& smap = getStyleMap();
    auto flaterIter = maps.find(name);
    auto styleIter = smap.find(name);

    /*disable widget inflater's hack*/
    if(flaterIter!=maps.end() ){
        LOGW("%s is registed to %p",name.c_str(),static_cast<void*>(&flaterIter->second));
        return false;
    }
    maps.insert(INFLATERMAPPER::value_type(name,inflater));
    smap.insert(std::pair<const std::string,const std::string>(name,defstyle));
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Context*LayoutInflater::getContext()const{
    return mContext;
}

LayoutInflater::Factory LayoutInflater::getFactory()const{
    return mFactory;
}

LayoutInflater::Factory2 LayoutInflater::getFactory2()const{
    return mFactory2;
}
void LayoutInflater::setFactory(Factory factory){
    mFactory = factory;
}

void LayoutInflater::setFactory2(Factory2 factory){
    mFactory2 = factory;
}

void LayoutInflater::setPrivateFactory(Factory2 factory){
    if(mFactory==nullptr){
        mPrivateFactory = factory;
    }else{
    }
}

LayoutInflater::Filter LayoutInflater::getFilter()const{
    return mFilter;
}

void LayoutInflater::setFilter(Filter f){
    mFilter = f;
}

View* LayoutInflater::inflate(const std::string&package,std::istream&stream,ViewGroup*root,bool attachToRoot,AttributeSet*){
    auto strm = std::make_unique<std::istream>(stream.rdbuf());
    XmlPullParser parser(mContext,std::move(strm));
    return inflate(parser,root,attachToRoot);
}

View* LayoutInflater::inflate(XmlPullParser& parser,ViewGroup* root){
    return inflate(parser,root,root!=nullptr);
}

View* LayoutInflater::inflate(XmlPullParser& parser,ViewGroup* root, bool attachToRoot){
    int type;
    View*result = root;
    AttributeSet& attrs = parser;
    while(((type=parser.next())!=XmlPullParser::START_TAG)
            &&(type!=XmlPullParser::END_DOCUMENT)){
        //Empty
    }

    if(type!=XmlPullParser::START_TAG){
        LOGE("No start tag found");
        return nullptr;
    }
    const std::string name = parser.getName();
    if(name.compare(TAG_MERGE)==0){
        rInflate(parser,root,mContext,attrs,false);
    }else{
        View*temp = createViewFromTag(root,name,mContext,attrs,false);
        ViewGroup::LayoutParams*params = nullptr;
        if(root!=nullptr){
            params =root->generateLayoutParams(attrs);
            if(!attachToRoot)temp->setLayoutParams(params);
        }else if(dynamic_cast<ViewGroup*>(temp)){
            params =((ViewGroup*)temp)->generateLayoutParams(attrs);
            temp->setLayoutParams(params);
        }
        rInflateChildren(parser,temp,attrs,true);
        if((root!=nullptr)&&attachToRoot){
            root->addView(temp,params);
            root->requestLayout();
            root->startLayoutAnimation();
        }
        //if((root==nullptr)||(attachToRoot==false)) result = temp;

        return temp;
    }
    return result;
}

View* LayoutInflater::inflate(const std::string&resource,ViewGroup* root){
    return inflate(resource,root,root!=nullptr);
}

View* LayoutInflater::inflate(const std::string&resource,ViewGroup* root, bool attachToRoot){
    XmlPullParser parser(mContext,resource);
    return inflate(parser,root,attachToRoot);
}

View* LayoutInflater::createView(const std::string& name, const std::string& prefix,const AttributeSet& attrs){
    return nullptr;
}

View* LayoutInflater::createViewFromTag(View* parent,const std::string& name, Context* context,AttributeSet& attrs,bool ignoreThemeAttr) {
#if 0
    if (name.compare("view")==0) {
        //name = attrs.getAttributeValue(nullptr, "class");
    }

    // Apply a theme wrapper, if allowed and one is specified.

    View* view;
    if (mFactory2 != null) {
        view = mFactory2.onCreateView(parent, name, context, attrs);
    } else if (mFactory != null) {
        view = mFactory.onCreateView(name, context, attrs);
    } else {
        view = null;
    }

    if (view == null && mPrivateFactory != null) {
        view = mPrivateFactory.onCreateView(parent, name, context, attrs);
    }

    if (view == null) {
        final Object lastContext = mConstructorArgs[0];
        mConstructorArgs[0] = context;
        try {
            if (-1 == name.indexOf('.')) {
                view = onCreateView(parent, name, attrs);
            } else {
                view = createView(name, null, attrs);
            }
        } finally {
            mConstructorArgs[0] = lastContext;
        }
    }
    return view;
#else
    LayoutInflater::ViewInflater inflater = nullptr;
    if(name.compare("view")==0){
        const std::string clsName =  attrs.getString("class");
        inflater = LayoutInflater::getInflater(clsName);
    }else{
        inflater = LayoutInflater::getInflater(name);
    }
    std::string styleName = attrs.getString("style");
    if(!styleName.empty()) {
        AttributeSet style = context->obtainStyledAttributes(styleName);
        attrs.inherit(style);
    }
    styleName = LayoutInflater::from(context)->getDefaultStyle(name);
    if(!styleName.empty()) {
        AttributeSet defstyle = context->obtainStyledAttributes(styleName);
        attrs.inherit(defstyle);
    }
    View*view = inflater(context,attrs);
    return view;
#endif
}

void LayoutInflater::rInflateChildren(XmlPullParser& parser, View* parent,AttributeSet& attrs,bool finishInflate){
    rInflate(parser, parent, parent->getContext(), attrs, finishInflate);
}

void LayoutInflater::rInflate(XmlPullParser& parser, View* parent, Context* context,AttributeSet& attrs, bool finishInflate){
    int type;
    const int depth = parser.getDepth();
    bool pendingRequestFocus = false;

    while (((type = parser.next()) != XmlPullParser::END_TAG ||
            parser.getDepth() > depth) && (type != XmlPullParser::END_DOCUMENT)) {

        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        const std::string name = parser.getName();
        if (name.compare(TAG_REQUEST_FOCUS)==0) {
            pendingRequestFocus = true;
            consumeChildElements(parser);
        } else if (name.compare(TAG_TAG)==0) {
            parseViewTag(parser, parent, attrs);
        } else if (name.compare(TAG_INCLUDE)==0) {
            if (parser.getDepth() == 0) {
                throw std::logic_error("<include/> cannot be the root element");
            }
            parseInclude(parser, context, parent, attrs);
        } else if (name.compare(TAG_MERGE)==0) {
            throw std::logic_error("<merge/> must be the root element");
        } else {
            View* view = createViewFromTag(parent, name, context, attrs,false);
            ViewGroup* viewGroup = (ViewGroup*) parent;
            ViewGroup::LayoutParams* params = viewGroup->generateLayoutParams(attrs);
            rInflateChildren(parser, view, attrs, true);
            viewGroup->addView(view, params);
        }
    }

    if (pendingRequestFocus) {
        parent->restoreDefaultFocus();
    }

    if (finishInflate) {
        parent->onFinishInflate();
    }
}

void LayoutInflater::parseViewTag(XmlPullParser& parser, View* view,const AttributeSet& attrs){
    const int key = attrs.getResourceId("id", 0);
    const std::string value = attrs.getString("value");
    //view->setTag(key, value);
    consumeChildElements(parser);
}

void LayoutInflater::parseInclude(XmlPullParser& parser, Context* context, View* parent,const AttributeSet& attrs){
    if (dynamic_cast<ViewGroup*>(parent)) {
        // Apply a theme wrapper, if requested. This is sort of a weird
        // edge case, since developers think the <include> overwrites
        // values in the AttributeSet of the included View. So, if the
        // included View has a theme attribute, we'll need to ignore it.

        // If the layout is pointing to a theme attribute, we have to
        // massage the value to get a resource identifier out of it.
        const bool hasThemeOverride = false;
        const std::string layout = attrs.getString("layout");
        if (layout.empty()) {
            throw std::logic_error("You must specify a layout in the include tag: <include layout=\"@layout/layoutID\" />");
            // Attempt to resolve the "?attr/name" string to an attribute within the default (e.g. application) package.
            // layout = context.getResources().getIdentifier(value.substring(1), "attr", context.getPackageName());
        }
        int type;
        XmlPullParser childParser(context,layout);
        while ((type = childParser.next()) != XmlPullParser::START_TAG &&
                type != XmlPullParser::END_DOCUMENT) {
            // Empty.
        }

        if (type != XmlPullParser::START_TAG) {
            throw std::logic_error(childParser.getPositionDescription()+": No start tag found!");
        }

        const std::string childName = childParser.getName();
        AttributeSet& childAttrs = childParser;

        if (childName.compare(TAG_MERGE)==0){
            // The <merge> tag doesn't support android:theme, so nothing special to do here.
            rInflate(childParser, parent, context, childAttrs, false);
        } else {
            childAttrs.inherit(attrs);
            View* view = createViewFromTag(parent, childName,context, childAttrs, hasThemeOverride);
            ViewGroup* group = (ViewGroup*) parent;
            // We try to load the layout params set in the <include /> tag.
            // If the parent can't generate layout params (ex. missing width
            // or height for the framework ViewGroups, though this is not
            // necessarily true of all ViewGroups) then we expect it to throw
            // a runtime exception.
            // We catch this exception and set localParams accordingly: true
            // means we successfully loaded layout params from the <include>
            // tag, false means we need to rely on the included layout params.
            ViewGroup::LayoutParams* params = group->generateLayoutParams(childAttrs);
            view->setLayoutParams(params);

            // Inflate all children.
            rInflateChildren(childParser, view, childAttrs, true);
            group->addView(view);
        }
    } else {
        throw std::logic_error("<include /> can only be used inside of a ViewGroup");
    }
    LayoutInflater::consumeChildElements(parser);
}

void LayoutInflater::consumeChildElements(XmlPullParser& parser){
    int type;
    const int currentDepth = parser.getDepth();
    while (((type = parser.next()) != XmlPullParser::END_TAG ||
            parser.getDepth() > currentDepth) && type != XmlPullParser::END_DOCUMENT) {
        // Empty
    }
}

}//endof namespace
