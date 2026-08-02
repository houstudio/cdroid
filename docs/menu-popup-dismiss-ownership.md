# 菜单弹出层关闭链与所有权设计(CascadingMenuPopup / StandardMenuPopup / MenuPopupHelper / ActionMenuPresenter)

> 分支:`dev` · 日期:2026-08-02
> 关联修复:valgrind 逐条暴露的菜单溢出菜单泄漏链(5 层),全部修复。
> 参考源:`/opt/android-sdk/sources/android-36/`(androidx.appcompat.view.menu)

---

## 0. TL;DR

> 本文覆盖菜单的**两层所有权**:① 弹出层(window/popup/adapter/helper,A–E,§1–§6);② item 层(`MenuItemImpl` / `SubMenuBuilder`,F,§5.6)。两层独立,修法不同——弹出层是"同步重入 dismiss 链不能当场 delete",item 层是"AOSP GC → C++ `delete` 的所有权具象"。

菜单弹出窗口的关闭是**全同步、可重入**的:`PopupWindow::dismiss()` 在最后一句**同步**触发 `OnDismissListener`,而该 listener 在菜单层会一路重入回 `onCloseMenu` / `onDismiss`。因此**绝不能在 dismiss listener 的调用栈内 `delete` 弹出窗口、popup、helper 或 adapter**——否则触发 use-after-free(`ListPopupWindow::dismiss()` 在触发 listener 之后还要访问 `this` 成员)。

本设计用**两套互补的纪律**把"对象释放"推迟到安全点:

- **纪律一 · contained 所有权**:dismiss 链内**不孤立、不删除**对象,改在「下次 show 前 lazy 回收」或「析构」释放。用于 window/popup/adapter。
- **纪律二 · deferred-delete(主 looper MessageQueue)**:dismiss 链末尾把 `delete this` post 到主 looper,等调用栈退栈后下一轮执行。用于 helper 自身(因其 owner 退出时不析构,lazy/dtor 都失效)。

对外接口零改动,完全安卓兼容。

---

## 1. 背景与现象

在带 ActionBar 溢出菜单的 sample 下跑 valgrind,大泄漏修掉后小的逐层浮出,共 5 条 `definitely lost`(按发现顺序):

| # | 对象 | 分配点 | 修法(纪律) |
|---|------|--------|------------|
| A | `MenuPopupWindow` | `CascadingMenuPopup::createPopupWindow` | 一:info 持 window |
| B | `CascadingMenuPopup` | `MenuPopupHelper::createPopup` | 一:getPopup lazy + onDismiss 不置 null |
| C | `ListMenuItemView`(测量用) | `MenuPopup::measureIndividualMenuWidth` | inflate-测完即删 |
| D | `MenuAdapter` | `CascadingMenuPopup::showMenu` | 一:info 持 adapter(先 window 后 adapter) |
| E | `OverflowPopup`/`ActionButtonSubmenu`(helper) | `ActionMenuPresenter::showOverflowMenu`/`onSubMenuSelected` | 二:主 looper deferred 自删 |
| F | `MenuItemImpl`(及所持 `SubMenuBuilder`) | `MenuBuilder::createNewMenuItem` | item 层所有权:见 §5.6 |

A–E 的入口都是 `ActionMenuPresenter::showOverflowMenu()`(ActionBar 溢出按钮);F 的入口是 `invalidateOptionsMenu` → `ToolbarActionBar::populateOptionsMenu` → `menu->clear()`(每次菜单重建),属 item 层、与弹出层无关。

---

## 2. 涉及的类与所有权层次

```
ActionMenuPresenter                                  ← 退出时不析构(ActionMenuView~/Toolbar~ 都不删它)
  ├─ OverflowPopup*   mOverflowPopup      : MenuPopupHelper   ← 泄漏 E
  └─ ActionButtonSubmenu* mActionButtonPopup : MenuPopupHelper ← 泄漏 E
        └─ owns MenuPopup* mPopup = CascadingMenuPopup        ← 泄漏 B
                    └─ vector<CascadingMenuInfo*> mShowingMenus / mRecycledMenus
                              └─ CascadingMenuInfo { MenuPopupWindow* window; MenuAdapter* adapter; ... }
                                                                          ↑泄漏 A        ↑泄漏 D
```

