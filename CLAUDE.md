# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What CDROID is

CDROID is a **line-by-line C++ port of Android's Java UI SDK** (the `android.widget` / `android.view` / `android.text` / `android.graphics.drawable` / `android.animation` frameworks) on top of **Cairo** (vector graphics), targeting embedded systems. The overriding rule: **fidelity to the official Android implementation** — match class names, member names, method signatures, and behavior; translate Android's logic verbatim rather than redesign. `AGENTS.md` (Chinese) states the principles and is authoritative. Reference sources: `/opt/android-sdk/sources/android-36/` (primary) and `android-34/`. Read the Android original before porting or fixing.

The whole tree lives under `src/gui/` (mirrors `android.*` packages) plus `src/porting/` (platform backends), `src/3rdparty/`, `src/modules/`. `apps/` holds per-product apps and one-file `apps/samples/`.

## Build / run

- Configure once per platform: `bash build.sh -p x64 -b Debug` (creates `outX64-Debug/`). Cross builds use `build.sh --product=<chipset>`; per-platform `.kl`/`.kcm` keymaps + glue live in `src/porting/<chipset>/`.
- Incremental build (the common case): `cd outX64-Debug && make cdroid -j44`. **Always `-j44`** — single-threaded `make cdroid` can exceed tool timeouts.
- A single sample/app is its own target: `make <name> -j44` (e.g. `make edittext`; source at `apps/samples/<name>.cc`).
- `make` alone reconfigures CMake when a `CMakeLists.txt` changes (auto-reconfigure). Note `src/gui/text/CMakeLists.txt` is an **explicit source list** (not a glob) — new `text/method/*.cc` must be added there.
- Don't pipe make through `tail`/`grep` if you need the real exit code (it masks `$?`).
- `cdtext` (the text library) builds with `-Werror`.
- **Ignore IDE/clangd errors** like `cairo.h: not found` / `unknown type` — clangd lacks the `src/gui` include root; trust the real g++ build.
- Tests: `tests/` (out of scope for this analysis) — run via the usual make targets in the build dir.

## Architecture

### Runtime & main loop (`src/gui/core/`, `src/gui/app/`)
- **`App`** (`core/app.h:34`) is the process singleton (`App::getInstance()`) and **is-a `Assets`/`Context`** — there is no separate Activity/Application; one object answers `getString`/`getDrawable`/`loadImage` from `.pak` resources. `App()` bootstraps everything (flags, fonts, `cdroid.pak`, registers input); `exec()` is literally `while(!mQuitFlag) looper->pollAll(1);`.
- **`Looper`** (`core/looper.h:77`) is a near line-by-line port of AOSP `libutils/Looper` (epoll + eventfd, TLS, `addFd`/`LooperCallback`). The main looper = the **UI thread**. CDROID adds **`EventHandler`** (`checkEvents()`/`handleEvents()`) drained each iteration.
- **Three `EventHandler`s drive everything:**
  - **`InputEventSource`** (`core/inputeventsource.h:31`) — spawns its **own detached thread** blocked in evdev `InputGetEvents()`; the main thread only drains the produced queue → `WindowManager::processEvent`. Analog of `InputManager`/`EventHub`.
  - **`UIEventSource`** (one per `Window`) — the **frame driver**: `handleEvents` = run posted runnables → run layout → `Window::draw()` → `GraphDevice::flip()` → `composeSurfaces()`.
  - **`Choreographer`** (`view/choreographer.h:23`) — time-based `doFrame` (INPUT/ANIMATION/TRAVERSAL/COMMIT). **No real VSYNC** — self-paces against `SystemClock` + looper poll timeout.
- **Two threads total:** InputEventSource's input thread + the main looper (everything else). Events cross via the device queue.

### Rendering path (the seam everything draws through)
- **`Canvas`** (`core/canvas.h:30`) **is-a `Cairo::Context`** — a Canvas *is* a `cairo_t` (cairomm wrapper, `src/gui/cairomm/`); there is no intermediate abstraction. `View::onDraw(Canvas&)` and `Drawable::draw(Canvas&)` program that cairo context directly (`set_source`, `paint_with_alpha`, `fill`, `push_group`…). **There is no `Bitmap` class — that role is `Cairo::ImageSurface`.**
- **`GraphDevice`** (`core/graphdevice.{h,cc}`) is the compositor: owns the on-screen `Cairo::Surface` + a `Canvas* mPrimaryContext`. `composeSurfaces()` (`graphdevice.cc:324`) intersects each window's dirty/visible regions and blits onto the primary, then `GFXFlip`. Rendering is **damage-region + blit, not full repaint**.
- **Display/input backends** are abstract C APIs: `src/porting/include/cdgraph.h` (impls in `src/porting/common/`: `graph_drm`, `graph_fb`, `graph_dfb`, `graph_sdl`, `graph_xcb/xlib`, `graph_rfb`/VNC) + SoC dirs; `cdinput.h` → `input_linux.cc` (evdev `/dev/input`).

