# a11y auto-test teardown crashes: ownership model findings

Date: 2026-08-29/30
Scope: `--auto-test` sweep under gdb/valgrind on `preferencedemo` (framework
`src/gui`), part of the valgrind leak campaign. This document records the
crash investigation chain, the ownership principles it exposed, and the
techniques used — so the reasoning does not have to be re-derived.

## TL;DR

Four distinct crash families shared one theme: **a component that owns a view
in AOSP only "virtually" (GC reclaims it) must own it explicitly here, or must
not be able to reach it after teardown.** Fixes landed:

| Crash | Root cause | Fix |
|---|---|---|
| round ~14, deferred selection notify dispatches into a freed child | `~View` deleted a **shared** `AccessibilityDelegate` (`RecyclerViewAccessibilityDelegate::ItemDelegate` is set on EVERY item view) | `View::mAccessibilityDelegate` is now refcounted: raw-pointer setter = borrowed (AOSP contract, no-op deleter), `shared_ptr` overload = owning; RecyclerView chain + both picker TouchHelpers migrated |
| round ~36, right after a fragment-exit animation | lingering exit view stayed **parented (GONE)** while a window/host teardown could delete the tree racing the clone-end reclaim post | `AnimationEffect` now detaches the view when parking it in `mLingeryExitViews` — the clone's animator only needs the view *alive* (`setTransitionAlpha`), not attached |
| round ~36, Toast scene | Toast layouts lacked AOSP's `importantForAccessibility="no"`, so Toast content entered the a11y tree and outlived its `ToastWindow` in node ids / flush sources | `transient_notification{,_with_icon}.xml` gained the attribute (android-36 parity) |
| deferred selection notify on a dead `selectedView` | `AdapterView` consumers read `getSelectedView()` which can be a stale entry (AOSP keeps it alive via GC) | guarded: `isAttachedToWindow()` at the four consumption sites; `fireOnSelected` skips the callback when the view is gone (its `View&` cannot be null) |

One family remains open (see *Open item*).

## The ownership rules that fell out

1. **A delegate set via a raw pointer is borrowed** — the view never frees it
   (AOSP `setAccessibilityDelegate` is a plain assignment under GC). Framework
   code that *creates* a delegate uses the `shared_ptr` overload. One shared
   instance on many hosts (the RecyclerView item delegate) is exactly why the
   field cannot be a plain owned pointer.
2. **A view parked for deferred deletion must be off-tree at park time.**
   While it sits in `mChildren`, *any* window/host teardown that deletes the
   tree races the deferred reclaim; whoever runs second touches freed memory.
   Off-tree, the tree delete can never reach it and the reclaim stays the
   single owner.
3. **Events follow the obtain-then-drop contract**: every exit of the send
   pipeline that does not reach `AccessibilityManager::sendAccessibilityEvent`
   must `recycle()`. The exits found (and fixed) in `View`:
   - `isShown()` gate (was already correct),
   - bubble refused (`requestSendAccessibilityEvent` returned false),
   - **`parent == nullptr` with `mAttachInfo` still set** — the mid-teardown
     window where a *delayed* event (`SendViewScrolledAccessibilityEvent`
     posts hundreds of ms) fires after the tree detached the view. This was
     the largest family (19 blocks).
4. **AOSP drop-and-GC sites in RecycleBin delete explicitly here**, and every
   such delete must first remove the view from `mChildren` when it is only
   TEMP-detached (scrap taken by `trackMotionScroll` still sits in the array):
   `pruneScrapViews` (3 sites), `removeSkippedScrap`, `clearTransientStateViews`
   (2 sites), `scrapActiveViews` (2 discard branches). `fillActiveViews` must
   clear its slots first — a stale slot surviving into `scrapActiveViews`
   resurrects a freed view into `mChildren`.
5. **Re-entrant waits need orphan recycling**: `UiAutomation::
   executeAndWaitForEvent` pumps the looper; an already-due caller step can
   re-enter and overwrite a match the outer wait has not returned. The entry
   now recycles a stale `mWaitMatch`, and the return transfers ownership
   (member cleared).

## Investigation techniques (reusable)

- **Line-buffered stdout** (`stdbuf -oL`) — redirected runs otherwise buffer
  everything and a crash loses the log.
- **gdb scripted breakpoints after `break main; run`** (library is dynamic;
  breakpoints set before the load fail). `commands` blocks only work from a
  script file (`-x`), not chained `-ex`. Inline frames mean `this` is often
  unavailable — print `$rdi` instead.
