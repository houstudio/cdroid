# CDROID Span 内存管理与使用说明

> 适用范围:`cdroid::ParcelableSpan` 体系及所有挂入 `Spannable` 文本的 span(style span、selection marker、watcher 等)。
> 对标:Android 官方 `android.text.*` 的 span 引用语义 + GC,在 C++ 无 GC 环境下的确定性等价实现。
> 落地版本:2026-07-01。

---

## 1. 背景与目标

### 1.1 旧实现的问题

CDROID 早期 span 存储是裸指针数组,存在三类确定性内存缺陷:

| 缺陷 | 旧代码位置 | 后果 |
|---|---|---|
| **泄漏** | `SpannableStringInternal` 析构 `= default`、`removeSpan` 只 erase、`clearSpans` 只 clear | 所有 `new` 出来的 style span 永不释放(Html 解析每次泄漏数十个) |
| **UAF/悬空** | 拷贝文本时共享裸指针(拷贝构造、`subSequence`、`TextUtils::copySpansFrom`) | 副本与原件互为悬空指针源 |
| **所有权混乱** | watcher 既是外部对象成员、又被塞进文本的 mSpans | `DynamicLayout::mWatcher` 析构不删(泄漏);若被容器删则与外部 owner 双重释放 |

### 1.2 设计目标

1. **确定性、单一所有者**:无 GC 也要明确"谁负责释放"。
2. **不使用 smart_ptr**:`shared_ptr`/`unique_ptr` 与 Android 的引用语义风格不符(项目明确要求),且 span 体系需要的是"容器拥有 or 借用"的二元判定,不是引用计数。
3. **全面兼容 Android 语义**:成员变量名、方法签名、`NoCopySpan` 拷贝过滤、selection 单例身份语义均与官方一致。
4. **可验证**:任何修改后 `cd outX64-Debug && make cdroid -j44` 必须绿。

---

## 2. 设计总览

整套模型基于**一个二元判定 + 一个克隆协议**:

```
                ┌─────────────────────────────────────┐
   setSpan ───► │ addSpan() 计算 owned 标志并缓存       │
                │   owned = (dynamic_cast<NoCopySpan*> │
                │            (span) == nullptr)        │
                └──────────────┬──────────────────────┘
                               │
              ┌────────────────┴────────────────┐
              ▼                                 ▼
        owned == true                       owned == false
      (非 NoCopySpan)                       (NoCopySpan)
              │                                 │
   容器拥有其生命周期                       容器只是借用
   • dtor / removeSpan / clearSpans         • 永不 delete
     里 disposeSpan() 真删                  • 永不 clone
   • 拷贝文本时 clone() 深拷贝              • 拷贝文本时按
                                             ignoreNoCopySpan 跳过或共享裸指针
```

**一句话**:`owned` 不是"追踪这个 span 是不是当前容器分配的"(那会退化成变相引用计数),而是**缓存的 `!NoCopySpan` 类型判定**——是 span 类型层面的属性,在 `setSpan` 插入时算一次存下来。

### 2.1 为什么按 NoCopySpan 分类,而不是按 ParcelableSpan 的子类

`ParcelableSpan` 的直接子类(`CharacterStyle` / `ParagraphStyle` / `UpdateAppearance` / `CharSequence` / `SpellCheckSpan` / `NoCopySpan`)是**行为/语义分类**,与内存所有权**正交**。Android 自己也是统一 GC,从不按这些子类区别对待内存。唯一与所有权相关的标记是 `NoCopySpan`,所以判定只切这一刀,不为其它子类写分支。

---

## 3. 核心契约(开发者必读)

### 契约 1:setSpan 的所有权转移

```cpp
void setSpan(const ParcelableSpan* what, int start, int end, int flags);
```

- **非 NoCopySpan(owned)**:调用方 `new` 出来后,**所有权移交给 Spannable**。调用方此后不得再 `delete` 它,也不得把它塞进第二个 Spannable(否则双重释放)。想跨容器使用,先 `clone()`。
- **NoCopySpan(borrowed)**:调用方保留所有权,Spannable 只存指针、不释放、不拷贝。watcher、selection 单例走这条。

> 这条契约与 Java `setSpan(Object)` 完全对应——只是把"GC 管可达性"换成"非 NoCopySpan 即归容器"。

