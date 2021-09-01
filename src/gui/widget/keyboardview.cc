#include <widget/keyboardview.h>
#include <cdlog.h>
#include <fstream>


namespace cdroid{

KeyboardView::KeyboardView(int w,int h):View(w,h){
    kcol=krow=0;
    setFocusable(true);
    onButton=nullptr;
    onAction=nullptr;
    mTextColor=0xFFFFFFFF;
}

void KeyboardView::setButtonListener(ButtonListener listener){
    onButton=listener; 
}

void KeyboardView::setActionListener(ActionListener listener){
    onAction=listener;
}

void KeyboardView::setKeyBgColor(UINT cl){
    //key_bg_color=cl;
}

UINT KeyboardView::getKeyBgColor(){
    return 0;//key_bg_color;
}

UINT KeyboardView::getButtons(){
    int count=0;
    for(int i=0;i<kbd->getRows();i++)
        count+=kbd->getKeyRow(i).size();
    return count;
}

int KeyboardView::getKeyButton(int px,int py){
    for(int i=0;i<kbd->getRows();i++){
        Keyboard::KeyRow& row=kbd->getKeyRow(i);
        for(int j=0;j<row.size();j++){
            Keyboard::Key&k=row[j];
            Rect r=Rect::Make(k.x,k.y,k.width,k.height);
            if(r.contains(px,py))return (i<<16)|j;
        } 
    }
    return -1;
}

void KeyboardView::setKeyboard(std::shared_ptr<Keyboard>k){
    kbd=k;
	if(kbd) kbd->resize(getWidth(),getHeight());
    invalidate(true);
}

void KeyboardView::invalidateKey(int row,int col){
    if(row<0||row>=kbd->getRows())return;
    Keyboard::KeyRow&keyrow=kbd->getKeyRow(row);
    if(col<0||col>=keyrow.size())return;
    Keyboard::Key&k=keyrow[col];
    Rect r=Rect::Make(k.x,k.y,k.width,k.height);
    LOGV("====btn[%d,%d](%d,%d,%d,%d)",row,col,r.x,r.y,r.width,r.height);
    invalidate(&r);
}

void KeyboardView::onSizeChanged(int w,int h,int ow,int oh){
    View::onSizeChanged(w,h,ow,oh);
    if(kbd)kbd->resize(w,h);
}

void KeyboardView::onDraw(Canvas&canvas){
    View::onDraw(canvas);
    canvas.set_color(getKeyBgColor());
    canvas.set_font_size(18);
	if(kbd==nullptr)
		return;
    for(int i=0; i<kbd->getRows() ;i++){
         const Keyboard::KeyRow& row=kbd->getKeyRow(i);
         LOGV("      ROW[%d]btns=%d",i,row.size());
         for(int j=0;j<row.size();j++){
             const Keyboard::Key& k=row[j];
             canvas.set_color((kcol==j&&krow==i)?0xFF444444:0xFFAAAAAA);
             Rect rect=Rect::Make(k.x,k.y,k.width,k.height);
             canvas.rectangle(rect);
             canvas.fill();
             LOGV("btn[%d,%d]=(%d,%d,%d,%d) %s  textColor=%x",i,j,k.x,k.y,k.width,k.height,k.label.c_str());
             canvas.set_color(mTextColor);
             canvas.draw_text(rect,k.label,DT_VCENTER|DT_CENTER);
         }
    }
}

void KeyboardView::ButtonClick(const Keyboard::Key&k){
    if(onButton)onButton(k); 
}

bool KeyboardView::onKeyDown(int keyCode,KeyEvent&k){
    int rows=kbd?kbd->getRows():0;
    if(rows==0)return false;
    Keyboard::KeyRow row=kbd->getKeyRow(krow);
    invalidateKey(krow,kcol);
    switch(keyCode){
    case KEY_LEFT: 
             kcol=(kcol-1+row.size())%row.size();
             break;
    case KEY_RIGHT:
             kcol=(kcol+1)%row.size();
             break;
    case KEY_UP:   
             krow=(krow-1+rows)%rows;
             row=kbd->getKeyRow(krow);
             if(row.size()<=kcol) kcol=row.size()-1;
             break;
    case KEY_DOWN: 
             krow=(krow+1)%rows;
             row=kbd->getKeyRow(krow);
             if(row.size()<=kcol) kcol=row.size()-1;      
             break;
    case KEY_ENTER:
             ButtonClick(row[kcol]);             
             break; 
    default: return false;
    }
    invalidateKey(krow,kcol);
    return true;
}

bool KeyboardView::onTouchEvent(MotionEvent&event){
    int cell;
    switch(event.getActionMasked()){
    case MotionEvent::ACTION_UP:
         cell=getKeyButton((int)event.getX(),(int)event.getY()); 
         LOGD("touch(%.f,%.f)action=%d from %d,%d-->%d,%d",event.getX(),event.getY(),event.getActionMasked(),krow,kcol,cell>>16,cell&0xFFFF);
         if(cell==-1)return false;
         invalidateKey(krow,kcol);
         krow=cell>>16;
         kcol=cell&0xFFFF;
         ButtonClick(kbd->getKeyRow(krow).at(kcol));
         invalidateKey(krow,kcol);
         break;
    case MotionEvent::ACTION_DOWN:break;
    }
    return false;  
}

}//namespace
