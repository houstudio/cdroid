# **V4.x.x
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

