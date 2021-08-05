#ifndef __KEYBOARD_VIEW_H__
#define __KEYBOARD_VIEW_H__

#include <widget/view.h>
#include <widget/edittext.h>
#include <vector>
#include <core/keyboard.h>

namespace cdroid{

class KeyboardView:public View{
DECLARE_UIEVENT(void,ButtonListener,const Keyboard::Key&k);
DECLARE_UIEVENT(void,ActionListener,UINT action);
private:
   INT kcol,krow;
   std::shared_ptr<Keyboard> kbd;
   ButtonListener onButton;
   ActionListener onAction;
   int mTextColor;
   void ButtonClick(const Keyboard::Key&k);
protected:
   void invalidateKey(int row,int col);
   int getKeyButton(int px,int py);
public:
   KeyboardView(int w,int h);
   void setKeyBgColor(UINT cl);
   UINT getKeyBgColor();
   void onSizeChanged(int w,int h,int ow ,int oh)override;
   UINT getButtons();
   Keyboard&getKeyboard(){return *kbd;}
   void setKeyboard(std::shared_ptr<Keyboard>k);
   virtual void setButtonListener(ButtonListener listener);
   virtual void setActionListener(ActionListener listener);
   virtual void onDraw(Canvas&canvas)override;
   virtual bool onKeyDown(int keyCode,KeyEvent&k)override;
   virtual bool onTouchEvent(MotionEvent&event)override;
};
}//namespace
#endif
