# 【可直发 / V2EX · 知乎 · 嵌入式社区】

> 下面是一份可直接复制发布的中文招募帖。建议发到 V2EX(`分享创造`或`程序员`节点)、知乎(想法或文章)、以及嵌入式/QT 相关的中文社区。发之前把截图/GIF 换成最新版,帖子末尾的链接确认有效。

---

## 标题候选

- 我把 Android 的 UI 框架逐行移植成了 C++,现在跑在 32M 内存的嵌入式设备上
- 不想再在嵌入式上重造 UI 轮子,我把 Android 的 View 体系整个搬过来了 —— 开源,求一起搬
- 一个能把 Android Studio 设计的 UI 直接跑进车机/机顶盒的 C++ 引擎(附学习路线)

## 正文

做嵌入式的同学大概都经历过这个循环:产品要个"好看点"的 UI → 自己撸一套 → 撸完发现动画卡、控件少、布局难写 → 回头看 Android 那套,真香,但 Android 跑不进来。

我选了第三条路:**别重造了,把 Android 真东西移植过来。**

这个项目叫 **CDroid** —— Android 的 `android.widget` / `android.view` / `android.text` / `android.graphics.drawable` / `android.animation` 一整套,逐行用 C++ 移植,跑在 **Cairo** 矢量图形上,目标是中高端嵌入式设备,**最低 32M 内存**就能跑起来。

**它不是"类 Android",是 Android 本身。** 类名、成员名、方法签名、控制流都紧跟 AOSP 源码。你能把 Android 参考代码和 C++ 实现并排打开,逐行对照。用 Android Studio 画好 XML 布局,直接喂给 CDroid 渲染,没有 JVM。

**目前已经搬完的:**

- 50+ 控件、20+ Drawable,API 与 Android 兼容;
- 完整的 AndroidX:RecyclerView、ConstraintLayout + MotionLayout、FragmentManager + Navigation、Flexbox、CoordinatorLayout、ViewPager2;
- Fragment + Transition 转场框架,含共享元素转场;
- 忠实的文本栈(spans / StaticLayout / DynamicLayout / minikin 断行)、`KeyCharacterMap` 原生级移植、输入法;
- 多窗口脏区合成器;DRM / fb / SDL / XCB / VNC 多后端。

**为什么我觉得你会想参与?**

- **想搞懂 Android 内部的人**:CDroid 是目前能找到的、最易读的 AOSP framework 镜像。View 的 measure/layout/draw、Choreographer、输入分发、文本布局——读干净的 C++ 比啃 AOSP 编译系统通透得多。
- **嵌入式老兵**:一套 API 兼容 Android 的 UI,直接用在你的 SoC 上。
- **C++ 玩家**:这里有真实的所有权/生命周期挑战(把一个 GC 语言移植到无 GC 的 C++,全是坑和学问),现代特性(动画/转场/Material),Cairo 渲染。

**怎么开始?**

我整理了一份[新手任务清单](docs/contributing/good-first-issues.cn.md)——每个任务都标好了:CDROID 里改哪个文件、对应的 AOSP 源码在哪、怎么验证。大多是"某方法签名有了但函数体是空的,照着 Android 原版翻译"这种 30 分钟到半天的小活,适合上手。还有[贡献指南](CONTRIBUTING.cn.md),从构建到提交一条龙。

- 仓库:https://gitee.com/houstudio/cdroid (镜像 https://github.com/houstudio/cdroid)
- QQ 群 / 联系:1225012331(微信 calfhou)

不夸张地说,这是个"用偏执的纪律把一件极难的事做到了不起的完成度"的活儿,现在缺的就是一起搬砖的人。欢迎来看看,哪怕只是跑个 sample 提个 bug。

---
