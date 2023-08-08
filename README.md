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
   * ListView (with both vertical and horizontal layout support)
   * GridView (with both vertical and horizontal layout support)
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
* 1.install dependencs: sudo apt install autoconf libtool build-essential cmake gdb pkg-config zip gettext libx11-dev bison python>=3.7
* 2.install vcpkg:<br>
&emsp;&emsp; git clone https://github.com/microsoft/vcpkg.git<br>
&emsp;&emsp; cd vcpkg<br>
&emsp;&emsp;./bootstrap-vcpkg.sh
* 3.install cdroid deplibs:(Recommand you use dynamic triplet(static is not tested)<br>
&emsp;&emsp;vcpkg install gtest sdl2 libunibreak libjpeg-turbo[jpeg8] libzip cairo --triplet=x64-linux-dynamic 
* 4.download cdroid source code:<br>
&emsp;&emsp;cd ~<br>
&emsp;&emsp;git clone http://www.gitee.com/houstudio/cdroid.git<br>
* 5.build cdroid:<br>
&emsp;&emsp;cd cdroid<br>
&emsp;&emsp;./build.sh --build=debug<br>
&emsp;&emsp;cd outX64-Debug<br>
&emsp;&emsp;make -j
* 6.prepare system and app resource<br>
&emsp;&emsp;cp src/gui/cdroid.pak ./<br>
&emsp;&emsp;cp apps/${app}/${app}/pak ./<br>
* 7.run samples(in directory outX64-Debug<br>
&emsp;&emsp;apps/samples/helloworld<br>
&emsp;&emsp;apps/uidemo1/uidemo1<br>


  
 


