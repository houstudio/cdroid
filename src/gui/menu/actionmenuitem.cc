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
#include <view/view.h>
#include <menu/actionmenuitem.h>
namespace cdroid{

void ActionMenuItem::applyIconTint() {
    if (mIconDrawable != nullptr && (mHasIconTint || mHasIconTintMode)) {
        mIconDrawable = mIconDrawable->mutate();

        if (mHasIconTint) {
            mIconDrawable->setTintList(mIconTintList);
        }

        if (mHasIconTintMode) {
            mIconDrawable->setTintMode(mIconTintMode);
        }
    }
}

ActionMenuItem::ActionMenuItem(Context* context, int group, int id, int categoryOrder, int ordering,const std::string& title) {
    mContext = context;
    mId = id;
    mGroup = group;
    mCategoryOrder = categoryOrder;
    mOrdering = ordering;
    mTitle = title;
}

int ActionMenuItem::getAlphabeticShortcut()const{
    return mShortcutAlphabeticChar;
}

int ActionMenuItem::getAlphabeticModifiers()const{
    return mShortcutAlphabeticModifiers;
}

int ActionMenuItem::getGroupId()const{
    return mGroup;
}

Drawable* ActionMenuItem::getIcon(){
    return mIconDrawable;
}

Intent* ActionMenuItem::getIntent(){
    return mIntent;
}

int ActionMenuItem::getItemId()const{
    return mId;
}

ContextMenuInfo* ActionMenuItem::getMenuInfo(){
    return nullptr;
}

int ActionMenuItem::getNumericShortcut() const{
    return mShortcutNumericChar;
}

int ActionMenuItem::getNumericModifiers() const{
    return mShortcutNumericModifiers;
}

int ActionMenuItem::getOrder()const{
    return mOrdering;
}

SubMenu* ActionMenuItem::getSubMenu(){
    return nullptr;
}

std::string ActionMenuItem::getTitle(){
    return mTitle;
}

std::string ActionMenuItem::getTitleCondensed(){
    return !mTitleCondensed.empty() ? mTitleCondensed : mTitle;
}

bool ActionMenuItem::hasSubMenu()const{
    return false;
}

bool ActionMenuItem::isCheckable() const{
    return (mFlags & CHECKABLE) != 0;
}

bool ActionMenuItem::isChecked() const{
    return (mFlags & CHECKED) != 0;
}

bool ActionMenuItem::isEnabled() const{
    return (mFlags & ENABLED) != 0;
}

bool ActionMenuItem::isVisible()const{
    return (mFlags & HIDDEN) == 0;
}

MenuItem& ActionMenuItem::setAlphabeticShortcut(int alphaChar){
    mShortcutAlphabeticChar = std::tolower(alphaChar);
    return *this;
}

MenuItem& ActionMenuItem::setAlphabeticShortcut(int alphachar, int alphaModifiers){
    mShortcutAlphabeticChar = std::tolower(alphachar);
    mShortcutAlphabeticModifiers = KeyEvent::normalizeMetaState(alphaModifiers);
    return *this;
}

MenuItem& ActionMenuItem::setCheckable(bool checkable){
    mFlags = (mFlags & ~CHECKABLE) | (checkable ? CHECKABLE : 0);
    return *this;
}

ActionMenuItem& ActionMenuItem::setExclusiveCheckable(bool exclusive) {
    mFlags = (mFlags & ~EXCLUSIVE) | (exclusive ? EXCLUSIVE : 0);
    return *this;
}

MenuItem& ActionMenuItem::setChecked(bool checked) {
    mFlags = (mFlags & ~CHECKED) | (checked ? CHECKED : 0);
    return *this;
}

MenuItem& ActionMenuItem::setEnabled(bool enabled){
    mFlags = (mFlags & ~ENABLED) | (enabled ? ENABLED : 0);
    return *this;
}

MenuItem& ActionMenuItem::setIcon(Drawable* icon){
    mIconDrawable = icon;
    mIconResId = NO_ICON;
    applyIconTint();
    return *this;
}

MenuItem& ActionMenuItem::setIcon(const std::string& iconRes){
    mIconResId = iconRes;
    mIconDrawable = mContext->getDrawable(iconRes);
    applyIconTint();
    return *this;
}

MenuItem& ActionMenuItem::setIconTintList(const ColorStateList* iconTintList){
    mIconTintList = iconTintList;
    mHasIconTint = true;
    applyIconTint();
    return *this;
}

const ColorStateList* ActionMenuItem::getIconTintList(){
    return mIconTintList;
}

MenuItem& ActionMenuItem::setIconTintMode(int iconTintMode){
    mIconTintMode = iconTintMode;
    mHasIconTintMode = true;
    applyIconTint();
    return *this;
}

int ActionMenuItem::getIconTintMode()const{
    return mIconTintMode;
}

MenuItem& ActionMenuItem::setIntent(Intent* intent){
    mIntent = intent;
    return *this;
}

MenuItem& ActionMenuItem::setNumericShortcut(int numericChar){
    mShortcutNumericChar = numericChar;
    return *this;
}

MenuItem& ActionMenuItem::setNumericShortcut(int numericChar, int numericModifiers){
    mShortcutNumericChar = numericChar;
    mShortcutNumericModifiers = KeyEvent::normalizeMetaState(numericModifiers);
    return *this;
}

MenuItem& ActionMenuItem::setOnMenuItemClickListener(const OnMenuItemClickListener& menuItemClickListener){
    mClickListener = menuItemClickListener;
    return *this;
}

MenuItem& ActionMenuItem::setShortcut(int numericChar, int alphaChar){
    mShortcutNumericChar = numericChar;
    mShortcutAlphabeticChar = std::tolower(alphaChar);
    return *this;
}

MenuItem& ActionMenuItem::setShortcut(int numericChar, int alphaChar, int numericModifiers, int alphaModifiers){
    mShortcutNumericChar = numericChar;
    mShortcutNumericModifiers = KeyEvent::normalizeMetaState(numericModifiers);
    mShortcutAlphabeticChar = std::tolower(alphaChar);
    mShortcutAlphabeticModifiers = KeyEvent::normalizeMetaState(alphaModifiers);
    return *this;
}

MenuItem& ActionMenuItem::setTitle(const std::string& title){
    mTitle = title;
    return *this;
}

MenuItem& ActionMenuItem::setTitleCondensed(const std::string& title){
    mTitleCondensed = title;
    return *this;
}

MenuItem& ActionMenuItem::setVisible(bool visible){
    mFlags = (mFlags & HIDDEN) | (visible ? 0 : HIDDEN);
    return *this;
}

bool ActionMenuItem::invoke(){
    if (mClickListener != nullptr && mClickListener(*this)) {
        return true;
    }
    if (mIntent != nullptr) {
        //mContext->startActivity(mIntent);
        return true;
    }
    return false;
}

void ActionMenuItem::setShowAsAction(int show){
    // Do nothing. ActionMenuItems always show as action buttons.
}

MenuItem& ActionMenuItem::setActionView(View* actionView){
    throw std::logic_error("UnsupportedOperationException");
}

View* ActionMenuItem::getActionView(){
    return nullptr;
}

MenuItem& ActionMenuItem::setActionView(const std::string& resId){
    throw std::logic_error("UnsupportedOperationException");
}

ActionProvider* ActionMenuItem::getActionProvider(){
    return nullptr;
}

MenuItem& ActionMenuItem::setActionProvider(ActionProvider* actionProvider){
    throw std::logic_error("UnsupportedOperationException");
}

MenuItem& ActionMenuItem::setShowAsActionFlags(int actionEnum){
    setShowAsAction(actionEnum);
    return *this;
}

bool ActionMenuItem::expandActionView(){
    return false;
}

bool ActionMenuItem::collapseActionView(){
    return false;
}

bool ActionMenuItem::isActionViewExpanded()const{
    return false;
}

MenuItem& ActionMenuItem::setOnActionExpandListener(const OnActionExpandListener& listener){
    // No need to save the listener; ActionMenuItem does not support collapsing items.
    return *this;
}

MenuItem& ActionMenuItem::setContentDescription(const std::string& contentDescription){
    mContentDescription = contentDescription;
    return *this;
}

std::string ActionMenuItem::getContentDescription(){
    return mContentDescription;
}

MenuItem& ActionMenuItem::setTooltipText(const std::string& tooltipText){
    mTooltipText = tooltipText;
    return *this;
}

std::string ActionMenuItem::getTooltipText(){
    return mTooltipText;
}
}/*endof namespace*/

