# CDroid

> **Android's UI framework, faithfully ported to C++ — Android-quality UI on embedded devices with no Android runtime, and a readable mirror of AOSP internals you can actually learn from.**

CDroid is a **line-by-line C++ port of Android's Java UI SDK** — `android.widget`, `android.view`, `android.text`, `android.graphics.drawable`, `android.animation` — built on top of **Cairo** (vector graphics) and targeting **embedded systems** (runs in as little as 32 MB of RAM). It is neither a wrapper around Android nor merely "Android-inspired": class names, member names, method signatures, and control flow track the AOSP sources closely enough that you can open the Android reference next to the C++ and follow along line by line.

If you can describe a UI in Android XML, you can run that same UI on a set-top box, an in-dash display, or an industrial panel — designed in Android Studio, rendered by Cairo, no JVM in sight.

* Main repository: [https://gitee.com/houstudio/cdroid](https://gitee.com/houstudio/cdroid)
* Mirror: [https://github.com/houstudio/cdroid](https://github.com/houstudio/cdroid)
* Contact: QQ 1225012331 / WeChat: calfhou

## Why does this exist?

Embedded teams keep reinventing UI frameworks, and the results are rarely as good as Android's. CDroid takes the other route: **port the real thing**, preserve a decade of Android UI engineering, and ship it where Android itself can't run. The guiding rule is *fidelity over redesign* — translate Android's logic verbatim instead of second-guessing it. (See [AGENTS.md](AGENTS.md).)

## Who is it for?

- **Embedded engineers** who need a polished, animatable, Material-style UI on constrained hardware (32 MB+) where full Android is too heavy.
- **Android developers** who want to understand *how the framework actually works* — View measure/layout/draw, Choreographer, input dispatch, text layout, drawables — by reading clean C++ instead of fighting the AOSP build. CDroid is the most readable mirror of AOSP framework internals that exists.
- **C++ engineers** looking for a substantial, well-architected codebase: real ownership/lifetime challenges, modern features (ConstraintLayout/MotionLayout, RecyclerView, fragments, transitions), and Cairo-based rendering.

## Features

- **100+ widgets & 50 drawables**, API-compatible with Android — design them in Android Studio / Eclipse.
- **Full AndroidX ports:** RecyclerView, ConstraintLayout + MotionLayout, FragmentManager + Navigation, Flexbox, CoordinatorLayout, ViewPager2.
- **Fragment + Transition framework**, including shared-element transitions.
- **Binary AXML resources end to end** — aapt2-compiled XML + `resources.arsc` packed into `.pak` archives, real resource ids in generated `R.h`, build-time resource overlay.
- **android.app services in-process** — AlarmManager (suspend-aware delivery, no dedicated thread), dialogs, a minimal PendingIntent.
- **In-process accessibility** — AccessibilityService + node tree, plus UiAutomation semantic test drivers (`--auto-test` sweep, `--test-script` DSL).
- **Vector graphics via Cairo** — no `Bitmap` class; `Cairo::ImageSurface` plays that role.
- **Multi-window / multi-layer compositor** with damage-region rendering.
- **Faithful text stack:** spans, `StaticLayout`/`DynamicLayout`, minikin line-breaking, fonts.xml family/fallback chain, color emoji (CBDT), `KeyCharacterMap`, IME.
- **Cross-platform backends:** DRM, fb, DirectFB, SDL, XCB/Xlib, VNC.

## Get involved 👋

CDroid is a large port — there is always more to translate from AOSP, and we'd love your help.

- 🆕 **[Good First Issues](docs/contributing/good-first-issues.md)** — concrete, self-contained tasks, each pointing to the exact AOSP source to port.
- 📖 **[Contributing guide](CONTRIBUTING.md)** — how to build, the fidelity methodology, and how to claim your first task.
- 中文说明见 [README.cn.md](README.cn.md).

# **Quick Start**
* VM(Ware):https://pan.baidu.com/s/1-v-rLcHxo5W5TXvJ2NUWxA fetchcode：spux (VM Ubuntu User:cdroid password:123456）
* After login pls run git pull to get new version of cdroid
* You'd better remove outXXX and run build.sh to rebuild makefiles after each git pull.
# **IDE(AndroidStudio/Eclipse)** 
![IDE](https://gitee.com/jiangcheng/cdroidX64/raw/master/apps/images/asd61236_ide.png)

# **ScreenShots**
![demo0](https://gitee.com/jiangcheng/cdroidX64/raw/master/apps/images/asd61236.gif)
![输入图片说明](https://foruda.gitee.com/images/1696897258873801535/181bd53c_8310459.png "coffee1.png")
![输入图片说明](https://foruda.gitee.com/images/1696897274979265997/cb22d7c6_8310459.png "coffee2.png")
![输入图片说明](https://foruda.gitee.com/images/1696897128191287720/7754542e_8310459.png "kdz10.png")
![输入图片说明](https://foruda.gitee.com/images/1696897669710472636/454e7f63_8310459.png "asd1.png")
![输入图片说明](https://foruda.gitee.com/images/1696897695571432137/8f6d2169_8310459.png "asd2.png")
![输入图片说明](https://foruda.gitee.com/images/1696897705672262478/c8736598_8310459.png "asd3.png")
![输入图片说明](https://foruda.gitee.com/images/1696897716776731960/47e420c7_8310459.png "asd4.png")
![Pott](https://gitee.com/houstudio/cdroid/raw/master/docs/images/screenshots/plot.png)

| View           | TextView             | Button               | ImageView    | ImageButton |
|----------------|----------------------|----------------------|--------------|-------------|
| CompoundButton | ToggleButton         | CheckBox             | RadioButton  | ProgressBar |
| SeekBar        | Chronometer          | AnalogClock          | ViewGroup    | RadioGroup  |
| ScrollView     | ViewPager            | SimpleMonthView      | Switch       | RatingBar   |
| NumberPicker   | ListView             | GridView             | RecyclerView | ViewFlipper |
| ViewAnimator   | AdapterViewAnimator  | CalendarView         | TabLayout    | DatePicker  |
| TimePicker     | NestedScrollView     | HorizontalScrollView | DateTimeView | ViewPager2  |
| YearPickerView | WearableRecyclerView | Toolbar              | QRCodeView   | CardView    |

# **Supported Layouts:**
| FrameLayout      | LinearLayout   | RelativeLayout | TableRow           | DrawerLayout      |
|------------------|----------------|----------------|--------------------|-------------------|
| TableLayout      | AbsoluteLayout | GridLayout     | GestureOverlayView | CoordinatorLayout |
| ConstraintLayout | MotionLayout   | FlexboxLayout  | SlidingPaneLayout  |                   |

# **Supported Drawables:**
| ColorDrawable     | BitmapDrawable     | NinePatchDrawable          | InsetDrawable        |
|-------------------|--------------------|----------------------------|----------------------|
| ShapeDrawable     | TransitionDrawable | AnimatedVectorDrawable     | StateListDrawable    |
| LevelListDrawable | ClipDrawable       | AnimatedRotateDrawable     | RotateDrawable       |
| GradientDrawable  | ScaleDrawable      | AnimatedImageDrawable      | VectorDrawable       |
| RippleDrawable    | AnimationDrawable  | AnimatedStateListDrawable  | LayerDrawable        |
| BadgeDrawable     | PictureDrawable    | AnimationScaleListDrawable | AdaptiveIconDrawable |

# **Porting guide:**

* 1 A new product porting should be placed to src/porting/xxx(where xxx is you chipset name)
* 2 implement your porting api to xxx directory
* 3 modify build.sh to support your port(you should configure sysroot toolchain...).
* 4 call build.sh --product=xxx
* 5 make you project(SeeAlso **Building CDROID**)

# **Building CDROID:**
### 1.install dependencies(Ubuntu 22+):
sudo apt install autoconf libtool build-essential aapt cmake gdb pkg-config zip curl unzip gettext libx11-dev libxcursor-dev libxcb1-dev libxcb-image0-dev libxcb-cursor-dev bison python3 python3-pip python3-lxml python3-pil meson
# notes: the "aapt" package ships /usr/bin/aapt2; all Python deps of the build
# scripts (lxml + Pillow) come from python3-lxml / python3-pil above -- no pip
# install is needed (and bare "pip install" is refused on Ubuntu 23.04+).
### 2.install vcpkg:
* git clone https://www.github.com/microsoft/vcpkg.git
* cd vcpkg
* ./bootstrap-vcpkg.sh
### 3.download cdroid source code:
* cd ~
* git clone http://www.gitee.com/houstudio/cdroid.git<br>
### 4. install cdroid supported toolchain's patch
* cp cdroid/scripts/vcpkgpatch4cdroid.tar.gz vcpkg/
* cd vcpkg
* tar -zxvf vcpkgpatch4cdroid.tar.gz 
### 5.install cdroid deplibs:
* ./cdroid_install_libs.sh --triplet=x64-linux-dynamic<br>
### 6.build cdroid:
* cd cdroid
* ./build.sh --build=debug
* cd outX64-Debug
* make -j
### 7.prepare system and app resource
* No manual copy in the build tree: cdroid.pak / widgetex.pak are generated at the out-root and every app locates them automatically (probe order: the executable's directory and its parents, then cwd, then /usr/share/cdroid and /opt/cdroid).
* An app's own `<app>.pak` is picked up beside its binary (override the directory with `--data`).
* For device installs put cdroid.pak / widgetex.pak under /usr/share/cdroid (or /opt/cdroid).
### 8.build & run samples(in directory outX64-Debug)
* each sample is its own make target, e.g. `make buttons` then run `apps/samples/buttons`
* `make alarmmanager && apps/samples/alarmmanager` (AlarmManager demo)