### 契约 1a:同一个 span 实例能不能 set 多次?(关键,易忘)

直接关系到会不会双重释放,分三种情况记牢:

| 场景 | 是否允许 | 行为 |
|---|---|---|
| **同一 Spannable** 内,set 同一 span 多次(不同区间) | ✅ 允许 | 去重:就地更新 `start/end/flags`(= Android 的"重绑区间"),只保留**一条**记录;`SpannableString` 还会发 `spanChanged` |
| **跨 Spannable**,同一 **owned** span 实例 | ❌ 禁止 | 两个容器都记 `owned=true` → 各自析构 `delete` 同一对象 → **double-free**。要同款样式请 `clone()` 或重新 `new` |
| **跨 Spannable**,同一 **NoCopySpan** 实例(selection 单例 / watcher) | ✅ 允许 | borrowed,永不删除,本就设计为可共享(如 `SELECTION_START` 同时挂多段有选区的文本) |

- **为什么跨容器 owned 不行**:Android 靠 GC 共享引用,CDROID 是"单一所有者 + 确定性释放",一个对象不能有两个 owner。
- **为什么代码不拦**:检测"此 span 是否已被别的容器拥有"需要全局归属登记,很重且不安卓;靠契约保证。真实调用模式(`setSpan(new StyleSpan(...))` 这种 inline new 后不复用)天然不触犯。
- **速记**:owned span 实例**一次只能属于一段文本**;同段内重复 set 只是改区间,跨段必须 clone。

### 契约 2:owned span 在容器销毁/移除时被释放

容器(`SpannableStringInternal` 体系)在以下 4 处释放 owned span:

| 时机 | 实现 |
|---|---|
| 容器析构 | `~SpannableStringInternal()` → `deleteAllOwnedSpans()` |
| `removeSpan` | `SpannableString::removeSpan` / `SSB::removeSpan` → `disposeSpan()` |
| `clearSpans` / `clear` | `deleteAllOwnedSpans()` |
| 拷贝构造/赋值赋值前 | `operator=` 先 `deleteAllOwnedSpans()` 释放自身旧的 owned span |

borrowed(NoCopySpan)span 在以上任何处都**不被释放**。

### 契约 3:拷贝文本时 owned span 深拷贝,borrowed span 跳过/共享

拷贝路径(拷贝构造、`subSequence`、`TextUtils::copySpansFrom`、SSI 拷贝赋值)统一走 `appendSpanCopy`:

- owned → `span->clone()` 产生独立副本(断言非空);
- NoCopySpan + `ignoreNoCopySpan==true` → 跳过(= Android `ignoreNoCopySpan`);
- NoCopySpan + `ignoreNoCopySpan==false` → 共享裸指针(借用,永不删除,安全)。

**绝不共享 owned 指针**——否则两个容器都以为自己拥有同一对象 → 双重释放。

---

## 4. 内部实现

### 4.1 存储结构

[spannablestring.h](../src/gui/text/spannablestring.h) `SpannableStringInternal`:

```cpp
struct SpanRecord {
    ParcelableSpan* span;   // 非 const;const 在 addSpan() 里唯一地 cast 掉
    int start;
    int end;
    int flags;
    bool owned;             // true => 容器管理其生命周期(非 NoCopySpan)
};
std::u16string mText;
std::vector<SpanRecord> mSpans;
```

### 4.2 集中 helper(owned/borrowed 逻辑只存在于这些方法)

| helper | 作用 |
|---|---|
| `addSpan(span, start, end, flags)` | **唯一** `const_cast` 点 + 计算 `owned` + push。两处 `setSpan` 都走它 |
| `removeSpanRecord(span)` | 按指针身份找首条命中 → `disposeSpan` → erase |
| `deleteAllOwnedSpans()` | 遍历 `disposeSpan` 后 clear(dtor/clearSpans 用) |
| `appendSpanCopy(dest, srcSpan, ...)` | **唯一** clone 决策点(拷贝路径用) |
| `disposeSpan(record)` | `if (record.owned) delete record.span;` |

`addSpan` 体内**唯一一次** `const_cast<ParcelableSpan*>(span)`:所有 span 都是非 const 堆对象,经 const 指针暴露仅为 API 便利,cast 后 delete 良定义。

### 4.3 数据流示例

