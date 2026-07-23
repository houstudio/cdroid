# Layout 家族 vs android-36 差异清单(CDROID)

逐行对照 `/opt/android-sdk/sources/android-36/android/text/{Layout,StaticLayout,DynamicLayout,BoringLayout}.java` 的审计结果。**只列不改**,供逐条验证修补。优先级:**P0** = 字段/签名/缺失公开 API 或明显 bug;**P1** = 逻辑差异;**P2** = 次要/C++ 惯用法差异。

> **前提更正**:android-36 `DynamicLayout.java:1406-1408` **确实**重新声明了 `mEllipsize`(bool)/`mEllipsizedWidth`/`mEllipsizeAt`(遮蔽基类 `Layout`),CDROID `dynamiclayout.h:149-151` 与之一致 —— **不是差异,不用改**。真冗余在 StaticLayout(见 S-1.1)。

---

## 跨类共性问题(根因在一处,多处表现)

| Pri | 问题 | 根因/位置 | 影响面 |
|---|---|---|---|
| P0 | **`TextLine::metrics/measure/justify` 没穿 `useBoundsForWidth` + `LineInfo`**:CDROID 是 1/3/1 参,android 是 4/5/2 参 | `textline.cc` 的 metrics/measure/justify 签名 | Layout 的 `getLineExtent`/`getHorizontal`/`measurePara`、StaticLayout `generate`、BoringLayout `init`/`isBoring` 全受影响 |
| P0 | **`useBoundsForWidth` 没传给 `LineBreaker`** | StaticLayout `generate` 构造 LineBreaker 处 | 断行宽度计算路径 |
| P0 | **`TruncateAt::NONE` 当 null 哨兵** —— 把"显式设 NONE"和"从没设过"混为一谈 | Builder 的 ellipsize 哨兵(StaticLayout/DynamicLayout Builder) | obtain/setWidth/generate/out 一串 |
| P0 | **`Builder()=default` 留下未初始化 POD**,池化复用带旧值 | StaticLayout Builder obtain 只重置 ~16/~25 字段 | 每次经池复用的 Builder |

---

## 一、Layout 基类(`layout.h` / `layout.cc` ↔ `Layout.java`)

### 字段 / 常量
- **P0** 缺 `mWorkPlainPaint`、`mSpanColors`、`mLineInfo`(TextLine.LineInfo)、静态 `sTempRect` —— 高对比度文本 + `getLineLetterSpacingUnitCount` 子系统整块缺失。`layout.h` vs `Layout.java:3690/3697/3715/3695`
- **P0** 缺常量 `INCLUSION_STRATEGY_ANY_OVERLAP/CONTAINS_CENTER/CONTAINS_ALL`、`HIGH_CONTRAST_TEXT_*`。`layout.h` vs `Layout.java:212-230,76-83`
- **P2** 多出 `ELLIPSIS_NONE/...` 别名(Java 这些在 TextUtils);`Alignment` 多出 `NONE=-1`(Java 用 null);`HYPHENATION_FREQUENCY_*` 用 `LineBreaker::` 别名而非字面量。

### 缺失公开 API(P0,android-36 有、CDROID 无)
- `Layout.Builder` 内部类(18 setter + `build()` 选 Boring/Static)整块缺失 —— `Layout.java:3825-4311`
- `TextInclusionStrategy`、`CharacterBoundsListener` 接口缺失
- `draw` 系列:`draw(Canvas)` 无参、`drawBackground(Canvas)` 无参、`draw(Canvas,List<Path>,List<Paint>,Path,Paint,int)` 6 参、`drawWithoutText`、`drawHighlights`、`drawHighContrastBackground`、`shouldDrawHighlightsOnTop`/`setToHighlightPaint`/`determineHighContrastHighlightBlendMode`/`isHighContrastTextDark` 全缺
- `getLineSpacingMultiplier()`、`getLeftIndents()/getRightIndents()`、`getLineBreakConfig()`、`getUseBoundsForWidth()`、`getShiftDrawingOffsetForStartOverhang()`、`getMinimumFontMetrics()` 这些 getter 缺(字段有、getter 无)
- `fillHorizontalBoundsForLine`、`fillCharacterBounds`、`forEachCharacterBounds`、`getRangeForRect(RectF,SegmentFinder,TextInclusionStrategy)`、`getStartOrEndOffsetForAreaWithinLine` 系列、`getLineLetterSpacingUnitCount(line,bool)` 缺
- `getLineBottom(line, bool includeLineSpacing)` 重载缺(只有 `getLineBottomWithoutSpacing`)
- `getLineVisibleEnd(line,start,end,bool)` 4 参变体缺
- `getDesiredWidthWithLimit` / `measurePara` 缺 `useBoundsForWidth` 参数

