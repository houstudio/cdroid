# 二进制 XML 移植:难度评估与路线(2026-07-15)

> 把 Android 的编译期二进制 XML(AXML,即 AAPT2 产出的 chunk 格式,`XmlBlock`/`ResXMLTree` 读的那个)移植到 CDROID 的可行性评估。**只分析,未实现。** 动机:让 `@string/@layout/@drawable/@id` 等引用在构建期固化成 int,运行期不再按字符串查表。

---

## 一句话结论

**整体中等难度、可控。** 本地有 C++ 金标准源(`ResourceTypes.{h,cpp}`)是最大利好。但要先拆清楚:

> **「二进制 XML(chunk 格式)」和「引用从 string 变 int」是两件正交的事。** 要的收益(引用变 int)不一定非要上二进制格式;而 Android AXML 把两者绑死,还顺带绑了一个对 CDROID 零收益的大坑(框架 attr-id 表)。

真正的难度不在「二进制格式能不能抄」,而在 **CDROID 目前压根没有「全资源 ID 表」**。

---

## 现状(三条关键事实)

来源:三个 recon agent 实测,文件行号均为当前 dev 分支。

1. **全程字符串键,零构建期数字解析。**
   - 解析器是 **expat**:`src/gui/core/xmlpullparser.cc:22` `#include <expat.h>`,`:199` `XML_Parse` 喂字节。
   - `XmlPullParser` IS-A `AttributeSet`(`xmlpullparser.h:25`),属性存 `shared_ptr<unordered_map<string,string>>`(`attributeset.h:37`)。
   - ~50 个 widget 全按属性名字符串读,如 `textview.cc:97/103/108/131`、`layoutparams.cc:11-12`。
   - `@id/@string/@drawable/@color` 运行期靠 `mIDS/mStrings/mColors` 字符串哈希懒查:`assets.cc:441-454`(getId)、`:460-474`(getString)、`:623-651`(getColor)。pak 加载时从 `values/*.xml` 填表(`assets.cc:135-146`)。

2. **idgen 只给 `@id/@+id` 分配了 int。**
   - `scripts/idgen.py` 扫描 `layout/drawable/navigation/values/menu`,只收 `@id/@+id`,产 `R.h`(只有 `id` 命名空间,`widget/R.h` 258 行) + `values/ID.xml`。
   - 布局 XML 里写的还是字面 `"@id/foo"`,**没固化成数字**;运行期 `mIDS.find("pkg:id/foo")`。
   - `@string/@drawable/@layout` **连数字 ID 都没有**,运行期纯按名字。

3. **没有任何 attr-id 表。**
   - `src/gui/res/values/attrs.xml`(10725 行)是原样抄的 Android,**不带数字 ID、运行期从不解析**,仅作 schema/文档。
   - `R.h` 无 `attr`/`styleable`;`R.styleable.*` 全是 `#if 0`/注释死代码(`timepickerclockdelegate.cc:1`、`animatorinflater.cc:578-584`)。
   - 属性名 → 框架 attr ID(`layout_width`→`0x010100f4`)这条路 **在 CDROID 没有消费者**。

zip 层(`ZIPArchive`/`Assets::getInputStream`,`assets.cc:401-414`)格式无关,**不用动**。

---

## 三方案对比

| | A. 全量 Android AXML(字节兼容) | B. 轻量二进制 XML | C. 只做引用 int 化(**推荐起步**) |
|---|---|---|---|
| 编码器 | 完整 chunk + resource-map + Res_value | 精简 chunk,属性名按字符串索引,值引用固化为 int | 不改格式,XML 仍文本 |
| 运行期 reader | 移植 `ResXMLParser` | 精简 chunk parser,边界 int↔string 翻译 | 不动 `XmlPullParser` |
| 框架 attr-id 表 | **必须建**且没人读 → 纯负担 | 不需要 | 不需要 |
| `AttributeSet`/widget | 改 int 键 + ~50 widget 全改 | 基本不动 | 加 by-int 快路,字符串路径并存 |
| 难度 | **极高** | **中偏上** | **中** |
| 「引用变 int」 | ✅ | ✅ | ✅ |
| 体积/解析速度 | ✅ | ✅ | ❌ |
| 字节兼容外部 APK | ✅(CDROID 不需要) | ❌ | ❌ |

**A 不做**:attr-id 列产出了没人读,大半工作量白费。

---

## 真正的前置工作(无论 B 还是 C 都躲不开)

「引用变 int」的前提是 **全资源 ID 表** —— 给 `string/drawable/layout/color/id` 全部分配**稳定** int ID。CDROID 目前完全没有这块基建:

