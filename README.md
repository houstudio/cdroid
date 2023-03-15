# **Introduction**
**CDroid** is a android like GUI toolkit base on C++11,It is a cross-platform GUI engine for embedded system.

# **Features:**
* Multi Layer/Multi Window 
* Integrated with 40+ UI Components and 20+ Drawables(compatible with android)
* All UI Components(Layouts,Drawables...)can be designed in **AndroidStudio or Eclipse** 
* Vector Graph supported via **Cairo**
# **Screen Shots**
![demo1](https://gitee.com/houstudio/cdroid/raw/master/docs/videos/uidemo1.gif)
![demo2](https://gitee.com/houstudio/cdroid/raw/master/docs/videos/uidemo2.gif)
![Pott](https://gitee.com/houstudio/cdroid/raw/master/docs/images/screenshots/plot.png)
![ListView+Progress](https://gitee.com/houstudio/cdroid/raw/master/docs/videos/list_with_progress.gif)
# **UI Components:**
   * View
   * TextView 
   * EditText
   * Button 
   * ImageView 
   * ImageButton,
   * CompoundButton 
   * ToggleButton 
   * CheckBox 
   * RadioButton
   * ProgressBar 
   * SeekBar 
   * Chronometer 
   * AnalogClock,
   * SimpleMonthView
   * ViewGroup 
   * RadioGroup 
   * ScrollView 
   * HorizontalScrollView 
   * CalendarView 
   * ViewPager 
   * TabWidget 
   * NumberPicker
   * AdapterView 
   * AbsListView 
   * Spinner 
   * ListView 
   * GridView 
   * YearPickerView

# **Supported Layouts:**
   * FrameLayout 
   * LinearLayout 
   * TableRow 
   * TableLayout 
   * AbsoluteLayout 
   * GridLayout

# **Supported Drawables:**
   * ColorDrawable 
   * BitmapDrawable 
   * NinepatchDrawable
   * InsetDrawable 
   * ShapeDrawable
   * TransitionDrawable
   * LayerDrawable 
   * StateListDrawable 
   * LevelListDrawable
   * ClipDrawable
   * GradientDrawable 
   * RotateDrawable
   * ScaleDrawable 
   * AnimatedRotateDrawable
   * AnimatedImageDrawable 
   * TransitionDrawable

# **Porting guide:**

* 1, A new product porting should be placed to src/porting/xxx(where xxx is you chipset name)
* 2, implement your porting api to xxx directory
* 3, modify build.sh to support your port(you should configure sysroot toolchain...).
* 4, call build.sh --product=xxx
* 5, make you project(SeeAlso **Building CDROID**) 

# **Building CDROID:**
* 1.install dependencs: sudo apt install build-essential cmake gdb pkg-config zip gettext libx11-dev
* 2.install vcpkg
* 3.install cdroid deplibs:  vcpkg install gtest sdl2 libunibreak libjpeg-turbo[jpeg8] libzip cairo --triplet=x64-linux-dynamic 
  (Recommand you use dynamic triplet(static is not tested)
* 4.create buildings:./build.sh --product=x64 --build=debug
* 5.enter directory out${product}-${build} and make the cdroid project
  
 


