#pragma once
#include <animation/layoutanimationcontroller.h>

namespace cdroid{

class GridLayoutAnimationController:public LayoutAnimationController{
public:
    /* Animates the children starting from the left of the grid to the right.*/
    static constexpr int DIRECTION_LEFT_TO_RIGHT = 0x0;
    /* Animates the children starting from the right of the grid to the left.*/
    static constexpr int DIRECTION_RIGHT_TO_LEFT = 0x1;
    /* Animates the children starting from the top of the grid to the bottom.*/
    static constexpr int DIRECTION_TOP_TO_BOTTOM = 0x0;
    /* Animates the children starting from the bottom of the grid to the top.*/
    static constexpr int DIRECTION_BOTTOM_TO_TOP = 0x2;
    /* Bitmask used to retrieve the horizontal component of the direction.*/
    static constexpr int DIRECTION_HORIZONTAL_MASK = 0x1;
    /* Bitmask used to retrieve the vertical component of the direction.*/
    static constexpr int DIRECTION_VERTICAL_MASK   = 0x2;
    /* Rows and columns are animated at the same time.*/
    static constexpr int PRIORITY_NONE   = 0;
    /* Columns are animated first.*/
    static constexpr int PRIORITY_COLUMN = 1;
    /* Rows are animated first.*/
    static constexpr int PRIORITY_ROW    = 2;
    
    class AnimationParameters:public LayoutAnimationController::AnimationParameters {
    public:
        /**The view group's column to which the view belongs.*/
        int column;

        /* The view group's row to which the view belongs.*/
        int row;

        /**The number of columns in the view's enclosing grid layout.*/
        int columnsCount;

        /* The number of rows in the view's enclosing grid layout. */
        int rowsCount;
    };
private:
    float mColumnDelay;
    float mRowDelay;
    int mDirection;
    int mDirectionPriority;
    int getTransformedColumnIndex(const AnimationParameters* params);
    int getTransformedRowIndex(const AnimationParameters* params);
protected:
    long getDelayForView(View* view)override;
public:
    GridLayoutAnimationController(Context* context,const AttributeSet& attrs);
    GridLayoutAnimationController(Animation* animation);
    GridLayoutAnimationController(Animation* animation, float columnDelay, float rowDelay);
    float getColumnDelay();
    void setColumnDelay(float columnDelay);
    float getRowDelay();
    void setRowDelay(float rowDelay);
    int getDirection();
    void setDirection(int direction);
    int getDirectionPriority();
    void setDirectionPriority(int directionPriority);
    bool willOverlap()override;
};
}//endof namespace
