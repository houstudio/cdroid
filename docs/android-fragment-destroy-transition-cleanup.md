# Android Activity destroy 时 transition / animator / overlay 清理机制

> 调研目的:对照 CDROID 的修复(forceCompleteAllSpecialEffects 里 endTransitions)是否正确、
> Android 的权威清理点在哪。参考源码:`$HOME/research/androidx`(androidx fragment/transition)、
> `/opt/android-sdk/sources/android-36/`(android framework)。

## Q1 — androidx Fragment 销毁路径:**不**调 `TransitionManager.endTransitions`,只走 SEC Effect 层 cancel

调用链:

- `FragmentActivity.onDestroy()` → `mFragments.dispatchDestroy()` (`FragmentActivity.java:254-256`)
- `FragmentManager.dispatchDestroy()` (`FragmentManager.java:3245`) → **`endAnimatingAwayFragments()`** (`:3248`)
- `endAnimatingAwayFragments()` (`:2320`) → 遍历每个 SEC 调 **`controller.forceCompleteAllOperations()`** (`:2323`)
- `SEC.forceCompleteAllOperations()` (`SpecialEffectsController.kt:332`) → 对 running + pending ops 调 **`operation.cancel(container)`** (`:364`/`:387`)

`Operation.cancel` (`SpecialEffectsController.kt:642`):

```kotlin
fun cancel(container: ViewGroup) {
    isCanceled = true
    if (_effects.isEmpty()) complete()
    else effects.toList().forEach { it.cancel(container) }
}
```

派发到具体 Effect 的 `onCancel`:

- `AnimatorEffect.onCancel` (`DefaultSpecialEffectsController.kt:713`) → **`animator.end()`** (`:725`,非 seeking 时)—— 强制到终态
- `TransitionEffect.onCancel` (`DefaultSpecialEffectsController.kt:1220`) → **`transitionSignal.cancel()`**(一个 `CancellationSignal`)

关键转折:`transitionSignal` 经 `transitionImpl.setListenerForTransitionEnd` 接到 framework Transition。但
**`FragmentTransitionCompat21.setListenerForTransitionEnd`** (`FragmentTransitionCompat21.java:293-314`) 只挂了个
`onTransitionEnd` 监听跑 `transitionCompleteRunnable`,**完全没把 CancellationSignal 接到 `forceToEnd`/`endTransitions`**。
即 `transitionSignal.cancel()` 在 framework 转场路径上是**空操作**。

源码自证(`FragmentTransitionCompat21.java:289` 注释):*"Destroying the view of the Fragment is how the Transition gets canceled."*

**结论 Q1**:androidx Fragment 销毁时**不显式 end/cancel framework Transition 的 clone**(不调
`TransitionManager.endTransitions` 也不调 `forceToEnd`)。只对 Animator 类 Effect 调 `animator.end()`,对 Transition 类
Effect 仅发个空操作信号;framework Transition clone 靠后续自然结束 + GC 兜底。

## Q2 — TransitionManager 在 sceneRoot detach 时**不**自动 end running transition

`beginDelayedTransition` 注册的 `MultiListener`(`TransitionManager.java:252`)在 sceneRoot detach 时:

```java
// TransitionManager.java:279
public void onViewDetachedFromWindow(View v) {
    removeListeners();
    sPendingTransitions.remove(mSceneRoot);
    ArrayList<Transition> runningTransitions = getRunningTransitions().get(mSceneRoot);
    if (runningTransitions != null && runningTransitions.size() > 0) {
        for (Transition runningTransition : runningTransitions)
            runningTransition.resume(mSceneRoot);   // 仅 resume,不 end/cancel
    }
    mTransition.clearValues(true);                  // 只清"还没启动"的 pending clone 的值
}
```

**只 resume + 清自己(未启动)的 values,不结束正在跑的 transition。**

`TransitionManager.endTransitions(sceneRoot)` (`:456`)是**唯一**调 `transition.forceToEnd(sceneRoot)` 的地方,
是显式 API,**不在 detach 自动触发**。

