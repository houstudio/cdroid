# Contributing to CDroid

First: thank you for considering a contribution. CDroid is a large port and there is always more to translate from AOSP — every widget, drawable, or method you port closes a real gap. This guide gets you from `git clone` to a merged change as fast as possible.

中文版见 [CONTRIBUTING.cn.md](CONTRIBUTING.cn.md).

## Ways to contribute

- **Port a stub.** Many methods have a faithful signature but an empty/passthrough body. Find one, read the AOSP original, translate the logic. (See [Good First Issues](docs/contributing/good-first-issues.md).)
- **Add a missing sample.** Every ported widget deserves a runnable demo under `apps/samples/`.
- **Fix a bug** you hit while running a sample.
- **Add a platform backend** under `src/porting/<chipset>/` for your hardware.
- **Improve docs.**

## The one rule: fidelity

CDroid ports Android **faithfully, not creatively**. When in doubt:

1. **Read the Android original first.** Reference sources live at `/opt/android-sdk/sources/android-36/` (primary) and `android-34/`. On your own machine, point your editor at the AOSP `frameworks/base` tree or the SDK `sources/`.
2. **Match names and signatures.** Class, member, and method names track the Java/Kotlin original. `java.awt`/Android conventions, not reinvented ones.
3. **Translate the logic verbatim**, not a redesign. If Android branches, you branch. If Android calls a helper, you call (or port) that helper.
4. **Differences must be justified** by C++ or the platform (e.g. no GC → explicit ownership; no `Bitmap` → `Cairo::ImageSurface`; `char16_t`/`u16string` internally, UTF-8 only at I/O). Note these in a comment when they're subtle.

See [AGENTS.md](AGENTS.md) — it's authoritative on the porting principles.

## Your first contribution, step by step

1. **Pick a task** from [Good First Issues](docs/contributing/good-first-issues.md). Each one names the CDROID file, the AOSP reference, and how to verify.
2. **Build the baseline** (below) so you have a working tree.
3. **Open the AOSP source** cited in the task and read the method you're porting.
4. **Port it.** Copy the sibling-pattern conventions you see in nearby files (e.g. for a new `KeyListener`, mirror `DialerKeyListener`).
5. **Rebuild green.** The bar is a clean `make`, not IDE diagnostics (see "Ignore clangd" below).
6. **Run the sample** if there is one, and eyeball the result.
7. **Open a PR** (Gitee or GitHub). Branch off `dev`, keep the change focused.

## Build fast-path

```bash
# one-time configure (creates outX64-Debug/)
bash build.sh -p x64 -b Debug

# incremental — the common case. ALWAYS use -j44
cd outX64-Debug
make cdroid -j44

# a single sample is its own target (source: apps/samples/<name>.cc)
make spinner -j44
./spinner        # run it from outX64-Debug/
```

Notes that will save you time:

- **Always `-j44`.** Single-threaded `make cdroid` can exceed tool timeouts.
- **`make` alone reconfigures CMake** when a `CMakeLists.txt` changes.
- **`src/gui/text/CMakeLists.txt` is an explicit source list, not a glob** — new `text/method/*.cc` must be added there by hand.
- **Adding a new widget needs only `DECLARE_WIDGET(T)`** in its header — it self-registers with the inflater at static-init. No central registration file.
- **Ignore clangd errors** like `cairo.h: not found` / `unknown type`. clangd lacks the `src/gui` include root; trust the real `g++` build.

## Codebase map

| Area | Location | Android analog |
|---|---|---|
| Widgets | `src/gui/widget/` | `android.widget.*` |
| View system | `src/gui/view/` | `android.view.*` |
| Text | `src/gui/text/` | `android.text.*` |
| Drawables | `src/gui/drawable/` | `android.graphics.drawable.*` |
| Animation | `src/gui/animation/` | `android.animation.*` + legacy `view.animation` |
| AndroidX ports | `src/gui/widgetEx/` | `androidx.*` (RecyclerView, ConstraintLayout, …) |
| Core runtime | `src/gui/core/` | `App`, `Looper`, `Canvas`, `GraphDevice` |
| Platform backends | `src/porting/<chipset>/` | DRM/fb/SDL/XCB/VNC glue |
| Samples | `apps/samples/` | one-file runnable demos |

## Conventions that bite

These have caused real bugs — read once, remember forever:

- **`Canvas` *is* a `cairo_t`.** There is no separate rendering abstraction and no `Bitmap` class (`Cairo::ImageSurface` plays that role). `onDraw(Canvas&)` programs cairo directly.
- **Text is `char16_t`/`u16string` internally**, UTF-8 only at I/O boundaries. Convert via `TextUtils::utf8_utf16` / `utf16_utf8`.
- **Span ownership is manual** (`owned` flag, gated by `NoCopySpan`). `TextWatcher`/`SpanWatcher` must `virtual public NoCopySpan`. No smart pointers in the span layer.
- **`setText(CharSequence*)` transfers ownership;** text must outlive its `Layout`.
- **C++14 has no `std::optional` here** — Android `null` is an enum sentinel (e.g. `TruncateAt::NONE`, `Alignment::NONE = -1`), by design.
- **Read-only methods get `const`** (getters, `is*`, `has*`); methods returning a mutable pointer/reference stay non-const.

## Commit & PR conventions

- Branch off `dev`. Keep one PR focused on one task.
- Commit prefix follows the existing history: `:construction:` for ports/WIP, `:bug:` for fixes, e.g. `:construction: port Spinner.setOnItemClickListener`.
- End commit messages with:
  ```
  Co-Authored-By: <your name> <email>
  ```
- Reference the AOSP class/method you ported in the PR description, and attach a screenshot if the change is visible.

## Getting help

- Open an issue on [Gitee](https://gitee.com/houstudio/cdroid) or [GitHub](https://github.com/houstudio/cdroid).
- QQ: 1225012331 / WeChat: calfhou.

Happy porting! 🚧
