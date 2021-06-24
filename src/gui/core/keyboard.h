#ifndef __SOFT_KEYBOARD_H__
#define __SOFT_KEYBOARD_H__
#include <windows.h>
#include <istream>

namespace cdroid{


class Keyboard{
public:
   enum{
       KEYCODE_SHIFT    = -1,
       KEYCODE_ALT      = -2,
       KEYCODE_CANCEL   = -3,
       KEYCODE_DONE     = -4,
       KEYCODE_DELETE   = -5,
       KEYCODE_BACKSPACE= -6,
       KEYCODE_MODE_CHANGE=-7/*used change number/letter keyboardlayout*/
   };
   class Key{
   public:
       wchar_t codes[4];
       BYTE isModifier;/*backspace,delete...*/
       BYTE isSticky;/*shift: two state key*/
       BYTE on;/*for sticky keys*/
       BYTE pressed;
       int x;
       int y;
       int width;
       int height;
       int gap;/*The horizontal gap before this key*/
       std::string label;
       std::string text;
       std::string icon;
       std::string action;
       Key();
   };
   typedef std::vector<Key>KeyRow;
private:
    int rowGap;
    int keyGap;
    int keyWidth;
    int keyHeight;
    int totalWidth;
    int totalHeight;
    float marginTB;
    float marginLR;
protected:
    std::vector<KeyRow>keyRows;
    int keyboardWidth;
    int keyboardHeight;
    Key*getKeyByCode(int code);
public:
    Keyboard(int w,int h);
    int loadLayout(std::istream&in);
    static std::shared_ptr<Keyboard>loadFrom(const std::string&resname,int w=1280,int h=360);
    void resize(int w,int h);
    void setMargin(float tb,float lr);
    int getRows()const;
    KeyRow&getKeyRow(int row);
    void setShifted(int code,bool state);
    bool getShifted(int code)const;
};

}//namespace
#endif