**结论 Q2**:framework 没有任何"sceneRoot detach 自动结束 running transition"的机制。

## Q3 — animator 的 UAF:Android 靠 GC 兜底,不是弱引用也不是 detach 取消

`Transition.forceToEnd` (`Transition.java:1954`):

```java
void forceToEnd(ViewGroup sceneRoot) {
    ...
    for (int i = numOldAnims - 1; i >= 0; i--) {
        AnimationInfo info = oldAnimators.valueAt(i);
        if (info.view != null && windowId != null && windowId.equals(info.windowId)) {
            Animator anim = oldAnimators.keyAt(i);
            anim.end();                      // 强制到终态,触发 onAnimationEnd
        }
    }
}
```

`sRunningTransitions` 是 `ThreadLocal<WeakReference<ArrayMap<ViewGroup, ArrayList<Transition>>>>`
(`TransitionManager.java:79`),Transition clone 持有其 Animator,ObjectAnimator 强引用 target View
(`PropertyValuesHolder`)。

`View.onDetachedFromWindowInternal` (`View.java:22923-22941`) 只置 `mCurrentAnimation = null`(遗留 `Animation`),
**完全不动 transition 的 `ObjectAnimator`**(那由 transition clone 持有)。所以 detach **不会**取消 transition 的 animator。

**结论 Q3**:Android **不会 UAF,因为根本没有手动释放**。transition clone + animator + 被 animate 的 view 通过
`sRunningTransitions` + Choreographer 互相可达,自然存活到动画结束(或整棵树 drop 后被 GC 一并回收)。没有弱引用、
没有 detach-cancel,纯 GC 兜底。CDROID 是 C++ 手动释放,fragment view 在 destroy 路径被 `delete`,而 transition clone 的
animator 仍持裸指针 → 这就是 CDROID 必须处理而 Android 不必处理的根因。

## Q4 — ViewRootImpl.dispatchDetachedFromWindow:不清理 running animator / transition

`ViewRootImpl.dispatchDetachedFromWindow` (`ViewRootImpl.java:6580-6631`):`mView.dispatchDetachedFromWindow()`
(向树广播 detach)、`destroyHardwareRenderer()`、`destroySurface()`、`unscheduleTraversals()`(只移除 traversal 的
choreographer callback)、`mWindowSession.remove()`。**没有遍历 running animator / transition / 调 endTransitions。**

framework 全树 `endTransitions` 调用点仅:

- `ExitTransitionCoordinator.resetViews` (`ExitTransitionCoordinator.java:128`) —— **Activity 级 shared-element 退出转场**
  (`resetViews()` 在 Activity 退出/finish 时由 coordinator 调)
- `ActivityOptions.java:1164`
- `PopupWindow.dismiss` 三处

**结论 Q4**:ViewRootImpl 层**不**做 animator/transition 清理;framework 唯一的 destroy 时点 force-end 是 Activity 级转场的
`ExitTransitionCoordinator`。Fragment 转场和 view detach 都不触发 `endTransitions`。

## 总结论:CDROID 的修复方向正确,且比 androidx 更显式

**Android 的权威清理点(按场景分裂)**:

| 场景 | 清理点 | 机制 |
|---|---|---|
| Activity shared-element 退出 | `ExitTransitionCoordinator.resetViews` → `TransitionManager.endTransitions` | 显式 `forceToEnd` |
| PopupWindow 关闭 | `PopupWindow.dismiss` → `endTransitions` | 显式 `forceToEnd` |
| **Fragment destroy** | **无显式清理** | `transitionSignal.cancel()`(空操作)+ `animator.end()` for Animator Effect + **GC 兜底** |
| View/sceneRoot detach | `MultiListener.onViewDetachedFromWindow` | 仅 resume + clearValues,**不 end** |
| ViewRootImpl die | `dispatchDetachedFromWindow` | **不碰** animator/transition |