| 类 | 文件 | 关键成员 | 原状态 |
|----|------|----------|--------|
| `ActionMenuPresenter` | menu/actionmenupresenter.{h,cc} | `OverflowPopup* mOverflowPopup; ActionButtonSubmenu* mActionButtonPopup` | **无析构**;onDismiss 置 null 不删 → helper 泄漏(**E**)。且 owner 退出不析构它 |
| `MenuPopupHelper` | menu/menupopuphelper.{h,cc} | `MenuPopup* mPopup` | 析构 `delete mPopup`,但 `onDismiss()` 里 `mPopup=nullptr` → 析构空操作 → **B** |
| `CascadingMenuPopup` | menu/cascadingmenupopup.{h,cc} | `vector<CascadingMenuInfo*> mShowingMenus` | 析构删 info,但 info 不删 window/adapter → **A/D**;`onCloseMenu` erase info 后孤立 |
| `CascadingMenuInfo`(内嵌) | 同上 | `MenuPopupWindow* window` | 析构只 `LOGD`,不删 window/adapter |
| `MenuPopup` | menu/menupopup.{h,cc} | `measureIndividualMenuWidth`(静态) | 测量 inflate 的 view 不释放 → **C** |
| `StandardMenuPopup` | menu/standardmenupopup.{h,cc} | `MenuPopupWindow* mPopup; MenuAdapter* mAdapter` | **无析构、零 delete**(latent,当前死路径) |

### PopupWindow 的注册与销毁(决定能否安全 `delete`)

- `PopupWindow` 对象**本身从不**向 `WindowManager` 注册;只有它的 `mDecorView`(`PopupDecorView`,is-a `Window`)在 `Window` 构造时自动 `WindowManager::addWindow(this)`(cdwindow.cc:55-69)。
- `PopupWindow::dismiss()`(popupwindow.cc:854)→ `dismissImmediate()`(popupwindow.cc:932)→ `((Window*)mDecorView)->close()`。`Window::close()`(cdwindow.cc:893):① 立即 `removeWindow(this)`(**不** delete);② `post` 一个 lambda 异步 `delete self`(decor)。该 lambda 捕获 `self`(decor),**与 PopupWindow 对象生命周期无关**。
- `~WindowManager` 兜底 `delete` 每个残留 window。

**结论:** `delete MenuPopupWindow` 不会与 WindowManager 冲突;decor 异步自删是独立对象。强制退出时仍显式的 popup,decor 由 `~WindowManager` 兜底。

### 虚析构链

`MenuPresenter` 有 `virtual ~MenuPresenter() = default;`,`MenuPopup : MenuPresenter`,`CascadingMenuPopup/StandardMenuPopup : MenuPopup`。故 `delete mPopup`(基类指针)正确走到派生析构。✅

---

## 3. 根因:同步重入 dismiss 链(核心难点)

**调用链(关闭一个 window 时):**

```
[某处] menuPopupWindow->dismiss()
  = ListPopupWindow::dismiss()                    (listpopupwindow.cc:377)
  ├─ mPopup->dismiss()                            (line 378)  ← PopupWindow::dismiss()
  │    ├─ dismissImmediate() → decor->close()     (异步 post delete decor; mDecorView=null; mIsShowing=false)
  │    └─ mOnDismissListener()                    (line 905)  ← ★同步触发★,dismiss() 最后一句
  │         = CascadingMenuPopup::onDismiss()     (window 的 dismiss listener)
  │         └─ dismissedInfo->menu->close(false)
  │              = MenuBuilder::close()           (按索引遍历 mPresenters)
  │              └─ presenter->onCloseMenu(...)
  │                   = CascadingMenuPopup::onCloseMenu()
  │                      ├─ erase info / removeMenuPresenter
  │                      ├─ info->window->dismiss()  ← 二次 dismiss,mIsShowing 已 false → 早退
  │                      └─ count==0 时 mOnDismissListener()
  │                            = (OverflowPopup::onDismiss →) MenuPopupHelper::onDismiss
  └─ removePromptView(); mPopup->setContentView(null); mDropDownList=null; mHandler->...  (379-384) ← ★仍访问 this★
```