```
Html 解析:  mBuilder.setSpan(new StyleSpan(BOLD), s, e, flags)
              └─► SpannableStringBuilder::setSpan
                    └─► addSpan  → owned=true(StyleSpan 非 NoCopySpan)
                                   mSpans.push_back({new StyleSpan, s, e, flags, true})

mBuilder 析构: ~SSI → deleteAllOwnedSpans → 遍历 disposeSpan → delete 每个 owned StyleSpan ✓

拷贝成快照: SpannableString snapshot(mBuilder)
              └─► SSI 拷贝构造 → appendSpanCopy(mSpans, srcStyleSpan, ...)
                    └─► owned → srcStyleSpan->clone() → 新独立 StyleSpan(owned=true)
              (原件与快照各持有独立 StyleSpan,互不影响)✓
```

---

## 5. clone() 协议

### 5.1 根类声明

[parcelablespan.h](../src/gui/text/parcelablespan.h):

```cpp
class ParcelableSpan {
public:
    virtual ~ParcelableSpan() = default;
    virtual ParcelableSpan* clone() const { return nullptr; }  // 非纯虚
};
```

**为什么非纯虚**:`ParcelableSpan` 根类被直接实例化(Selection marker 历史上 `new ParcelableSpan()`),纯虚会编不过。默认返回 `nullptr`;拷贝点用 `assert(clone()!=nullptr)` 抓遗漏的 override。

### 5.2 子类协变 override

每个**具体 owned span** override `clone()`,**协变返回各自类型**:

```cpp
class ForegroundColorSpan : public CharacterStyle {
public:
    ForegroundColorSpan* clone() const override { return new ForegroundColorSpan(*this); }
};
```

体基本就是 `return new XxxSpan(*this);`(用编译器生成的拷贝构造)。仅当 span 持有需要特殊处理的状态时才手写。

### 5.3 哪些需要 override,哪些不需要

| 类型 | 是否 override clone | 原因 |
|---|---|---|
| owned 值型 span(StyleSpan/ForegroundColorSpan/BulletSpan/URLSpan/ImageSpan/…) | **必须** | 会被拷贝,需深拷贝 |
| 抽象基类(CharacterStyle/MetricAffectingSpan/ReplacementSpan/ClickableSpan/…) | 不需要 | 不可实例化,继承默认 nullptr |
| NoCopySpan / watcher(TextWatcher/SpanWatcher/MemoryTextWatcher/ChangeWatcher) | 不需要 | 永不参与拷贝,继承默认 nullptr |
| ParcelableSpan 根类实例(Selection marker) | 不需要 | 是 NoCopySpan marker,永不拷贝 |

> **ImageSpan 说明**:`mDrawable`/`mContext` 是**借用**的(span 不 delete 它们),故 `new ImageSpan(*this)` 浅拷贝指针即正确,与 Android"drawable 跨副本共享"一致。

---

## 6. NoCopySpan 与虚继承(关键陷阱)

### 6.1 NoCopySpan 的作用

`NoCopySpan` 是个 marker 接口,标明"这个 span 在文本切片/拷贝时不带入新 Spanned,且容器不拥有它"。语义见 Android `android.text.NoCopySpan`。

### 6.2 ⚠️ 菱形陷阱(曾导致 double-free)

`TextWatcher` 和 `SpanWatcher` 都继承 `NoCopySpan`。若它们**非虚**继承,则任何同时继承 `TextWatcher + SpanWatcher` 的类(如 `TextView::ChangeWatcher`)会有**两个 NoCopySpan 子对象**。此时 `addSpan` 里的 `dynamic_cast<NoCopySpan*>(span)` 会**歧义 → 返回 nullptr** → `owned` 误判为 `true` → 容器在析构时 delete 该 watcher,而外部 owner(如 `~TextView`)也 delete 它 → **双重释放**。

**这是编译期查不出的 bug**(下转型语法合法),只在运行时崩。

**强制约束**:[textwatcher.h](../src/gui/text/textwatcher.h) / [spanwatcher.h](../src/gui/text/spanwatcher.h) 中:

```cpp
class TextWatcher : virtual public NoCopySpan { ... };   // 必须是 virtual
class SpanWatcher : virtual public NoCopySpan { ... };   // 必须是 virtual
```

