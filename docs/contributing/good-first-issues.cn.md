# 新手任务清单（Good First Issues）

刚接触 CDroid？下面都是具体、独立的小任务 —— 每个都标好了要改哪个文件、从哪段 AOSP 源码翻译、怎么验证。挑一个,跟着[贡献指南](../../CONTRIBUTING.cn.md)走,提个 PR 就行。

唯一准则是**保真**:先读 `/opt/android-sdk/sources/android-36/` 下的 Android 原版,把逻辑逐行翻译过来。详见 [AGENTS.md](../../AGENTS.md)。

任何任务的验证方法:
```bash
cd outX64-Debug
make cdroid -j44          # 库内修复（A 组）
make <sample名> -j44      # 新增 sample（B 组）;CMake 会自动重配 glob
```

English: [good-first-issues.md](good-first-issues.md).

---

## A 组 —— 补完「已移植但 stub 掉」的方法

这些方法签名已经保真,但函数体是空的、被注释掉、或返回占位值。读 AOSP 原版,把逻辑翻译过来。

### A1. `Chronometer::formatDuration` 返回 `""`  ·  简单
- **文件:** `src/gui/widget/chronometer.cc:194`(stub 在 `return "";`,完整函数体 `:194-226`)
- **缺什么:** 从毫秒正确算出了 `h`/`m`/`s`,然后扔掉、返回 `""`。`getContentDescription()`(227 行)调它 —— 于是计时器对无障碍是静默的。
- **AOSP:** `android-36/android/widget/Chronometer.java` 的 `formatDuration`(约 373 行)。它拼出本地化的"h hours m minutes s seconds",跳过前导的 0 分量。
- **注意:** CDroid 没有 ICU `MeasureFormat`;忠实的 C++ 翻译是用 `std::to_string` 拼,镜像 AOSP 的 `if (h>0)/if (m>0)` 守卫。

### A2. `DatePickerSpinnerDelegate` 用英文短月份名  ·  简单
- **文件:** `src/gui/widget/daypickerspinnerdelegate.cc:34-40`
- **缺什么:** `mShortMonths` 填的是 `"1".."12"`,注释写着 `// TODO: DateFormatSymbols.getShortMonths()`。月份滚轮显示数字而不是 "Jan"、"Feb"……
- **AOSP:** `android-36/android/widget/DatePickerSpinnerDelegate.java:414` 与 `java/text/DateFormatSymbols.java:535`(`getShortMonths`)。
- **注意:** 没有 Locale 设施,用静态 12 项表。兄弟写法:`SimpleMonthView::updateDayOfWeekLabels`(`src/gui/widget/simplemonthview.cc:146-154`)就是同样的静态表技巧。

### A3. `AbsSeekBar` 无障碍 `ACTION_SET_PROGRESS`  ·  简单
- **文件:** `src/gui/widget/absseekbar.cc:765`(`case R::id::accessibilityActionSetProgress:`)
- **缺什么:** 这个分支直接 `return false;`,真正的逻辑被注释掉(`//setProgressInternal((int) value, true, true);`),连 `containsKey` 守卫也被注释了。
- **AOSP:** `android-36/android/widget/AbsSeekBar.java:1108-1119` —— 读 `arguments.getFloat(ACTION_ARGUMENT_PROGRESS_VALUE)` 后 `setProgressInternal((int) value, true, true)`。
- **注意:** `Bundle::getFloat`、`Bundle::containsKey` 都已存在(`src/gui/core/basebundle.h:100,150`)。照抄下方 `ACTION_SCROLL_FORWARD/BACKWARD` 那段做模板。

---

## B 组 —— 加一个 demo sample（最适合入门）

下面每个控件都已移植,但在 `apps/samples/` 里**没有** sample(已核实)。任务就是新建一个 `apps/samples/<名>.cc`;`apps/samples/CMakeLists.txt` 自动 glob `*.cc`,无需改构建。照抄 `apps/samples/analogclock.cc`(约 14 行)或 `apps/samples/buttons.cc`。

下列全部:**难度简单** —— 单个新文件、不动库、`Window`+`addView` 写法与现有 sample 一致。控件的公开 API 对应 `android-36/android/widget/<类>.java`。

| # | 要加的 sample | 控件实现 | AOSP API 参考 |
|---|---|---|---|
| B1 | `Chronometer` | `src/gui/widget/chronometer.cc`(`setBase`、`setFormat`、`setCountDown`、`start`、`stop`) | `android/widget/Chronometer.java` |
| B2 | `TextClock` | `src/gui/widget/textclock.cc`(`setTextFormat`、`setTimeZone`、`refreshTime`) | `android/widget/TextClock.java` |
| B3 | `RatingBar` | `src/gui/widget/ratingbar.cc`(`setNumStars`、`setRating`、`setStepSize`、`setOnRatingBarChangeListener`) | `android/widget/RatingBar.java` |
| B4 | `Switch` | `src/gui/widget/switch.h`(`: public CompoundButton`) | `android/widget/Switch.java` |
| B5 | `CheckedTextView` | `src/gui/widget/checkedtextview.h`(`: public TextView, public Checkable`) | `android/widget/CheckedTextView.java` |
| B6 | `TableLayout` | `src/gui/widget/tablelayout.cc` + `tablerow.cc` | `android/widget/TableLayout.java` |
| B7 | `RadioGroup` | `src/gui/widget/radiogroup.cc`(含 `LayoutParams`,`radiogroup.h:62`) | `android/widget/RadioGroup.java` |
| B8 | `CalendarView` | `src/gui/widget/calendarview.cc` | `android/widget/CalendarView.java` |
| B9 | `DatePicker` | `src/gui/widget/datepicker.cc`(`datePickerMode` = spinner/calendar、`init`、`updateDate`)—— 配合 A2 | `android/widget/DatePicker.java` |
| B10 | `TimePicker` | `src/gui/widget/timepicker.cc`(`setIs24HourView`、`setHour`、`setMinute`、`setOnTimeChangedListener`) | `android/widget/TimePicker.java` |
| B11 | `TextSwitcher` | `src/gui/widget/textswitcher.cc`(`setText`、`showNext`、`getNextView` 继承自 `ViewSwitcher`) | `android/widget/TextSwitcher.java` |
| B12 | `ImageSwitcher` | `src/gui/widget/imageswitcher.h`(`: public ViewSwitcher`)—— 照抄 `viewflipper` sample | `android/widget/ImageSwitcher.java` |

**验证:** 在 `outX64-Debug/` 下 `make <sample名> -j44`,然后运行它。

---

## 认领任务

- 在对应 issue 下留个言(或开一个),免得两个人撞车。
- 从 `dev` 切分支,一个 PR 只做一个任务。
- PR 描述里贴上你移植的 AOSP 类/方法;改动可见就附张截图。
- 卡住了?QQ 1225012331 / 微信 calfhou,或开 issue。

想要更难的?在 `src/gui/` 里 grep `TODO`、`DEFERRED`、`NOT IMPLEMENTED`、`LOGW("...not yet implemented")` —— 但**避开** span 所有权、fragment view 生命周期、window 销毁、Layout↔minikin 缝;那些是深坑,不是入门活。
