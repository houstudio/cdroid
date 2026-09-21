# CDROID 应用目录指南(程序 / 资源 / 数据)

> 通俗版:一个 CDROID 应用跑起来,**程序放哪、资源去哪找、数据往哪写**。
> 本文描述现状(2026-09-18,dev 分支),按"三种跑法"讲;规划沿革见
> [app-data-directory-layout-plan.md](app-data-directory-layout-plan.md)。

## 一句话总览

**程序和 pak 是"只读"的**——放哪都能跑,框架有一套自动寻找顺序;
**数据是"可写"的**——按包名一人一个抽屉,跟着"数据根"走,根在哪由环境决定。

---

## 1. 一个应用由哪些文件组成

```
myapp              可执行程序
myapp.pak          应用自己的资源(布局 XML/图片/字符串,构建期由 pakbuilder 打包)
cdroid.pak         框架公共资源(所有应用共用,主题/默认样式/内置图片)
widgetex.pak       androidx 控件的公共资源(共用)
fonts.xml          字体表(可选;没有的话框架会去几个固定位置找)
```

前两个属于应用,后三个属于框架。**应用只需要管好自己的 exe + 自己的 pak**,
共享 pak 由部署环境提供。

## 2. 程序和 pak 怎么被找到(三种跑法)

### 跑法一:开发机(构建树)——什么都不用做

```
outX64-Debug/
├── cdroid.pak          ← 共享 pak 生成在这里
├── widgetex.pak
├── apps/
│   └── samples/
│       └── buttons     ← 可执行文件在这里
```

框架从**可执行文件所在目录往上走**(最多 3 级)找 `cdroid.pak`——
从 `apps/samples/buttons` 往上两级正好到 out 根,直接命中。
所以开发机上 `make buttons && apps/samples/buttons` 就能跑,**不用拷贝任何文件**。

### 跑法二:设备安装(pm install 布局)

```
/data/app/cdroid.<包名>/
├── myapp
└── myapp.pak           ← 应用自己的 pak 与 exe 同目录,天然命中
```

共享 pak 放系统位置(见下面探测顺序 ④⑤),一次部署全体应用受益。

### 跑法三:手工部署(任意目录)

把 exe 和自己的 pak 放同一目录即可;共享 pak 按下面顺序放一个地方。
目录不凑手时,用 `--data=<目录>` 显式指定"exe 旁目录"。

### 共享 pak(cdroid.pak / widgetex.pak)的寻找顺序

| 顺序 | 位置 | 说明 |
|---|---|---|
| ① | `--data` 指定的目录(默认 exe 所在目录) | |
| ② | exe 所在目录的上级,最多向上 3 级 | 构建树靠它 |
| ③ | `$CDROID_PAK_PATH` 里的目录(冒号分隔可多个) | 部署自定义位 |
| ④ | `/usr/share/cdroid/` | 设备/系统安装态主位置 |
| ⑤ | `/opt/cdroid/` | 备选 |

**刻意不探测当前工作目录(cwd)**:从哪个目录启动进程不应决定用哪个 pak。
代码:`App::onInit` 的 `findSharedPak`(app.cc:341)。
应用自己的 pak 同理只看一处:exe 同目录的 `<应用名>.pak`(app.cc:187),
不命中就报错,不再有 cwd 兜底。

## 3. 字体从哪来

CDROID **不再依赖 fontconfig**:字体表来自 `fonts.xml`(Android 格式),
按以下顺序找(typeface.cc:723):

1. 代码里显式设置的路径(`Typeface::setFontConfigXml`),或环境变量 `CDROID_FONTS_XML`
2. 从 exe 目录向上逐级找 `fonts.xml`(构建树里由 `build.sh` 调
   `scripts/genfontsxml.sh` 生成——把宿主机的字体清单快照成这份文件)
3. `/usr/share/cdroid/fonts.xml`、`/opt/cdroid/fonts.xml`(与 pak 探测对齐的系统位)
4. `/system/etc/font_fallback.xml`、`/system/etc/fonts.xml`
5. `/etc/fonts/fonts.xml`

一个都没找到 = 没有系统字体,启动日志会明确警告并提示跑 build.sh。

## 4. 数据写到哪(重点)

### 数据根:三步决策

1. 设了环境变量 `ANDROID_DATA` → 数据根就是它的值;
2. 没设,但 `/data` 可写(设备上通常如此)→ 数据根 = `/data`;
3. 否则(典型:开发机无 root)→ 数据根 = `~/.cdroid`。

代码:`appDataRoot()`(context.cc:184)。

### 每个应用一个抽屉

```
<数据根>/data/<包名>/
├── files/            应用普通文件(Context.getFilesDir)
├── cache/            可随时清掉的缓存(Context.getCacheDir)
├── shared_prefs/     偏好设置 XML,一个名字一个文件
└── app_<名字>/       应用私有杂项目录(Context.getDir)
```

**包名**来自应用 manifest(`cdroid.deskclock` 这种);没有 manifest 的合成 pak
退化为 exe 文件名。目录第一次访问时自动创建,不用预建。

### 真实例子(本开发机)

remusic 记住"最近播放"的那个文件,实际在:

```
~/.cdroid/data/cdroid.remusic/shared_prefs/recenthistory.xml
```

deskclock 的闹钟设置在 `~/.cdroid/data/cdroid.deskclock/shared_prefs/DeskClock.xml`。
同一套代码跑到设备上,只是把 `~/.cdroid` 换成 `/data`,其余不变。

外置存储一句话:设了 `EXTERNAL_STORAGE` 环境变量时,
`getExternalFilesDir()` 走 `<storage>/Android/data/<包名>/files`(未设则返回空)。

## 5. 环境变量 / 参数速查

| 名字 | 谁用 | 现状 |
|---|---|---|
| `ANDROID_DATA` | 数据根 | 有效 |
| `EXTERNAL_STORAGE` | 外置存储根 | 有效 |
| `--data=<目录>` | exe 旁资源目录 | 有效 |
| `CDROID_PAK_PATH` | 共享 pak 额外搜索目录(冒号分隔) | 有效 |
| `CDROID_FONTS_XML` | 显式指定 fonts.xml | 有效 |
| `CDROID_OVERLAY` | 共享 pak 的构建期资源 overlay | 构建期(pakbuilder) |

## 6. 已知不一致(待办)

1. **`App::getDataPath()` 名不副实**:它返回的是"exe 旁资源目录",与 AOSP 的
   dataDir 无关(已在 app.h 加注释说明)。更名(如 `getResourceDir`)留作后续,
   涉及存量调用方,单独做。

> i18n.dat 的 cwd 兜底(`./i18n.dat`)是刻意保留的开发态逃生口:正常路径下
> i18n.dat 已打包进 cdroid.pak,pak 整体缺失时应用本就没有框架资源可用,
> 不值得为兜底再建探测链。

## 附:代码索引

| 主题 | 位置 |
|---|---|
| 共享 pak 探测 | src/gui/core/app.cc:341(`findSharedPak`) |
| 应用自身 pak | src/gui/core/app.cc:187 |
| `--data` / exe 旁目录 | src/gui/core/app.cc:390(`getDataPath`) |
| 字体探测 | src/gui/core/typeface.cc:723 |
| i18n.dat | src/gui/core/app.cc:373 |
| 数据根回退 | src/gui/core/context.cc:184(`appDataRoot`) |
| 每应用目录形状 | src/gui/core/context.cc:208-255 |
| 包名 | src/gui/core/app.cc:948(`getPackageName`) |
| 环境变量/系统目录 | src/gui/core/environment.cc |
