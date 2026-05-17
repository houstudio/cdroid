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
RoundChart::RoundChart(const std::shared_ptr<CategorySeries>& dataset, const std::shared_ptr<DefaultRenderer>& renderer)
    :AbstractChart(){
    mDataset = dataset;
    mRenderer = renderer;
    mPieMapper = new PieMapper();
}

RoundChart::~RoundChart(){
    delete mPieMapper;
}

int RoundChart::getSeriesSelectionColor(int seriesIndex)const{
    const int size = mDataset->getItemCount();
    auto seriesRenderer = mRenderer->getSeriesRendererAt((seriesIndex + 1)%size);
    return seriesRenderer->getColor();
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

}/*endof namespace*/