- **Death notes**: temporary instrumentation in `~View` printing
  `backtrace()` frames symbolized with `dladdr()` to stderr (unbuffered).
  Match the crash-time object address against its death record; the outermost
  caller names the deleter. This is what pinned the family-4 scheduleViewReclaim
  lambda (`0x...+off` → `scheduleViewReclaim(...)::{lambda}#1::operator()`).
- A freed-but-unreused corpse shows a malloc chunk header in the vtable slot
  (`0xf1`); a reused one shows foreign data (e.g. `0x1f0000001f000000`, two
  ints). Both defeat `isAttachedToWindow()`-style guards — the guard reads
  stale/garbage fields — so guards are a backstop, not the fix.
- `sendAccessibilityEventUncheckedInternal`'s `mChildren` self-comparison is
  **not** a liveness proof: a dangling entry compares equal to itself.

## Open item — CLOSED (2026-08-30, fix pending commit)

The round-36/37 crash family is solved. It was never a stale `mChildren`
entry: the faulting frame chain (`AdapterView::dispatchPopulateAccessibility
EventInternal` → `View::dispatchPopulateAccessibilityEvent` → delegate
virtual call) had a LIVE child whose `mAccessibilityDelegate` held heap
garbage. Death notes proved the "corpse" address had never passed `~View`,
and a RAWSET trace pinned the writer: `AbsListView::obtainView`'s
AOSP-parity block —

```java
if (mAccessibilityDelegate == null) { mAccessibilityDelegate = new ListItemAccessibilityDelegate(); }
if (child.getAccessibilityDelegate() == null) { child.setAccessibilityDelegate(mAccessibilityDelegate); }
```