> ⚠️ **任何人**把这两处的 `virtual` 去掉,都会让所有多重继承 watcher 触发 double-free。这是 load-bearing 约束,改动前务必理解。

---

## 7. Watcher / Selection 所有权模式

borrowed(NoCopySpan)span 容器不管释放,所以必须有**外部 owner** 明确负责:

| span | 外部 owner | 释放点 |
|---|---|---|
| `Selection::SELECTION_START/END/MEMORY` | 进程(单例) | 永不释放(进程级 NoCopySpan marker) |
| `Selection::MemoryTextWatcher` | `Selection`(静态方法逻辑) | `removeMemory()` 里 `removeSpan` 后 `delete` |
| `TextView::ChangeWatcher` | `TextView`(`mChangeWatcher` 成员) | `~TextView` 里 `delete mChangeWatcher` |
| `DynamicLayout::ChangeWatcher` | `DynamicLayout`(`mWatcher` 成员) | `~DynamicLayout` 里先 `removeSpan` 再 `delete mWatcher` |
| `TransformationMethod` | `TextView`(`mTransformation` 成员) | `~TextView` 释放 |

**原则**:watcher 是"借用进文本、由宿主对象独占"的对象。宿主析构时,先 `removeSpan`(防止析构过程中回调打到半销毁的宿主),再 `delete`。

---

## 8. 开发者使用指南

### 8.1 新增一个 owned 值型 span

```cpp
// myspan.h
class MySpan : public CharacterStyle {
public:
    explicit MySpan(int v) : mVal(v) {}
    void updateDrawState(TextPaint& paint) const override { /* ... */ }
    MySpan* clone() const override { return new MySpan(*this); }   // ← 必加
private:
    int mVal;
};
```

使用(所有权随即移交文本):
```cpp
SpannableStringBuilder b;
b.setSpan(new MySpan(42), 0, 5, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
// 之后不要再持有/释放这个 MySpan 指针;容器接管。
```

**检查清单**:
- [ ] 加了协变 `clone()`(返回 `MySpan*`)。
- [ ] 若持有指针成员,确认其所有权语义(借用 → 浅拷贝 OK;独占 → clone 里深拷贝或重构)。
- [ ] 不要继承 `NoCopySpan`(否则会被当借用,容器不释放 → 泄漏)。
- [ ] 同一实例**只 set 进一段文本**;要用于另一段文本先 `clone()`(见 §3 契约 1a,否则双重释放)。

### 8.2 新增一个 watcher / 借用型 span

```cpp
class MyWatcher : public TextWatcher {   // TextWatcher 已是 NoCopySpan
    // 不需要 clone()
};
```

使用(宿主拥有,文本借用):
```cpp
// 宿主成员:mWatcher = new MyWatcher(this);
sp->setSpan(mWatcher, 0, len, Spanned::SPAN_INCLUSIVE_INCLUSIVE);
// ~宿主: sp->removeSpan(mWatcher); delete mWatcher; mWatcher=nullptr;
```

**检查清单**:
- [ ] 继承自 `NoCopySpan`(直接或经 `TextWatcher`/`SpanWatcher`)。
- [ ] 若同时继承 `TextWatcher` + `SpanWatcher`,两者已虚继承 `NoCopySpan`,无需额外处理。
- [ ] 宿主对象负责 `delete`,并在析构时先 `removeSpan`。
- [ ] 不要 `new` 后无主挂入文本(泄漏)。

### 8.3 拷贝文本

直接用拷贝构造 / `subSequence` / `TextUtils::copySpansFrom` 即可,owned span 自动深拷贝、borrowed 自动按 `ignoreNoCopySpan` 处理。**不要**手动把源文本的 owned span 指针塞进新文本——那是双重释放的根源。

### 8.4 容器选择

| 类型 | 用途 | span 可变 |
|---|---|---|
| `SpannableStringBuilder` | 可编辑文本(TextView 编辑态) | 可 setSpan/removeSpan,文本可改 |
| `SpannableString` | 不可改文本但 span 可调 | 可 setSpan/removeSpan |
| `SpannedString` | 完全只读快照 | 不可改 |

三者共享 `SpannableStringInternal` 的存储与所有权逻辑。

---

## 9. 常见陷阱(避免再踩)