**致命点:** `ListPopupWindow::dismiss()` 在 `mPopup->dismiss()`(同步触发 listener)**之后**,还要访问 `this` 成员(`mPromptView`/`mPopup`/`mDropDownList`/`mHandler`,listpopupwindow.cc:379-384)。

若在 `onCloseMenu` / `onDismiss` 里当场 `delete` 该 window(或拥有它的 popup/helper),就等于**在 window 自身 `dismiss()` 的栈内析构它** → `~ListPopupWindow` `delete mPopup` → 回到 379 行 `this`/`mPopup` 已悬空 → **UAF/崩溃**。

> AOSP 全程同步无妨,因 GC 在 listener 链里谁都不"删除"对象;C++ 一旦把所有权具象成 `delete`,同步重入即"在自身方法栈内 delete this"。

---

## 4. 方案评估

### 4.1 方案一:全局异步派发 OnDismissListener(**放弃**)

把 `PopupWindow::dismiss()` 里 `mOnDismissListener()` 改成 post 到 looper,关闭链全异步化,菜单代码退回最简所有权。

放弃:全局影响 Spinner/AutoCompleteTextView/浮动工具栏等所有 Popup 使用者,有些依赖同步 dismiss;且 Android 仅 exit-transition 路径 defer。风险/收益不划算。

### 4.2 方案二:contained 所有权(**采用,纪律一**)

不动 `PopupWindow`,菜单层把"同步重入 dismiss 链"与"对象释放"解耦:

> **铁律:window/popup/adapter 一律在「无 dismiss 在栈」的安全点释放。**

安全点:`MenuPopupHelper::getPopup()`(下次 show 前 lazy)、`~MenuPopupHelper()`、`~CascadingMenuPopup`/`~StandardMenuPopup`。dismiss 链内**只做状态流转,零 delete**。

### 4.3 方案三:deferred-delete via 主 looper MessageQueue(**采用,纪律二**)

helper 自身(OverflowPopup/ActionButtonSubmenu)的特殊性:`ActionMenuPresenter` **退出时根本不析构**(`ActionMenuView::~ActionMenuView` 只删 mMenu、`Toolbar::~Toolbar` 不删 presenter → presenter 是 still-reachable)。故:
- 加 `~ActionMenuPresenter` **没用**(永不执行);
- lazy-delete 对"开一次就退出"**无效**(没有下次 show)。

所以 helper 必须在 **dismiss 时**自我释放,且必须**延迟到关闭链退栈后**。解法:dismiss 链末尾把 `delete this` post 到主 looper 的 MessageQueue(Window::close 同款),下一轮执行——此时链已退栈,安全。这比纪律一彻底(每次 dismiss 当场回收,不依赖 owner 析构,契合 CDROID 退出不拆 view 树)。

**为何安全(见 §6.2):** CDROID looper 派发完回调后只 `recycleUnchecked()`(置空 target,**不解引用 handler**);`~Handler` 自移出 `mHandlers`;甚至内置 `SelfDestroyHandler`(`FLAG_OWNED`/`FLAG_REMOVED`)支持「handler 自毁」。

---

## 5. 最终修复(五层)

### 5.1 CascadingMenuPopup —— window + adapter 所有权(泄漏 A + D)

**(a) `CascadingMenuInfo` 持有 window 和 adapter**(cascadingmenupopup.h):
```cpp
class CascadingMenuPopup::CascadingMenuInfo {
public:
    MenuPopupWindow* window;
    MenuAdapter* adapter;          // ← 新增
    MenuBuilder* menu;
    int position;
    CascadingMenuInfo(MenuPopupWindow* window, MenuAdapter* adapter, MenuBuilder* menu, int position);
    ...
};
```

**(b) 析构释放,顺序关键**(cascadingmenupopup.cc `~CascadingMenuInfo`):
```cpp
// 切勿在此 dismiss():本析构也从 ~CascadingMenuPopup 走到,dismiss 会重入 onCloseMenu
// 并改写正在遍历的 vector。
// 顺序:先 window 再 adapter。ListView(~PopupWindow 拆除时)只借用 adapter、
// 析构里要 unregisterDataSetObserver;先删 adapter 会让它访问已释放内存 → UAF。
delete window;
window = nullptr;
delete adapter;
adapter = nullptr;
```

