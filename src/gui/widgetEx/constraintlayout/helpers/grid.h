/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.helper.widget.Grid.
 *
 * Arranges referenced views into a rows×columns grid. Owns NO core widget — instead it creates
 * invisible "box" Views as container children: the first `columns` boxes form a horizontal chain and
 * the first `rows` boxes a vertical chain, both anchored to this Grid's own id (its ConstraintWidget
 * is the bounding box). Each referenced view is constrained left/top/right/bottom to box[column]/
 * box[row] (with span support), so the solver lays them out in cells. Spans/skips are tracked via a
 * boolean position matrix. Faithful View-helper port (no core GridCore/GridEngine — that's the
 * Compose/JSON path).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_GRID_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_GRID_H

#include <string>
#include <vector>

#include <widgetEx/constraintlayout/helpers/constrainthelper.h>

namespace cdroid {

class Grid : public ConstraintHelper {
  public:
    static constexpr int HORIZONTAL = 0;
    static constexpr int VERTICAL   = 1;

    Grid(Context* ctx, const AttributeSet& attrs);
    explicit Grid(int width, int height);

    // --- programmatic config (each rebuilds the grid) ---
    void setRows(int rows);
    void setColumns(int columns);
    void setOrientation(int orientation);
    void setSpans(const std::string& spans);
    void setSkips(const std::string& skips);
    void setRowWeights(const std::string& weights);
    void setColumnWeights(const std::string& weights);
    void setHorizontalGaps(float gaps);
    void setVerticalGaps(float gaps);

    int  getRows() const { return mRowsSet; }
    int  getColumns() const { return mColumnsSet; }
    int  getOrientation() const { return mOrientation; }

  protected:
    void init(const AttributeSet& attrs) override;
    void onAttachedToWindow() override;
    void updatePreLayout(ConstraintLayout* container) override;

  private:
    void updateActualRowsAndColumns();
    void initVariables();
    bool generateGrid(bool isUpdate);
    void buildBoxes();
    void setBoxViewHorizontalChains();
    void setBoxViewVerticalChains();
    View* makeNewView();
    void connectView(View* view, int row, int column, int rowSpan, int columnSpan);
    bool arrangeWidgets();
    bool handleSpans(const std::vector<int>& ids, const std::vector<std::vector<int>>& spansMatrix);
    bool handleSkips(const std::vector<std::vector<int>>& skipsMatrix);
    bool invalidatePositions(int startRow, int startColumn, int rowSpan, int columnSpan);
    int  getNextPosition();
    int  getRowByIndex(int index) const;
    int  getColByIndex(int index) const;
    void clearHParams(View* view);
    void clearVParams(View* view);
    std::vector<View*> getViews();
    std::vector<float> parseWeights(int size, const std::string& str);
    // parse "index:rowxcol,..." into rows of [index, row_span, col_span]
    std::vector<std::vector<int>> parseSpans(const std::string& str);

    static constexpr int mMaxRows = 50;
    static constexpr int mMaxColumns = 50;

    std::vector<View*> mBoxViews;
    std::vector<int>   mBoxViewIds;
    ConstraintLayout*  mContainer = nullptr;

    int mRows = 0, mRowsSet = 0;
    int mColumns = 0, mColumnsSet = 0;
    std::string mStrSpans, mStrSkips, mStrRowWeights, mStrColumnWeights;
    float mHorizontalGaps = 0, mVerticalGaps = 0;
    int mOrientation = HORIZONTAL;
    int mNextAvailableIndex = 0;
    bool mValidateInputs = false;
    bool mUseRtl = false;
    bool mGridBuilt = false;  // boxes created once (buildBoxes is idempotent)
    std::vector<std::vector<bool>> mPositionMatrix;
    std::vector<int> mSpanIds;  // view ids already placed via handleSpans
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_GRID_H