### 签名差异(P0)
- `drawBackground` 签名整体不同:CDROID `(Canvas&,Path*,Paint*,int cursorOffset,int first,int last)` vs Java `(Canvas,int first,int last)` —— CDROID 把高亮 path 绘制塞进去了,Java 已移走。`layout.h:111` vs `Layout.java:944`
- 多出非 Android 的 `setJustificationMode(int)` setter、非 const `TextPaint* getPaint()` 重载

### 行为 bug(P0,必修)
- **`layout.cc:91`** 全构造把 `mJustificationMode = JUSTIFICATION_MODE_NONE` 硬编码 —— **丢弃了 ctor 的 `justificationMode` 参数**
- **`layout.cc:1690-1698`** `ellipsize()` 有未初始化 `char c`(赋值被注释,Java 用 `ELLIPSIS_FILLER`)
- **`layout.cc:582-608`** `computeDrawingBoundingBox()` 基本没实现(justification/metrics 都注释,rectF 空 → 返回全 0)
- **`layout.cc:1726-1730`** `Ellipsizer::toString()` 返回 `""`(Java 真转);**`layout.cc:1720-1724`** `subSequence` 返回 `SpannedString`(Java 的 Ellipsizer 返回 `String`);**SpannedEllipsizer 不 override subSequence**(Java override 拷贝 span)

### 行为差异(P1)
- `getLineVisibleEnd` 末行恒返回 `end`(Java 仅 `trailingSpaceAtLastLineIsVisible==true` 才如此)—— 影响两端对齐。`layout.cc:1177` vs `Layout.java:2804`
- `getParagraphSpans` 的 `SpannableStringBuilder→false(ignoreDirty)` 分支被注释。`layout.cc:1668` vs `Layout.java:3463`
- `replaceWith` 没调 `initSpanColors()`;全 ctor 也没调。`layout.cc:107` vs `Layout.java:427`
- `draw(4 参)` 没走 Java 的 6 参 draw(缺 leftShift 平移、drawWithoutText/drawText/drawHighlights 顺序、高对比度)。`layout.cc:125` vs `Layout.java:467`
- `getLineExtent`/`measurePara` 的 `tl->metrics` 调用没传 `useBoundsForWidth`(见跨类)
- **P2** 多处 `>>`(有符号)对 Java 的 `>>>`(无符号)—— 非负时等价(`isLevelBoundary`/`isRtlCharAt`/`getRunRange`/`primaryIsTrailingPrevious`/`Directions::getRunLevel`)

### 嵌套类
- `Directions`、`TabStops` 在 CDROID 是顶层类(Java 是 Layout 的静态嵌套类);`Ellipsizer` 只继承 CharSequence(Java 还 implements GetChars);`HorizontalMeasurementProvider` 多存了 `Layout* mLayout`(Java 内部类隐式引用外围实例)。

---

## 二、StaticLayout(`staticlayout.h` / `staticlayout.cc` ↔ `StaticLayout.java`)

### 字段
- **P0** **`mEllipsizedWidth` 真冗余**:android StaticLayout 不声明(继承 Layout),CDROID 重声明 + override `getEllipsizedWidth()`。`staticlayout.h:119,107` vs 缺
- **P2** `mDrawingBounds` 用 `+mDrawingBoundsValid` 标志而非可空 ref;`mLeftIndents/mRightIndents` 用空 vector 当 null;`LineBreaks` 用定长数组。

### 构造/签名(P0)
- 私有 Builder ctor:CDROID `(Builder&)` 1 参 vs Java `(Builder,bool trackPadding,int columnSize)` 3 参。`staticlayout.h:14`
- 私有无参(DynamicLayout 回收用)ctor:CDROID `(CharSequence*)` 只传 7 参给 Layout(用 `Alignment::NONE`,漏 maxLines/breakStrategy/indents/justification/lineBreakConfig/useBoundsForWidth/shiftOverhang/minimumFontMetrics)vs Java `()` 调全 21 参 super。`staticlayout.h:15`
- `out()` 的 `chooseHt` 参数:CDROID `vector<ParcelableSpan*>`(基类,需 downcast)vs Java `LineHeightSpan[]`;`chooseHtv`:CDROID `vector<int>*`(可空)vs Java `int[]`。`staticlayout.h:17-19`

