/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.helper.widget.Grid.
 */
#include <widgetEx/constraintlayout/helpers/grid.h>

#include <algorithm>
#include <cmath>
#include <sstream>

#include <widgetEx/constraintlayout/constraintlayout.h>

DECLARE_WIDGET(Grid)

namespace cdroid {

Grid::Grid(Context* ctx, const AttributeSet& attrs)
    : ConstraintHelper(ctx, attrs) {
}

Grid::Grid(int width, int height)
    : ConstraintHelper(width, height) {
}

using LP = ConstraintLayout::LayoutParams;
static LP* gparams(View* v) { return dynamic_cast<LP*>(v->getLayoutParams()); }

void Grid::init(const AttributeSet& attrs) {
    ConstraintHelper::init(attrs);
    mUseViewMeasure = true;
    mRowsSet = attrs.getInt("grid_rows", 0);
    mColumnsSet = attrs.getInt("grid_columns", 0);
    mStrSpans = attrs.getString("grid_spans", "");
    mStrSkips = attrs.getString("grid_skips", "");
    mStrRowWeights = attrs.getString("grid_rowWeights", "");
    mStrColumnWeights = attrs.getString("grid_columnWeights", "");
    mOrientation = attrs.getInt("grid_orientation",
            std::unordered_map<std::string,int>{{"horizontal", (int) HORIZONTAL}, {"vertical", (int) VERTICAL}},
            HORIZONTAL);
    mHorizontalGaps = attrs.getDimension("grid_horizontalGaps", 0);
    mVerticalGaps = attrs.getDimension("grid_verticalGaps", 0);
    mValidateInputs = attrs.getBoolean("grid_validateInputs", false);
    mUseRtl = attrs.getBoolean("grid_useRtl", false);
    updateActualRowsAndColumns();
    initVariables();
}

void Grid::updateActualRowsAndColumns() {
    int count = (int) mIds.size();
    if (mRowsSet == 0 || mColumnsSet == 0) {
        if (mColumnsSet > 0) {
            mColumns = mColumnsSet;
            mRows = (count + mColumns - 1) / mColumnsSet; // round up
        } else if (mRowsSet > 0) {
            mRows = mRowsSet;
            mColumns = (count + mRowsSet - 1) / mRowsSet; // round up
        } else { // as close to square as possible favoring more rows
            mRows = (int) (1.5 + std::sqrt((double) count));
            if (mRows < 1) mRows = 1;
            mColumns = (count + mRows - 1) / mRows;
        }
    } else {
        mRows = mRowsSet;
        mColumns = mColumnsSet;
    }
}

void Grid::initVariables() {
    mPositionMatrix.assign(mRows, std::vector<bool>(mColumns, true));
}

void Grid::onAttachedToWindow() {
    ConstraintHelper::onAttachedToWindow();
    mContainer = dynamic_cast<ConstraintLayout*>(getParent());
    if (mContainer != nullptr && !mGridBuilt) {
        generateGrid(false);
        mGridBuilt = true;
    }
}

void Grid::updatePreLayout(ConstraintLayout* container) {
    mContainer = container;
    if (mContainer != nullptr && !mGridBuilt) {
        generateGrid(false);
        mGridBuilt = true;
    }
}

bool Grid::generateGrid(bool isUpdate) {
    if (mContainer == nullptr || mRows < 1 || mColumns < 1) {
        return false;
    }
    if (isUpdate) {
        for (auto& row : mPositionMatrix) std::fill(row.begin(), row.end(), true);
        mSpanIds.clear();
    }
    mNextAvailableIndex = 0;
    bool isSuccess = true;
    buildBoxes();
    if (!mStrSkips.empty()) {
        auto skips = parseSpans(mStrSkips);
        isSuccess &= handleSkips(skips);
    }
    if (!mStrSpans.empty()) {
        auto spans = parseSpans(mStrSpans);
        isSuccess &= handleSpans(mIds, spans);
    }
    isSuccess &= arrangeWidgets();
    return isSuccess || !mValidateInputs;
}

void Grid::buildBoxes() {
    int boxCount = std::max(mRows, mColumns);
    if (mBoxViews.empty()) {
        mBoxViews.resize(boxCount);
        for (int i = 0; i < boxCount; i++) mBoxViews[i] = makeNewView();
    } else if ((int) mBoxViews.size() != boxCount) {
        std::vector<View*> temp(boxCount);
        for (int i = 0; i < boxCount; i++) {
            temp[i] = (i < (int) mBoxViews.size()) ? mBoxViews[i] : makeNewView();
        }
        for (int j = boxCount; j < (int) mBoxViews.size(); j++) {
            mContainer->removeView(mBoxViews[j]);
        }
        mBoxViews = temp;
    }
    mBoxViewIds.resize(boxCount);
    for (int i = 0; i < boxCount; i++) mBoxViewIds[i] = mBoxViews[i]->getId();
    setBoxViewVerticalChains();
    setBoxViewHorizontalChains();
}

View* Grid::makeNewView() {
    View* v = new View(0, 0);
    v->setId(View::generateViewId());
    v->setVisibility(View::INVISIBLE);
    auto* p = new ConstraintLayout::LayoutParams(0, 0);
    mContainer->addView(v, p);
    return v;
}

void Grid::clearHParams(View* view) {
    auto* p = gparams(view);
    if (p == nullptr) return;
    p->horizontalWeight = ConstraintWidget::UNKNOWN;
    p->leftToRight = LP::UNSET;
    p->leftToLeft = LP::UNSET;
    p->rightToLeft = LP::UNSET;
    p->rightToRight = LP::UNSET;
    p->leftMargin = 0;
}

void Grid::clearVParams(View* view) {
    auto* p = gparams(view);
    if (p == nullptr) return;
    p->verticalWeight = ConstraintWidget::UNKNOWN;
    p->topToBottom = LP::UNSET;
    p->topToTop = LP::UNSET;
    p->bottomToTop = LP::UNSET;
    p->bottomToBottom = LP::UNSET;
    p->topMargin = 0;
}

void Grid::setBoxViewHorizontalChains() {
    int gridId = getId();
    int maxVal = std::max(mRows, mColumns);
    std::vector<float> columnWeights = parseWeights(mColumns, mStrColumnWeights);
    if (mColumns == 1) {
        clearHParams(mBoxViews[0]);
        auto* p = gparams(mBoxViews[0]);
        p->leftToLeft = gridId;
        p->rightToRight = gridId;
        return;
    }
    for (int i = 0; i < mColumns; i++) {
        clearHParams(mBoxViews[i]);
        auto* p = gparams(mBoxViews[i]);
        // AndroidX leaves unweighted 0dp boxes to the solver's default equal split; CDROID's chain
        // solver sizes match_constraint elements by weight, so default each box to weight 1.
        // Unweighted 0dp boxes default to weight 1 in the chain solver (Chain.cc: currentWeight<0→1),
        // so only forward explicit per-column weights (faithful to AndroidX, which sets none here).
        if (!columnWeights.empty()) p->horizontalWeight = columnWeights[i];
        if (i > 0) p->leftToRight = mBoxViewIds[i - 1]; else p->leftToLeft = gridId;
        if (i < mColumns - 1) p->rightToLeft = mBoxViewIds[i + 1]; else p->rightToRight = gridId;
        if (i > 0) p->leftMargin = (int) mHorizontalGaps;
    }
    for (int i = mColumns; i < maxVal; i++) {
        clearHParams(mBoxViews[i]);
        auto* p = gparams(mBoxViews[i]);
        p->leftToLeft = gridId;
        p->rightToRight = gridId;
    }
}

void Grid::setBoxViewVerticalChains() {
    int gridId = getId();
    int maxVal = std::max(mRows, mColumns);
    std::vector<float> rowWeights = parseWeights(mRows, mStrRowWeights);
    if (mRows == 1) {
        clearVParams(mBoxViews[0]);
        auto* p = gparams(mBoxViews[0]);
        p->topToTop = gridId;
        p->bottomToBottom = gridId;
        return;
    }
    for (int i = 0; i < mRows; i++) {
        clearVParams(mBoxViews[i]);
        auto* p = gparams(mBoxViews[i]);
        if (!rowWeights.empty()) p->verticalWeight = rowWeights[i];
        if (i > 0) p->topToBottom = mBoxViewIds[i - 1]; else p->topToTop = gridId;
        if (i < mRows - 1) p->bottomToTop = mBoxViewIds[i + 1]; else p->bottomToBottom = gridId;
        if (i > 0) p->topMargin = (int) mVerticalGaps;
    }
    for (int i = mRows; i < maxVal; i++) {
        clearVParams(mBoxViews[i]);
        auto* p = gparams(mBoxViews[i]);
        p->topToTop = gridId;
        p->bottomToBottom = gridId;
    }
}

void Grid::connectView(View* view, int row, int column, int rowSpan, int columnSpan) {
    auto* p = gparams(view);
    if (p == nullptr) return;
    p->leftToLeft = mBoxViewIds[column];
    p->topToTop = mBoxViewIds[row];
    p->rightToRight = mBoxViewIds[column + columnSpan - 1];
    p->bottomToBottom = mBoxViewIds[row + rowSpan - 1];
}

std::vector<View*> Grid::getViews() {
    std::vector<View*> views;
    if (mContainer == nullptr) return views;
    for (int id : mIds) views.push_back(mContainer->findViewById(id));
    return views;
}

bool Grid::arrangeWidgets() {
    std::vector<View*> views = getViews();
    for (size_t i = 0; i < mIds.size(); i++) {
        if (std::find(mSpanIds.begin(), mSpanIds.end(), mIds[i]) != mSpanIds.end()) continue;
        int position = getNextPosition();
        if (position == -1) return false;
        if (i >= views.size() || views[i] == nullptr) continue;
        connectView(views[i], getRowByIndex(position), getColByIndex(position), 1, 1);
    }
    return true;
}

int Grid::getRowByIndex(int index) const {
    return (mOrientation == VERTICAL) ? (index % mRows) : (index / mColumns);
}

int Grid::getColByIndex(int index) const {
    return (mOrientation == VERTICAL) ? (index / mRows) : (index % mColumns);
}

int Grid::getNextPosition() {
    int position = 0;
    bool positionFound = false;
    while (!positionFound) {
        if (mNextAvailableIndex >= mRows * mColumns) return -1;
        position = mNextAvailableIndex;
        int row = getRowByIndex(mNextAvailableIndex);
        int col = getColByIndex(mNextAvailableIndex);
        if (mPositionMatrix[row][col]) {
            mPositionMatrix[row][col] = false;
            positionFound = true;
        }
        mNextAvailableIndex++;
    }
    return position;
}

bool Grid::invalidatePositions(int startRow, int startColumn, int rowSpan, int columnSpan) {
    for (int i = startRow; i < startRow + rowSpan; i++) {
        for (int j = startColumn; j < startColumn + columnSpan; j++) {
            if (i >= (int) mPositionMatrix.size() || j >= (int) mPositionMatrix[0].size()
                    || !mPositionMatrix[i][j]) {
                return false;
            }
            mPositionMatrix[i][j] = false;
        }
    }
    return true;
}

bool Grid::handleSkips(const std::vector<std::vector<int>>& skipsMatrix) {
    for (const auto& s : skipsMatrix) {
        if (!invalidatePositions(getRowByIndex(s[0]), getColByIndex(s[0]), s[1], s[2])) return false;
    }
    return true;
}

bool Grid::handleSpans(const std::vector<int>& ids, const std::vector<std::vector<int>>& spansMatrix) {
    std::vector<View*> views = getViews();
    for (size_t i = 0; i < spansMatrix.size(); i++) {
        int row = getRowByIndex(spansMatrix[i][0]);
        int col = getColByIndex(spansMatrix[i][0]);
        if (!invalidatePositions(row, col, spansMatrix[i][1], spansMatrix[i][2])) return false;
        if (i < views.size() && views[i] != nullptr) {
            connectView(views[i], row, col, spansMatrix[i][1], spansMatrix[i][2]);
        }
        if (i < ids.size()) mSpanIds.push_back(ids[i]);
    }
    return true;
}

std::vector<float> Grid::parseWeights(int size, const std::string& str) {
    std::vector<float> arr;
    if (str.empty()) return arr;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ',')) arr.push_back((float) std::atof(token.c_str()));
    if ((int) arr.size() != size) arr.clear(); // mismatch → treat as unspecified
    return arr;
}

