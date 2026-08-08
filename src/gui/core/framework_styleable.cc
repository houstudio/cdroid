#include "framework_styleable.h"
namespace cdroid {
namespace styleable {
    namespace View {
        const uint32_t IDS[] = {
            fw_attr::View::background,
            fw_attr::View::padding, fw_attr::View::paddingLeft, fw_attr::View::paddingTop,
            fw_attr::View::paddingRight, fw_attr::View::paddingBottom,
            fw_attr::View::paddingStart, fw_attr::View::paddingEnd,
            fw_attr::View::minWidth, fw_attr::View::minHeight,
            fw_attr::View::visibility, fw_attr::View::id, fw_attr::View::tag,
            fw_attr::View::scrollX, fw_attr::View::scrollY,
            fw_attr::View::fitsSystemWindows, fw_attr::View::scrollbars, fw_attr::View::fadingEdge,
            fw_attr::View::fadingEdgeLength,
            fw_attr::View::nextFocusLeft, fw_attr::View::nextFocusRight,
            fw_attr::View::nextFocusUp, fw_attr::View::nextFocusDown,
            fw_attr::View::clickable, fw_attr::View::longClickable,
            fw_attr::View::saveEnabled, fw_attr::View::drawingCacheQuality,
            fw_attr::View::duplicateParentState,
            fw_attr::View::focusable, fw_attr::View::focusableInTouchMode,
            fw_attr::View::soundEffectsEnabled, fw_attr::View::hapticFeedbackEnabled,
            fw_attr::View::onClick, fw_attr::View::contentDescription,
            fw_attr::View::isScrollContainer, fw_attr::View::foregroundGravity,
            fw_attr::View::scrollbarStyle, fw_attr::View::scrollbarSize,
            fw_attr::View::scrollbarFadeDuration, fw_attr::View::scrollbarDefaultDelayBeforeFade,
            fw_attr::View::fadeScrollbars, fw_attr::View::filterTouchesWhenObscured,
            fw_attr::View::keepScreenOn, fw_attr::View::layerType,
            fw_attr::View::layoutDirection, fw_attr::View::textDirection,
            fw_attr::View::textAlignment, fw_attr::View::importantForAccessibility,
            fw_attr::View::requiresFadingEdge, fw_attr::View::overScrollMode,
            fw_attr::View::verticalScrollbarPosition,
            fw_attr::View::rotation, fw_attr::View::rotationX, fw_attr::View::rotationY,
            fw_attr::View::scaleX, fw_attr::View::scaleY,
            fw_attr::View::transformPivotX, fw_attr::View::transformPivotY,
            fw_attr::View::translationX, fw_attr::View::translationY, fw_attr::View::translationZ,
            fw_attr::View::alpha, fw_attr::View::elevation,
            fw_attr::View::transitionName, fw_attr::View::stateListAnimator,
            fw_attr::View::nestedScrollingEnabled,
            fw_attr::View::backgroundTint, fw_attr::View::backgroundTintMode,
            fw_attr::View::foregroundTint, fw_attr::View::foregroundTintMode,
            fw_attr::View::outlineProvider,
            fw_attr::View::scrollIndicators, fw_attr::View::nextFocusForward,
            fw_attr::View::nextClusterForward,
            fw_attr::View::keyboardNavigationCluster,
            fw_attr::View::focusedByDefault, fw_attr::View::allowClickWhenDisabled,
            fw_attr::View::enabled,
        };
    }
    namespace ViewGroup {
        const uint32_t IDS[] = {
            fw_attr::ViewGroup::clipChildren, fw_attr::ViewGroup::clipToPadding,
            fw_attr::ViewGroup::layoutAnimation, fw_attr::ViewGroup::descendantFocusability,
            fw_attr::ViewGroup::animateLayoutChanges, fw_attr::ViewGroup::layoutMode,
            fw_attr::ViewGroup::addStatesFromChildren, fw_attr::ViewGroup::splitMotionEvents,
            fw_attr::ViewGroup::alwaysDrawnWithCache, fw_attr::ViewGroup::transitionGroup,
            fw_attr::ViewGroup::touchscreenBlocksFocus,
        };
    }
    namespace LinearLayout {
        const uint32_t IDS[] = {
            fw_attr::View::orientation, fw_attr::View::gravity,
            fw_attr::LinearLayout::baselineAligned,
            fw_attr::LinearLayout::baselineAlignedChildIndex,
            fw_attr::LinearLayout::weightSum,
            fw_attr::LinearLayout::measureWithLargestChild,
            fw_attr::LinearLayout::showDividers,
            fw_attr::LinearLayout::divider,
            fw_attr::LinearLayout::dividerPadding,
            fw_attr::LinearLayout::measureAllChildren,
        };
    }

    namespace ImageView {
        const uint32_t IDS[] = {
            fw_attr::ImageView::src, fw_attr::ImageView::scaleType,
            fw_attr::ImageView::adjustViewBounds, fw_attr::ImageView::maxWidth,
            fw_attr::ImageView::maxHeight, fw_attr::ImageView::tint,
            fw_attr::ImageView::cropToPadding, fw_attr::ImageView::drawablePadding,
            fw_attr::ImageView::baseline, 0 /* baselineAlignBottom */,
            fw_attr::View::alpha,
        };
    }
    namespace CompoundButton {
        const uint32_t IDS[] = {
            fw_attr::CompoundButton::checked, fw_attr::CompoundButton::button,
        };
    }
    namespace FrameLayout {
        const uint32_t IDS[] = {
            fw_attr::LinearLayout::measureAllChildren, 0 /* foreground */, 0 /* foregroundInsidePadding */,
        };
    }
}
}