- **构建期**:扩展 `idgen.py` 或放进 `pakbuilder.py`,遍历 `res/` 全部资源,确定性排序后分配 ID(`0xPPTTEEEE` 或更简方案),产 `资源名→int` 表。难点是 **ID 跨构建必须稳定**(否则外部 `R.h` 式引用失效)。
- **运行期**:`Assets` 加 **by-int 查询**层,与现有 by-name 并存,渐进迁移。
- **边界 case**:
  - `?attr/xxx`(主题引用,运行期对着 theme 解析,**构建期无法固化成 int**) → 必须保留运行期解析路径,所以「全部引用变 int」做不到 100%,约 95%。
  - `<include layout=...>`、style 继承链 —— 引用构建期可解析。
  - `@+id`(首次定义)。

---

## 推荐分阶段路线(每步独立可回退)

1. **第一步(C):全资源 ID 表 + 运行期 by-int 查表层。**
   XML 仍是文本,`@id/@string/@drawable/@layout` 在打包期解析成 int 写入(旁路表或 token),运行期优先 int。**这一步就拿到核心收益**,不碰解析器,风险最低。

2. **第二步(可选 B):把文本换成轻量 chunk 格式。**
   复用本地 `ResourceTypes.h` 结构体(string pool + START/END element + attribute,值带 int 引用),写精简 reader 挂在 `XmlPullParser` 边界(检测 magic 分流),下游全不动。拿体积 + 解析速度收益。

3. **永远不做 A 的 attr-id 表。**

---

## 卡点 / 风险

- **ID 稳定性**(头号风险):必须确定性分配,增删资源不能让既有 ID 漂移。
- **`?attr` 主题引用**:构建期固化不了,保留运行期解析。
- **二进制 XML 不可读**:要留文本回退或写 dump 工具,否则调试倒退。
- `TypedValue`(`src/gui/core/typedvalue.{h,cc}`)已有 dimension/fraction 解码,但 `Res_value` 其余 `TYPE_*`(reference/int/color/string)+ string-pool 间接 + resource-ID map 要补。
- reader 必须在 `XmlPullParser` 边界把 int 引用**翻译回 string**喂现有 `AttributeSet`,否则要改 ~50 widget —— 前者风险小得多。

---

## 可复用资产(现成)

**C++ 金标准(可直接抄结构体):**
- `/home/git/android_12.0_mid_rkr13/frameworks/base/libs/androidfw/include/androidfw/ResourceTypes.h`(2224 行):`ResChunk_header`(213)、chunk 类型枚举(`RES_XML_TYPE=0x0003`/`RES_XML_START_ELEMENT_TYPE=0x0102`/`RES_XML_RESOURCE_MAP_TYPE=0x0180`,234-248)、`Res_value`(282)、`ResStringPool_header`(456)、`ResXMLTree_node`(611)、`ResXMLTree_attrExt`(667)、`ResXMLTree_attribute`(697)。
- 同目录 `ResourceTypes.cpp`(7810 行):`ResXMLTree::setTo`/`ResXMLParser::next/getAttribute*` 完整实现。
- 同目录 `Chunk.h`/`AttributeResolution.h`/`LoadedArsc.h`:更现代的 chunk 读取助手。
- 镜像:`/home/houzh/research/aapt2/src/base/libs/androidfw/...`(同内容)。
- ⚠️ `/home/huangzj/android/` 树 **沙箱不可读**(ENOENT),别用。

**Java 可读参考:**
- SDK 源:`/opt/android-sdk/sources/android-3{4,5,6}/android/content/res/XmlBlock.java`(JNI 壳,真逻辑在 native)、`android/util/AttributeSet.java`。
- Robolectric 纯 Java:`/home/git/android_12.0_mid_rkr13/external/robolectric-shadows/resources/src/main/java/org/robolectric/res/android/ResXMLTree.java` + `ResXMLParser.java`(高层逻辑更易读)。

**CDROID 自有:** `TypedValue`(`typedvalue.{h,cc}`,复用)、`idgen.py`(扩展起点)、`XmlPullParser`/`AttributeSet`(挂接点)。

**不能直接用:** 系统 `/usr/bin/aapt`、`aapt2` —— 要对着 framework arsc/android.jar 解析,CDROID 不是那套结构,改造输出 ≈ 自己写最小编码器。

---

## 决定

先做 C(全资源 ID 表 + 引用 int 化),要不要上 B(二进制 chunk)看后续是否需要体积/速度收益;不碰 A。展开 C 的落地设计待用户确认后另起。
