# App 包与数据目录布局规划(2026-09-04)

> 把 CDROID 的应用包位置、应用数据位置对齐 AOSP 目录模型的规划。**只规划,未实施。**
> 起因:prefs 存储乱象(`~/.cdroid/prefs/DeskClock.xml` 平铺 vs preferencedemo 生成
> `~/.cdroid/prefs/home/houzh/…/preferencedemo/` 嵌套路径树)——定性为**核心问题**,
> 病灶是 `App::getPackageName()`(app.cc:937)返回 exe 路径拼法,而非 manifest 包名。

---

## 一、AOSP 依据

### 1. 环境变量三元组(Runtime 契约)

| env | 默认 | 语义 |
|---|---|---|
| `$ANDROID_ROOT` | `/system` | Runtime 寻找核心系统库(libandroid_runtime.so、libart.so)与系统框架(framework.jar)的根 |
| `$ANDROID_DATA` | `/data` | Runtime 运行时数据、AOT 缓存、**全部应用沙盒数据**(/data/data/)的根 |
| `$ANDROID_STORAGE` | `/storage` | 解析外部存储挂载点的基础路径 |

CDROID 的 `android.os.Environment` 移植(`src/gui/core/environment.cc`)**已合**:
`getRootDirectory()/getDataDirectory()/getStorageDirectory()` 均为
`getDirectory(env, default)` 形态,另有 ANDROID_EXPAND/DOWNLOAD_CACHE/OEM/ODM/VENDOR/PRODUCT 全套。

### 2. Context 数据面 API(全部 public abstract,android-36 Context.java 行号)

| 方法 | 行号 | 落点 |
|---|---|---|
| `getDataDir()` | :1437 | `/data/data/<pkg>`(API 24+)|
| `getFilesDir()` | :1454 | `<dataDir>/files` |
| `getCacheDir()` | :1805 | `<dataDir>/cache` |
| `getDir(String, int)` | :2040 | `<dataDir>/<name>` |
| `getExternalFilesDir(@Nullable String)` | :1589 | `<storage>/…/Android/data/<pkg>/files[/<type>]` |
| `getExternalCacheDir()` | :1883 | `…/Android/data/<pkg>/cache` |
| `getSharedPreferencesPath(String)` | :1420 | `<dataDir>/shared_prefs/<name>.xml`(AOSP 本尊的 prefs 路径构造器)|
| `getFileStreamPath(String)` | :1404 | `<filesDir>/<name>`(openFileOutput 配套)|

CDROID 的 Context 层这些接口**零存量**(context.h 无一声明)。

---

## 二、现状盘点(已就位的底座)

| 件 | 状态 |
|---|---|
| pm/am 安装链(2c64b5e36) | **包层已 AOSP 形态**:`$CDROID_DATA_DIR(默认 /data)/app/<pkg>-<serial>/{<exe>,<exe>.pak}` + `/system/packages.xml` |
| Environment 移植 | 全量在(core/environment.cc):三元组 + per-app 构建器 `getDataUserCePackageDirectory(vol,uid,pkg)`(:135,=`/data/user/<uid>/<pkg>`,即 `/data/data/<pkg>` 规范形)、`buildExternalStorageApp{Data,Files,Cache}Dirs(pkg)`(:291-314)|
| findSharedPak 探测链 | exe 旁 → `$CDROID_PAK_PATH` → /usr/share/cdroid → /opt/cdroid |
| manifest 包名 | `parsePackageManifest`(app.cc:524)启动即解析;pakbuilder 侧 package(`cdroid.<ns>`)现成——**只差接到 getPackageName()** |
| SharedPreferences | 平铺约定:`~/.cdroid/prefs/<name>.xml`(sharedpreferences.cc:844,"无沙箱"设计注释)|

**已知 UB 挂账**:`UserEnvironment::getExternalDirs()` 返回空 vector(StorageManager 未移植)
→ `getExternalStorageDirectory()` 取 `[0]` 越界解引用,谁调谁崩。

---

## 三、目标布局(三层)

### A. 系统层(`/system`,只读随固件)——框架自身

