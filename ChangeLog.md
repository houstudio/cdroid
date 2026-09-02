# **5.
  - android.transition framework (31 classes: scene transitions, shared elements)
  - androidx.navigation (NavHost / NavController / NavGraph)
  - Fragment / FragmentManager / FragmentStateManager (nested hosts, back stack)
  - ConstraintLayout and MotionLayout (full features: chains, helpers, Carousel)
  - new TextView fully compatible with Android, spannable rich text
  - text layout family on minikin: StaticLayout, DynamicLayout, BoringLayout
  - FlexboxLayout and FlexboxLayoutManager
  - Binary AXML resources end to end: androidfw AssetManager port, PakBuilder (aapt2-compiled
    AXML + resources.arsc + cdNp 9-patch chunks), R.h from real arsc ids; idgen retired
  - AttributeSet migrated to int resource ids tree-wide (DECLARE_WIDGET2, int defStyle,
    framework-private attr block); string-key attribute lookups retired
  - TypedArray/TypedValue aligned with AOSP ResourcesImpl; styleable generation; theme attr
    chain and themed-cache keys fixed; ResourcesImpl density initialized
  - Build-time resource overlay (AOSP static-overlay semantics); slim framework-res base;
    i18n.dat packed into pak; build-time fonts.xml; multi-pak 0x7f id pinning policy
  - AOSP Music app facade port (apps/music) plus the core fixes it surfaced
  - Preference listener interfaces moved to value semantics (EventSet/CallbackBase)
  - Teardown/UAF/leak campaign under valgrind: exit sentinel, window teardown ordering,
    observer pinning, AbsListView touchMode death-belt, RecycleBin same-key overwrite,
    AlertController GC points; preferencedemo dialog crashes eliminated
  - Drawable module audited against AOSP (47 findings); tint/PorterDuff emulation hardened;
    VectorDrawable cache/tint fixes; AnimatedVectorDrawable leak closed
  - ContextImpl split into three layers with Context as a pure interface
  - View/ViewGroup refreshed against android-36; ScrollView/NestedScrollView fixes
  - Tests: libutils Looper suite, coretests text/ and i18n, key-navigation suite,
    drawable/os CTS ports, gui_test shared-looper harness
  - In-process accessibility service end to end: AccessibilityService/
    AccessibilityServiceInfo port, AccessibilityManager as the in-process AMS
    (registry + dual-layer event filtering), walkable node tree (providers/
    virtual views included), ByText/ByViewId search, focus highlight, full
    event/node recycle contract (zoo valgrind 3.2MB -> 256B)
  - android.app.UiAutomation port + semantic test drivers: --auto-test sweep
    (per-step snapshot follows pager/tab navigation, node isVisibleToUser
    filtering, auto-scroll between cycles) and --test-script line DSL
    (Tokenizer lexer, wait/click/assert/dump, CI exit code)
  - androidx ViewCompat a11y action API collapsed onto View
    (add/remove/replaceAccessibilityAction + hasAccessibilityDelegate),
    wired into ViewPager2 / DrawerLayout / SlidingPaneLayout /
    AppBarLayout / ViewPager
  - Material default styles (defStyleRes) for AppBarLayout / CollapsingToolbarLayout /
    BottomNavigationView / NavigationView / TabLayout: per-module Widget_Design_* styles
    with pinned 0x0209xxxx ids; gen_styleable emits typed public pins (style/layout/id/
    drawable); widgetex.pak ships compiled res files (uses-sdk added — without it aapt2
    strips attributes into -v1 variants); wear ConfirmationOverlay on pinned R constants
  - preferencedemo: portrait settings chrome, Slide fragment transitions (legacy animation
    path kept behind PREFDEMO_ANIM), AUTOCYCLE valgrind driver
  - Emoji editing semantics on a par with Android: myicu binary properties now generated
    from UCD emoji-data.txt (Emoji/Modifier/Component/Extended_Pictographic/Variation_
    Selector; accessor enum-numbering fixed), BaseKeyListener deletes by emoji state
    machine (flag pairs, ZWJ sequences, variation selectors, keycaps, skin tones, tags),
    grapheme-cluster cursor movement verified; coretests text battery 276 passed
  - Emoji display end to end: Paint::getFontMetricsInt fills real top/bottom/leading from
    the font file, TypedArray::getString astral-codepoint UTF-8 fixed (was mangling emoji
    into tofu + garbage), TextView renders emoji through the fonts.xml fallback chain;
    widgetsDemo Text page gained an Emoji card
  - CBDT color emoji (Noto Color Emoji): color bitmap glyphs render as scaled images —
    Typeface serves per-glyph ARGB32 bitmaps (dedicated FT faces, strike-bound, cached),
    Paint::drawTextRun does the blit; requires a freetype built with PNG support
  - Font system reduced to fonts.xml + R.font: fontconfig-era matching retired
    (parseStyle/fetchProps/isSameFamily/SYSLANG hack; fallback chain deduped by font file,
    one mmap per file), legacy string-array PAK font loader removed, TextView resolves
    android:fontFamily="@font/x" resources with no app code (android-36 parity), plain
    family names no longer leak into the font asset loader
