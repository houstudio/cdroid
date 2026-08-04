# 参与贡献 CDroid

首先:感谢你愿意为 CDroid 出力。这是一个庞大的移植工程,永远有更多 AOSP 代码等着翻译 —— 你移植的每一个控件、Drawable、方法,都在补上一个真实的缺口。本指南帮你尽快从 `git clone` 走到一个被合入的改动。

English: [CONTRIBUTING.md](CONTRIBUTING.md).

## 可以怎么贡献

- **补一个 stub。** 很多方法签名已经保真,但函数体是空的或直接透传。找一个,读 AOSP 原版,把逻辑翻译过来。(见[新手任务清单](docs/contributing/good-first-issues.cn.md)。)
- **补一个缺失的 sample。** 每个已移植的控件都该在 `apps/samples/` 下有个能跑的 demo。
- **修一个 bug**——你在跑 sample 时撞到的。
- **为你的硬件加一个平台后端**,放在 `src/porting/<芯片>/`。
- **改进文档。**

## 唯一的准则:保真

CDroid 是**忠实移植,不是再创作**。拿不准时:

1. **先读 Android 原版。** 参考源码在 `/opt/android-sdk/sources/android-36/`(主)和 `android-34/`。你自己机器上,把编辑器指到 AOSP 的 `frameworks/base` 或 SDK 的 `sources/`。
2. **对齐名字和签名。** 类名、成员名、方法名都紧跟 Java/Kotlin 原版。
3. **逻辑逐行翻译,不要重新设计。** Android 分支你就分支,Android 调一个 helper 你就调(或移植)那个 helper。
4. **差异必须有理由**,而且理由是 C++ 或平台带来的(比如:无 GC → 显式所有权;无 `Bitmap` → `Cairo::ImageSurface`;内部用 `char16_t`/`u16string`,边界才用 UTF-8)。差异微妙时,在注释里说明。

详见 [AGENTS.md](AGENTS.md)——它是移植原则的权威。

## 第一个贡献,一步步来

1. **认领任务**:从[新手任务清单](docs/contributing/good-first-issues.cn.md)里挑一个。每个任务都标好了 CDROID 文件、AOSP 参考、验证方法。
2. **构建基线**(见下),确保你有一棵能编的树。
3. **打开任务里标注的 AOSP 源码**,读你要移植的那个方法。
4. **移植它。** 照搬邻近文件的兄弟写法(比如新写一个 `KeyListener`,就照 `DialerKeyListener`)。
5. **编绿。** 标准是 `make` 干净通过,不是 IDE 诊断(见下方「忽略 clangd」)。
6. **跑一下 sample**(如果有的话),肉眼看看效果。
7. **提 PR**(Gitee 或 GitHub)。从 `dev` 分支切出来,改动聚焦一件事。

## 构建快速通道

```bash
# 一次性配置(生成 outX64-Debug/)
bash build.sh -p x64 -b Debug

# 增量编译——日常用。务必 -j44
cd outX64-Debug
make cdroid -j44

# 单个 sample 是独立目标(源码:apps/samples/<name>.cc)
make spinner -j44
./spinner        # 在 outX64-Debug/ 下运行
```

能帮你省时间的注意事项:

- **务必 `-j44`。** 单线程 `make cdroid` 可能超时。
- **改了 `CMakeLists.txt` 后,`make` 会自动重新配置 CMake。**
- **`src/gui/text/CMakeLists.txt` 是显式源码清单(非 glob)** —— 新增 `text/method/*.cc` 必须手动加进去。
- **新增控件只需在头文件里 `DECLARE_WIDGET(T)`** —— 它在静态初始化期自注册到 inflater,不需要中央注册文件。
- **忽略 clangd 报错**,比如 `cairo.h: not found` / `unknown type`。clangd 没有 `src/gui` 的 include 根,信真实的 `g++` 构建。

## 代码库地图

| 模块 | 位置 | Android 对应 |
|---|---|---|
| 控件 | `src/gui/widget/` | `android.widget.*` |
| View 体系 | `src/gui/view/` | `android.view.*` |
| 文本 | `src/gui/text/` | `android.text.*` |
| Drawable | `src/gui/drawable/` | `android.graphics.drawable.*` |
| 动画 | `src/gui/animation/` | `android.animation.*` + 旧版 `view.animation` |
| AndroidX 移植 | `src/gui/widgetEx/` | `androidx.*`(RecyclerView、ConstraintLayout……) |
| 核心运行时 | `src/gui/core/` | `App`、`Looper`、`Canvas`、`GraphDevice` |
| 平台后端 | `src/porting/<芯片>/` | DRM/fb/SDL/XCB/VNC 胶水 |
| 示例 | `apps/samples/` | 单文件可跑 demo |

## 那些会咬人的约定

下面这些踩过真实 bug——读一遍,记一辈子:

- **`Canvas` *就是* `cairo_t`。** 没有独立的渲染抽象,也没有 `Bitmap` 类(由 `Cairo::ImageSurface` 承担)。`onDraw(Canvas&)` 直接操作 cairo。
- **文本内部是 `char16_t`/`u16string`**,只有 I/O 边界用 UTF-8。用 `TextUtils::utf8_utf16` / `utf16_utf8` 转换。
- **Span 所有权是手动的**(`owned` 标志,由 `NoCopySpan` 把关)。`TextWatcher`/`SpanWatcher` 必须 `virtual public NoCopySpan`。span 层不用智能指针。
- **`setText(CharSequence*)` 转移所有权;** 文本必须比它的 `Layout` 活得久。
- **这里没有 C++14 的 `std::optional`** —— Android 的 `null` 用枚举哨兵表示(如 `TruncateAt::NONE`、`Alignment::NONE = -1`),这是设计如此。
- **只读方法加 `const`**(getter、`is*`、`has*`);返回可变指针/引用的方法保持 non-const。

## 提交与 PR 约定

- 从 `dev` 切分支。一个 PR 只做一件事。
- 提交前缀沿用历史:`:construction:` 表示移植/WIP,`:bug:` 表示修 bug,例如 `:construction: 移植 Spinner.setOnItemClickListener`。
- 提交信息末尾加上:
  ```
  Co-Authored-By: <你的名字> <邮箱>
  ```
- PR 描述里写清你移植的 AOSP 类/方法;改动可见的话附张截图。

## 寻求帮助

- 在 [Gitee](https://gitee.com/houstudio/cdroid) 或 [GitHub](https://github.com/houstudio/cdroid) 提 issue。
- QQ:1225012331 / 微信:calfhou。

移植愉快!🚧