std::vector<std::vector<int>> Grid::parseSpans(const std::string& str) {
    std::vector<std::vector<int>> matrix;
    std::stringstream ss(str);
    std::string span;
    while (std::getline(ss, span, ',')) {
        // format: index:rowSpanxcolSpan
        size_t colon = span.find(':');
        if (colon == std::string::npos) continue;
        int index = std::atoi(span.substr(0, colon).c_str());
        std::string rest = span.substr(colon + 1);
        size_t x = rest.find('x');
        int rowSpan = std::atoi(rest.substr(0, x).c_str());
        int colSpan = (x == std::string::npos) ? 1 : std::atoi(rest.substr(x + 1).c_str());
        matrix.push_back({index, rowSpan, colSpan});
    }
    return matrix;
}

// --- programmatic setters ---

void Grid::setRows(int rows) {
    if (rows > mMaxRows || mRowsSet == rows) return;
    mRowsSet = rows;
    updateActualRowsAndColumns();
    initVariables();
    mGridBuilt = false; // force rebuild on next layout pass
}

void Grid::setColumns(int columns) {
    if (columns > mMaxColumns || mColumnsSet == columns) return;
    mColumnsSet = columns;
    updateActualRowsAndColumns();
    initVariables();
    mGridBuilt = false;
}

