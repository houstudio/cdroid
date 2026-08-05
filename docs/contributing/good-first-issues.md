# Good First Issues

New to CDroid? These are concrete, self-contained tasks — each one names the exact file to change, the exact AOSP source to port from, and how to verify your work. Pick one, follow the [contributing guide](../../CONTRIBUTING.md), and open a PR.

The golden rule is **fidelity**: read the AOSP original at `/opt/android-sdk/sources/android-36/` and translate its logic verbatim. See [AGENTS.md](../../AGENTS.md).

How to verify any task below:
```bash
cd outX64-Debug
make cdroid -j44          # for library fixes (Section A)
make <sample_name> -j44   # for new samples (Section B); CMake auto-reconfigs the glob
```

中文版见 [good-first-issues.cn.md](good-first-issues.cn.md).

---

## Section A — Finish a ported-but-stubbed method

These have a faithful signature already; the body is empty, commented out, or returns a placeholder. Read the AOSP method, translate the logic.

### A1. `Chronometer::formatDuration` returns `""`  ·  easy
- **File:** `src/gui/widget/chronometer.cc:194` (the `return "";` is the stub, full body `:194-226`)
- **Missing:** Correctly computes `h`/`m`/`s` from milliseconds, then throws them away and returns `""`. `getContentDescription()` (line 227) calls it — so the chronometer is silent to accessibility.
- **AOSP:** `android-36/android/widget/Chronometer.java` `formatDuration` (~line 373). It builds a localized "h hours m minutes s seconds" string, skipping zero leading components.
- **Note:** CDroid has no ICU `MeasureFormat`; the faithful C++ translation is plain `std::to_string` concatenation mirroring AOSP's `if (h>0)/if (m>0)` guards.

### A2. Short month names in `DatePickerSpinnerDelegate`  ·  easy
- **File:** `src/gui/widget/daypickerspinnerdelegate.cc:34-40`
- **Missing:** `mShortMonths` is filled with `"1".."12"`; the comment says `// TODO: DateFormatSymbols.getShortMonths()`. The month spinner shows numerals instead of "Jan", "Feb", …
- **AOSP:** `android-36/android/widget/DatePickerSpinnerDelegate.java:414` and `java/text/DateFormatSymbols.java:535` (`getShortMonths`).
- **Note:** No Locale infra, so use a static 12-entry table. Sibling pattern: `SimpleMonthView::updateDayOfWeekLabels` (`src/gui/widget/simplemonthview.cc:146-154`) does the same static-table trick.

### A3. `AbsSeekBar` accessibility `ACTION_SET_PROGRESS`  ·  easy
- **File:** `src/gui/widget/absseekbar.cc:765` (`case R::id::accessibilityActionSetProgress:`)
- **Missing:** The branch short-circuits `return false;` with the real logic commented out (`//setProgressInternal((int) value, true, true);`) and the `containsKey` guard commented out too.
- **AOSP:** `android-36/android/widget/AbsSeekBar.java:1108-1119` — reads `arguments.getFloat(ACTION_ARGUMENT_PROGRESS_VALUE)` then `setProgressInternal((int) value, true, true)`.
- **Note:** `Bundle::getFloat` and `Bundle::containsKey` already exist (`src/gui/core/basebundle.h:100,150`). Copy the `ACTION_SCROLL_FORWARD/BACKWARD` block right below as a template.

### A4. `ColorMatrixColorFilter::apply`  ·  medium
- **File:** `src/gui/drawable/colorfilters.cc:27`
- **Missing:** Body is `LOGW("ColorMatrixColorFilter::apply not yet implemented");` — no per-pixel color-matrix transform happens. The comment sketches the approach (`get_group_target()` → `ImageSurface` → `mCM.transform(...)` → `mark_dirty()`).
- **AOSP:** `android-36/android/graphics/ColorMatrixColorFilter.java`. The CDroid `ColorMatrix` class already exposes a `transform(ImageSurface)` helper.
- **Sibling pattern:** `PorterDuffColorFilter::apply` / `BlendModeColorFilter::apply` in the same file show the `canvas.get_group_target()` contract.