1. **同一 owned span 实例 set 多次**(见 §3 契约 1a):同一 Spannable 内合法(去重、改区间);**跨 Spannable 禁止**(双重释放),跨容器必须先 `clone()`。NoCopySpan(selection 单例 / watcher)是例外,可跨容器共享。
2. **去掉 `TextWatcher`/`SpanWatcher` 的 `virtual` 继承** → 多重继承 watcher 的 `dynamic_cast<NoCopySpan*>` 歧义 → double-free(见 §6.2)。
3. **把 selection marker 或 watcher 当 owned 释放** → 会删进程单例或外部成员;靠 NoCopySpan 判定规避,别手动 delete 容器里的 span。
4. **owned span 忘加 `clone()`** → 拷贝时 `assert` 触发(debug);release 下静默丢失该 span。新 span 务必加。
5. **中间构建态泄露**:`disposeSpan` 若被改回空操作(如调试),所有 owned span 会泄漏——这是调试用,不得留在正式逻辑。

---

## 10. 决策记录

| 决策 | 选择 | 理由 |
|---|---|---|
| 智能指针 | **不用** shared_ptr/unique_ptr | 项目要求;Android 风格是引用语义+GC,owned/borrowed 二元判定更贴切 |
| 所有权判定轴 | **NoCopySpan 一刀切** | 类型继承是行为分类,与所有权正交;Android 也只在拷贝时 `instanceof NoCopySpan` |
| clone 返回类型 | **协变,各自子类指针** | 类型安全;Android 风格 |
| clone 纯虚? | **非纯,默认 nullptr** | 根类被直接实例化(Selection marker) |
| 拷贝语义 | **深拷贝 owned**(clone) | 避免 shared_ptr 的前提下,唯一能杜绝双重释放的确定性方案;clone 成本可忽略(span 少) |
| SSI 拷贝 ops | **深拷贝实现**(非 `= delete`) | C++14 + `SpannedString::valueOf` 按值返回,delete 会编不过 |
| ChangeWatcher 接口 | **直接 `: NoCopySpan`**,不恢复 TextWatcher+SpanWatcher 双继承 | 双继承非虚 NoCopySpan 造菱形;接口本就注释掉,无功能回归 |

---

## 11. 已知遗留项

1. **`MemoryTextWatcher::afterTextChanged` 仍是注释状态**(Android 半移植功能)。若文本在 `removeMemory` 前被销毁,该 watcher 会泄漏——属该功能未完成部分,非本模型引入。
2. **运行时内存验证**:构建绿是 AGENTS.md 硬性标准(已满足),但构建绿 ≠ 运行时零 UAF。建议用 ASan/valgrind 跑 textview 样例(多次 setText/fromHtml、选区拖拽、编辑重排)做运行时确认。
3. **`assert(clone()!=nullptr)` 仅 debug 生效**:release 下遗漏的 clone 会静默丢失 span。当前所有 owned span 均已覆盖。

---

## 12. 关键文件索引

| 文件 | 角色 |
|---|---|
| [parcelablespan.h](../src/gui/text/parcelablespan.h) | `ParcelableSpan::clone()` 根;NoCopySpan;CharacterStyle/Passthrough |
| [spannablestring.h](../src/gui/text/spannablestring.h) / [.cc](../src/gui/text/spannablestring.cc) | `SpanRecord`、集中 helper、析构、深拷贝 copy ops、setSpan/removeSpan 收口 |
| [spannablestringbuilder.cc](../src/gui/text/spannablestringbuilder.cc) | SSB 的 setSpan/removeSpan/clearSpans/clear 走 helper |
| [textutils.cc](../src/gui/text/textutils.cc) | `copySpansFrom` clone |
| [selection.cc](../src/gui/text/selection.cc) | NoCopySpan marker;`removeMemory` 释放 watcher |
| [dynamiclayout.h](../src/gui/text/dynamiclayout.h) / [.cc](../src/gui/text/dynamiclayout.cc) | `ChangeWatcher : NoCopySpan`;析构 removeSpan+delete |
| [textwatcher.h](../src/gui/text/textwatcher.h) / [spanwatcher.h](../src/gui/text/spanwatcher.h) | **`virtual public NoCopySpan`**(load-bearing) |
| `src/gui/text/style/*.h` | 各 owned span 的协变 `clone()` |