### generate() 行为(P0/P1)
- **P0** 整条 `FLAG_FIX_LINE_HEIGHT_FOR_LOCALE` / minimum-font-metrics 路径缺失(CDROID 把 fmTop/fmBottom/fmAscent/fmDescent 硬编码 0)。`staticlayout.cc:490,559` vs `Layout.java:805-824,1004-1007,1100-1107`
- **P0** `getBaseHyphenationFrequency()`(HYPHENATION_FREQUENCY_* → LineBreaker.* 映射)缺,直接把 `b.mHyphenationFrequency` 透传给 LineBreaker。`staticlayout.cc:343`
- **P1** `generate()` 顶部没把 `mDrawingBoundsValid=false`(DynamicLayout 回收复用后会返回**陈旧** bounds)。`staticlayout.cc:295` vs `StaticLayout.java:780`
- **P1** fallback-line-spacing 的 fmTop/fmBottom 扩展没做;断点间 metrics 重置没和 default 取 min/max。
- **P2** `Paint::FontMetricsInt fm = b.mFontMetricsInt` 是值拷(Java 按引用复用 Builder 的);`measuredPara` 的 delete 时机要注意所有权。

### out() 行为(P1)
- `!doEllipsis` 时没把 `ELLIPSIS_START/COUNT` 清零(复用 layout 时留陈旧值)。`staticlayout.cc:614` vs `.java:1185`
- 省略边的 hyphenEdit 没按 ellipsize 方向重写(START/MIDDLE/MARQUEE/END 各自的清零规则)。`staticlayout.cc:691` vs `.java:1264`

### Builder(P0/P1)
- **P0** ellipsize 哨兵 `TruncateAt::NONE` vs Java `null`(见跨类)。
- **P0** `Builder()=default` 未初始化 POD,`obtain()` 只重置部分字段(见跨类)。
- **P1** `recycle()/finish()` 没清 `mMinimumFontMetrics`;`setIndents` 取 `const vector<int>&` 不能传 null(Java `@Nullable int[]`)。
- **P2** `obtain/build/buildPartialStaticLayoutForDynamicLayout` 返回裸指针(Java 按值返回,Caller 管 Builder 生命周期);缺 Trace 段。
- `calculateEllipsis()` 逐行等价,**无差异**。

---

## 三、DynamicLayout(`dynamiclayout.h` / `dynamiclayout.cc` ↔ `DynamicLayout.java`)

### ChangeWatcher / OffsetMapping(P0,最大缺口)
- **P0** `ChangeWatcher` 只继承 `NoCopySpan`,**没实现 `TextWatcher`/`SpanWatcher`**(文档说是 C++ 多继承 workaround)—— 回调是普通方法,不能多态注册成 watcher。`dynamiclayout.h:129`
- **P0** **完全没有 `OffsetMapping` 支持**:缺 `mTransformedTextUpdate`、缺 `beforeTextChanged` 的 offset 变换、缺 `transformAndReflow` helper → **变换文本(密码)的 reflow 不正确**。`dynamiclayout.cc:888` vs `DynamicLayout.java:1231-1308,1368`
- **P1** `onSpanAdded/Removed/Changed` 直接调 `reflow` 而非 `transformAndReflow`;缺 `Flags.insertModeCrash*` 门控和 OffsetMapping fallback。`dynamiclayout.cc:900-920`
- **P1** `mLayout` 是裸 `DynamicLayout*`(Java 是 `WeakReference`)—— 靠析构手动 remove。

### generate / 其它(P1/P2)
- **P1** `generate()` 初始 reflow 用 `baseLength = mBase->length()` 作 `after`,Java 用 `mDisplay.length()` —— **base≠display(密码/变换)时出错**。`dynamiclayout.cc:314` vs `DynamicLayout.java:618`
- **P2** `sLock` 缺,所有 `synchronized(sLock)` 被注释 → `sStaticLayout`/`sBuilder` 回收非同步;`getBlocksAlwaysNeedToBeRedrawn`/`getBlockEndLines`/`getBlockIndices` 按值返回(Java 返回引用);deprecated ctor 没 `@Deprecated`。
- **P2** `contentMayProtrudeFromLineTopOrBottom` 对 `mTempRect` 用 `.top`(字段)和 `.bottom()`(方法)不一致,核实 Rect API 是否 bug。`dynamiclayout.cc:564`
- **by-design** Directions 用 clone-and-own(`cloneOwnedDirections`/`freeOwnedDirections`)—— CDROID 移植决策(2026-07-07 修 UAF 时加),非差异。

