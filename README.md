# **Introduction**
**CDroid** is a android like GUI toolkit base on C++11,It is a cross-platform GUI engine for embedded system.

# **Features:**
* Multi Layer/Multi Window 
* Integrated with 30+ UI Components and 15+ Drawables(compatibal with android)  
* Full compatible with Android
* Smooth Scrolling
* Scrolling with SpringBack Support
# **Screen Shots**
![ListView](https://gitee.com/houstudio/cdroid/raw/master/docs/videos/listview_springback.gif)
![ViewPager](https://gitee.com/houstudio/cdroid/raw/master/docs/videos/viewpagerimg.gif)
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

* 1, A new porting must be placed to src/porting/xxx(where xxx is you chipset name)
* 2, implement your porting api to xxx directory
* 3, enter source root directory 
* 4, make a dir named outxxx,enter outxxx)
* 5, cmake -DCMAKE_TOOLCHAN_FILE="your cmake toolchain path" \
        -DCDROID_CHIPSET=xxx \
        ..
* 7,after step 4,makefile is created in directory outxxx where you can type make to build your program.
* 8,you can export USE_RFB_GRAPH=ON before cmake is executed to to use remote framebuffer for test.

# **X86 Emulator:**
* 1.Ubuntu 18(64bit)Desktop
* 2.install dependencs: sudo apt install build-essential cmake gdb pkg-config zip gettext libx11-dev
* 3.Window'user can use MobaXterm's x11 display(or VNCViewer)
 


