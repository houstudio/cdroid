#ifndef __MENU_H__
#define __MENU_H__
#include <string>
#include <widget/view.h>

namespace cdroid{

class Menu;
typedef class Menu SubMenu;

class MenuItem{
protected:
   std::string mTitle;
   std::string mTooltip;
   int mItemId;
   int mGroupId;
   int mOrder;
   bool mCheckable;
   bool mChecked;
   bool mVisible;
   bool mEnabled;
   RefPtr<ImageSurface>mIcon;
   RefPtr<SubMenu>mSubMenu;
   friend class Menu;
public:
   MenuItem();
   MenuItem& setEnabled(bool v);
   MenuItem& setVisible(bool v);
   MenuItem& setTitle(const std::string&);
   std::string getTitle()const{return mTitle;}
   MenuItem& setTooltipText(const std::string&tips);
   std::string getTooltipText()const{return mTooltip;}
   SubMenu* getSubMenu()const;
   bool hasSubMenu()const;

   bool isEnabled()const{return mEnabled;}
   bool isVisible()const{return mVisible;}
   bool isCheckable()const{return mCheckable;}
   bool isChecked()const{return mChecked;}
   int getGroupId()const{return mGroupId;}
   int getItemId()const{return mItemId;}
   int getOrder()const{return mOrder;}
};

class Menu{
protected:
   std::vector<MenuItem*>mMenuItems;
   void *owner;/*menu owner/container*/
public:
   Menu();
   ~Menu();
   MenuItem&add(const std::string&title);
   MenuItem& add(int groupId, int itemId, int order,const std::string&title);
   SubMenu& addSubMenu(const std::string&title);
   SubMenu& addSubMenu(int groupId,int itemId,int order,const std::string& title);
   void popup(int x,int y);
   void close();
   void removeItem(int id);
   void removeGroup(int groupId);
   void clear();
   void setGroupCheckable(int group, bool checkable, bool exclusive);
   void setGroupEnabled(int group, bool enabled);
   bool hasVisibleItems()const;
   MenuItem*findItem(int id)const;
   int size()const;
   MenuItem* getItem(int index)const;
};

}

#endif
