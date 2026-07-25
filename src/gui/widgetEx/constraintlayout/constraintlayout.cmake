# ConstraintLayout (incl. MotionLayout) sources — ported from AndroidX.
# Kept in its own .cmake because the module is large; included by widgetex.cmake.
# Explicit source list, paths relative to src/gui/ (same style as wear.cmake).
# Add new files here.
if(ENABLE_CONSTRAINTLAYOUT)
    set(CONSTRAINTLAYOUT_SOURCES
        # --- widget: ConstraintLayout (Stage 4) ---
        widgetEx/constraintlayout/constraintlayout.cc
        # --- widget: virtual helpers (Stage 5) ---
        widgetEx/constraintlayout/constrainthelper.cc
        widgetEx/constraintlayout/barrier.cc
        widgetEx/constraintlayout/group.cc
        widgetEx/constraintlayout/placeholder.cc

        # --- core: solver math kernel + LinearSystem ---
        widgetEx/constraintlayout/core/arraylinkedvariables.cc
        widgetEx/constraintlayout/core/arrayrow.cc
        widgetEx/constraintlayout/core/goalrow.cc
        widgetEx/constraintlayout/core/linearsystem.cc
        widgetEx/constraintlayout/core/metrics.cc
        widgetEx/constraintlayout/core/prioritygoalrow.cc
        widgetEx/constraintlayout/core/solvervariable.cc
        widgetEx/constraintlayout/core/solvervariablevalues.cc

        # --- core/widgets: constraint model ---
        widgetEx/constraintlayout/core/widgets/constraintanchor.cc
        widgetEx/constraintlayout/core/widgets/constraintwidget.cc
        widgetEx/constraintlayout/core/widgets/chain.cc
        widgetEx/constraintlayout/core/widgets/chainhead.cc
        widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.cc
        widgetEx/constraintlayout/core/widgets/guideline.cc
        widgetEx/constraintlayout/core/widgets/optimizer.cc
        widgetEx/constraintlayout/core/widgets/widgetcontainer.cc
        # --- core/widgets: virtual helpers (Stage 5) ---
        widgetEx/constraintlayout/core/widgets/helperwidget.cc
        widgetEx/constraintlayout/core/widgets/barrier.cc

        # --- core/widgets/analyzer: graph solver (Stage 3) ---
        widgetEx/constraintlayout/core/widgets/analyzer/basicmeasure.cc
        widgetEx/constraintlayout/core/widgets/analyzer/dependencynode.cc
        widgetEx/constraintlayout/core/widgets/analyzer/dimensiondependency.cc
        widgetEx/constraintlayout/core/widgets/analyzer/baselinedimensiondependency.cc
        widgetEx/constraintlayout/core/widgets/analyzer/widgetrun.cc

        # --- core/motion: interpolation + curve-fit bedrock (Stage 6) ---
        widgetEx/constraintlayout/core/motion/curvefit.cc
        widgetEx/constraintlayout/core/motion/monotoniccurvefit.cc
        widgetEx/constraintlayout/core/motion/linearcurvefit.cc
        widgetEx/constraintlayout/core/motion/hyperspline.cc
        widgetEx/constraintlayout/core/motion/easing.cc
        widgetEx/constraintlayout/core/motion/schlick.cc
        widgetEx/constraintlayout/core/motion/stepcurve.cc
        widgetEx/constraintlayout/core/motion/oscillator.cc
    )
    list(APPEND WIDGETEX_SOURCES ${CONSTRAINTLAYOUT_SOURCES})
endif()
