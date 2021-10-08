#ifndef __SOFT_KEYBOARD_H__
#define __SOFT_KEYBOARD_H__
#include <windows.h>
#include <istream>

namespace cdroid{


class Keyboard{
public:
    static constexpr int EDGE_LEFT  = 0x01;
    static constexpr int EDGE_RIGHT = 0x02;
    static constexpr int EDGE_TOP   = 0x04;
    static constexpr int EDGE_BOTTOM= 0x08;
    static constexpr int KEYCODE_SHIFT    = -1;
    static constexpr int KEYCODE_ALT      = -2;
    static constexpr int KEYCODE_CANCEL   = -3;
    static constexpr int KEYCODE_DONE     = -4;
    static constexpr int KEYCODE_DELETE   = -5;
    static constexpr int KEYCODE_BACKSPACE= -6;
    static constexpr int KEYCODE_MODE_CHANGE=-7;/*used change number/letter keyboardlayout*/
    class Key{
    private:
        void*parent;
    public:
        std::vector<int> codes;
        bool modifier;/*backspace,delete...*/
        bool sticky;/*shift: two state key*/
        bool on;/*for sticky keys*/
        bool pressed;
        bool repeatable; /** Whether this key repeats itself when held down */
        int x;
        int y;
        int width;
        int height;
        int gap;/*The horizontal gap before this key*/
        int edgeFlags;
        std::string label;
        std::string text;
        Drawable* icon;
        Drawable* iconPreview;
        std::string action;
        std::string popupResId;
        Key(void*parent=nullptr);
        Key(void*parent,int x,int y,Context*ctx,const AttributeSet&attrs);
        void onPressed();
        void onReleased(bool inside);
        int parseCSV(const std::string& value,std::vector<int>& codes);
        bool isInside(int x, int y);
        int squaredDistanceFrom(int x, int y);
        std::vector<int>getCurrentDrawableState()const;
    };
    class Row{
    public:
       Keyboard*parent;
       int defaultWidth;
       /** Default height of a key in this row. */
       int defaultHeight;
       /** Default horizontal gap between keys in this row. */
       int defaultHorizontalGap;
       /** Vertical gap following this row. */
       int verticalGap;
       int rowEdgeFlags;
       int mode;
       std::vector<Key*>mKeys;
    public:
       Row(Keyboard*parent,Context*ctx,const AttributeSet&attrs);
    };
private:
    static constexpr int GRID_WIDTH = 10;
    static constexpr int GRID_HEIGHT = 5;
    static constexpr int GRID_SIZE = GRID_WIDTH * GRID_HEIGHT;
    std::string mLabel;
    int  mDefaultHorizontalGap;
    int  mDefaultWidth;
    int  mDefaultHeight;
    int  mDefaultVerticalGap;
    bool mShifted;
    int  rowGap;
    int  keyGap;
    int  mKeyWidth;
    int  mKeyHeight;
    int  mTotalWidth;
    int  mTotalHeight;
    int  mDisplayWidth;
    int  mDisplayHeight;
    int  mKeyboardMode;
    int mCellWidth;
    int mCellHeight;
    int mProximityThreshold;
    std::vector<std::vector<int>>mGridNeighbors;
    std::vector<Key*> mKeys;
    std::vector<Key*> mModifierKeys;
    std::vector<Key*> mShiftKeys;
    std::vector<int > mShiftKeyIndices;
    std::vector<Row*> rows;
    friend Row;
    friend Key;
    void computeNearestNeighbors();
protected:
    int  keyboardWidth;
    int  keyboardHeight;
    Key* getKeyByCode(int code);
public:
    Keyboard(Context* context,const std::string&resid,int w,int h);
    Keyboard(Context* context,const std::string& xmlLayoutResId, int modeId);
    ~Keyboard();
    void loadKeyboard(Context*,const std::string&);
    void resize(int w,int h);
    std::vector<Key*>& getKeys();
    std::vector<Key*>& getModifierKeys();
    int  getHorizontalGap()const;
    void setHorizontalGap(int gap);
    int  getVerticalGap()const;
    void setVerticalGap(int gap);
    int  getKeyHeight()const;
    void setKeyHeight(int height);
    int  getKeyWidth()const;
    void setKeyWidth(int width);
    int  getHeight()const;
    int  getMinWidth()const;
    bool setShifted(bool shiftState);
    bool isShifted()const;
    std::vector<int>& getShiftKeyIndices();
    int  getShiftKeyIndex()const;

    void setMargin(float tb,float lr);
    int  getRows()const;
    void setShifted(int code,bool state);
    bool getShifted(int code)const;
    std::vector<int> getNearestKeys(int x, int y);
};

}//namespace
#endif