**(c) `mRecycledMenus` 承接被关闭的 info**(cascadingmenupopup.h):dismiss 链内不能当场删 info(可能在该 window 自身 dismiss 栈上),故 `onCloseMenu` erase 后 `mRecycledMenus.push_back(info)`,留待 popup 销毁时释放。`~CascadingMenuPopup` 统一 delete `mShowingMenus` + `mRecycledMenus`(info 析构不 dismiss,遍历不重入)。

### 5.2 MenuPopupHelper —— popup 生命周期(泄漏 B)

- `onDismiss` **不再** `mPopup=nullptr`(链内同步 delete mPopup 会 UAF);
- `getPopup` 下次 show 前 lazy 回收已 dismiss 的旧 popup(安全点);
- `~MenuPopupHelper` 的 `delete mPopup` 兜底。

`isShowing()=(mPopup!=null)&&mPopup->isShowing()`,关闭后 `mPopup->isShowing()` 为 false(mShowingMenus 空),语义与 AOSP"置 null 后下次重建"一致。

### 5.3 measureIndividualMenuWidth —— 测量视图(泄漏 C)

`MenuPopup::measureIndividualMenuWidth`(menupopup.cc)原用 convertView 回收 + 靠 GC,但 convertView 在 item-type 切换时被孤立、early-return 时当前 view 也漏。改为:
```cpp
for (int i = 0; i < count; i++) {
    // 测量一次性 view:每次 inflate 新的、测完即删。MenuAdapter::getView 不保留 view。
    View* itemView = adapter->getView(i, nullptr, parent);
    itemView->measure(widthMeasureSpec, heightMeasureSpec);
    const int itemWidth = itemView->getMeasuredWidth();
    delete itemView;                 // early-return 之前已删,无遗漏
    if (itemWidth >= maxAllowedWidth) return maxAllowedWidth;
    else if (itemWidth > maxWidth) maxWidth = itemWidth;
}
```
菜单项少,inflate 开销可忽略;测得宽度与是否回收无关。

### 5.4 StandardMenuPopup —— 单 window + adapter(latent)

补 `~StandardMenuPopup(){ delete mPopup; delete mAdapter; }`(不 dismiss,避免重入;decor 由 `~WindowManager` 兜底)。当前死路径(`enableCascadingSubmenus||1`),但已无泄漏。

### 5.5 ActionMenuPresenter —— helper deferred 自删(泄漏 E)

`OverflowPopup::onDismiss` / `ActionButtonSubmenu::onDismiss`(都 extends MenuPopupHelper)在 dismiss 链末尾:
```cpp
void ActionMenuPresenter::OverflowPopup::onDismiss() {
    if (mPresenter->mMenu != nullptr) mPresenter->mMenu->close();
    mPresenter->mOverflowPopup = nullptr;        // presenter 先放手
    MenuPopupHelper::onDismiss();
    // 链内不能 delete(会从 window 自身 dismiss 栈内析构它→UAF);
    // 交给主 looper 的 MessageQueue,下一轮再自删(Window::close 同款)。
    OverflowPopup* self = this;
    Handler* h = new Handler(Looper::getMainLooper());
    h->post([self, h](){ delete self; delete h; });
}
```

helper 自删 → `~MenuPopupHelper` → `delete mPopup` → `~CascadingMenuPopup` → 释放 windows + adapters(此时 dismiss 链早退栈,安全)。与 5.1/5.2 两层协同,不冲突。`ActionButtonSubmenu::onDismiss` 同构。

### 5.6 MenuItemImpl 层 —— item 所有权(泄漏 F)

> 与 A–E 不同层:不在 dismiss 链上,无重入风险;纯粹是 AOSP GC → C++ `delete` 的所有权具象。

**根因:** `MenuBuilder` 拥有 `mItems` 里的 `MenuItemImpl*`(`~MenuBuilder` 遍历 `delete`),但 `clear()` / `removeItemAtInt()` 只 `erase` / `clear` vector、**不 delete**。AOSP `MenuBuilder.clear()` 同样只 `mItems.clear()`,靠 GC 回收;C++ 不删则每次 `invalidateOptionsMenu()` 重建(`clear` + 重新 `add`)都漏掉上一批 item。`MenuPopupHelper` / `MenuPopupWindow` 持的是 item 的**借用**,不持所有权,故 item 必须由 `MenuBuilder` 释放。

