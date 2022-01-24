#ifndef __ALERT_DIALOG_H__
#define __ALERT_DIALOG_H__
#include <app/dialoginterface.h>
#include <widget/button.h>
#include <widget/listview.h>

namespace cdroid{
	
class AlertDialog :public DialogInterface{
public:
    static constexpr int LAYOUT_HINT_NONE = 0;
    static constexpr int LAYOUT_HINT_SIDE = 1;
protected:
    class AlertController* mAlert;
public:
    AlertDialog(Context*ctx,const std::string&resid);
    Button*getButton(int whichButton);
    ListView* getListView();
    void setTitle(const std::string& title);
    void setMessage(const std::string& message);
    void setButton(int whichButton,const std::string&text, OnClickListener listener);
};

}//namespace 
#endif 
