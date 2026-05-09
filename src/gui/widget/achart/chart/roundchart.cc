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
#include<widget/achart/chart/roundchart.h>
namespace cdroid{
RoundChart::RoundChart(const std::shared_ptr<CategorySeries>& dataset, const std::shared_ptr<DefaultRenderer>& renderer) {
    mDataset = dataset;
    mRenderer = renderer;
}

void RoundChart::drawTitle(Canvas& canvas, int x, int y, int width,  Paint& paint) {
    if (mRenderer->isShowLabels()) {
        canvas.set_color(mRenderer->getLabelsColor());
        paint.setTextAlign(Align::CENTER);
        canvas.set_font_size(mRenderer->getChartTitleTextSize());
        drawString(canvas, mRenderer->getChartTitle(), x + width / 2,
                   y + mRenderer->getChartTitleTextSize(), paint);
    }
}

int RoundChart::getLegendShapeWidth(int seriesIndex) const{
    return SHAPE_WIDTH;
}

void RoundChart::drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
        float x, float y,int seriesIndex,  Paint& paint) {
    //canvas.drawRect(x, y - SHAPE_WIDTH / 2, x + SHAPE_WIDTH, y + SHAPE_WIDTH / 2, paint);
    canvas.rectangle(x,y-SHAPE_WIDTH/2,SHAPE_WIDTH,SHAPE_WIDTH);
    if(paint.style==Style::FILL)canvas.fill();else canvas.stroke();
}

const std::shared_ptr<DefaultRenderer>& RoundChart::getRenderer() const{
    return mRenderer;
}

int RoundChart::getCenterX() const{
    return mCenterX;
}

int RoundChart::getCenterY() const{
    return mCenterY;
}

void RoundChart::setCenterX(int centerX) {
    mCenterX = centerX;
}

void RoundChart::setCenterY(int centerY) {
    mCenterY = centerY;
}

SeriesSelection* RoundChart:: getSeriesAndPointForScreenCoordinate(const PointF& point)const{
    SeriesSelection* selection = AbstractChart::getSeriesAndPointForScreenCoordinate(point);
    if (selection != nullptr) {
        return selection;
    }    
    return getSectorForScreenCoordinate(point);
}

SeriesSelection* RoundChart::getSectorForScreenCoordinate(const PointF& point) const{
    if (mDataset == nullptr || mDataset->getItemCount() == 0) {
        return nullptr;
    }
#if 0 
    double radius = getRadius();
    // 计算点击点与圆心的距离
    double distance = std::sqrt(
        std::pow(point.x - mCenterX, 2) + std::pow(point.y - mCenterY, 2)
    );
    
    // 检查是否在饼图有效半径范围内
    double outerRadius = getOuterRadius();
    double innerRadius = getInnerRadius(); // 甜甜圈图的内半径
    
    if (distance > outerRadius || distance < innerRadius) {
        return nullptr; // 点不在扇形区域内
    }
    
    // 计算角度
    double angle = std::atan2(point.y - mCenterY, point.x - ,mCenterX)*180.0/M_PI;
    if (angle < 0) angle += 360; // 转换为0-360度范围
    
    // 找到对应的扇形
    const int itemCount = mDataset->getItemCount();
    double totalValue = calculateTotalValue(mDataset, 0);
    double currentAngle = mRenderer->getStartAngle(); // 起始角度
    
    for (int i = 0; i < itemCount; i++) {
        double value = mDataset->getValue(0, i);
        double sectorAngle = (value / totalValue) * 360.0;
        
        if (angle >= currentAngle && angle < currentAngle + sectorAngle) {
            return new SeriesSelection(0, i, value); // seriesIndex=0, pointIndex=i
        }
        currentAngle += sectorAngle;
    }
#endif
    return nullptr;
}
}/*endof namespace*/