— was ported with the lazy `new` TODO'd out while the member stayed a raw
`ListItemAccessibilityDelegate*` that the constructor never initialized.
The uninit garbage survived the `== nullptr` check and was raw-set on every
measured child; when the accessibility manager is enabled (auto-test), the
first event dispatch through any such child jumps through a garbage
vtable. Same disease class as the `ExploreByTouchHelper::mNodeProvider`
poisoning. Fix (android-36 parity): the member is a null-default
`std::shared_ptr<ListItemAccessibilityDelegate>`, lazily created via
`make_shared` and handed to children through the OWNING delegate setter
(one instance on every child — the shared-delegate rule), and the
previously dormant `#if 0` delegate bodies
(`onInitializeAccessibilityNodeInfo` / `performAccessibilityAction` with
SELECT/CLEAR_SELECTION/CLICK/LONG_CLICK) are enabled behind a borrowed
`AbsListView* mHost` back-pointer (AOSP's inner-class `this`).

An adjacent hole closed in the same sweep: `AbsListView::resetList` deletes
the old children while the previous layout's `fillActiveViews` mirror
(`mActiveViews`) still lists them — a later `getActiveView` handed the
freed view back as a convertView and re-attached it into `mChildren`
(AOSP survives this on GC). `RecycleBin::forgetViews` now purges every
recycler array before those deletes.

## Files touched

- `src/gui/view/view.{h,cc}` — delegate refcounting, dual setter, pipeline
  recycle exits (bubble-refused, no-parent), corpse-safe `getAccessibilityDelegate`.
- `src/gui/widgetEx/recyclerview/recyclerviewaccessibilitydelegate.{h,cc}`,
  `recyclerview.{h,cc}` — shared item delegate, owning ctor delegate, dtor
  cleanup, owning set on item bind.
- `src/gui/widget/simplemonthview.*`, `radialtimepickerview.*` — TouchHelpers
  refcounted.
- `src/gui/widget/recyclebin.cc` — the six explicit-delete-with-detach fixes,
  slot reset in `fillActiveViews`.
- `src/gui/widget/adapterview.cc` — stale-selection guards.
- `src/gui/fragment/defaultspecialeffectscontroller.cc` — park-off-tree.
- `src/gui/res/layout/transient_notification{,_with_icon}.xml` — a11y=NO.
- `src/gui/app/uiautomation.cc` — orphan-match recycle + ownership transfer.
- `src/gui/preference/preferencegroupadapter.{h,cc}` — ExpandButton
  ephemerals freed on rebuild and teardown.

## Leak campaign results (2026-08-30)

Same runs under valgrind (--auto-test / WIDGETSDEMO_AUTOCYCLE+A11Y_DUMP),
"definitely lost" excluding libc/cairo/fontconfig internals:

| app | before | after |
|---|---|---|
| preferencedemo (--auto-test, 36-58 sweep rounds) | 19,880 B / 67 blocks | 0 |
| widgetsDemo (AUTOCYCLE+A11Y_DUMP, 29 pages) | 12,860 B / 43 blocks | 0 |
| printerdemo (--auto-test, 20 rounds, then the open crash) | n/a | 0 |
| kaidu_ms7 (1920x440 retest) | framework side clean, zero crashes | framework-clean; sweep found **zero targets** — the app's own UI exposes no clickable a11y nodes (app-side gap, not a framework leak) |

Additional leak fixes found by these sweeps:

- `View::setStateDescription` routes through
  `notifyViewAccessibilityStateChangedIfNeeded(CONTENT_CHANGE_TYPE_STATE_DESCRIPTION)`
  (AOSP parity — the port's direct obtain+send leaked on every drop exit);
  the missing `CONTENT_CHANGE_TYPE_STATE_DESCRIPTION` (0x40) constant was added.
- `RecyclerView::sendAccessibilityEventUnchecked` under
  `shouldDeferAccessibilityEvent` now accumulates the change types into
  `mEatenAccessibilityChangeFlags` (androidx parity — the port dropped both
  the accumulation and the event) and recycles the event. This single site
  was the largest family (~16 blocks per preferencedemo run).
- `ViewGroup::dispatchPopulateAccessibilityEventInternal` recycles the pooled
  `ChildListForAccessibility` on the early-return path (AOSP's try/finally);
  `ChildListForAccessibility::recycle` and `ViewLocationHolder::recycle`
  delete themselves on pool-full overflow.
- `NumberPicker` (updateInputTextView, AccessibilityNodeProviderImpl x2):
  the unsent event from `requestSendAccessibilityEvent` is recycled (AOSP:
  GC). This fired on every NumberPicker inflation — construction-time
  requests hit the parentless branch and dropped the event.
- `ExploreByTouchHelper::sendEventForVirtualView` / `invalidateVirtualView`
  were dormant stubs that obtained an event and dropped it; both now send
  via the parent (restored AOSP wiring) and recycle on refusal. The lazily
  created `ExploreByTouchNodeProvider` is freed in a new dtor.
- `ViewGroup::requestSendAccessibilityEvent` is public now (AOSP ViewParent
  surface — ExploreByTouchHelper, a non-child caller, needs it).

Measurement notes: put env vars BEFORE the tool (`VAR=1 valgrind ./app`) —
`valgrind ... env VAR=1 ./app` loses the summary report on exec; verify the
library really rebuilt (a make run from the source root silently does
nothing — run make from outX64-Debug).

## A-fix verification matrix (2026-08-30, fix in working tree, pending commit)

Runs with the AbsListView delegate fix + `RecycleBin::forgetViews` purge +
sweeper escape hatch applied, under valgrind. All rounds: **0 invalid
reads**; the only "definitely lost" residue is third-party (fontconfig
256 B / glibc baseline).

| app | rounds | result |
|---|---|---|
| preferencedemo | 3 | 242 / 197 / 113 PASS — historically it crashed by round 58 at the deepest |
| widgetsDemo | 3 | all RC=0; 364 B / 2 blocks = fontconfig + glibc baseline |
| printerdemo | 2 | 21 PASS, clean self-stop (historically crashed within 20 rounds) |
| hauswirt_63343 | 1 | 309 PASS, then SIGABRT — **pure app-side bug**: assigning to a joinable `std::thread` at `apps/hauswirt_63343/src/app/windows/layout/layout_curve.cc:446` → `std::terminate`. Its 12,984 B / 62 blocks split into two families: 10,824 B of `ValueAnimator`s new'd and never deleted by the app's `ViewUtils::animateBackgroundColor` (`view_utils.cc:63`, pop_home_sidebar chain), and 6,562 B of the driver's own a11y node snapshots — lost only because the abort skipped teardown (clean-stop apps show none) |
| kaidu_ms7 (1920x440) | 1 | framework side clean, zero crashes; sweep found zero targets — the app does not expose clickable a11y nodes (app-side gap) |

Sweeper escape hatch (new blind spot reached once A opened up CLICK): with
zero target windows (e.g. the "more options" PopupWindow), take 3 empty
steps then BACK; if BACK fails three times in a row, `stop()` ends the sweep
instead of spinning.

Temporary instrumentation (death notes / RAWSET trace) was removed and the
final rounds re-verified on a clean build. Working tree awaiting commit:
`abslistview.{h,cc}`, `recyclebin.{h,cc}`, `autotest.{h,cc}`, this document,
plus the input trio (`inputeventsource.{h,cc}`, `input_linux.cc`,
`wininput.cc`) held back per instruction.
