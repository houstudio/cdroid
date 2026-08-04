# [Ready-to-post / Hacker News · Reddit r/programming · r/embedded]

> A ready-to-paste English post. For HN, use the **Show HN** title. For Reddit, the body works as a
> self-post on r/programming or r/embedded. Swap in fresh screenshots/GIF before posting and check
> the links resolve.

---

## Suggested titles

- **Show HN: CDroid – Android's UI framework, ported line-by-line to C++, runs in 32 MB**
- I ported Android's entire widget/view/text/drawable framework to C++ (no JVM, Cairo-rendered, for embedded)
- CDroid — design UI in Android Studio, run it on a set-top box (open source, C++)

## Body

Embedded teams keep rebuilding UI frameworks, and they're almost never as good as Android's. CDroid takes the opposite bet: **port the real thing.**

CDroid is a line-by-line C++ port of Android's Java UI SDK — `android.widget`, `android.view`, `android.text`, `android.graphics.drawable`, `android.animation` — on top of Cairo. It targets embedded systems and runs in as little as 32 MB of RAM. No JVM, no Android runtime.

The guiding rule is *fidelity over redesign*: class names, member names, method signatures, and control flow track the AOSP sources closely enough that you can open the Android reference next to the C++ and follow along. If you can write the UI in Android XML, the same XML renders on a set-top box / in-dash display / industrial panel via Cairo.

What's already ported:

- 50+ widgets and 20+ drawables, API-compatible with Android.
- Full AndroidX ports: RecyclerView, ConstraintLayout + MotionLayout, FragmentManager + Navigation, Flexbox, CoordinatorLayout, ViewPager2.
- Fragment + Transition framework, including shared-element transitions.
- A faithful text stack (spans, StaticLayout/DynamicLayout, minikin line-breaking), a native-level KeyCharacterMap port, and an IME.
- A multi-window, damage-region compositor with DRM/fb/SDL/XCB/VNC backends.

A few things that make it interesting to read even if you never ship it:

- `Canvas` *is* a `cairo_t` — there's no rendering abstraction and no `Bitmap` class; `ImageSurface` plays that role. `onDraw` programs cairo directly.
- No real VSYNC on most embedded SoCs, so the Choreographer self-paces — a faithful port with a pragmatic workaround.
- Porting a GC language to manual-ownership C++ turns up a steady stream of real lifetime/ownership puzzles (span ownership, fragment view lifecycles, window teardown).

It's a large port, so there's always more to translate. I put together a [Good First Issues](docs/contributing/good-first-issues.md) list — each task points at the exact CDROID file and the exact AOSP method to port, mostly "signature exists, body is a stub, translate the logic" sized work — and a [contributing guide](CONTRIBUTING.md) with the build path and the fidelity methodology.

- Repo: https://github.com/houstudio/cdroid (mirror https://gitee.com/houstudio/cdroid)
- Contact: QQ 1225012331 / WeChat: calfhou

Happy to answer questions about the architecture, the Cairo rendering path, or how a specific Android subsystem was ported. Also genuinely looking for contributors — if you've ever wanted to understand how Android's View/Choreographer/text-layout machinery actually works, this is a readable way in.

---
