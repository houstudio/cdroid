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
#include <drawables/drawable.h>
#include <menu/menuinflater.h>
#include <core/attributeset.h>
#include <core/xmlpullparser.h>
#include <menu/menuitem.h>
#include <menu/submenu.h>
#include <menu/menuitemimpl.h>
#include <view/actionprovider.h>
#include <view/layoutinflater.h>

namespace cdroid{ 
MenuInflater::MenuInflater(Context* context) {
    mContext = context;
    //mActionViewConstructorArguments = new Object[] {context};
    //mActionProviderConstructorArguments = mActionViewConstructorArguments;
}

MenuInflater::MenuInflater(Context* context, void* realOwner) {
    mContext = context;
    mRealOwner = realOwner;
    //mActionViewConstructorArguments = new Object[] {context};
    //mActionProviderConstructorArguments = mActionViewConstructorArguments;
}

void MenuInflater::inflate(const std::string&menuRes, Menu* menu) {
    XmlPullParser parser(mContext,menuRes);
    AttributeSet& attrs = parser;
    parseMenu(parser, attrs, menu);
}

void MenuInflater::parseMenu(XmlPullParser& parser,const AttributeSet& attrs, Menu* menu){
    MenuState* menuState = new MenuState(menu,mContext);

    int eventType = parser.getEventType();
    bool lookingForEndOfUnknownTag = false;
    bool reachedEndOfMenu = false;
    std::string tagName;
    std::string unknownTagName;

    // This loop will skip to the menu start tag
    do {
        if (eventType == XmlPullParser::START_TAG) {
            tagName = parser.getName();
            if (tagName.compare("menu")==0) {
                // Go to next tag
                eventType = parser.next();
                break;
            }
            throw std::runtime_error("Expecting menu, got " + tagName);
        }
        eventType = parser.next();
    } while (eventType != XmlPullParser::END_DOCUMENT);

    while (!reachedEndOfMenu) {
        switch (eventType) {
        case XmlPullParser::START_TAG:
            if (lookingForEndOfUnknownTag) {
                break;
            }

            tagName = parser.getName();
            if (tagName.compare("group")==0) {
                menuState->readGroup(attrs);
            } else if (tagName.compare("item")==0) {
                menuState->readItem(attrs);
            } else if (tagName.compare("menu")==0) {
                // A menu start tag denotes a submenu for an item
                SubMenu* subMenu = menuState->addSubMenuItem();
                registerMenu(subMenu, attrs);

                // Parse the submenu into returned SubMenu
                parseMenu(parser, attrs, subMenu);
            } else {
                lookingForEndOfUnknownTag = true;
                unknownTagName = tagName;
            }
            break;

        case XmlPullParser::END_TAG:
            tagName = parser.getName();
            if (lookingForEndOfUnknownTag && tagName.compare(unknownTagName)==0) {
                lookingForEndOfUnknownTag = false;
                unknownTagName.clear();
            } else if (tagName.compare("group")==0) {
                menuState->resetGroup();
            } else if (tagName.compare("item")==0) {
                // Add the item if it hasn't been added (if the item was
                // a submenu, it would have been added already)
                if (!menuState->hasAddedItem()) {
                    if ((menuState->itemActionProvider != nullptr) &&
                            menuState->itemActionProvider->hasSubMenu()) {
                        registerMenu(menuState->addSubMenuItem(), attrs);
                    } else {
                        registerMenu(menuState->addItem(), attrs);
                    }
                }
            } else if (tagName.compare("menu")==0) {
                reachedEndOfMenu = true;
            }
            break;

        case XmlPullParser::END_DOCUMENT:
            throw std::runtime_error("Unexpected end of document");
        }

        eventType = parser.next();
    }
}

/**
 * The method is a hook for layoutlib to do its magic.
 * Nothing is needed outside of LayoutLib. However, it should not be deleted because it
 * appears to do nothing.
 */
void MenuInflater::registerMenu(MenuItem* item,const AttributeSet& set) {
}

/**
 * The method is a hook for layoutlib to do its magic.
 * Nothing is needed outside of LayoutLib. However, it should not be deleted because it
 * appears to do nothing.
 */
void MenuInflater::registerMenu(SubMenu* subMenu,const AttributeSet& set) {
}

Context* MenuInflater::getContext() {
    return mContext;
}

/*private static class InflatedOnMenuItemClickListener
        implements MenuItem::OnMenuItemClickListener {
    private static final Class<?>[] PARAM_TYPES = new Class[] { MenuItem.class };

    private Object mRealOwner;
    private Method mMethod;

    public InflatedOnMenuItemClickListener(Object realOwner, const std::string& methodName) {
        mRealOwner = realOwner;
        Class<?> c = realOwner.getClass();
        try {
            mMethod = c.getMethod(methodName, PARAM_TYPES);
        } catch (Exception e) {
            InflateException ex = new InflateException(
                    "Couldn't resolve menu item onClick handler " + methodName +
                    " in class " + c.getName());
            ex.initCause(e);
            throw ex;
        }
    }

    public bool onMenuItemClick(MenuItem& item) {
        try {
            if (mMethod.getReturnType() == Boolean.TYPE) {
                return (Boolean) mMethod.invoke(mRealOwner, item);
            } else {
                mMethod.invoke(mRealOwner, item);
                return true;
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }
}*/

Object* MenuInflater::getRealOwner() {
    if (mRealOwner == nullptr) {
        mRealOwner = findRealOwner((Object*)mContext);
    }
    return (Object*)mRealOwner;
}

Object* MenuInflater::findRealOwner(Object* owner) {
    /*if (owner instanceof Activity) {
        return owner;
    }
    if (owner instanceof ContextWrapper) {
        return findRealOwner(((ContextWrapper) owner).getBaseContext());
    }*/
    return owner;
}

////////////////////////////////////////////////////////////////////////////////////////////

MenuInflater::MenuState::MenuState(Menu* menu,Context*ctx) {
    this->menu = menu;
    this->mContext = ctx;
    resetGroup();
}

MenuInflater::MenuState::~MenuState(){
}

void MenuInflater::MenuState::resetGroup() {
    groupId = defaultGroupId;
    groupCategory = defaultItemCategory;
    groupOrder = defaultItemOrder;
    groupCheckable = defaultItemCheckable;
    groupVisible = defaultItemVisible;
    groupEnabled = defaultItemEnabled;
}

/**
 * Called when the parser is pointing to a group tag.
 */
void MenuInflater::MenuState::readGroup(const AttributeSet& attrs) {

    groupId = attrs.getResourceId("id", defaultGroupId);
    groupCategory = attrs.getInt("menuCategory", defaultItemCategory);
    groupOrder = attrs.getInt("orderInCategory", defaultItemOrder);
    groupCheckable = attrs.getInt("checkableBehavior", defaultItemCheckable);
    groupVisible = attrs.getBoolean("visible", defaultItemVisible);
    groupEnabled = attrs.getBoolean("enabled", defaultItemEnabled);
}

/**
 * Called when the parser is pointing to an item tag.
 */
void MenuInflater::MenuState::readItem(const AttributeSet& attrs) {

    // Inherit attributes from the group as default value
    itemId = attrs.getResourceId("id", defaultItemId);
    const int category = attrs.getInt("menuCategory", groupCategory);
    const int order = attrs.getInt("orderInCategory", groupOrder);
    itemCategoryOrder = (category & Menu::CATEGORY_MASK) | (order & Menu::USER_MASK);
    itemTitle = attrs.getString("title");//getText
    itemTitleCondensed = attrs.getString("titleCondensed");//getText
    itemIconResId = attrs.getString("icon");
    if (attrs.hasAttribute("iconTintMode")) {
        //mItemIconBlendMode = Drawable::parseBlendMode(attrs.getInt("iconTintMode", -1), mItemIconBlendMode);
    } else {
        // Reset to null so that it's not carried over to the next item
        mItemIconBlendMode = -1;
    }
    if (attrs.hasAttribute("iconTint")) {
        itemIconTintList = attrs.getColorStateList("iconTint");
    } else {
        // Reset to null so that it's not carried over to the next item
        itemIconTintList = nullptr;
    }

    itemAlphabeticShortcut = getShortcut(attrs.getString("alphabeticShortcut"));
    itemAlphabeticModifiers = attrs.getInt("alphabeticModifiers", KeyEvent::META_CTRL_ON);
    itemNumericShortcut = getShortcut(attrs.getString("numericShortcut"));
    itemNumericModifiers = attrs.getInt("numericModifiers", KeyEvent::META_CTRL_ON);
    if (attrs.hasAttribute("checkable")) {
        // Item has attribute checkable, use it
        itemCheckable = attrs.getBoolean("checkable", false) ? 1 : 0;
    } else {
        // Item does not have attribute, use the group's (group can have one more state
        // for checkable that represents the exclusive checkable)
        itemCheckable = groupCheckable;
    }
    itemChecked = attrs.getBoolean("checked", defaultItemChecked);
    itemVisible = attrs.getBoolean("visible", groupVisible);
    itemEnabled = attrs.getBoolean("enabled", groupEnabled);
    itemShowAsAction = attrs.getInt("showAsAction",std::unordered_map<std::string,int>{
            {"always",(int)MenuItem::SHOW_AS_ACTION_ALWAYS},
            {"ifRoom",(int)MenuItem::SHOW_AS_ACTION_IF_ROOM},
            {"never" ,(int)MenuItem::SHOW_AS_ACTION_NEVER},
            {"withText",(int)MenuItem::SHOW_AS_ACTION_WITH_TEXT},
            {"collapseActionView",(int)MenuItem::SHOW_AS_ACTION_COLLAPSE_ACTION_VIEW}
        },-1);
    itemListenerMethodName = attrs.getString("onClick");
    itemActionViewLayout = attrs.getString("actionLayout");
    itemActionViewClassName = attrs.getString("actionViewClass");
    itemActionProviderClassName = attrs.getString("actionProviderClass");

    const bool hasActionProvider = !itemActionProviderClassName.empty();
    if (hasActionProvider && itemActionViewLayout.empty() && itemActionViewClassName.empty()) {
        //itemActionProvider = newInstance(itemActionProviderClassName,ACTION_PROVIDER_CONSTRUCTOR_SIGNATURE,mActionProviderConstructorArguments);
    } else {
        LOGW_IF(hasActionProvider,"Ignoring attribute 'actionProviderClass'. Action view already specified.");
        itemActionProvider = nullptr;
    }

    itemContentDescription = attrs.getString("contentDescription");//getText
    itemTooltipText = attrs.getString("tooltipText");//getText

    itemAdded = false;
}

char MenuInflater::MenuState::getShortcut(const std::string& shortcutString) {
    if (shortcutString.empty()) {
        return 0;
    } else {
        return shortcutString.at(0);
    }
}

void MenuInflater::MenuState::setItem(MenuItem* item) {
    item->setChecked(itemChecked)
        .setVisible(itemVisible)
        .setEnabled(itemEnabled)
        .setCheckable(itemCheckable >= 1)
        .setTitleCondensed(itemTitleCondensed)
        .setIcon(itemIconResId)
        .setAlphabeticShortcut(itemAlphabeticShortcut, itemAlphabeticModifiers)
        .setNumericShortcut(itemNumericShortcut, itemNumericModifiers);

    if (itemShowAsAction >= 0) {
        item->setShowAsAction(itemShowAsAction);
    }

    if (mItemIconBlendMode != -1) {
        //item->setIconTintBlendMode(mItemIconBlendMode);
    }

    if (itemIconTintList != nullptr) {
        item->setIconTintList(itemIconTintList);
    }

    if (!itemListenerMethodName.empty()) {
        /*if (mContext.isRestricted()) {
            throw new IllegalStateException("The android:onClick attribute cannot be used within a restricted context");
        }*/
        //item->setOnMenuItemClickListener(new InflatedOnMenuItemClickListener(getRealOwner(), itemListenerMethodName));
    }

    if (dynamic_cast<MenuItemImpl*>(item)) {
        MenuItemImpl* impl = (MenuItemImpl*) item;
        if (itemCheckable >= 2) {
            impl->setExclusiveCheckable(true);
        }
    }

    bool actionViewSpecified = false;
    if (!itemActionViewClassName.empty()) {
        AttributeSet atts(mContext,"cdroid");
        View* actionView = LayoutInflater::from(mContext)->createViewFromTag(nullptr,itemActionViewClassName,mContext,atts,true);
            //(View*) newInstance(itemActionViewClassName,ACTION_VIEW_CONSTRUCTOR_SIGNATURE, mActionViewConstructorArguments);
        item->setActionView(actionView);
        actionViewSpecified = true;
    }
    if (!itemActionViewLayout.empty()) {
        if (!actionViewSpecified) {
            item->setActionView(itemActionViewLayout);
            actionViewSpecified = true;
        } else {
            LOGW("Ignoring attribute 'itemActionViewLayout'. Action view already specified.");
        }
    }
    if (itemActionProvider != nullptr) {
        item->setActionProvider(itemActionProvider);
    }

    item->setContentDescription(itemContentDescription);
    item->setTooltipText(itemTooltipText);
}

MenuItem* MenuInflater::MenuState::addItem() {
    itemAdded = true;
    MenuItem* item = menu->add(groupId, itemId, itemCategoryOrder, itemTitle);
    setItem(item);
    return item;
}

SubMenu* MenuInflater::MenuState::addSubMenuItem() {
    itemAdded = true;
    SubMenu* subMenu = menu->addSubMenu(groupId, itemId, itemCategoryOrder, itemTitle);
    setItem(subMenu->getInvokerItem());
    return subMenu;
}

bool MenuInflater::MenuState::hasAddedItem() const{
    return itemAdded;
}

}/*endof namespace*/
