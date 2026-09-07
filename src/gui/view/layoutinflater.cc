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
#include <view/viewgroup.h>
#include <view/layoutinflater.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <porting/cdlog.h>
#include <fstream>
#include <iomanip>

namespace cdroid {
using namespace cdroid::internal;

static constexpr const char* TAG_MERGE = "merge";
static constexpr const char* TAG_INCLUDE = "include";
static constexpr const char* TAG_1995 = "blink";
static constexpr const char* TAG_REQUEST_FOCUS = "requestFocus";
static constexpr const char* TAG_TAG = "tag";
static constexpr const char* ATTR_LAYOUT = "layout";
// AOSP LayoutInflater.ATTRS_THEME = { android.R.attr.theme } — sentinel-terminated
// attr-id array (trailing 0), the C++ analog of AOSP's int[].
static const uint32_t ATTRS_THEME[] = { (uint32_t)cdroid::internal::R::attr::theme, 0 };

static std::unordered_map<std::string,LayoutInflater::ViewInflater> mFlateMapper;
static std::unordered_map<Context*,std::shared_ptr<LayoutInflater>> mInflaters;

LayoutInflater::LayoutInflater(Context*context) {
    mContext = context;
    mFactorySet = false;
}

LayoutInflater*LayoutInflater::from(Context*context) {
    auto it = mInflaters.find(context);
    if(it== mInflaters.end()){
        LayoutInflater*layoutInfater = new LayoutInflater(context);
        mInflaters.insert({context,std::shared_ptr<LayoutInflater>(layoutInfater)});
        return layoutInfater;
    }
    return it->second.get();
}

LayoutInflater::ViewInflater LayoutInflater::getInflater(const std::string&name) {
    const size_t  pt = name.rfind('.');
    auto &maps = mFlateMapper;
    const std::string sname = (pt!=std::string::npos)?name.substr(pt+1):name;
    auto it = maps.find(sname);
    return (it!=maps.end())?it->second:nullptr;
}

bool LayoutInflater::registerInflater(const std::string&name,int defStyleAttr,LayoutInflater::ViewInflater inflater) {
    auto& maps = mFlateMapper;
    auto flaterIter = maps.find(name);

    /*disable widget inflater's hack*/
    if(flaterIter!=maps.end() ){
        LOGW("%s is registed to %p",name.c_str(),static_cast<void*>(&flaterIter->second));
        return false;
    }
    maps.insert({name,inflater});
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

void LayoutInflater::setFactory(const Factory& factory){
    if(mFactorySet){
        throw std::runtime_error("A factory has already been set on this LayoutInflater");
    }
    if(factory==nullptr){
        throw std::invalid_argument("Given factory can not be null");
    }
    mFactorySet = true;
    if(mFactory2==nullptr){
        mFactory = factory;
    }else{
        mFactoryMerger = std::make_shared<FactoryMerger>(factory, nullptr, nullptr, mFactory2);
        mFactory = nullptr;  // reset factory
        mFactory2 = [this](View* parent, const std::string& name, Context* context, const AttributeSet& attrs) -> View* {
            return mFactoryMerger->onCreateView(parent, name, context, attrs);
        };
    }
}

void LayoutInflater::setFactory2(const Factory2& factory){
    if(mFactorySet){
        throw std::runtime_error("A factory has already been set on this LayoutInflater");
    }
    if(factory==nullptr){
        throw std::invalid_argument("Given factory can not be null");
    }
    mFactorySet = true;
    if(mFactory==nullptr){
        mFactory2 = factory;
    }else{
        mFactoryMerger = std::make_shared<FactoryMerger>(nullptr, factory, mFactory, nullptr);
        mFactory = nullptr;
        mFactory2 = [this](View* parent, const std::string& name, Context* context, const AttributeSet& attrs) -> View* {
            return mFactoryMerger->onCreateView(parent, name, context, attrs);
        };
    }
}

void LayoutInflater::setPrivateFactory(const Factory2& factory){
    if(mFactory==nullptr){
        mPrivateFactory = factory;
    }else{
        auto merger = std::make_shared<FactoryMerger>(nullptr, factory, nullptr, mPrivateFactory);
        mPrivateFactory = [merger](View* parent, const std::string& name, Context* context, const AttributeSet& attrs) -> View* {
            return merger->onCreateView(parent, name, context, attrs);
        };
    }
}

LayoutInflater::Filter LayoutInflater::getFilter()const{
    return mFilter;
}

void LayoutInflater::setFilter(const Filter& f){
    mFilter = f;
}

void LayoutInflater::advanceToRootNode(XmlPullParser& parser){
    // Look for the root node.
    int type;
    while ((type = parser.next()) != XmlPullParser::START_TAG &&
        type != XmlPullParser::END_DOCUMENT) {
        // Empty
    }

    if (type != XmlPullParser::START_TAG) {
        throw std::runtime_error(parser.getPositionDescription() + ": No start tag found!");
    }
}

View* LayoutInflater::inflate(XmlPullParser& parser,ViewGroup* root){
    return inflate(parser,root,root!=nullptr);
}

View* LayoutInflater::inflate(XmlPullParser& parser,ViewGroup* root, bool attachToRoot){
    int type;
    View*result = root;
    const AttributeSet& attrs = parser;
    if(!parser){
        return nullptr;
    }
    advanceToRootNode(parser);

    const std::string name = parser.getName();
    if(name.compare(TAG_MERGE)==0){
        if(root==nullptr||!attachToRoot){
             throw std::runtime_error("<merge /> can be used only with a valid "
                            "ViewGroup root and attachToRoot=true");
        }
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

View* LayoutInflater::inflate(int resource, ViewGroup* root){
    return inflate(resource,root,root!=nullptr);
}

View* LayoutInflater::inflate(int resource, ViewGroup* root, bool attachToRoot){
    auto parser = mContext->getResources().getXml(resource);
    return inflate(*parser,root,attachToRoot);
}

View* LayoutInflater::createView(const std::string& name, const std::string& prefix,const AttributeSet& attrs){
    return createView(mContext,name,prefix,attrs);
}

View* LayoutInflater::createView(Context* viewContext, const std::string& name, const std::string& prefix,const AttributeSet& attrs){
    LayoutInflater::ViewInflater inflater;

    if(name.compare("view")==0){
        // AOSP createViewFromTag: name = attrs.getAttributeValue(null, "class").
        const std::string clsName =  attrs.getClassAttribute();
        inflater = LayoutInflater::getInflater(clsName);
    }else{
        inflater = LayoutInflater::getInflater(name);
    }
    std::string clazz;
    if(inflater==nullptr){
        clazz = !prefix.empty()?(prefix+name):name;
        if(mFilter!=nullptr){
            const bool allowed = mFilter(clazz);
            if(!allowed){
                failNotAllowed(name,prefix,viewContext,attrs);
            }
        }
    }else{
        if(mFilter!=nullptr){
            auto it = mFilterMap.find(name);
            if(it==mFilterMap.end()){
                clazz = !prefix.empty()?(prefix+name):name;
                const bool allowed = !clazz.empty()&&mFilter(clazz);
                mFilterMap.insert({name,allowed});
                if(!allowed){
                    failNotAllowed(name,prefix,viewContext,attrs);
                }
            }else{
                failNotAllowed(name,prefix,viewContext,attrs);
            }
        }
    }
    // AOSP createView applies no style here: the tag's style= attribute is
    // resolved natively inside obtainStyledAttributes (binary AXML ResXMLTree
    // path), and each widget's default style comes from the defStyleAttr its
    // constructor resolves (registered via DECLARE_WIDGET2).
    View*view = inflater(viewContext,attrs);
    return view;
}


void LayoutInflater::failNotAllowed(const std::string& name, const std::string& prefix, Context* context,const AttributeSet& attrs) {
    throw std::runtime_error(//getParserStateDescription(context, attrs) +
            ": Class not allowed to be inflated "+ (!prefix.empty() ? (prefix + name) : name));
}

View* LayoutInflater::onCreateView(const std::string& name,const AttributeSet& attrs){
    return createView(name, "android.view.", attrs);
}

View* LayoutInflater::onCreateView(View* parent, const std::string& name,const AttributeSet& attrs){
    return onCreateView(name,attrs);
}

View* LayoutInflater::onCreateView(Context* viewContext, View* parent, const std::string& name,const AttributeSet& attrs){
    return onCreateView(parent,name,attrs);
}

View* LayoutInflater::createViewFromTag(View* parent,const std::string& name, Context* context,const AttributeSet& attrs,bool ignoreThemeAttr) {
    if (!ignoreThemeAttr) {
        // AOSP: apply a theme wrapper if the tag carries android:theme.
        auto ta = context->obtainStyledAttributes(&attrs, ATTRS_THEME);
        const int themeResId = ta ? ta->getResourceId(0, 0) : 0;
        if (themeResId != 0) {
            mThemeContexts.emplace_back(new ContextThemeWrapper(context, themeResId));
            context = mThemeContexts.back().get();
        }
    }
    try{
        View* view = tryCreateView(parent, name, context, attrs);

        if (view == nullptr) {
            //lastContext = mConstructorArgs[0];
            //mConstructorArgs[0] = context;
            //try {
                if (std::string::npos == name.find('.')) {
                    view = onCreateView(context,parent, name, attrs);
                } else {
                    view = createView(context,name,"", attrs);
                }
            /*} finally {
                mConstructorArgs[0] = lastContext;
            }*/
        }
        return view;
    }catch(std::exception&e){
        LOGE("%s:Error %s",name.c_str(),e.what());
        throw e;
    }
}

View* LayoutInflater::tryCreateView(View* parent,const std::string& name, Context* context,const AttributeSet& attrs) {
    if (name.compare(TAG_1995)==0) {
        // Let's party like it's 1995!
        return nullptr;//new BlinkLayout(context, attrs);
    }

    View* view;
    if (mFactory2 != nullptr) {
        view = mFactory2/*.onCreateView*/(parent, name, context, attrs);
    } else if (mFactory != nullptr) {
        view = mFactory/*.onCreateView*/(name, context, attrs);
    } else {
        view = nullptr;
    }

    if ((view == nullptr) && (mPrivateFactory != nullptr)) {
        view = mPrivateFactory/*onCreateView*/(parent, name, context, attrs);
    }

    return view;
}

void LayoutInflater::rInflateChildren(XmlPullParser& parser, View* parent,const AttributeSet& attrs,bool finishInflate){
    rInflate(parser, parent, parent->getContext(), attrs, finishInflate);
}

void LayoutInflater::rInflate(XmlPullParser& parser, View* parent, Context* context,const AttributeSet& attrs, bool finishInflate){
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
    // AOSP View.parseViewTag: R.styleable.ViewTag (id ref + value string).
    Context* context = view->getContext();
    auto ta = context->obtainStyledAttributes(&attrs, R::styleable::ViewTag);
    const int key = ta->getResourceId(R::styleable::ViewTag_id, 0);
    const std::string value = ta->getString(R::styleable::ViewTag_value);
    // CDROID setTag is (int, void*); AOSP stores a CharSequence value object.
    if (key != 0 && !value.empty()) {
        view->setTag(key, new std::string(value), [](void*p){ delete (std::string*)p; });
    }
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
        auto ta = context->obtainStyledAttributes(&attrs, ATTRS_THEME);
        int themeResId = ta->getResourceId(0, 0);
        bool hasThemeOverride = themeResId != 0;
        int layout = attrs.getAttributeResourceValue(std::string(), ATTR_LAYOUT, 0);
        if (layout==0) {
            throw std::logic_error("You must specify a layout in the include tag: <include layout=\"@layout/layoutID\" />");
            // Attempt to resolve the "?attr/name" string to an attribute within the default (e.g. application) package.
            // layout = context.getResources().getIdentifier(value.substring(1), "attr", context.getPackageName());
        }
        int type;
        auto childParser = context->getResources().getXml(layout);
        while ((type = childParser->next()) != XmlPullParser::START_TAG &&
                type != XmlPullParser::END_DOCUMENT) {
            // Empty.
        }

        if (type != XmlPullParser::START_TAG) {
            throw std::logic_error(childParser->getPositionDescription()+": No start tag found!");
        }

        const std::string childName = childParser->getName();
        const AttributeSet& childAttrs = *childParser;

        if (childName.compare(TAG_MERGE)==0){
            // The <merge> tag doesn't support android:theme, so nothing special to do here.
            rInflate(*childParser, parent, context, childAttrs, false);
        } else {
            View* view = createViewFromTag(parent, childName,context, childAttrs, hasThemeOverride);
            ViewGroup* group = (ViewGroup*) parent;

            // AOSP: read id/visibility off the <include /> tag itself and apply
            // them to the included root AFTER inflation (setId/setVisibility).
            // The old CDROID inherit() merge only reached the text-XML string
            // map — invisible to the binary AXML typed path — so the include
            // tag's android:id/visibility were silently dropped in binary mode.
            auto ta = context->obtainStyledAttributes(&attrs, R::styleable::Include);
            const int id = ta ? ta->getResourceId(R::styleable::Include_id, View::NO_ID) : View::NO_ID;
            const int visibility = ta ? ta->getInt(R::styleable::Include_visibility, -1) : -1;

            // We try to load the layout params set in the <include /> tag.
            // If the parent can't generate layout params (ex. missing width
            // or height for the framework ViewGroups, though this is not
            // necessarily true of all ViewGroups) then we expect it to throw
            // a runtime exception.
            // We catch this exception and set localParams accordingly: true
            // means we successfully loaded layout params from the <include>
            // tag, false means we need to rely on the included layout params.
            // AOSP: generateLayoutParams(include-attrs) THROWS when the
            // <include> tag lacks layout_width/height (TypedArray
            // .getLayoutDimension throws on a missing dimension) and the
            // catch falls back to the included root's LayoutParams — that's
            // how an attribute-less <include> keeps the root's 110dp/
            // match_parent. CDROID's getLayoutDimension returns a default
            // instead of throwing, so the fallback never fired and the
            // include inherited junk dimensions (the status bar stretched
            // over the whole page and its centered content drifted to the
            // middle). Detect the missing pair explicitly instead.
            ViewGroup::LayoutParams* params = nullptr;
            {
                auto lta = context->obtainStyledAttributes(&attrs, R::styleable::Layout);
                const bool hasSize = lta && lta->hasValue(R::styleable::Layout_layout_width)
                        && lta->hasValue(R::styleable::Layout_layout_height);
                if (hasSize) {
                    try {
                        params = group->generateLayoutParams(attrs);
                    } catch (std::exception&) {
                        // Ignore, just fail over to child attrs.
                    }
                }
            }
            if (params == nullptr) {
                params = group->generateLayoutParams(childAttrs);
            }
            view->setLayoutParams(params);

            // Inflate all children.
            rInflateChildren(*childParser, view, childAttrs, true);

            if (id != View::NO_ID) {
                view->setId(id);
            }

            switch (visibility) {
                case 0:
                    view->setVisibility(View::VISIBLE);
                    break;
                case 1:
                    view->setVisibility(View::INVISIBLE);
                    break;
                case 2:
                    view->setVisibility(View::GONE);
                    break;
            }
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

LayoutInflater::FactoryMerger::FactoryMerger(const Factory& f1,const Factory2& f12,const Factory& f2,const Factory2& f22)
   :mF1(f1), mF12(f12), mF2(f2), mF22(f22) {
}
 
View* LayoutInflater::FactoryMerger::onCreateView(View* parent, const std::string& name, Context* context, const AttributeSet& attrs) {
    View* view = nullptr;
    
    if (mF12 && (view = mF12(parent, name, context, attrs))) {
        return view;
    }
    
    if (mF1 && (view = mF1(name, context, attrs))) {
        return view;
    }
    
    if (mF22 && (view = mF22(parent, name, context, attrs))) {
        return view;
    }
    
    if (mF2 && (view = mF2(name, context, attrs))) {
        return view;
    }
    
    return nullptr;
}
}//endof namespace