### View tree & widgets (`src/gui/view/`, `src/gui/widget/`)
- **`View`** (`view/view.h:83`, ~10k lines) `: public Drawable::Callback, public KeyEvent::Callback`. **`ViewGroup`** (`view/viewgroup.h:30`) adds children + `LayoutParams`. `AttachInfo` (`view.h:1541`) is the per-tree context (analog of `ViewRootImpl`'s AttachInfo).
- **Pipeline:** `measure` (`view.cc:9752`) → virtual `onMeasure` (`:7524`) → `layout` (`:7456`) → `onLayout` → `draw` (`:3055`, the 6-step Android order: background→`onDraw`→`dispatchDraw`→foreground→scrollbars→focus). `ViewGroup::dispatchDraw` (`viewgroup.cc:2339`)/`drawChild` (`:2257`).
- **Input dispatch:** `Window` (`widget/cdwindow.h:114`) → key to focused view (`cdwindow.cc:575`), touch to topmost hit (`:712`). `ViewGroup::dispatchTouchEvent` (`viewgroup.cc:3728`) + `onInterceptTouchEvent` (`:3607`) walk children back-to-front; `View::dispatchTouchEvent` (`view.cc:7981`) → `onTouchEvent` (`:8481`).
- **XML inflation:** `LayoutInflater` (`view/layoutinflater.h`) uses a **static factory registry, no reflection**. Widgets self-register at static-init via `DECLARE_WIDGET(T)` (`layoutinflater.h:118`) → `registerInflater(name, [](ctx,attr){return new T(ctx,attr);})`. Adding a widget needs only that macro — no central registration file.
- **`Editor`** (`widget/editor.{h,cc}`) is a port of `android.widget.Editor`: **not a View**, a helper with back-pointer `TextView* mTextView`; `TextView` declares `friend class Editor`. Owns the editable UX (caret blink/draw, selection, key/touch editing). `EditText : TextView` wires it.
- Adapter lists (`AbsListView`/`ListView`/`GridView`), scrollers, pickers (Android "delegate" pairs), etc. — all faithful `android.widget.*` ports.

### Text (`src/gui/text/`) — `android.text.*`
- **Layout family** (`layout`, `staticlayout`, `dynamiclayout`, `boringlayout`) — line breaking & measurement, on top of **minikin** (`src/gui/text/minikin/`, vendored from **Android 14**, treated as third-party — **do not modify**). The Layout↔minikin version seam is the main place text bugs hide.
- **Spans** (`spannablestring.{h,cc}`): `SpannableStringBuilder`/`SpannableString`/`SpannedString`. See **Span ownership** below.
- **`method/`** — `KeyListener`s (`Qwerty`/`Digits`/`Dialer`/`Date`/`DateTime`/`Time`/`MultiTap`), `MovementMethod`s, `TransformationMethod`s. C++14 port of `android.text.method`.
- `selection.cc` (`cdroid::Selection`), `textpaint`, `androidbidi` (uses **myicu** — a TRAE-generated C-API ICU subset, known-buggy in places like `ubrk_*`; verify before trusting).
- Char convention: **`char16_t`/`std::u16string` primary** internally; `std::string` (UTF-8) only at I/O boundaries. Convert via `TextUtils::utf8_utf16`/`utf16_utf8`.

### Input (`src/gui/core/`) — key/motion resolution
- **`InputEventSource`** is the **device registry** (`unordered_map<fd, InputDevice>`) = `InputManagerGlobal.getInputDevice(id)`. `InputDevice`/`KeyDevice`/`TouchDevice` constructed only here.
- **`KeyCharacterMap`** (`private/keycharactermap.h`, impl `core/keycharactermap.cc`) is a faithful port of Android **native** `KeyCharacterMap.cpp` (the `.kcm` parser + Key/Behavior model), **per-device**, owned by `InputDevice` (loaded like `.kl`). `KeyEvent::getKeyCharacterMap()` resolves chars via the device's KCM — **not via IMM**.
- **`InputMethodManager`** is the **IME manager only** (on-screen `IMEWindow`/`keyboardview`), driven by focus. It does NOT do hardware key→char.
- `getGlobalMetaState` ORs every `KeyDevice::getMetaState()` — the InputReader-level modifier view attached to all events.

### Graphics / Drawable / Image / Animation
- **`Drawable`** (`drawable/drawable.h:66`) base; ~50 subclasses (`BitmapDrawable`, `StateListDrawable`/`DrawableContainer`, `LayerDrawable`, `GradientDrawable`, `VectorDrawable`/`AnimatedVectorDrawable`, `RippleDrawable`, `NinePatchDrawable`, `AnimationDrawable`…). `Drawable::draw(Canvas&)` issues raw cairomm calls. **Tint** is emulated via `beginTintGroup`→`canvas.push_group()`→filter→`pop_group_to_source()`→`paint` (cairo has no per-pixel source filter).
- **`DrawableInflater`** (`drawable/drawableinflater.cc:68`) — static `drawableParsers` map: XML tag → factory (`selector`→`StateListDrawable`, `layer-list`→`LayerDrawable`, `shape`→`GradientDrawable`, `vector`→`VectorDrawable`, …).
- **`ImageDecoder`** (`image-decoders/imagedecoder.h:31`) — registry of `{factory, verifier, magicSize}` by mime; builtins PNG/GIF/JPEG/WEBP/JP2/BMP (feature-flagged). `loadImage` → `Cairo::ImageSurface` (ARGB32); animated → `AnimatedImageDrawable` via `FrameSequence`.
- **`cairomm/`** — vendored cairomm; `RefPtr<T>` = `shared_ptr<T>`. No `Bitmap` — `Cairo::ImageSurface` plays that role.
- **Animation** (`animation/`): `ValueAnimator`/`ObjectAnimator`/`AnimatorSet` (`android.animation`) + legacy `Animation`s + physics (`SpringAnimation`). Advanced per-frame by **`Choreographer → AnimationHandler → ValueAnimator::doAnimationFrame`**, mutating properties → `invalidate()` → re-compose.

### Beyond-stock (`src/gui/widgetEx/`) — AndroidX/Material ports
`recyclerview/` (full `androidx.recyclerview` 1:1: Adapter/ViewHolder/LayoutManager/Recycler/ItemAnimator…), `flexbox/`, `coordinatorlayout/`, `viewpager2`, plus CDROID-specific `barcodeview`/`qrcodeview`/`rlottieview`/`plotview`. Precedent for the Android→C++ porting style (commits tagged `:construction:`). `src/gui/menu/` and `src/gui/navigation/` are `androidx.appcompat.view.menu` and `androidx.navigation` ports; `src/gui/gesture/` is the learning-based `android.gesture` library (GestureDetector/ScaleGestureDetector detectors live in `view/`).

### Supporting
- **`i18n/`** (header-only): date/number/measure/locale formatting (ICU-style data tables).
- **`utils/`**: Android-style `RefBase`/`StrongPointer`/`LightRefBase` (the sp/wp smart-pointer model), `Flattenable`, `ArrayBlockingQueue`, `StringTokenizer`.
- **`private/`**: impl headers (`keycharactermap`, `keylayoutmap`, `ziparchive`, input labels).
- **Resources**: `src/gui/res/` (built-in color XML etc.) compiled into **`.pak`** ZIPs (`cdroid.pak` + one per app) by `scripts/pakbuilder.py` — one `PakBuilder` class does it all: stripped XML + aapt-compiled 9-patch (cdNp chunk) + verbatim binaries, and it reuses `scripts/idgen.py` (via inheritance) to generate `R.h` IDs.
- **`modules/wpa`** (WiFi/wpa_supplicant), **`3rdparty/pinyin`** (Google Pinyin IME).

## Critical, non-obvious conventions (caused real bugs)

- **Span ownership** (`spannablestring.h`): raw `ParcelableSpan*` + an `owned` flag — `owned = (dynamic_cast<NoCopySpan*>(span)==nullptr)`. Owned spans are deleted by the container on removal and deep-cloned on copy; **NoCopySpan** (watchers, Selection markers, KeyListeners) are borrowed — never freed/cloned by the container. **`TextWatcher`/`SpanWatcher` must `virtual public NoCopySpan`** (a class multiply inheriting both otherwise has two NoCopySpan subobjects → `dynamic_cast` misclassifies as owned → double-free). No smart pointers.
- **CharSequence lifetime in TextView**: `DynamicLayout::~DynamicLayout` derefs its base text (`mBase == mText`), so **text must outlive the layout**. TextView's dtor deletes layouts before text; `setText` frees old text only after `checkForRelayout`. `setText(CharSequence*)` **transfers ownership**.
- **`TransformationMethod::getTransformation`** returns an **owned `CharSequence*`** the host `TextView` `delete`s on replace — `new` a wrapper, never return `&source`.
- **`Paint::getTextRunCursor`** returns a **text-absolute** offset (both overloads) — callers must not subtract `start` from the result.
- **Canvas = cairo_t**: don't look for a separate rendering abstraction; `onDraw`/`Drawable::draw` program cairo directly.

## Workflow

- After any change, **rebuild green** (`make cdroid -j44` or the target) before considering the task done — that's the bar, not IDE diagnostics.
- For runtime behavior (cursor, selection, rendering) the developer runs the GUI and reports; instrument with temporary `LOGD` (from `<porting/cdlog.h>`) to diagnose, then remove.
- When porting a new Android class, follow the sibling pattern (e.g. `DialerKeyListener` for KeyListeners; `method/` lowercase filenames) and add to the explicit `text/CMakeLists.txt`.
- Some Android classes are intentionally **stubbed** when deps aren't ported (e.g. `InsertModeTransformationMethod`/`TranslationTransformationMethod` depend on `@hide` frameworks; IME/InputConnection is out of scope). A faithful stub (interface present, pass-through body + `TODO`/`DEFERRED`) matches the codebase pattern.