**修复(三处,全在 .cc,无 ABI 影响):**

1. `MenuBuilder::clear()` —— 清空前 `for(auto item:mItems) delete item;`。**先删后 `onItemsChanged`**:`updateMenuView` 重读 `getVisibleItems()`(已空),旧 item view 由 `removeViewAt` 摘除——`removeViewInternal` 只 `removeFromArray`、不同步删 view,且 item view 析构不解引用 `mItemData`(`ActionMenuItemView::~` 只删 `mForwardingListener`),故删 item 在前安全。
2. `MenuBuilder::removeItemAtInt()` —— `erase` 后 `delete item`(`removeItem` / `removeGroup` 共用)。
3. `MenuItemImpl::~MenuItemImpl()` —— `delete mSubMenu`。`MenuItemImpl` 是 `SubMenuBuilder` 的**唯一所有者**:`addSubMenu` / `performItemAction` `new` 后存入 `mSubMenu`,全库无别处释放(`MenuPopupHelper::~` 只 `delete mPopup`、不删借来的 `mMenu`)。不删则带子菜单的 item 在 `clear` 时连带泄漏 SubMenuBuilder;`SubMenuBuilder` 的 `~MenuBuilder` 释放其自身 items、**不解引用 `mItem` 回指针**,安全。

**与弹出层的边界:** 弹出层(A–E)解决"dismiss 链内不能 delete 持有 window 的对象";item 层(F)解决"menu 持有的 item 离开时必须释放"。二者唯一潜在交集——"`clear()` 时某 item 的子菜单弹出层正显示"——在常规 options-menu 流程不会发生(invalidate 不在子菜单交互中触发),且属 CDROID 裸指针共享占有的既有设计范畴(见 §8),不在本修范围。

---

## 6. 安全性验证

### 6.1 逐路径(window 是否会在自身 dismiss listener 栈内被删)

| # | 触发场景 | window 何时被删 | dismiss 在栈? |
|---|---------|----------------|---------------|
| 1 | 正常关闭(选项/点外)→ `onCloseMenu` | helper deferred 自删 → ~CascadingMenuPopup | 否 ✅ |
| 2 | 外部 `menu->close()` → `onCloseMenu` | 同上 | 否 ✅ |
| 3 | 反复开关子菜单(级联) | info 移入 `mRecycledMenus`,popup 销毁时释放 | 否 ✅ |
| 4 | 强制退出时仍显式 | info 析构 `delete window`(不 dismiss);decor 由 `~WindowManager` 兜底 | 否 ✅ |

**不变式:** dismiss-listener 一路到 `onCloseMenu`/`onDismiss`,**链内零 delete**(只状态流转 + post);window/popup/adapter/helper 仅在 `getPopup`、各析构、deferred 回调释放——均无 dismiss 在栈。

### 6.2 deferred-delete 为何安全(handler 自毁)

回调里 `delete self` 连带 `delete h`(handler),而此时正处在 h 的 `dispatchMessage` 栈上。安全依据(均经源码核实):
1. `Looper::drainMessageQueue`(looper.cc:286-287)派发后**只 `msg->recycleUnchecked()`**——把 `msg->target` 置空,**不解引用 handler**;
2. `Handler::~Handler()` 调 `mLooper->removeHandler(this)` + 清队列;
3. `Looper::removeHandler`(looper.cc:857-867)对外部拥有的 handler **立即从 `mHandlers` 擦除**(注释明说"外部 delete 后留悬挂项 → doEventHandlers 读已释放内存 → UAF");looper-owned 的则 `FLAG_REMOVED` 延迟到 `doEventHandlers` 删——这正是内置 `SelfDestroyHandler` 模式。

故 `delete h` 在自身回调末尾执行,派发返回后 looper 不再碰它,mHandlers 也已干净。

### 6.3 为何 `mRecycledMenus` 不是过度设计

