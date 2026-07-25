# ConstraintLayout (incl. MotionLayout) sources — ported from AndroidX.
# Kept in its own .cmake because the module is large; included by widgetex.cmake.
# Explicit source list, paths relative to src/gui/ (same style as wear.cmake).
# Add new files here.
if(ENABLE_CONSTRAINTLAYOUT)
    set(CONSTRAINTLAYOUT_SOURCES
        # --- core: solver math kernel + LinearSystem ---
        widgetEx/constraintlayout/core/array_linked_variables.cc
        widgetEx/constraintlayout/core/array_row.cc
        widgetEx/constraintlayout/core/goal_row.cc
        widgetEx/constraintlayout/core/linear_system.cc
        widgetEx/constraintlayout/core/metrics.cc
        widgetEx/constraintlayout/core/priority_goal_row.cc
        widgetEx/constraintlayout/core/solver_variable.cc
        widgetEx/constraintlayout/core/solver_variable_values.cc

        # --- core/widgets: constraint model ---
        widgetEx/constraintlayout/core/widgets/constraint_anchor.cc
        widgetEx/constraintlayout/core/widgets/constraint_widget.cc
        widgetEx/constraintlayout/core/widgets/chain.cc
        widgetEx/constraintlayout/core/widgets/chain_head.cc
        widgetEx/constraintlayout/core/widgets/constraint_widget_container.cc
        widgetEx/constraintlayout/core/widgets/guideline.cc
        widgetEx/constraintlayout/core/widgets/optimizer.cc
        widgetEx/constraintlayout/core/widgets/widget_container.cc

        # --- core/widgets/analyzer: graph solver (Stage 3) ---
        widgetEx/constraintlayout/core/widgets/analyzer/basic_measure.cc
        widgetEx/constraintlayout/core/widgets/analyzer/dependency_node.cc
        widgetEx/constraintlayout/core/widgets/analyzer/dimension_dependency.cc
        widgetEx/constraintlayout/core/widgets/analyzer/baseline_dimension_dependency.cc
        widgetEx/constraintlayout/core/widgets/analyzer/widget_run.cc
    )
    list(APPEND WIDGETEX_SOURCES ${CONSTRAINTLAYOUT_SOURCES})
endif()
