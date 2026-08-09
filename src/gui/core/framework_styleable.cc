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
    namespace TextView {
        const uint32_t IDS[] = {
            fw_attr::TextView::bufferType,
            fw_attr::TextView::text,
            fw_attr::TextView::hint,
            fw_attr::TextView::textColor,
            fw_attr::TextView::textColorHighlight,
            fw_attr::TextView::searchResultHighlightColor,
            fw_attr::TextView::focusedSearchResultHighlightColor,
            fw_attr::TextView::textColorHint,
            fw_attr::TextView::textAppearance,
            fw_attr::TextView::textSize,
            fw_attr::TextView::textScaleX,
            fw_attr::TextView::typeface,
            fw_attr::TextView::textStyle,
            fw_attr::TextView::textFontWeight,
            fw_attr::TextView::fontFamily,
            fw_attr::TextView::textLocale,
            fw_attr::TextView::textColorLink,
            fw_attr::TextView::cursorVisible,
            fw_attr::TextView::maxLines,
            fw_attr::TextView::maxHeight,
            fw_attr::TextView::lines,
            fw_attr::TextView::height,
            fw_attr::TextView::minLines,
            fw_attr::TextView::minHeight,
            fw_attr::TextView::maxEms,
            fw_attr::TextView::maxWidth,
            fw_attr::TextView::ems,
            fw_attr::TextView::width,
            fw_attr::TextView::minEms,
            fw_attr::TextView::minWidth,
            fw_attr::TextView::gravity,
            fw_attr::TextView::scrollHorizontally,
            fw_attr::TextView::password,
            fw_attr::TextView::singleLine,
            fw_attr::TextView::enabled,
            fw_attr::TextView::selectAllOnFocus,
            fw_attr::TextView::includeFontPadding,
            fw_attr::TextView::maxLength,
            fw_attr::TextView::shadowColor,
            fw_attr::TextView::shadowDx,
            fw_attr::TextView::shadowDy,
            fw_attr::TextView::shadowRadius,
            fw_attr::TextView::autoLink,
            fw_attr::TextView::linksClickable,
            fw_attr::TextView::numeric,
            fw_attr::TextView::digits,
            fw_attr::TextView::phoneNumber,
            fw_attr::TextView::inputMethod,
            fw_attr::TextView::capitalize,
            fw_attr::TextView::autoText,
            fw_attr::TextView::editable,
            fw_attr::TextView::freezesText,
            fw_attr::TextView::ellipsize,
            fw_attr::TextView::drawableTop,
            fw_attr::TextView::drawableBottom,
            fw_attr::TextView::drawableLeft,
            fw_attr::TextView::drawableRight,
            fw_attr::TextView::drawableStart,
            fw_attr::TextView::drawableEnd,
            fw_attr::TextView::drawablePadding,
            fw_attr::TextView::drawableTint,
            fw_attr::TextView::drawableTintMode,
            fw_attr::TextView::lineSpacingExtra,
            fw_attr::TextView::lineSpacingMultiplier,
            fw_attr::TextView::lineHeight,
            fw_attr::TextView::firstBaselineToTopHeight,
            fw_attr::TextView::lastBaselineToBottomHeight,
            fw_attr::TextView::marqueeRepeatLimit,
            fw_attr::TextView::inputType,
            fw_attr::TextView::allowUndo,
            fw_attr::TextView::imeOptions,
            fw_attr::TextView::privateImeOptions,
            fw_attr::TextView::imeActionLabel,
            fw_attr::TextView::imeActionId,
            fw_attr::TextView::editorExtras,
            fw_attr::TextView::textSelectHandleLeft,
            fw_attr::TextView::textSelectHandleRight,
            fw_attr::TextView::textSelectHandle,
            fw_attr::TextView::textEditPasteWindowLayout,
            fw_attr::TextView::textEditNoPasteWindowLayout,
            fw_attr::TextView::textEditSidePasteWindowLayout,
            fw_attr::TextView::textEditSideNoPasteWindowLayout,
            fw_attr::TextView::textEditSuggestionItemLayout,
            fw_attr::TextView::textEditSuggestionContainerLayout,
            fw_attr::TextView::textEditSuggestionHighlightStyle,
            fw_attr::TextView::textCursorDrawable,
            fw_attr::TextView::textIsSelectable,
            fw_attr::TextView::textAllCaps,
            fw_attr::TextView::elegantTextHeight,
            fw_attr::TextView::fallbackLineSpacing,
            fw_attr::TextView::letterSpacing,
            fw_attr::TextView::fontFeatureSettings,
            fw_attr::TextView::fontVariationSettings,
            fw_attr::TextView::breakStrategy,
            fw_attr::TextView::hyphenationFrequency,
            fw_attr::TextView::lineBreakStyle,
            fw_attr::TextView::lineBreakWordStyle,
            fw_attr::TextView::autoSizeTextType,
            fw_attr::TextView::autoSizeStepGranularity,
            fw_attr::TextView::autoSizePresetSizes,
            fw_attr::TextView::autoSizeMinTextSize,
            fw_attr::TextView::autoSizeMaxTextSize,
            fw_attr::TextView::justificationMode,
            fw_attr::TextView::useBoundsForWidth,
            fw_attr::TextView::shiftDrawingOffsetForStartOverhang,
            fw_attr::TextView::useLocalePreferredLineHeightForMinimum,
        };
    }
    namespace ProgressBar {
        const uint32_t IDS[] = {
            fw_attr::ProgressBar::min,
            fw_attr::ProgressBar::max,
            fw_attr::ProgressBar::progress,
            fw_attr::ProgressBar::secondaryProgress,
            fw_attr::ProgressBar::indeterminate,
            fw_attr::ProgressBar::indeterminateOnly,
            fw_attr::ProgressBar::indeterminateDrawable,
            fw_attr::ProgressBar::progressDrawable,
            fw_attr::ProgressBar::indeterminateDuration,
            fw_attr::ProgressBar::indeterminateBehavior,
            fw_attr::ProgressBar::minWidth,
            fw_attr::ProgressBar::maxWidth,
            fw_attr::ProgressBar::minHeight,
            fw_attr::ProgressBar::maxHeight,
            fw_attr::ProgressBar::interpolator,
            fw_attr::ProgressBar::animationResolution,
            fw_attr::ProgressBar::mirrorForRtl,
            fw_attr::ProgressBar::progressTint,
            fw_attr::ProgressBar::progressTintMode,
            fw_attr::ProgressBar::progressBackgroundTint,
            fw_attr::ProgressBar::progressBackgroundTintMode,
            fw_attr::ProgressBar::secondaryProgressTint,
            fw_attr::ProgressBar::secondaryProgressTintMode,
            fw_attr::ProgressBar::indeterminateTint,
            fw_attr::ProgressBar::indeterminateTintMode,
            fw_attr::ProgressBar::backgroundTint,
            fw_attr::ProgressBar::backgroundTintMode,
        };
    }
    namespace Switch {
        const uint32_t IDS[] = {
            fw_attr::Switch::thumb,
            fw_attr::Switch::thumbTint,
            fw_attr::Switch::thumbTintMode,
            fw_attr::Switch::track,
            fw_attr::Switch::trackTint,
            fw_attr::Switch::trackTintMode,
            fw_attr::Switch::textOn,
            fw_attr::Switch::textOff,
            fw_attr::Switch::thumbTextPadding,
            fw_attr::Switch::switchTextAppearance,
            fw_attr::Switch::switchMinWidth,
            fw_attr::Switch::switchPadding,
            fw_attr::Switch::splitTrack,
            fw_attr::Switch::showText,
        };
    }
    namespace SeekBar {
        const uint32_t IDS[] = {
            fw_attr::SeekBar::thumb, fw_attr::SeekBar::thumbOffset,
            fw_attr::SeekBar::splitTrack, fw_attr::SeekBar::disabledAlpha,
            fw_attr::SeekBar::thumbTint, fw_attr::SeekBar::tickMark,
            fw_attr::SeekBar::tickMarkTint,
        };
    }
    namespace ToggleButton {
        const uint32_t IDS[] = {
            fw_attr::ToggleButton::textOn, fw_attr::ToggleButton::textOff,
            fw_attr::ToggleButton::disabledAlpha,
        };
    }
    namespace CheckedTextView {
        const uint32_t IDS[] = {
            fw_attr::CheckedTextView::checked, fw_attr::CheckedTextView::checkMark,
            fw_attr::CheckedTextView::checkMarkTint, fw_attr::CheckedTextView::checkMarkTintMode,
        };
    }
    namespace ListView {
        const uint32_t IDS[] = {
            fw_attr::ListView::divider, fw_attr::ListView::dividerHeight,
            fw_attr::ListView::headerDividersEnabled, fw_attr::ListView::footerDividersEnabled,
            fw_attr::ListView::overScrollHeader, fw_attr::ListView::overScrollFooter,
        };
    }
    namespace RadioGroup {
        const uint32_t IDS[] = {
            fw_attr::RadioGroup::checkedButton, fw_attr::View::orientation,
        };
    }
    namespace NestedScrollView {
        const uint32_t IDS[] = { fw_attr::NestedScrollView::fillViewport };
    }
    namespace AdapterViewAnimator {
        const uint32_t IDS[] = {
            fw_attr::AdapterViewAnimator::inAnimation,
            fw_attr::AdapterViewAnimator::outAnimation,
            fw_attr::AdapterViewAnimator::animateFirstView,
            fw_attr::AdapterViewAnimator::loopViews,
        };
    }
    namespace AdapterViewFlipper {
        const uint32_t IDS[] = {
            fw_attr::AdapterViewFlipper::flipInterval,
            fw_attr::AdapterViewFlipper::autoStart,
        };
    }
    namespace TextClock {
        const uint32_t IDS[] = {
            fw_attr::TextClock::format12Hour,
            fw_attr::TextClock::format24Hour,
            fw_attr::TextClock::timeZone,
        };
    }
    namespace AnalogClock {
        const uint32_t IDS[] = {
            fw_attr::AnalogClock::dial,
            fw_attr::AnalogClock::hand_hour,
            fw_attr::AnalogClock::hand_minute,
            fw_attr::AnalogClock::hand_second,
        };
    }
    namespace GridView {
        const uint32_t IDS[] = {
            fw_attr::GridView::horizontalSpacing, fw_attr::GridView::verticalSpacing,
            fw_attr::GridView::stretchMode, fw_attr::GridView::columnWidth,
            fw_attr::GridView::numColumns, fw_attr::View::gravity,
        };
    }
    namespace GridLayout {
        const uint32_t IDS[] = {
            fw_attr::View::orientation, fw_attr::GridLayout::rowCount,
            fw_attr::GridLayout::columnCount, fw_attr::GridLayout::useDefaultMargins,
            fw_attr::GridLayout::alignmentMode, fw_attr::GridLayout::rowOrderPreserved,
            fw_attr::GridLayout::columnOrderPreserved,
        };
    }
    namespace Spinner {
        const uint32_t IDS[] = {
            fw_attr::Spinner::spinnerMode, fw_attr::Spinner::prompt,
            fw_attr::Spinner::popupBackground, fw_attr::Spinner::dropDownSelector,
            fw_attr::Spinner::dropDownWidth, fw_attr::Spinner::dropDownAnchor,
            fw_attr::View::gravity,
        };
    }
    namespace RatingBar {
        const uint32_t IDS[] = {
            fw_attr::RatingBar::numStars, fw_attr::RatingBar::rating,
            fw_attr::RatingBar::stepSize, fw_attr::RatingBar::isIndicator,
        };
    }
    namespace Chronometer {
        const uint32_t IDS[] = { fw_attr::Chronometer::format, fw_attr::Chronometer::countDown };
    }
    namespace ScrollView {
        const uint32_t IDS[] = { fw_attr::ScrollView::fillViewport };
    }
    namespace RelativeLayout {
        const uint32_t IDS[] = { fw_attr::RelativeLayout::ignoreGravity, fw_attr::View::gravity };
    }
    namespace Layout {
        const uint32_t IDS[] = { fw_attr::Layout::layout_width, fw_attr::Layout::layout_height };
    }
    namespace MarginLayout {
        const uint32_t IDS[] = {
            fw_attr::MarginLayout::layout_margin,
            fw_attr::MarginLayout::layout_marginLeft, fw_attr::MarginLayout::layout_marginTop,
            fw_attr::MarginLayout::layout_marginRight, fw_attr::MarginLayout::layout_marginBottom,
            fw_attr::MarginLayout::layout_marginStart, fw_attr::MarginLayout::layout_marginEnd,
            fw_attr::MarginLayout::layout_marginHorizontal, fw_attr::MarginLayout::layout_marginVertical,
        };
    }
    namespace LinearLayoutLayout {
        const uint32_t IDS[] = {
            fw_attr::LinearLayoutLayout::layout_weight, fw_attr::LinearLayoutLayout::layout_gravity,
        };
    }
    namespace RelativeLayoutLayout {
        const uint32_t IDS[] = {
            fw_attr::RelativeLayoutLayout::layout_toLeftOf, fw_attr::RelativeLayoutLayout::layout_toRightOf,
            fw_attr::RelativeLayoutLayout::layout_above, fw_attr::RelativeLayoutLayout::layout_below,
            fw_attr::RelativeLayoutLayout::layout_alignBaseline, fw_attr::RelativeLayoutLayout::layout_alignLeft,
            fw_attr::RelativeLayoutLayout::layout_alignTop, fw_attr::RelativeLayoutLayout::layout_alignRight,
            fw_attr::RelativeLayoutLayout::layout_alignBottom,
            fw_attr::RelativeLayoutLayout::layout_alignParentLeft, fw_attr::RelativeLayoutLayout::layout_alignParentTop,
            fw_attr::RelativeLayoutLayout::layout_alignParentRight, fw_attr::RelativeLayoutLayout::layout_alignParentBottom,
            fw_attr::RelativeLayoutLayout::layout_centerInParent, fw_attr::RelativeLayoutLayout::layout_centerHorizontal,
            fw_attr::RelativeLayoutLayout::layout_centerVertical,
            fw_attr::RelativeLayoutLayout::layout_toStartOf, fw_attr::RelativeLayoutLayout::layout_toEndOf,
            fw_attr::RelativeLayoutLayout::layout_alignStart, fw_attr::RelativeLayoutLayout::layout_alignEnd,
            fw_attr::RelativeLayoutLayout::layout_alignParentStart, fw_attr::RelativeLayoutLayout::layout_alignParentEnd,
        };
    }
    namespace TableRowLayout {
        const uint32_t IDS[] = {
            fw_attr::TableRowLayout::layout_column, fw_attr::TableRowLayout::layout_span,
        };
    }
}
}