# **4.9.6
  - some memleaks
  - add TouchDevice VirtualKeyMap support
  - fix NumberPicker::onTouchEvent's behavior(ACTION_CANCEL)
  - add AChartEngine(Kplot is removed)
# **V4.8.6
  - AnimatedImageDrawable add decodeWorker thread.
  - Fix ColorStateLists's defaultColor
  - add BadgeDrawable
  - Elegant Resource Cleanup and Exit
  - add CardView
  - add RoundRectDrawable,RoundRectDrawableWithShadow
  - Fix GridLayout's layout issues
  - Change RecyclerView::setLayoutManager(LayoutManager*) to setLayoutManager(std::unique_ptr<LayoutManager>)
# **V4.6.9
  - add MotionEvent for MouseDevice support.
  - add Tablayout::setTabIndicatorAnimationMode(tabIndicatorAnimationMode:linear,elastic,fade)
  - add TabLayoutMediator(make TabLayout working friendly with ViewPager2)
  - add single Frame's Webp to Bitmapdrawable supported
  - add Bitmap,Jpeg2000 support
  - fix HorizontalScrollView's fling.
  - fix RippleDrawable's memleak
# **V4.4.8
  - DrawerLayout add Top/Bottom slider,friendly for landscape and portrait
  - Add XCB Graph support,faster than XLIB and less tearing
  - Add AsyncInflater
  - Add SlidingPaneLayout
  - Fix TabLayout's double free & memleak issues
  - Fix ViewDragHelper's memleak
  - TextView's fontstyle add Italic support
  - Fix NumberPicker's divider position in horizontal mode
  - Remove InputDevice::popEvent,Add InputDevice::drainEvents
# **V4.3.3
  - Toolbar
  - GestureOverlayView OK.
  - fix TextView::getBaseline
  - fix GridLayout::LayoutParams's layout_gravity
  - add BaseBundle,Bundle
  - add BitmapDrawable::isStateful
  - add DynamicAnimation,SpringAnimation,FlingAnimation
  - add porting/android
  - fix Spinner,try it now:)
  - fix ViewPage2
  - fix Assets's parser
  - fix ViewOverlay's memleaks
  - fix ViewPager's crash issue while Adapter.getItemPosition return NONE
# **V3.8.6
  - XmlPullParser is more compatible with android.
  - Resource parser changed to xmlpullparser,more faster versions.
  - Bug fixes
# **V3.8.0
  - Add VectorDrawable
  - Add AnimatedVectorDrawable
  - Add AnimationScaleListDrawable
  - Add XmlPullParser
  - Add DrawableInflater
  - Fix LayoutDirection's inherite
  - AnimatedStateListDrawable working
  - Change all drawables's parser to XmlPullPaser,drawables'inflate is more strong than ever
  - Fix WindowManager::sendToBack
  - Other's bug fixes"
# **V3.6.0
  - Add SoundPool,Environment
  - Fix AnimatorSet,ObjectAnimator's memleak
  - Fix some Recyclerview's memleak
  - Fix RotateDrawable AnimatedRotateDrawable's invalidate issues
  - Fix Globale Properties's memleak(make valgrind happy)
  - Fix DefaultItemAnimator's memleak
  - Fix StateListAnimator's memleak
  - Fix Switch's memleak caused by thumb's animator
  - RecyclerView add GapWorker support(with prefetch support features)

# **V3.4.5
  - AnimatorInflater
  - AnimatorSet,PropertyValueHolder OK.
  - GradientDrawable add pattern(image surface)gradient.
  - Add StateListAnimator support.
  - Fix ScrollView.onOverScrolled
  - Fix several Invalidate Issues
  - Fix RecyclerView's dragdrop issues
  - ViewPager2
  - FastScroll add animator support
  - Add Accessibility api entries
  - Some Bug fixes
# **V3.0.0
  - add CoordniatorLayout
  - add Win32 porting
  - add some losting fucntions
  - add allwinner tina's porting
# **V2.1.0**
  - Some bug fix.
  - add CMS(Color Manager System).
  - Cairo::Pattern add dither support.
  - Apng animation supported reopened(with less memory).
  - BitmapDrawable,GradientDrawable add Dither support,GradientDrawable with alpha optimized.
  - RecyclerView can working on lower(default) poolsize.
  - add new image-decoders,all image is decoded to RGBA32. ui layer compsition can be faster than ever.
  - add GestureDetector

# **V2.0.66**
  - Some bug fixes
  - RecyclerView's ItemAnimator and ItemTouchHelper is supported
  - Animation's Callback has moved to Choregrapher
  - Add HandlerActionQueue,View's Runnable can be post/removes while view it not attached.

# **V2.0.0**
  - The first stable public version.
  - fix many small memleak
  - ColorStateList is designed to no freed(managed by map)
  - Each interpolater is designed to be has a global intstance,and io free is needed;other customer instance is owned by caller

# **V1.0.0**
  - Commemorating the Miniwin versions, do not use for commercial purposes. 