void Grid::setOrientation(int orientation) {
    if (orientation != HORIZONTAL && orientation != VERTICAL) return;
    if (mOrientation == orientation) return;
    mOrientation = orientation;
    if (mGridBuilt) generateGrid(true);
}

void Grid::setSpans(const std::string& spans) { if (mStrSpans != spans) { mStrSpans = spans; if (mGridBuilt) generateGrid(true); } }
void Grid::setSkips(const std::string& skips) { if (mStrSkips != skips) { mStrSkips = skips; if (mGridBuilt) generateGrid(true); } }
void Grid::setRowWeights(const std::string& w) { if (mStrRowWeights != w) { mStrRowWeights = w; if (mGridBuilt) generateGrid(true); } }
void Grid::setColumnWeights(const std::string& w) { if (mStrColumnWeights != w) { mStrColumnWeights = w; if (mGridBuilt) generateGrid(true); } }
void Grid::setHorizontalGaps(float gaps) { if (gaps >= 0 && mHorizontalGaps != gaps) { mHorizontalGaps = gaps; if (mGridBuilt) generateGrid(true); } }
void Grid::setVerticalGaps(float gaps) { if (gaps >= 0 && mVerticalGaps != gaps) { mVerticalGaps = gaps; if (mGridBuilt) generateGrid(true); } }

} // namespace cdroid