级联里 `onCloseMenu` 必须把 info 从活动列表(`mShowingMenus`,供 isShowing/索引/count)移除,但此刻不能释放它(可能在该 window 的 dismiss 栈上)。被移除的 info 须由某处持有到安全释放点——`mRecycledMenus` 即此。单 window 的 StandardMenuPopup 直接拥有 mPopup,不需要它。每次完整关闭后 popup 进入 dismissed,下次 getPopup 回收整个 popup(含 mRecycledMenus),不无限增长。

---

## 7. 验证方法

1. **构建**:`cd outX64-Debug && make cdroid -j44`(全在 libcdroid.so,绿)。
2. **ABI**:改动头文件仅限 `src/gui/menu/` 内部类(CascadingMenuInfo 加成员/签名、StandardMenuPopup 加析构声明)。sample 走 ActionBar/PopupMenu 公共 API,不直接构造这些类,故动态链接的 sample 无需重编——直接重跑。
3. **valgrind 回归**:原 sample 重跑,确认 A–E 五条 `definitely lost` 全消失,且不引入新的 `Invalid read/write`(UAF)。菜单链路应只剩 0 条 definitely lost。

> 参考:[[stale-binary-abi-mismatch-valgrind]]——若报"N bytes after block of size M"且块大小像整数,多半是 sample 带旧 sizeof(ABI 错配),`make <sample>` 重编。

---

## 8. 已知遗留 / 后续

- **`core/path.cc:26` 静态 `mPathSurface`**(1×1 A8 scratch surface):退出时 lost,根因是 **app 退出不拆 view 树** → 泄漏的 Path 实例的 Context 持该 surface → refcount 不归零。非性增长(~5KB 一次性),根治须排查 view 树析构,**跨子系统,本次未处理**。
- **`menupopuphelper.cc:130` 的 `enableCascadingSubmenus||1`**:强制走 CascadingMenuPopup,与安卓"按最小屏宽判定"不符。两路径现已都无泄漏,可安全恢复——属功能行为变更,待决策。
- **`StandardMenuPopup::onSubMenuSelected` 里 `new MenuPopupHelper(...)`(standardmenupopup.cc:195)**:返回值未持有/未释放,另一处潜在泄漏,死路径,未处理。
- **`PopupDecorView::mPop`**:回指 PopupWindow,全库只 `=nullptr` 从未赋值;`onTouchEvent` 无条件 `mPop->dismiss()` 是潜在 null-deref,未触及。
- **`localfloatingtoolbarpopup.cc:130-154`**:本仓库"dismiss 先于 delete、且不在 listener 内"的正解范例;本设计的 deferred-delete(纪律二)是其泛化(允许在 listener 内 post)。

---

## 9. 改动文件清单

| 文件 | 改动 |
|------|------|
| src/gui/menu/cascadingmenupopup.h | `CascadingMenuInfo` 加 `adapter` 成员 + ctor 形参;新增 `mRecycledMenus` |
| src/gui/menu/cascadingmenupopup.cc | `~CascadingMenuInfo` 删 window + adapter(先 window 后 adapter);`~CascadingMenuPopup` 释两列表;`onCloseMenu` 移入 `mRecycledMenus`;`showMenu` 传 adapter 给 info |
| src/gui/menu/menupopuphelper.cc | `onDismiss` 不置 null;`getPopup` lazy 回收 |
| src/gui/menu/menupopup.cc | `measureIndividualMenuWidth` 测量 view 测完即删 |
| src/gui/menu/standardmenupopup.h | 新增 `~StandardMenuPopup()` 声明 |
| src/gui/menu/standardmenupopup.cc | 新增 `~StandardMenuPopup()`(删 mPopup + mAdapter) |
| src/gui/menu/actionmenupresenter.cc | `OverflowPopup`/`ActionButtonSubmenu::onDismiss` 主 looper deferred 自删;加 `<core/handler.h>` `<core/looper.h>` |
| src/gui/menu/menubuilder.cc | `clear()` / `removeItemAtInt()` 删 item(item 层所有权,§5.6) |
| src/gui/menu/menuitemimpl.cc | `~MenuItemImpl()` `delete mSubMenu`(sole owner) |

对外接口零改动(含 item 层:仅 .cc,无头文件 / sizeof 变化)。
