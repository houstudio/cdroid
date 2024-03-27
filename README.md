# **Introduction**
**CDroid** is a android like GUI toolkit base on C++11,It is a cross-platform GUI engine for embedded system.
* Main repository: [https://gitee.com/houstudio/cdroid](https://gitee.com/houstudio/cdroid)
* Mirrored repository: [https://github.com/houstudio/cdroid](https://github.com/houstudio/cdroid)
# **Features:**
* Multi Layer/Multi Window 
* Integrated with 40+ UI Components and 20+ Drawables(compatible with android)
* All UI Components(Layouts,Drawables...)can be designed in **AndroidStudio or Eclipse** 
* Vector Graph supported via **Cairo**
# **Quick Start**
* Start experimenting now with X86 Simulator https://gitee.com/jiangcheng/cdroidX64
# **IDE(AndroidStudio/Eclipse)** 
![IDE](https://gitee.com/jiangcheng/cdroidX64/raw/master/apps/images/asd61236_ide.png)

# **ScreenShots**
![demo0](https://gitee.com/jiangcheng/cdroidX64/raw/master/apps/images/asd61236.gif)
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
   * RecyclerView
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

* 1 A new product porting should be placed to src/porting/xxx(where xxx is you chipset name)
* 2 implement your porting api to xxx directory
* 3 modify build.sh to support your port(you should configure sysroot toolchain...).
* 4 call build.sh --product=xxx
* 5 make you project(SeeAlso **Building CDROID**) 

# **Building CDROID:**
###  1.install dependencs:
 sudo apt install autoconf libtool build-essential cmake gdb pkg-config zip gettext libx11-dev bison python>=3.7 pip3-python meson
###  2.install vcpkg:
* git clone https://gitee.com/houstudio/vcpkg.git
* cd vcpkg
* ./bootstrap-vcpkg.sh
### 3.install cdroid deplibs:
* ./cdroid_install_libs.sh --triplet=x64-linux-dynamic<br>
### 4.download cdroid source code:
* cd ~
* git clone http://www.gitee.com/houstudio/cdroid.git<br>
### 5.build cdroid:
* cd cdroid
* ./build.sh --build=debug
* cd outX64-Debug
* make -j
### 6.prepare system and app resource
*The cdroid.pak and yourapp's pak must be in your working directory*
* cp src/gui/cdroid.pak ./
* cp apps/appname/appname.pak ./
### 7.run samples(in directory outX64-Debug
* apps/samples/helloworld
* apps/uidemo1/uidemo1
### 8.QuickStart
* VM(Ware):https://pan.baidu.com/s/1-v-rLcHxo5W5TXvJ2NUWxA fetchcode：spux (VM Ubuntu User:cdroid password:123456）
* After login pls run git pull to get new version of cdroid
* You'd better remove outXXX and run build.sh to rebuild makefiles after each git pull.

  
 


