public class ActionMenu implements Menu {
private:
    Context* mContext;
    bool mIsQwerty;
    std::vector<ActionMenuItem*> mItems;

    public ActionMenu(Context* context) {
        mContext = context;
    }

    public Context getContext() {
        return mContext;
    }

    public MenuItem add(const std::string& title) {
        return add(0, 0, 0, title);
    }

    public MenuItem add(int titleRes) {
        return add(0, 0, 0, titleRes);
    }

    public MenuItem add(int groupId, int itemId, int order, int titleRes) {
        return add(groupId, itemId, order, mContext.getResources().getString(titleRes));
    }

    public MenuItem add(int groupId, int itemId, int order, const std::string& title) {
        ActionMenuItem* item = new ActionMenuItem(getContext(),groupId, itemId, 0, order, title);
        mItems.insert(mItems.begin()+order, item);
        return item;
    }

    public int addIntentOptions(int groupId, int itemId, int order,
            ComponentName caller, Intent[] specifics, Intent intent, int flags,
            MenuItem[] outSpecificItems) {
        PackageManager pm = mContext.getPackageManager();
        final List<ResolveInfo> lri = pm.queryIntentActivityOptions(caller, specifics, intent, 0);
        final int N = lri != null ? lri.size() : 0;

        if ((flags & FLAG_APPEND_TO_GROUP) == 0) {
            removeGroup(groupId);
        }

        for (int i=0; i<N; i++) {
            final ResolveInfo ri = lri.get(i);
            Intent rintent = new Intent(
                ri.specificIndex < 0 ? intent : specifics[ri.specificIndex]);
            rintent.setComponent(new ComponentName(
                    ri.activityInfo.applicationInfo.packageName,
                    ri.activityInfo.name));
            final MenuItem item = add(groupId, itemId, order, ri.loadLabel(pm))
                    .setIcon(ri.loadIcon(pm))
                    .setIntent(rintent);
            if (outSpecificItems != null && ri.specificIndex >= 0) {
                outSpecificItems[ri.specificIndex] = item;
            }
        }

        return N;
    }

    public SubMenu addSubMenu(const std::string& title) {
        // TODO Implement submenus
        return nullptr;
    }

    public SubMenu addSubMenu(int titleRes) {
        // TODO Implement submenus
        return nullptr;
    }

    public SubMenu addSubMenu(int groupId, int itemId, int order,const std::string& title) {
        // TODO Implement submenus
        return nullptr;
    }

    public SubMenu addSubMenu(int groupId, int itemId, int order, int titleRes) {
        // TODO Implement submenus
        return nullptr;
    }

    public void clear() {
        mItems.clear();
    }

    public void close() {
    }

    private int findItemIndex(int id) {
        const int itemCount = mItems.size();
        for (int i = 0; i < itemCount; i++) {
            if (mItems.at(i)->getItemId() == id) {
                return i;
            }
        }

        return -1;
    }

    public MenuItem* findItem(int id) {
        return mItems.at(findItemIndex(id));
    }

    public MenuItem* getItem(int index) {
        return mItems.at(index);
    }

    public bool hasVisibleItems() {
        const int itemCount = mItems.size();
        for (int i = 0; i < itemCount; i++) {
            if (mItems.at(i)->isVisible()) {
                return true;
            }
        }

        return false;
    }

    private ActionMenuItem findItemWithShortcut(int keyCode, KeyEvent& event) {
        // TODO Make this smarter.
        const bool qwerty = mIsQwerty;
        const int itemCount = mItems.size();
        const int modifierState = event.getModifiers();
        for (int i = 0; i < itemCount; i++) {
            ActionMenuItem* item = mItems.at(i);
            const int shortcut = qwerty ? item->getAlphabeticShortcut() : item->getNumericShortcut();
            const int shortcutModifiers = qwerty ? item->getAlphabeticModifiers() : item->getNumericModifiers();
            const bool is_modifiers_exact_match = (modifierState & SUPPORTED_MODIFIERS_MASK)
                    == (shortcutModifiers & SUPPORTED_MODIFIERS_MASK);
            if ((keyCode == shortcut) && is_modifiers_exact_match) {
                return item;
            }
        }
        return null;
    }

    public bool isShortcutKey(int keyCode, KeyEvent& event) {
        return findItemWithShortcut(keyCode, event) != nullptr;
    }

    public bool performIdentifierAction(int id, int flags) {
        const int index = findItemIndex(id);
        if (index < 0) {
            return false;
        }

        return mItems.at(index)->invoke();
    }

    public bool performShortcut(int keyCode, KeyEvent& event, int flags) {
        ActionMenuItem* item = findItemWithShortcut(keyCode, event);
        if (item == nullptr) {
            return false;
        }

        return item->invoke();
    }

    public void removeGroup(int groupId) {
        int itemCount = mItems.size();
        int i = 0;
        while (i < itemCount) {
            if (mItems.at(i)->getGroupId() == groupId) {
                mItems.remove(i);
                itemCount--;
            } else {
                i++;
            }
        }
    }

    public void removeItem(int id) {
        mItems.remove(findItemIndex(id));
    }

    public void setGroupCheckable(int group, bool checkable,bool exclusive) {
        const int itemCount = mItems.size();

        for (int i = 0; i < itemCount; i++) {
            ActionMenuItem* item = mItems.at(i);
            if (item->getGroupId() == group) {
                item->setCheckable(checkable);
                item->setExclusiveCheckable(exclusive);
            }
        }
    }

    public void setGroupEnabled(int group, bool enabled) {
        const int itemCount = mItems.size();

        for (int i = 0; i < itemCount; i++) {
            ActionMenuItem* item = mItems.at(i);
            if (item->getGroupId() == group) {
                item->setEnabled(enabled);
            }
        }
    }

    public void setGroupVisible(int group, bool visible) {
        const int itemCount = mItems.size();

        for (int i = 0; i < itemCount; i++) {
            ActionMenuItem* item = mItems.at(i);
            if (item->getGroupId() == group) {
                item->setVisible(visible);
            }
        }
    }

    public void setQwertyMode(bool isQwerty) {
        mIsQwerty = isQwerty;
    }

    public int size() {
        return mItems.size();
    }
}
