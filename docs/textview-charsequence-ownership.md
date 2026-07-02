# TextView 的 CharSequence 所有权与生命周期设计

> 适用范围:`cdroid::TextView` 及其文本相关成员(`mText` / `mTransformed` / `mHint` / `mCharWrapper`)。
> 配套文档:[span-memory-design.md](span-memory-design.md)(Span 容器内部的 owned / clone / NoCopySpan 模型)。
> 最后更新:2026-07-02

---

## 1. 背景:为什么需要这套规则

Android 的 `TextView.java` 用 Java 引用持有 `CharSequence`,`mText` / `mTransformed` 直接重新赋值即可,旧对象由 GC 回收。

CDROID 把 TextView 逐行移植到 C++,文本对象用**裸指针 + 手动 `new`/`delete`**(不引入智能指针,与 [span-memory-design.md](span-memory-design.md) 的整体方针一致)。因此**「生产/持有的 CharSequence 由谁、在何时释放」必须由 TextView 自己显式管理**——这正是 Android 源码不会写、移植时必须补上的语义缺口。

本文件给出这套管理的**明确规则**与一个**硬性生命周期约束**。

---

## 2. 硬性约束:文本必须比引用它的 DynamicLayout 活得更久

> 这条约束是 2026-07-02 用一次真实崩溃换来的结论,务必遵守。

`DynamicLayout` 的析构函数会**解引用它的基文本 `mBase`(即 TextView 的 `mText`)**:

```cpp
// src/gui/text/dynamiclayout.cc  ~DynamicLayout()
if (mWatcher) {
    if (Spannable* sp = dynamic_cast<Spannable*>(mBase)) {   // mBase == TextView::mText
        sp->removeSpan(mWatcher);                            // ← 解引用 mBase
    }
    delete mWatcher;
}
```

**原因**:DynamicLayout 把自己的 watcher 作为一个 NoCopySpan(借用)挂在基文本上;析构时要把它从文本里 `removeSpan` 掉,以免文本里残留指向已删 watcher 的悬空指针。这个 `removeSpan` 需要基文本仍然有效。

**推论**:**任何被 DynamicLayout 通过 `mBase` 引用的文本对象,都必须在该 DynamicLayout 被销毁之前保持有效。** 反过来说:**不能在引用它的 DynamicLayout 仍然存活时释放文本。**

违反它的后果:`makeNewLayout` 销毁旧 layout 时,`~DynamicLayout` 对已经 `delete` 过的 `mBase` 做 `dynamic_cast` → 在 `__dynamic_cast` 里崩溃(实测栈:NumberPicker 滚动 → `EditText::setText` → `checkForRelayout` → `makeNewLayout` → `~DynamicLayout`)。

---

## 3. 所有权规则

| 成员 | 归属 | 说明 |
|---|---|---|
| `mText` | **TextView 拥有(转移所有权)** | `setText(CharSequence*)` 即转移:调用方传堆对象,调用后**不得再释放或复用**。需保留副本请传 clone。 |
| `mTransformed` | **条件拥有** | `mTransformation == nullptr` 时 `mTransformed = mText`(别名,**不拥有**,随 mText 释放);否则 `mTransformed = getTransformation(...)` 返回的**新对象**(**TextView 拥有**)。判定所有权的唯一依据:`mTransformed == mText` 吗。 |
| `mHint` | **TextView 拥有(转移所有权)** | 同 mText。特例:`setHint(getText())` 使 `mHint` 别名 `mText` → 此时 hint 路径不拥有。 |
| `mCharWrapper` | **复用型成员** | 仅供 string/vector 重载复用,**setText 中永不释放**,析构释放一次。所有释放逻辑必须排除它。 |

### 3.1 调用方契约(富文本高频路径)
`setText(CharSequence*)` 在富文本里常用(如 `new SpannableStringBuilder(...)` 设 span 后 setText)。规则是**转移所有权**:

```cpp
auto* ssb = new SpannableStringBuilder(...);
ssb->setSpan(new StyleSpan(...), ...);
tv->setText(ssb);          // 之后 ssb 归 tv,调用方不得再用 ssb
// 若要继续持有/修改,改传 clone:tv->setText(ssb->clone());
```

**不支持**多个 TextView 共享同一个 Spannable(析构会 double-free),需各自 clone。span 容器已支持深拷贝(见 span-memory-design.md)。

### 3.2 `getTransformation` 契约
`TransformationMethod::getTransformation(src, view)` 必须**返回堆分配的 CharSequence**,所有权交给 TextView(=调用方)。TextView 在采纳新结果前释放上一个。`mTransformation` 指针本身是借用(Password 是单例;AllCaps 若堆分配需另行约定,见 §6 待办)。

---

## 4. 释放时机(安全模式)

把约束 §2 翻译成两条可落地的释放规则:

### 4.1 析构:先删 layout,再删 text
`~TextView` 中 `delete mLayout; delete mHintLayout;` 必须在 `delete mText / mTransformed / mHint` **之前**。

```cpp
// src/gui/widget/textview.cc  ~TextView()
delete mBoring;
delete mHintBoring;
delete mLayout;        // ← 先销毁 layout(其析构会 removeSpan,需要 mText 仍有效)
delete mHintLayout;
mLayout = mHintLayout = nullptr;
/* ... 文本别名 guard ... */
delete mCharWrapper;
delete mChangeWatcher;
delete mText;          // ← 再释放文本
delete mTransformed;
delete mHint;
```

