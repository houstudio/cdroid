/**
 * Copyright (C) 2009 - 2012 SC 4ViewSoft SRL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __ACHART_PIE_MAPPER_H__
#define __ACHART_PIE_MAPPER_H__
#include <vector>
#include <widget/achart/chart/piesegment.h>
#include <widget/achart/model/seriesselection.h>
namespace cdroid{
class SeriesSelection;
/**
 * PieChart Segment Selection Management.
 */
class PieMapper{
private:
    int mPieChartRadius;
    int mCenterX, mCenterY;
    std::vector<PieSegment> mPieSegmentList;
public:
    /**
     * Set PieChart location on screen.
     *
     * @param pieRadius
     * @param centerX
     * @param centerY
     */
    void setDimensions(int pieRadius, int centerX, int centerY) {
        mPieChartRadius = pieRadius;
        mCenterX = centerX;
        mCenterY = centerY;
    }

    /**
     * If we have all PieChart Config then there is no point in reloading it
     *
     * @param datasetSize
     * @return true if cfg for each segment is present
     */
    bool areAllSegmentPresent(int datasetSize) const{
        return mPieSegmentList.size() == datasetSize;
    }

    /**
     * Add configuration for a PieChart Segment
     *
     * @param dataIndex
     * @param value
     * @param startAngle
     * @param angle
     */
    void addPieSegment(int dataIndex, float value, float startAngle, float angle) {
        mPieSegmentList.push_back(PieSegment(dataIndex, value, startAngle, angle));
    }

    /**
     * Clears the pie segments list.
     */
    void clearPieSegments() {
        mPieSegmentList.clear();
    }

    /**
     * Fetches angle relative to pie chart center point where 3 O'Clock is 0 and
     * 12 O'Clock is 270degrees
     *
     * @param screenPoint
     * @return angle in degress from 0-360.
     */
    double getAngle(const PointF& screenPoint) const{
        const double dx = screenPoint.x - mCenterX;
        // Minus to correct for coord re-mapping
        const double dy = -(screenPoint.y - mCenterY);
        double inRads = std::atan2(dy, dx);
        // We need to map to coord system when 0 degree is at 3 O'clock, 270 at 12
        // O'clock
        if (inRads < 0)
            inRads = std::abs(inRads);
        else
            inRads = 2.0 * M_PI - inRads;
        return inRads*180.0/M_PI;
    }

    /**
     * Checks if Point falls within PieChart
     *
     * @param screenPoint
     * @return true if in PieChart
     */
    bool isOnPieChart(const PointF& screenPoint) const{
        const double sqValue = (std::pow(mCenterX - screenPoint.x, 2)+std::pow(mCenterY - screenPoint.y, 2));
        const double radiusSquared = mPieChartRadius * mPieChartRadius;
        const bool isOnPieChart = sqValue <= radiusSquared;
        return isOnPieChart;
    }

    /**
     * Fetches the SeriesSelection for the PieSegment selected.
     *
     * @param screenPoint - the user tap location
     * @return null if screen point is not in PieChart or its config if it is
     */
    SeriesSelection* getSeriesAndPointForScreenCoordinate(const PointF& screenPoint) const{
        if (isOnPieChart(screenPoint)) {
            const double angleFromPieCenter = getAngle(screenPoint);
            for (const PieSegment& pieSeg : mPieSegmentList) {
                if (pieSeg.isInSegment(angleFromPieCenter)) {
                    return new SeriesSelection(0, pieSeg.getDataIndex(), pieSeg.getValue(),
                                               pieSeg.getValue());
                }
            }
        }
        return nullptr;
    }
};
}/*endof namespace*/
#endif/*__ACHART_PIE_MAPPER_H__*/