---

## 四、BoringLayout(`boringlayout.h` / `boringlayout.cc` ↔ `BoringLayout.java`)

### 缺失 API(P0)
- 缺 `make(...)` 9 参重载(带 `useFallbackLineSpacing`,无 spacingMult/Add)。`boringlayout.h:31`
- 缺 `replaceOrMake(...)` 9 参重载。`boringlayout.h:38`
- **`isFallbackLineSpacingEnabled()` 头里有、定义被注释**([boringlayout.cc:344](src/gui/text/boringlayout.cc#L344))—— 直接调会链接失败;经基类指针走的是基类非虚版本。

### 行为(P1)
- `replaceOrMake` **硬编码 `mUseFallbackLineSpacing=false`** 并传 false(签名根本没这个参)—— 替换时丢 fallback spacing。`boringlayout.cc:60,62`
- `isBoring` 5 参 master **忽略 `minimumFontMetrics`**(无 `Flags.fixLineHeightForLocale()` 块)。`boringlayout.cc:241-276`
- `getLineMax`/`getLineWidth`/`draw` 没看 `getUseBoundsForWidth()` 去走 super 分支。`boringlayout.cc:312,352`
- `init`/`draw` 的 `mDirect` 快路径被注释 → 永远走 `Layout::draw`,无 String 快路径;`init` 调 `metrics(nullptr)` 1 参(见跨类)。
- **P2** `Metrics` 缺 `toString()`;不继承 `TextUtils::EllipsizeCallback`(注释了);`computeDrawingBoundingBox` 没 `override`。

---

## 建议修补顺序(你定)

> 下面的"待删清单"是**反向审计**(CDROID 有、android-36 没有 → 删除候选),补在本节"待加"之后。

## 待删清单(反向审计:CDROID 有、android-36 没有)

### Layout 基类
| 位置 | 项 | 状态 |
|---|---|---|
| [layout.h:55-59](src/gui/text/layout.h#L55) | `ELLIPSIS_NONE/START/MIDDLE/END/MARQUEE` 别名 | **REMOVE** —— android `Layout` 不声明(在 `TextUtils.TruncateAt`)。唯一消费者是废弃的 `textview_old.cc`;现役 `textview.cc` 已直接用 `TextUtils::TruncateAt::*`。删别名 + 迁移 textview_old |
| [layout.h:95](src/gui/text/layout.h#L95) | `setJustificationMode(int)` setter | **REMOVE** —— android `Layout` 无此 setter(仅 ctor/Builder 设)。grep 零调用,纯死代码 |
| [layout.h:408-410](src/gui/text/layout.h#L408) | `Directions::operator==` | **REMOVE** —— android 无,无内部调用方。无害但 CDROID-only |
| [layout.h:333](src/gui/text/layout.h#L333) | `Ellipsizer::toU16String()` override | **REMOVE-or-FIX** —— android Ellipsizer 只有 `toString()`;这个 override 直接委托 `mText->toU16String()` **跳过了省略**,是 bug。要么删(若 CharSequence 接口不再要求),要么改成走 `getChars` |

### StaticLayout
| 位置 | 项 | 状态 |
|---|---|---|
| [staticlayout.h:119](src/gui/text/staticlayout.h#L119) + [:107](src/gui/text/staticlayout.h#L107) | `mEllipsizedWidth` 字段 + `getEllipsizedWidth()` override | **REMOVE(必须一起)** —— android StaticLayout 不声明,继承基类。删字段 + 删 override,并把 `staticlayout.cc` 里 generate/out/calculateEllipsis 对 `mEllipsizedWidth` 的读写改成走基类字段 |

### BoringLayout / DynamicLayout
**0 待删**。两者 header 与 android-36 一一对应,无 A14/魔改残留。各自的 shadow 字段(`mEllipsizedWidth` 等)android-36 也都有(忠实)。`~Static/Dynamic/BoringLayout()` 析构、`clone/freeOwnedDirections`(UAF 修复)、`mutable`、`const`、vector 代替数组 都是必要 C++ 适配,保留。

### DISCUSS(载重的 CDROID 发明,删前要先重构)
| 位置 | 项 | 为什么不能直接删 |
|---|---|---|
| [layout.h:62](src/gui/text/layout.h#L62) | `Alignment::NONE = -1` | android 用 `null` 表示"未设";CDROID 值枚举不能返 null 才造了哨兵。`layout.cc` 的 null→CENTER 映射、staticlayout 默认 ctor 都用它。删需改 `std::optional<Alignment>` 或加 `mHasAlignment` 标志 |
| [layout.h:111](src/gui/text/layout.h#L111) | `drawBackground(Canvas&,Path*,Paint*,int,int,int)` 6 参(合并了 LineBackgroundSpan + 高亮 path) | android-36 拆成了 `drawBackground(Canvas,int,int)` + 独立的 `drawHighlights(...)`。`Layout::draw` 在用。删需先把 drawHighlights 那套补上(属"待加"的大块) |
| [layout.h:272](src/gui/text/layout.h#L272) | `getLineBottomWithoutSpacing(int)` | android 用 `getLineBottom(line, false)`(2 参重载,CDROID 缺)。内部 getSelection/getCursorPath + `editor.cc:385` 在用。删需先移植 2 参 overload |
| [layout.h:102](src/gui/text/layout.h#L102) | `getDesiredWidthWithLimit` 少了 `useBoundsForWidth` 参 | 签名与 android 不一致(连带 `measurePara`),对齐即可 |
| StaticLayout [:41/:176/:178/:202](src/gui/text/staticlayout.h#L41) | `generate`/`recycle`/`finish`/`buildPartialStaticLayoutForDynamicLayout` 是 public | android 是包级私有。CDROID 放宽成 public(DynamicLayout 已是 friend)。可收窄为 private+friend |
| [boringlayout.h:103](src/gui/text/boringlayout.h#L103) | `isFallbackLineSpacingEnabled /*override*/` | 基类 `Layout::isFallbackLineSpacingEnabled` 非虚 → 这里是**隐藏不是 override**。要真 override,给基类加 `virtual`(根因在 layout.h:174) |

### 反向审计小结
- **真待删只有 5 处**:Layout 4 个(`ELLIPSIS_*` 别名、`setJustificationMode`、`Directions::operator==`、`Ellipsizer::toU16String`)+ StaticLayout 1 对(`mEllipsizedWidth`+override)。且 Layout 那 4 个多是**零调用的死代码**,删起来低风险。
- BoringLayout / DynamicLayout **干净**,无需删。
- 一批 DISCUSS 是"android 拆/改了、CDROID 还停留在 A14 合并态"(drawBackground、getLineBottomWithoutSpacing),要删得先把 android 的新拆法补上 —— 属"待加",不是单纯删。

1. **真 bug 先修**(改动小、风险低、收益明确):
   - Layout ctor 丢弃 `justificationMode`(`layout.cc:91`)
   - `ellipsize()` 未初始化 `char c`(`layout.cc:1690`)
   - StaticLayout `generate()` 顶部 `mDrawingBoundsValid=false`(`staticlayout.cc:295`,一行,修 DynamicLayout 回收的陈旧 bounds)
   - DynamicLayout `generate()` 初始 reflow 用 `mDisplay.length()`(`dynamiclayout.cc:314`,密码/变换文本正确性)
   - BoringLayout `isFallbackLineSpacingEnabled` 定义取消注释(`boringlayout.cc:344`)
2. **StaticLayout `out()` 两个分支**:省略零清 + 省略边 hyphenEdit 重写(`staticlayout.cc:614,691`)
3. **`useBoundsForWidth` + `LineInfo` 穿进 `TextLine::metrics/measure/justify`**(跨类根因,改一处带起 Layout/Static/Boring 多处)
4. **Builder 哨兵/初始化**:`TruncateAt::NONE` vs null、`Builder()=default` POD(系统性,影响池复用)
5. **大块缺失**(按需):`Layout.Builder` + `TextInclusionStrategy` + `getRangeForRect`/`fillCharacterBounds`;高对比度文本子系统;DynamicLayout `OffsetMapping`(密码 reflow);BoringLayout `make/replaceOrMake` 9 参重载 + `minimumFontMetrics` 路径。