| AOSP 锚点 | CDROID 目标 | 决策点 |
|---|---|---|
| `framework/framework-res.apk` | `framework/cdroid.pak`(+`widgetex.pak`) | **D1**:是否升格;探测链兼容旧位 |
| `etc/fonts.xml` + `fonts/*.ttf` | 同位 | 现在 exe 旁/私有字体树 |
| `packages.xml` | `/system/packages.xml` | ✓ 已落地 |

### B. 应用包层(`/data/app`,pm 域)

| AOSP | CDROID | 状态 |
|---|---|---|
| `/data/app/<pkg>-<serial>/base.apk + lib/` | `/data/app/<pkg>-<serial>/{exe,exe.pak}` | ✓ 已落地 |
| `/system/app/<pkg>/`(预装) | 规范位;`/opt/<name>` 保留为兼容 | **D2** |

### C. 应用数据层(`/data/data/<pkg>`)

| 子目录 | 构建方式 |
|---|---|
| `files/` `cache/` `databases/` | `getDataUserCePackageDirectory(vol, uid, pkg)` + 后缀 |
| `shared_prefs/<n>.xml` | `Context::getSharedPreferencesPath(name)`(AOSP :1420)|
| external:`<storage>/…/Android/data/<pkg>/{files,cache}` + `obb` | `buildExternalStorageApp*Dirs(pkg)` |

---

## 四、host(开发机)态映射

`ANDROID_DATA` 未设且 `/data` 不可写时:`/data ≡ ~/.cdroid`

- 应用数据:`~/.cdroid/data/<pkg>/{files,cache,shared_prefs}/…`
- 存量迁移:`~/.cdroid/prefs/<n>.xml` → `~/.cdroid/data/<pkg>/shared_prefs/<n>.xml`(带读回退)
- 现存 exe 路径垃圾树(绝对/相对两拼法)随 P1 包名修复自然作废,清盘

---

## 五、实施分期

- **P1 包名地基**:`App::getPackageName()` → manifest 包名。B/C 两层的钥匙;
  下游(PreferenceManager 的 `pkg + "_preferences"` 等)全 AOSP 逐字,无需改。
- **P2 ContextImpl 数据面**:补 `getDataDir/getFilesDir/getCacheDir/getDir/
  getExternalFilesDir/getExternalCacheDir/getFileStreamPath/getSharedPreferencesPath`
  八接口,根走 Environment + host 兜底链(env → /data 可写? → ~/.cdroid)。
- **P3 SharedPreferences 接入**:`SharedPreferencesImpl` 改吃
  `getSharedPreferencesPath()` 的全路径(AOSP 本形状),平铺 dir 约定退役 + 存量迁移。
- **P4 系统层升格 + external**:fonts/框架 pak 迁 `/system`(产品态);
  `getExternalDirs()` 兜底默认卷 `/storage/emulated/<uid>`,消 [0] 越界 UB。

---

## 六、待拍板决策点

| # | 问题 | 建议 |
|---|---|---|
| D1 | 共享 cdroid.pak 是否升格 `/system/framework` | 升;findSharedPak 探测链兼容旧位零破坏 |
| D2 | 预装 app 规范位 | `/system/app/<pkg>/`;`/opt/<name>` 保留兼容 |
| D3 | host 兜底根 | 维持 `~/.cdroid`(延续现状)vs XDG `~/.local/share/cdroid` |
| D4 | `$CDROID_DATA_DIR`(pm 链自造)与 `ANDROID_DATA`(AOSP 契约)关系 | ANDROID_DATA 为正,CDROID_DATA_DIR 作别名过渡 |

---

## 附:证据链(prefs 乱象定性)

```
App::getPackageName() (app.cc:937) → 返回 mName = exe 路径拼法   ← 病灶
    ↓ launch 方式不同 → 绝对/相对两种拼法 → 磁盘两棵垃圾树实证
PreferenceManager::getDefaultSharedPreferencesName = pkg + "_preferences"  ← AOSP 逐字
deskclock 传 "DeskClock" 字面                                               ← AOSP 逐字
SharedPreferencesImpl → prefsDir + "/" + name + ".xml"                      ← 平铺约定
```

读 `ANDROID_*` env 是 AOSP 契约本身(Environment.java 惯例),不属于
"CDROID 自造环境变量入口",与"核心只认 CLI 开关"规范不冲突。
