// Headless reproduction for circularflowtest: 8 satellite widgets positioned on a circle around a
// center widget via connectCircularConstraint (exactly what CircularFlow.anchorReferences drives at
// the View layer). Runs the core solver directly so no display is needed.
#include <gtest/gtest.h>
#include <cmath>
#include <cstdio>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>

using namespace cdroid;

TEST(CLCircular, EightSatellitesOnCircle) {
    ConstraintWidgetContainer root("root", 320, 320);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, ConstraintWidget::DimensionBehaviour::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, ConstraintWidget::DimensionBehaviour::FIXED);

    ConstraintWidget center("center");
    center.setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
    center.setWidth(40);
    center.setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
    center.setHeight(40);
    center.mLeft.connect(root.mLeft, 0);  center.mRight.connect(root.mRight, 0);
    center.mTop.connect(root.mTop, 0);    center.mBottom.connect(root.mBottom, 0);
    root.add(&center);

    ConstraintWidget sats[8];
    float angles[8] = {0, 45, 90, 135, 180, 225, 270, 315};
    const int radius = 120;
    for (int i = 0; i < 8; i++) {
        sats[i].setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        sats[i].setWidth(40);
        sats[i].setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        sats[i].setHeight(40);
        sats[i].connectCircularConstraint(&center, angles[i], radius);
        root.add(&sats[i]);
    }

    root.layout();

    int cx = center.getX() + center.getWidth() / 2;
    int cy = center.getY() + center.getHeight() / 2;
    fprintf(stderr, "center cx,cy = (%d, %d)\n", cx, cy);
    for (int i = 0; i < 8; i++) {
        int sx = sats[i].getX() + sats[i].getWidth() / 2;
        int sy = sats[i].getY() + sats[i].getHeight() / 2;
        fprintf(stderr, "sat[%d] angle=%g -> pos(%d,%d) center(%d,%d)\n",
               i, angles[i], sats[i].getX(), sats[i].getY(), sx, sy);
        // Each satellite must sit roughly `radius` from the center.
        double dist = std::hypot((double)(sx - cx), (double)(sy - cy));
        EXPECT_NEAR(dist, radius, 3) << "sat " << i;
    }
    // No two satellites should share the same position (the bug "only button 8 visible" implies all
    // collapse onto one spot).
    for (int i = 0; i < 8; i++) {
        for (int j = i + 1; j < 8; j++) {
            EXPECT_FALSE(sats[i].getX() == sats[j].getX() && sats[i].getY() == sats[j].getY())
                << "sat " << i << " and sat " << j << " overlap";
        }
    }
}
