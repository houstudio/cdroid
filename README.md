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

|  View         |     TextView      |  Button            |  ImageView    |  ImageButton  |
|---------------|-------------------|--------------------|---------------|---------------|
|CompoundButton |    ToggleButton   |     CheckBox       | RadioButton   |  ProgressBar  |
|   SeekBar     |   Chronometer     |    AnalogClock     |   ViewGroup   |  RadioGroup   |
|  ScrollView   |    ViewPager      |  SimpleMonthView   |    Switch     |   RatingBar   |
| NumberPicker  |     ListView      |     GridView       | RecyclerView  |  ViewFlipper  |
| ViewAnimator  |AdapterViewAnimator|    Calendarview    |SimpleMonthView| Chronometer   |
|  ScrollView   | NestedScrollView  |HorizontalScrollView| DateTimeView  |   ViewPager2  |
|YearPickerView |                   |WearableRecyclerView|               |               |

# **Supported Layouts:**
| FrameLayout |  LinearLayout  | RelativeLayout |     TableRow     |
|-------------|----------------|----------------|------------------|
| TableLayout | AbsoluteLayout |  GridLayout    |GestureOverlayView|

# **Supported Drawables:**
|   ColorDrawable   |  BitmapDrawable  |    NinepatchDrawable    |  InsetDrawable  |
|-------------------|------------------|-------------------------|-----------------|
|ShapeDrawable      |TransitionDrawable|  AnimatedVectorDrawable |StateListDrawable|
| LevelListDrawable |   ClipDrawable   |  AnimatedRotateDrawable | RotateDrawable  |
|GradientDrawable   |  ScaleDrawable   |  AnimatedImageDrawable  | VectorDrawable  |
|  RippleDrawable   |AnimationDrawable |AnimatedStateListDrawable|  LayerDrawable  |

# **Porting guide:**

* 1 A new product porting should be placed to src/porting/xxx(where xxx is you chipset name)
* 2 implement your porting api to xxx directory
* 3 modify build.sh to support your port(you should configure sysroot toolchain...).
* 4 call build.sh --product=xxx
* 5 make you project(SeeAlso **Building CDROID**) 

# **Building CDROID:**
### 1.install dependencs:
 sudo apt install autoconf libtool build-essential cmake gdb pkg-config zip gettext libx11-dev bison python>=3.7 pip3-python python3-lxml meson
### 2.install vcpkg:
* git clone https://gitee.com/houstudio/vcpkg.git
* cd vcpkg
* ./bootstrap-vcpkg.sh
### 3. install cdroid supported toolchain's patch
* cp cdroid/script/vcpkgpatch4cdroid.patch vcpkg/
* cd vcpkg
* patch -p1< vcpkgpatch4cdroid.patch
### 4.install cdroid deplibs:
* ./cdroid_install_libs.sh --triplet=x64-linux-dynamic<br>
### 5.download cdroid source code:
* cd ~
* git clone http://www.gitee.com/houstudio/cdroid.git<br>
### 6.build cdroid:
* cd cdroid
* ./build.sh --build=debug
* cd outX64-Debug
* make -j
### 7.prepare system and app resource
*The cdroid.pak and yourapp's pak must be in your working directory*
* cp src/gui/cdroid.pak ./
* cp apps/appname/appname.pak ./
### 8.run samples(in directory outX64-Debug)
* apps/samples/helloworld
* apps/uidemo1/uidemo1