### A5. `LightingColorFilter::apply`  ·  medium
- **File:** `src/gui/drawable/colorfilters.cc:108`
- **Missing:** Body is `LOGW("LightingColorFilter::apply not yet implemented");`. Should compute per channel `R' = R*mul.R + add.R` on the active group surface.
- **AOSP:** `android-36/android/graphics/LightingColorFilter.java`. `LightingColorFilter` is the special case of `ColorMatrixColorFilter` (A4) — you can build a `ColorMatrix` from `mMul`/`mAdd` and delegate.

---

## Section B — Add a demo sample (most beginner-friendly)

Every widget below is ported but has **no** sample in `apps/samples/` (verified). The task is one new file `apps/samples/<name>.cc`; `apps/samples/CMakeLists.txt` auto-globs `*.cc`, so no build-system edits needed. Copy `apps/samples/analogclock.cc` (~14 lines) or `apps/samples/buttons.cc` as a skeleton.

For all of these: **difficulty easy** — one new file, no library changes, identical `Window` + `addView` pattern. The widget's public API mirrors its AOSP class under `android-36/android/widget/<Class>.java`.

| # | Sample to add | Widget impl | AOSP API reference |
|---|---|---|---|
| B1 | `Chronometer` | `src/gui/widget/chronometer.cc` (`setBase`, `setFormat`, `setCountDown`, `start`, `stop`) | `android/widget/Chronometer.java` |
| B2 | `TextClock` | `src/gui/widget/textclock.cc` (`setTextFormat`, `setTimeZone`, `refreshTime`) | `android/widget/TextClock.java` |
| B3 | `RatingBar` | `src/gui/widget/ratingbar.cc` (`setNumStars`, `setRating`, `setStepSize`, `setOnRatingBarChangeListener`) | `android/widget/RatingBar.java` |
| B4 | `Switch` | `src/gui/widget/switch.h` (`: public CompoundButton`) | `android/widget/Switch.java` |
| B5 | `CheckedTextView` | `src/gui/widget/checkedtextview.h` (`: public TextView, public Checkable`) | `android/widget/CheckedTextView.java` |
| B6 | `TableLayout` | `src/gui/widget/tablelayout.cc` + `tablerow.cc` | `android/widget/TableLayout.java` |
| B7 | `RadioGroup` | `src/gui/widget/radiogroup.cc` (incl. `LayoutParams`, `radiogroup.h:62`) | `android/widget/RadioGroup.java` |
| B8 | `CalendarView` | `src/gui/widget/calendarview.cc` | `android/widget/CalendarView.java` |
| B9 | `DatePicker` | `src/gui/widget/datepicker.cc` (`datePickerMode` = spinner/calendar, `init`, `updateDate`) — pairs with A2 | `android/widget/DatePicker.java` |
| B10 | `TimePicker` | `src/gui/widget/timepicker.cc` (`setIs24HourView`, `setHour`, `setMinute`, `setOnTimeChangedListener`) | `android/widget/TimePicker.java` |
| B11 | `TextSwitcher` | `src/gui/widget/textswitcher.cc` (`setText`, `showNext`, `getNextView` from `ViewSwitcher`) | `android/widget/TextSwitcher.java` |
| B12 | `ImageSwitcher` | `src/gui/widget/imageswitcher.h` (`: public ViewSwitcher`) — copy the `viewflipper` sample | `android/widget/ImageSwitcher.java` |

**Verify:** `make <sample_name> -j44` in `outX64-Debug/`, then run it.

---

## Claiming a task

- Comment on the matching issue (or open one) so two people don't duplicate work.
- Branch off `dev`, keep the PR to one task.
- In the PR description, link the AOSP class/method you ported; attach a screenshot for visible changes.
- Stuck? Ping QQ 1225012331 / WeChat calfhou, or open an issue.

Want something harder? Grep `src/gui/` for `TODO`, `DEFERRED`, `NOT IMPLEMENTED`, or `LOGW("...not yet implemented")` — but **avoid** span ownership, fragment view lifecycle, window teardown, and the Layout↔minikin seam; those are deep traps, not starter work.
