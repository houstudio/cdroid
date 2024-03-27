# **Introduction**
**CDroid** 是一套基于C++11开发的高仿安卓的跨平台GUI引擎，设计目标针对中高端嵌入式设备.旨在为中高端设备提供媲美安卓的UI设计效果。同时也让安卓开发者能轻松切入到嵌入式开发领域。
* 运行环境：32M以上内存的嵌入式系统。
* 主仓库: [https://gitee.com/houstudio/cdroid](https://gitee.com/houstudio/cdroid)
* 镜像仓库: [https://github.com/houstudio/cdroid](https://github.com/houstudio/cdroid)
# **Features:**
* Multi Layer/Multi Window 
* 集成40+ 安卓兼容的UI组件和20+ Drawables
* 所有UI组件(Layouts,Drawables...)可以在安卓兼容的开发环境(**AndroidStudio or Eclipse**)中进行设计 
* 基于 **Cairo**的矢量图形引擎
# **Quick Start**
* X86快速体验模拟器地址： https://gitee.com/jiangcheng/cdroidX64
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
虚拟机体验：
链接：https://pan.baidu.com/s/1-v-rLcHxo5W5TXvJ2NUWxA 提取码：spux （虚拟机登录用户:cdroid 密码:123456）
登录自会后请重新pull cdroid更新代码
每次更新代码建议干掉cdroid/outXXXX 然后重走 build.sh构建流程。

  
 