别名 guard(原有,保留):`mText==mTransformed` / `mTransformed==mCharWrapper` / `mText==mCharWrapper` 时先把别名置空,保证每个对象只释放一次;另加 `mHint==mText||mHint==mCharWrapper` 防 `setHint(getText())` double-free。

### 4.2 setText:在 checkForRelayout **之后**释放旧文本
`setText` 的核心 4 参重载,在函数入口捕获旧值,在 `checkForRelayout()` 之后释放:

```cpp
// src/gui/widget/textview.cc  setText(text, type, notifyBefore, oldlen)
CharSequence* prevText       = mText;        // 入口捕获
CharSequence* prevTransformed = mTransformed;
/* ... setTextInternal(text)、变换段、span 装配 ... */
if (mLayout != nullptr) checkForRelayout();  // ← 关键:先让 layout 释放对旧文本的引用
/* 此处释放 prevText / prevTransformed(带 guard) */
```

**为什么 checkForRelayout 之后是安全的**:`checkForRelayout()` 的三条出口,在到达释放点时都不再有 DynamicLayout 引用旧文本——

1. **静态宽度路径** → `makeNewLayout` 重建,新 layout 引用**新** `mText`/`mTransformed`;
2. **非静态宽度路径** → `nullLayouts()` 销毁旧 DynamicLayout、`mLayout=null`;
3. `mLayout` 本就为 `null` → `if(mLayout)` 跳过 checkForRelayout,本就没有 layout 引用旧文本。

> ⚠️ **不要**把释放放进 `setTextInternal`(即在 `mText = text` 那一刻就删旧值)。那是「layout 还引用着旧文本、却已把旧文本 delete」的状态,下一次 `makeNewLayout` 销毁旧 layout 时必崩。这正是 2026-07-02 那次崩溃的根因。

### 4.3 释放时的 aliasing guard
释放 `prevText` / `prevTransformed` 时,以下对象**不得释放**(仍在用 / 归属他处):
- incoming `text`(可能等于旧 `mText`,如 `setTransformationMethod → setText(mText)` 再入);
- `mCharWrapper`(复用成员);
- 当前的 `mText` / `mTransformed`(setText 赋值后的新值);
- `prevTransformed` 若别名 `prevText`(无变换时 `mTransformed==mText`),只通过 `prevText` 释放一次。

---

## 5. 当前实现状态(2026-07-02)

| 位置 | 状态 |
|---|---|
| `~TextView`:layout 先于 text 删除 + mHint 别名 guard | ✅ 已实现 |
| `setText`:入口捕获 + checkForRelayout 后释放旧 mText/mTransformed | ✅ 已实现 |
| `setTextInternal` / `setHintInternal` | ⛔ **不在此释放**(只有说明性注释)——必须走 §4.2 的 checkForRelayout 之后路径 |
| 构造函数初始 `new SpannedString` | ✅ 由首次 setText 的 §4.2 路径正确释放 |

构建验证:`cd outX64-Debug && make cdroid -j44` 通过;NumberPicker→EditText setText 场景崩溃已解决。

---

## 6. 仍待办(均可用 §4 的安全模式补齐)

1. **`setEditable`**(`textview.cc`,约 `new SpannableStringBuilder(mText)` 处):把 mText 升级为 Editable 后,旧 mText 泄漏。修法:捕获旧 mText → `if(mLayout) checkForRelayout();` → 带 guard 释放旧 mText(注意同步处理 `mTransformed`/`mHint` 对旧 mText 的别名)。
2. **`setHintInternal`**:旧 mHint 泄漏。同模式(`setHintInternal` 内已有 `checkForRelayout`,在其后释放旧 mHint,排除别名)。
3. **`mTransformation` 指针本身的归属**:AllCaps 风格(`new AllCapsTransformationMethod(getContext())`)是堆对象,目前 `setTransformationMethod` 替换时不释放 → 泄漏;Password 是单例不应释放。需约定一个所有权标志(如 `mOwnsTransformation`)或统一规则。
4. **ImageSpan / Html `<img>` 的 Drawable**:`ImageSpan::getDrawable()` 每次 `mContext->getDrawable()` 产生新 Drawable 且不缓存/不释放 → 每次布局/绘制泄漏。应仿 Android 缓存进 `mDrawable` 并由 ImageSpan 拥有;`ImageGetter` 的 Drawable 所有权也需明确。

---

## 7. 与 Span 容器所有权的关系

本文件只讨论 **TextView 层面持有的 CharSequence 指针** 的归属与释放。CharSequence 内部(如 `SpannableStringBuilder` 里各 span)的 owned / clone / NoCopySpan 模型由 [span-memory-design.md](span-memory-design.md) 规定,二者互补:

- 本文件解决:**整个 CharSequence 对象**何时被 TextView `delete`;
- span-memory-design.md 解决:CharSequence 被 `delete` 时,**其内部哪些 span** 跟着释放、哪些是借用。

两者共用同一个「裸指针 + 手动管理」总方针,不引入智能指针。
