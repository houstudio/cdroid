#include <view/menu.h>
namespace cdroid{

MenuItem::MenuItem(){
   mEnabled=true;
   mVisible=true;
   mSubMenu=nullptr;
   mGroupId=-1;
   mItemId=-1;
   mOrder=-1;
}

MenuItem& MenuItem::setEnabled(bool v){
    mEnabled=v;
    return *this;
}

MenuItem& MenuItem::setTitle(const std::string&txt){
    mTitle=txt;
    return *this;
}

MenuItem& MenuItem::setVisible(bool v){
    mVisible=v;
    return *this;
}

MenuItem& MenuItem::setTooltipText(const std::string&tip){
    mTooltip=tip;
    return *this;
}

SubMenu* MenuItem::getSubMenu()const{
    return mSubMenu;
}

bool MenuItem::hasSubMenu()const{
    return mSubMenu&&mSubMenu->size();
}

//###############################

Menu::Menu(){
   owner=nullptr;
}

Menu::~Menu(){
   clear();
}

void Menu::popup(int x,int y){
}

void Menu::close(){
}

MenuItem& Menu::add(const std::string&title){
   return add(-1,-1,-1,title);
}

MenuItem& Menu::add(int groupId, int itemId, int order,const std::string&title){
   MenuItem*itm=new MenuItem(); 
   itm->setTitle(title);
   itm->mGroupId=groupId;
   itm->mItemId=itemId;
   itm->mOrder=order;
   mMenuItems.push_back(itm);
   return *itm;
}

SubMenu& Menu::addSubMenu(const std::string&title){
   return addSubMenu(-1,-1,-1,title);
}

SubMenu& Menu::addSubMenu(int groupId,int itemId,int order,const std::string& title){
   MenuItem&itm=add(groupId,itemId,order,title);
   itm.mSubMenu=new SubMenu();
   return *itm.mSubMenu;
}

void Menu::removeItem(int id){
      
}

void Menu::removeGroup(int groupId){
   for(auto t=mMenuItems.begin();t!=mMenuItems.end();){
       if((*t)->mGroupId==groupId){
           delete (*t);
           t=mMenuItems.erase(t);
       }else t++;
   }
}

void Menu::clear(){
   while(mMenuItems.size()){
       auto t=mMenuItems.begin();
       delete (*t);
       mMenuItems.erase(t);
   }
}

void Menu::setGroupCheckable(int group, bool checkable, bool exclusive){
   for(auto t=mMenuItems.begin();t!=mMenuItems.end();){
       if((*t)->mGroupId==group)
          (*t)->mCheckable=checkable;
   }
}

void Menu::setGroupEnabled(int group, bool enabled){
    for(auto m:mMenuItems){
        if(m->mGroupId==group)
           m->mEnabled=enabled;
    }
}

bool Menu::hasVisibleItems()const{
    for(auto m:mMenuItems)
        if(m->mVisible)return true;
    return true;
}

MenuItem*Menu::findItem(int id)const{
}

int Menu::size()const{
    return mMenuItems.size();
}

MenuItem* Menu::getItem(int index)const{
    return mMenuItems.at(index);
}

}//end namespace