**CDROID 修复**(`fragmentmanager.cc` `forceCompleteAllSpecialEffects`):对每个 active fragment 的 container,
**先** `TransitionManager::endTransitions(f->mContainer)` → **再** `fsm->forceCompleteSpecialEffects()`。
`Transition::forceToEnd`(`transition.cc`)是 `Transition.java:1954` 的逐行移植(按 `windowId` 过滤跑着的 animator 调 `anim->end()`)。

**判定:方向正确,且是 CDROID 必须做的**。

- androidx 在 fragment destroy 不 force-end Transition clone,是因为有 GC 兜底 —— transition clone + animator + view
  一起自然回收,无 UAF。
- CDROID 没有 GC:fragment view 在 `dispatchStateChange(INITIALIZING)` / SEC 完成时被 `delete`,但 transition clone 的
  ObjectAnimator 仍在 sRunningTransitions 里,Choreographer 下一帧 dereference 已删 view → UAF。
- 因此 CDROID 必须在删 view 之前显式 force-end 所有跑着的 clone + animator,这与 Android framework 在 Activity 级退出时
  调 `endTransitions` 是**同一机制、同一 API**(`TransitionManager::endTransitions` → `Transition::forceToEnd` → `anim->end()`),
  只是 Android 把它限定在 Activity 退出/Popup dismiss,CDROID 因无 GC 不得不把它**前移到 fragment destroy**。
- 顺序也对:**先 endTransitions(让 animator 自然走 onAnimationEnd 回调链,把 transitionAlpha 等还原终态)再
  forceCompleteSpecialEffects(完成 SEC op)**,避免 animator 回调还在跑时 op/container 已析构。

唯一可注意的差异(非 bug):Android 的 `forceToEnd` 用 `WindowId` 对象 `equals` 比较(`Transition.java:1961/1967`),
CDROID 用裸指针 `windowId == info.windowId` 比较(`transition.cc`)。在 CDROID 的 WindowId 模型下功能等价,但若以后
windowId 被复用/重分配需留意。

## 关键文件引用

### androidx
- `fragment/fragment/src/main/java/androidx/fragment/app/FragmentActivity.java:254`(onDestroy→dispatchDestroy)
- `fragment/fragment/src/main/java/androidx/fragment/app/FragmentManager.java:3245`(dispatchDestroy)、`:2320`(endAnimatingAwayFragments)
- `fragment/fragment/src/main/java/androidx/fragment/app/SpecialEffectsController.kt:332`(forceCompleteAllOperations)、`:642`(Operation.cancel)
- `fragment/fragment/src/main/java/androidx/fragment/app/DefaultSpecialEffectsController.kt:713`(AnimatorEffect.onCancel→end)、`:1220`(TransitionEffect.onCancel→signal)、`:951`(beginDelayedTransition)
- `fragment/fragment/src/main/java/androidx/fragment/app/FragmentTransitionCompat21.java:289`(注释)、`:293-314`(signal 未接 forceToEnd)

### android-36 framework
- `android/transition/TransitionManager.java:252`(MultiListener)、`:279`(onViewDetachedFromWindow)、`:456`(endTransitions→forceToEnd)
- `android/transition/Transition.java:1954`(forceToEnd→anim.end())
- `android/view/ViewRootImpl.java:6580`(dispatchDetachedFromWindow,不清 animator)
- `android/view/View.java:22923`(onDetachedFromWindowInternal,只清 mCurrentAnimation)
- `android/app/ExitTransitionCoordinator.java:128`(Activity 级 endTransitions)

### CDROID
- `src/gui/fragment/fragmentmanager.cc` — `forceCompleteAllSpecialEffects` 里每个 container 先 `endTransitions` 再 `forceCompleteSpecialEffects`
- `src/gui/transition/transition.cc` — `Transition::forceToEnd`(Transition.java:1954 移植)
- `src/gui/transition/transitionmanager.cc` — `TransitionManager::endTransitions`(forceToEnd 唯一调用)、`MultiListener::onViewDetachedFromWindow`(resume 不 end,同 Android)
